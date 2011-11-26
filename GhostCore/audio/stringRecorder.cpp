/*
 * Ghost Play - Autonomous Violin-Player Imitation Device
 * 
 * Copyright (C) 2009-2012  Masato Fujino <fujino@fairydevices.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stringRecorder.hpp"
#include "../error.hpp"
#include "audioBuffer.hpp"
#include "audioAnalyzer.hpp"

//サンプリングレート
const int StringRecorder::samplerate = 44100;

//FFTウィンドウ幅(byte) 2の累乗
const int StringRecorder::windowSize = 1024;

//基本解析長（ウィンドウサイズの定数倍）；このフレームの平均値を後段処理に用いる
const int StringRecorder::analyzeFrameLength = StringRecorder::windowSize;

/*!
 * \brief
 * ストリングレコーダー．弦の振動を記録・解析するためのクラス．
 * 
 * \remarks
 * 時間対バイト数メモ
 *	samplesize	samplerate	bps	    byte per sec	kbyte per sec  
 *	2[byte]		44100[Hz]	705600	88200	        88.2		    
 *【メモ】
 * 1024 を秒で:
 * 1024/88200  = 0.0116秒（100分の1秒ちょっと）
 * 
 * \see
 * audioBuffer.hpp|audioAnalyzer.hpp
 */
StringRecorder::StringRecorder()
{
	audio = 0;
	buffer = 0;
	tdp = 0;
	fdp = 0;
	a_length = 0; //StartRecord()内で初期化
	flagRecording = false;
	initialized = false;
}

StringRecorder::~StringRecorder()
{
	delete audio;
	delete buffer;
	audio = 0;
	buffer = 0;
}

void StringRecorder::StartRecord()
{
	//PCM 44.1k/16bit
	QAudioFormat format;
	format.setCodec("audio/pcm");
	format.setSampleRate(samplerate);
	format.setSampleSize(16);
	format.setChannels(1);
	format.setSampleType(QAudioFormat::SignedInt);
	format.setByteOrder(QAudioFormat::LittleEndian);

	std::cout << "[ STRING RECORDER ] フォーマット設定" << std::endl;
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	if(!info.isFormatSupported(format)){
		Error::Critical(0, QObject::tr("カレントのオーディオデバイスが存在しないか，必要な録音フォーマットに対応していません．"));
	}

	//オーディオバッファの確保
	buffer = new AudioBuffer(format,this);
	//録音準備
	audio = new QAudioInput(format, this);
	//オーディオバッファの開始
	buffer->start();
	//録音開始
	audio->start(buffer);
	//自分が保持する解析結果保存用リングバッファの確保
	//もともとの録音リングバッファをanalyzeFrameLength長毎に分断して処理するので
	a_length = buffer->bufsize() / analyzeFrameLength + 1; // 解析結果保存用リングバッファの長さ

	//// 解析結果保存用リングバッファ確保

	//時間領域
	std::cout << "[ STRING RECORDER ] 解析結果保存用リングバッファ確保" << std::endl;
	std::cout << "[ STRING RECORDER ] 時間方向: " << a_length*sizeof(TimeDomainProfile)/1000000. + a_length*analyzeFrameLength*sizeof(double)/1000000. << "MB（バッファ長: "<< a_length << "） 初期化開始..." << std::endl;
	tdp = new TimeDomainProfile*[a_length];
	for(int i=0;i<a_length;i++){
		tdp[i] = new TimeDomainProfile(analyzeFrameLength);
	}

	//周波数領域
	std::cout << "[ STRING RECORDER ] 周波数方向: " << a_length*sizeof(FrequencyDomainProfile)/1000000. + a_length*windowSize*sizeof(double)*2/1000000.<< "MB（バッファ長:" << a_length << "） 初期化開始..."<< std::endl;
	fdp = new FrequencyDomainProfile*[a_length];
	for(int i=0;i<a_length;i++){
		fdp[i] = new FrequencyDomainProfile(windowSize);
	}

	std::cout << "[ STRING RECORDER ] 初期化終了" << std::endl;
	//分析スレッド開始
	start();	
}

void StringRecorder::EndRecord()
{
	flagRecording = false;
}


/*!
 * \brief
 * 解析スレッドの実装（外部変数の取り扱いに注意）
 * 
 * 録音デバイスのリングバッファから，設定された固定長の基本解析長ごとに解析を行い続ける．
 * 
 */
void StringRecorder::run()
{
	std::cout << "[ STRING RECORDER ] 解析スレッド開始 Thread ID: " << QThread::currentThreadId() << std::endl;
	//初期化完了
	initialized = true;
	flagRecording = true;
	//前回ポーリング時のフロント値（これは本来は前回ポーリング時のフロント位置だが，前回解析完了位置としても用いる）
	unsigned long prev_front = 0;
	//新規解析用一時バッファ（必ず基本分析長になる）
	double* analyze = (double*)calloc(analyzeFrameLength,sizeof(double));
	//リングバッファサイズ
	unsigned int buflen = buffer->bufsize();
	//解析結果保存用リングバッファフロント位置を初期化
	a_front = 0;
	while(flagRecording)
	{
		//リングバッファを再構成して取得
		double *buf = buffer->buffer();
		//現在のリングバッファ内フロント位置の取得
		unsigned int front  = buffer->front();
		//録音開始からの通算位置
		unsigned long long offset = buffer->offset();
		//同ミリ秒換算
		double timeoffset = offset*(1000./buffer->format().sampleRate());
		//新規解析用バッファのインデックス
		int index = 0;

		//フロント位置が変わっていなければ適当に待つ
		if(front == prev_front){
			msleep(100);
			continue;
		}
		//リングバッファから今回新たに取得された区間を analyze 配列に挿入する
		if(prev_front < front){
			//リングバッファ内で単調増加の場合
			for(int i = prev_front;i<=front;i++){
				if(index == analyzeFrameLength){
					prev_front = i; // 解析フロント位置を記録
					break;
				}
				analyze[index] = buf[i];
				index++;
			}
		}else{
			//ゼロをまたいだ場合
			for(int i = prev_front;i<buflen;i++){
				if(index == analyzeFrameLength){
					prev_front = i; // 解析フロント位置を記録
					break;
				}
				analyze[index] = buf[i];
				index++;
			}
			if(index >= analyzeFrameLength){
				//上でブレークしていたらこれ以上見ない
			}else{
				for(int i= 0;i<front;i++){
					if(index == analyzeFrameLength){
						prev_front = i; // 解析フロント位置を記録
						break;
					}
					analyze[index] = buf[i];
					index++;
				}
			}
		}

		//録音リングバッファから新たに取得された区間が解析バッファの最低サイズ以下の場合，適当に待つ
		if(index < analyzeFrameLength){
			msleep(100);
			continue;
		}

		//ここで analyze ベクタのサイズは常に analyzeFrameLength に揃っていることが保証される
		if(index != analyzeFrameLength){
			std::cerr << "ASSERT: index=" << index << ", analyzeFrameLength:" << analyzeFrameLength << std::endl;
			Q_ASSERT(index != analyzeFrameLength);
		}

		//// 解析実行

		//この解析バッファ全体の平均・最大値等
		TimeDomain(analyze, tdp[a_front]);
		tdp[a_front]->timeoffset = timeoffset;

		//この解析バッファ全体のFZERO等
		FrequencyDomain(analyze, fdp[a_front]);
		fdp[a_front]->timeoffset = timeoffset;

		a_front++;
		if(a_front >= a_length){			
			a_front = 0; // リングバッファ
		}
	}

	//解析バッファ不要
	free(analyze);
}


/*!
 * \brief
 * 時間方向の解析処理の実装．
 * 
 * \param v
 * 解析対象範囲のサンプル，値は[0,1]に正規化しておく必要がある．
 *
 * \param c
 * 書き換えられるTimeDomainProfileへのポインタ（初期化されている必要がある）
 * 
 * \remarks
 * 時間領域では今は単純なことしかしていないため，ベタ書きしているが，処理が増えそうなら，audioAnalyzer.hpp に移管するべき． 
 *
 */
void StringRecorder::TimeDomain(double* v, TimeDomainProfile* c)
{
	double sum = 0;
	double m_max = 0;
	double p = 0;
	for(int i=0;i<analyzeFrameLength;i++){
		//最大振幅
		if(m_max < qAbs(v[i])){
			m_max = qAbs(v[i]);
		}
		//振幅合計
		//パワー合計
		p += v[i]*v[i];
		sum += qAbs(v[i]);
		//データ
		c->subwindow[i] = v[i];
	}

	//最大振幅（最大振幅を保持する）
	c->maxAmplitude = m_max;
	//平均振幅
	c->averageAmplitude = sum/(double)analyzeFrameLength;
	//最大パワー
	c->maxPower = m_max*m_max;
	//平均パワー
	c->averagePower = p/(double)analyzeFrameLength;
	//初期化のみ（呼び出し元で値を代入します）
	c->timeoffset= 0;
}


/*!
 * \brief
 * 周波数方向の解析処理の実装（１つの解析について）
 * 
 * \param v
 * 解析対象範囲のサンプル，値は[0,1]に正規化しておく必要がある．
 * 
 * \returns
 * 解析結果の時間方向プロファイル構造体（１つの解析を返す）
 * 
 * \see
 * audioAnalyzer.hpp
 *
 */
void StringRecorder::FrequencyDomain(double *v, FrequencyDomainProfile *c)
{
	//ケプストラム解析
	QVector<double> cps  = AudioAnalyzer::CPS(v, analyzeFrameLength, windowSize);
	QVector<double> alog = AudioAnalyzer::ALOG(v, analyzeFrameLength, windowSize);
	for(int i=0;i<windowSize;i++){
		c->cps[i] = cps[i];
		c->dft[i] = alog[i];
	}
	//F0
	c->F0 = AudioAnalyzer::F0(cps, samplerate, windowSize);
	//初期化（呼び出し元で値を代入します）
	c->timeoffset = 0;
}

/*!
 * \brief
 * 時間領域の解析結果を任意の解析長で再解析して返す（ユーティリティ関数）
 * 
 * \param analyzeLength
 * 再解析長（ただし基本解析長での解析結果を用いるので再解析は高速に実行できる）
 * 基本解析長以上，かつ，基本解析長の整数倍の値を指定しなければならない．0を指定すると，基本解析長の結果をそのまま返す．
 * 
 * \param offset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 * 
 * \returns
 * 再解析結果
 * 
 */
QVector<TimeDomainProfile*> StringRecorder::TimeDomainAnalyze(int analyzeLength, double timeoffset) const
{
	int index = 0;
	if(a_front == 0){
		index = 0;
	}else{
		index = a_front-1;
	}
	double endoffset = tdp[index]->timeoffset;
	return TimeDomainAnalyze(analyzeLength, timeoffset, endoffset);
}

/*!
 * \brief
 * 時間領域の解析結果を任意の解析長で再解析して返す（本体実装）
 * 
 * \param analyzeLength
 * 再解析長（ただし基本解析長での解析結果を用いるので再解析は高速に実行できる）
 * 基本解析長以上，かつ，基本解析長の整数倍の値を指定しなければならない．0を指定すると，基本解析長の結果をそのまま返す．
 * 
 * \param startoffset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 *
 * \param endoffset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 *
 * \returns
 * 再解析結果
 * 
 */
QVector<TimeDomainProfile*> StringRecorder::TimeDomainAnalyze(int analyzeLength, double timeoffset, double endtimeoffset) const
{

	//不正な値をチェック
	if(analyzeLength != 0 && analyzeLength < analyzeFrameLength){
		//現在の仕様．短くすることも本来は出来るので，実装しても良い．
		std::cerr << "[ STRING RECORDER ] TimeDomainAnalyze() 第一引数で指定された再解析長が基本解析長を下回っています（基本解析長:" << analyzeFrameLength << " > 指定長:" << analyzeLength << "）"<< std::endl;		
		Q_ASSERT(false);
	}
	if(analyzeLength != 0 && analyzeLength % analyzeFrameLength != 0){
		std::cerr << "[ STRING RECORDER ] TimeDomainAnalyze() 第一引数で指定された再解析行が基本解析長の整数倍になっていないため結果に欠落が生じます．" << std::endl;
	}
	//戻り値
	QVector<TimeDomainProfile*> result_tdp;

	//初期化されていない場合
	if(!initialized){
		return result_tdp;//通し
	}

	//一時変数
	QVector<TimeDomainProfile*> temp; 

	//解析結果保存用リングバッファの中をシーケンシャルスキャン
	//古い方の先端が a_front である
	for(int i=a_front;i<a_length;i++){
		if(timeoffset <= tdp[i]->timeoffset && tdp[i]->timeoffset <= endtimeoffset){
			temp.append(tdp[i]);
		}
	}

	//ゼロクロスして新しい方へ
	for(int i=0;i<a_front;i++){
		if(timeoffset <= tdp[i]->timeoffset && tdp[i]->timeoffset <= endtimeoffset){
			temp.append(tdp[i]);
		}
	}

	//現在のリングバッファの中には指定されたタイムオフセット以降の結果は入っていなかった（未来を指定した）
	if(temp.size() == 0){
		std::cerr << "[ STRING RECORDER ] TimeDomainAnalyze() 時間オフセットの指定が未来です( timeoffset:" << timeoffset << " endtimeoffset:" << endtimeoffset << " )" << std::endl;
		std::cerr << "a_front: " << a_front << " 周辺のタイムオフセット値をダンプします." << std::endl;
		int j = 0;
		for(int i=a_front-10;i<a_length;i++){
			std::cout << "i:" << i << " t: " << tdp[i]->timeoffset << std::endl;
			j++;
			if(j > 20) break;
		}
		Error::Critical(0, QObject::tr("TimeDomainAnalyze() 時間オフセット指定エラー"));
	}

	//ゼロ指定の場合は基本解析長の結果をそのまま返す
	if(analyzeLength == 0){
		return temp;
	}

	//// 分析区間再構成（長くする方向のみ．短くする場合は別途，場合分けで実装が必要）

	//サブウィンドウ長さは，リングバッファーの１要素何個分？
	double subcount = analyzeLength / analyzeFrameLength;

	//平均計算用
	int j = 0;
	TimeDomainProfile *avg = temp[0];
	for(int i=1;i<temp.size();i++){
		(*avg) += *(temp[i]);
		j++;
		if(j >= subcount){
			avg->averageAmplitude /= subcount;
			avg->averagePower /= subcount;
			//平均
			for(int i=0;i<analyzeFrameLength;i++){
				avg->subwindow[i] /= subcount;
			}
			//再構成結果を戻り値に保存
			result_tdp.append(avg);
			j = 0;
		}
	}
	//分析区間を再構成した結果を返す
	return result_tdp;
}


/*!
 * \brief
 * 周波数領域の解析結果を返す（ユーティリティ関数）
 *
 * \param analyzeLength
 * 再解析長（ただし基本解析長での解析結果を用いるので再解析は高速に実行できる）
 * 基本解析長以上，かつ，基本解析長の整数倍の値を指定しなければならない．0を指定すると，基本解析長の結果をそのまま返す．
 * 
 * \param timeoffset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 *
 * \param endtimeoffset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 *
 * \returns
 * 周波数領域の解析結果
 * 
 * \remarks
 * 周波数領域の解析結果は解析長さを任意指定することはできない仕様．
 * 
 */
QVector<FrequencyDomainProfile*> StringRecorder::FrequencyDomainAnalyze(int analyzeLength, double timeoffset) const
{
	int index = 0;
	if(a_front == 0){
		index = 0;
	}else{
		index = a_front - 1;
	}
	double endtimeoffset = fdp[index]->timeoffset;
	return FrequencyDomainAnalyze(analyzeLength, timeoffset, endtimeoffset);
}

/*!
 * \brief
 * 周波数領域の解析結果を返す（本体実装）
 *
 * \param analyzeLength
 * 再解析長（ただし基本解析長での解析結果を用いるので再解析は高速に実行できる）
 * 基本解析長以上，かつ，基本解析長の整数倍の値を指定しなければならない．0を指定すると，基本解析長の結果をそのまま返す．
 * 
 * \param timeoffset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 *
 * \param endtimeoffset
 * 録音開始からの経過時間[ミリ秒]（この時間以降のデータが返される処理対象となる）
 *
 * \returns
 * 周波数領域の解析結果
 * 
 * \remarks
 * 周波数領域の解析結果は解析長さを任意指定することはできない仕様．
 * 
 */
QVector<FrequencyDomainProfile*> StringRecorder::FrequencyDomainAnalyze(int analyzeLength, double timeoffset, double endtimeoffset) const
{
	//不正な値をチェック
	if(analyzeLength != 0 && analyzeLength < analyzeFrameLength){
		//現在の仕様．短くすることも本来は出来るので，実装しても良い．
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() 第一引数で指定された再解析長が基本解析長を下回っています（基本解析長:" << analyzeFrameLength << " > 指定長:" << analyzeLength << "）"<< std::endl;		
		Q_ASSERT(false);
	}
	if(analyzeLength != 0 && analyzeLength % analyzeFrameLength != 0){
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() 第一引数で指定された再解析行が基本解析長の整数倍になっていないため結果に欠落が生じます．" << ( analyzeLength%analyzeFrameLength )<<std::endl;
	}

	//戻り値
	QVector<FrequencyDomainProfile*> result_fdp;

	if(!initialized){
		std::cout << "[ STRING RECORDER ] FrequencyDomainAnalyze() 初期化前に呼び出されました" << std::endl;
		return result_fdp;
	}
	
	//一時変数
	QVector<FrequencyDomainProfile*> temp;

	//解析結果保存用リングバッファの中をシーケンシャルスキャン
	//古い方の先端が a_front だよ
	for(int i=a_front;i<a_length;i++){
		if(timeoffset <= fdp[i]->timeoffset && fdp[i]->timeoffset <= endtimeoffset){
			temp.append(fdp[i]);
		}
	}
	for(int i=0;i<a_front;i++){
		if(timeoffset <= fdp[i]->timeoffset && fdp[i]->timeoffset <= endtimeoffset){
			temp.append(fdp[i]);
		}
	}

	//現在のリングバッファの中には指定されたタイムオフセット以降の結果は入っていなかった（未来を指定した）
	if(temp.size() == 0){
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() 時間オフセットの指定が異常( timeoffset:" << timeoffset << " endtimeoffset:" << endtimeoffset << " )" << std::endl;
		Error::Critical(0, QObject::tr("FrequencyDomainAnalyze() 時間オフセット指定エラー"));
		return result_fdp;
	}

	//再解析しないので基本解析長の結果をそのまま返す
	if(analyzeLength == 0){
		return temp;
	}

	//// 再解析
	
	double subcount = analyzeLength / analyzeFrameLength;
	//平均計算用
	int j = 0;
	FrequencyDomainProfile *avg = new FrequencyDomainProfile(windowSize);
	for(int i=0;i<temp.size();i++){
		avg->add(temp[i]);
		j++;
		if(j >= subcount){
			//平均
			avg->F0 = avg->F0/subcount;
			for(int k=0;k<windowSize;k++){
				avg->cps[k] = avg->cps[k]/subcount;
				avg->dft[k] = avg->dft[k]/subcount;
			}
			//再構成結果を戻り値に保存
			result_fdp.append(avg);
			avg = new FrequencyDomainProfile(windowSize);
			j = 0;
		}
	}
	//最後のメモリアロケーションは余計
	delete avg;

	if(result_fdp.size() == 0){
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() 結果が異常( timeoffset:" << timeoffset << " endtimeoffset:" << endtimeoffset << " )" << std::endl;
		Error::Critical(0, QObject::tr("FrequencyDomainAnalyze() 周波数再解析実行エラー"));
	}

	//分析区間を再構成した結果を返す
	return result_fdp;
}


