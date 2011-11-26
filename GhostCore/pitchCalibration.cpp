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

#include "pitchCalibration.hpp"
#include "calibration.hpp"
#include "play/pitchunit.hpp"
#include "constants.hpp"
#include "error.hpp"
#include "bowing.hpp"

#include <QSettings>
#include <QStringList>
#include <QString>

#include <cmath>

PitchCalibration::PitchCalibration(int _nb_pitchUnit) : nb_pitchUnit(_nb_pitchUnit)
{
	//// ポジション定義

	//G線上のポジション
	pg["G"]  = 195.97; // 開放弦 // 1
	pg["G#"] = 207.65; // 2
	pg["A"]  = 220.08; // 3
	pg["A#"] = 233.08; // 4
	pg["B"]  = 246.94; // 5
	pg["C"]  = 261.62; // 6
	pg["C#"] = 277.18; // 7
	pg["D"]  = 293.66; // 8
	//pg["D#"] = 311.12; // 9

	//D線上のポジション
	pd["D"]  = 293.66; // 開放弦 // 1
	pd["D#"] = 311.12; // 2
	pd["E"]  = 329.62; // 3
	pd["F"]  = 349.23; // 4
	pd["F#"] = 369.99; // 5
	pd["G"]  = 391.99; // 6
	pd["G#"] = 415.30; // 7
	pd["A"]  = 440.00; // 8
	//pd["A#"] = 466.16; // 9

	//A線上のポジション
	pa["A"]  = 440.00; // 開放弦 // 1
	pa["A#"] = 466.16; // 2
	pa["B"]  = 493.88; // 3
	pa["C"]  = 523.25; // 4
	pa["C#"] = 554.36; // 5
	pa["D"]  = 587.32; // 6
	pa["D#"] = 622.25; // 7
	pa["E"]  = 659.25; // 8
	//pa["F"]  = 698.45; // 9

	//E線上のポジション
	pe["E"]  = 659.25; // 開放弦 // 1
	pe["F"]  = 698.45; // 2
	pe["F#"] = 739.98; // 3
	pe["G"]  = 783.99; // 4
	pe["G#"] = 830.61; // 5
	pe["A"]  = 880.00; // 6
	pe["A#"] = 932.33; // 7
	pe["B"]  = 987.76; // 8
	//pe["C"]  = 1046.50;// 9

	//// その他

	currentString = "G"; // 初期化
	currentPitch  = "G"; // 初期化
	operationMode = 0;

	//// キャリブレーション用ボーイング設定
	MakeStandardBowing();
}

PitchCalibration::~PitchCalibration()
{
	pg.clear();
	pd.clear();
	pa.clear();
	pe.clear();
}

void PitchCalibration::MakeStandardBowing()
{
	//当たり前ですが；
	//1%->170=1.7mm
	//10msec(1/100秒)で1%->10msec(1/100秒)で1.7mm->1000msec(1秒)で170mm=秒速170mm=秒速17cm=秒速0.17m=1秒間でストロークを使い切る速度

	//10msecで2%->秒速34cm(0.34m/s)
	//10msecで3%->秒速51cm(0.51m/s)
	//10msecで4%->秒速68cm(0.68m/s)
	//10msecで5%->秒速85cm(0.85m/s)

	double p = 0;
	int c = 0;
	
	//// 片道
	pressure.clear();
	bowposition.clear();

	if(currentString != "G"){

		for(int k=0;k<2;k++){

			//突入速度は高速を保つ
			for(int i=0;i<=50;i+=25){
				pressure.append(i);
			}//20msec

			//50%定常状態 1960msec
			for(int i=0;i<196;i++){		
				pressure.append(50);
			}

			//離弦シーケンス
			for(int i=50;i>=0;i-=25){
				pressure.append(i);
			}//20msec

			//2220msec
		}

	}else{

		for(int k=0;k<2;k++){

			//突入速度は高速を保つ
			for(int i=0;i<=80;i+=10){ // 16sequence
				pressure.append(i);
			}//80msec

			//80%定常状態 1840msec
			for(int i=0;i<184;i++){		
				pressure.append(80);
			}

			//離弦シーケンス
			for(int i=80;i>=0;i-=10){
				pressure.append(i);
			}//80msec
		}

	}

	//// 弓決定

	//最初の150msecは動かない
	for(int i=0;i<15;i++){
		bowposition.append(0);
	}

	//2秒等速アップボウ

	for(int i=0;i<50;i++){
		bowposition.append(i);
		bowposition.append(i+0.25);
		bowposition.append(i+0.50);
		bowposition.append(i+0.75);
	}

	//50msec待つ
	for(int i=0;i<5;i++){
		bowposition.append(50);
	}

	//2200msec

	//圧力シーケンス終了

	//最初の150msecは動かない
	for(int i=0;i<15;i++){
		bowposition.append(50);
	}

	//2秒等速ダウンボウ
	for(int i=50;i>0;i--){
		bowposition.append(i);
		bowposition.append(i-0.25);
		bowposition.append(i-0.5);
		bowposition.append(i-0.75);
	}

	//50msec wait
	for(int i=0;i<5;i++){
		bowposition.append(0);
	}

	//2200msec
	//4400msec
}

/*!
 * \brief
 * ピッチキャリブレーション情報をロードする
 * 
 * 【形式】
 * ID番号, 弦名, ピッチ名，ピッチユニット番号，軸位置，サーボ押込位置
 *
 *
 */
void PitchCalibration::Load()
{
	//古いポジションはクリアする
	positions.clear();

	QSettings settings(Constants::INI_FILE_3, QSettings::IniFormat);

	QStringList keys = settings.allKeys();
	int size = keys.size();
	if(size == 0){
		std::cout << "[ ピッチキャリブレーション ] キャリブレーション設定ファイルが存在しません" << std::endl;
	}else{
		std::cout << "[ ピッチキャリブレーション ] " << size << " 個のキャリブレーション位置を読み込みました" << std::endl;
	}
	for(int i=0;i<size;i++){
		QString line = settings.value(keys[i]).toString();
		QStringList temp = line.split(",");
		if(temp.size() != 6){
			std::cout << "[ ピッチキャリブレーション ] 行形式が不正です: " << line.toStdString() << std::endl;
			Error::Critical(0, QObject::tr("[ ピッチキャリブレーション ] 行形式が不正です．"));
		}
		CalibratedPositions pos;
		pos.id           = temp[0].toInt();    // ID番号
		pos.stringName   = temp[1];            // 弦名
		pos.pitchName    = temp[2];            // ピッチ名
		pos.nb_pitchUnit = temp[3].toInt();    // 軸番号
		pos.z_value      = temp[4].toDouble(); // Z軸位置
		pos.c_value      = temp[5].toDouble(); // C軸位置
		positions.append(pos);		
	}
}

/*!
 * \brief
 * ピッチキャリブレーション情報をセーブする
 * 
 * 【形式】
 * ID番号, 弦名, ピッチ名，軸番号，軸位置，サーボ押込位置
 *
 *
 */
void PitchCalibration::Save()
{
	QSettings settings(Constants::INI_FILE_3, QSettings::IniFormat);

	int size = positions.size();
	if(size == 0){
		std::cout << "[ ピッチキャリブレーション ] キャリブレーションデータが存在しない状態で保存を試行しました．" << std::endl;
	}else{
		std::cout << "[ ピッチキャリブレーション ] " << size << " 個のキャリブレーションデータの保存を行います．" << std::endl;
	}

	//キャリブレーション全体を保存するので問答無用で追記されてしまうことに注意．
	for(int i=0;i<size;i++){
		CalibratedPositions pos = positions[i];

		QString writer = QString("%1,").arg(pos.id);
		writer.append(pos.stringName).append(",");
		writer.append(pos.pitchName).append(",");
		writer.append(QString("%1,").arg(pos.nb_pitchUnit));
		writer.append(QString("%1,").arg(pos.z_value));
		writer.append(QString("%1").arg(pos.c_value));

		QString key = QString();
		key.append(pos.stringName);
		key.append(pos.pitchName);
		key.append(QString("%1").arg(pos.nb_pitchUnit));

		settings.setValue(key, writer);
	}
	std::cout << "保存しました．" << std::endl;
}

void PitchCalibration::Save(double z_value, double c_value)
{

	int update_target = -1; // 既存データのインデックス
	for(int k=0;k<positions.size();k++){
		CalibratedPositions pos = positions[k];
		if(pos.stringName == currentString && 
		   pos.pitchName == currentPitch && 
		   pos.nb_pitchUnit == nb_pitchUnit){
			   std::cout << "[ ピッチキャリブレーション ] 弦: " << currentString.toStdString() << ", ピッチ:" << currentPitch.toStdString() << ", ユニット番号:" << nb_pitchUnit << std::endl;
				update_target = k;
				break;
		}
	}

	if(update_target == -1){
		//既存データにないので追加
		std::cout << "[ ピッチキャリブレーション ] 新規追加します．" << std::endl;
		CalibratedPositions pos;
		pos.id           = positions.size()+1; // ID番号
		pos.stringName   = currentString;      // 弦名
		pos.pitchName    = currentPitch;	   // ピッチ名
		pos.nb_pitchUnit = nb_pitchUnit;       // 軸番号
		pos.z_value      = z_value;				// Z軸位置
		pos.c_value      = c_value;			   // C軸位置
		//追加
		positions.append(pos);					
	}else{
		//既存データを更新
		std::cout << "[ ピッチキャリブレーション ] 更新します．" << std::endl;
		positions[update_target].z_value = z_value;
		positions[update_target].c_value = c_value;
	}
	Save();
}


/*!
 * \brief
 * 弦に含まれるすべてのポジション対するキャリブレーション実行
 * 
 * \param stringName
 * 対象弦
 * 
 */
void PitchCalibration::Start(QString stringName)
{
	currentString = stringName;	
	operationMode = 1;
	start();
}

/*!
 * \brief
 * 単音に対するキャリブレーション実行
 * 
 * \param stringName
 * 対象弦
 *
 * \param pitchName
 * 対象ピッチ名
 *
 */
void PitchCalibration::Start(QString stringName, QString pitchName)
{
	currentString = stringName;
	currentPitch  = pitchName;
	operationMode = 2;
	start();
}

void PitchCalibration::run()
{
	//ピッチキャリブレーション結果を強制ロード
	Load();

	//ボーイング再構築
	MakeStandardBowing();

	//一時保存
	QMap<double, double> scale;

	Controller *control = Controller::GetInstance();
	StringRecorder *recorder = control->GetStringRecorder();
	
	//// 利用するピッチユニットを構築
	PitchUnit *punit = 0;
	punit = new PitchUnit(nb_pitchUnit);

	//// 利用しないピッチユニットは退避（ピッチユニットが３個以上化した場合は別途対応せよ）
	int nb_otherPitchUnit = 0;
	if(nb_pitchUnit == 0){
		nb_otherPitchUnit = 1;
	}else{
		nb_otherPitchUnit = 0;
	}
	PitchUnit *others = 0;
	others = new PitchUnit(nb_otherPitchUnit);
	others->Evacuate();	

	if(operationMode == 1){
		//全ポジション

		//ボーイング準備
		Bowing *bowing = new Bowing();

		//ピッチユニット初期移動実行
		punit->JogGoToString(currentString);
		//0.5秒デフウェイト
		Sleep(500);

		//ピッチユニット実行, 70%まで140秒(1%/2秒)/押込90度

		double c_value = 0;
		if(nb_pitchUnit == 0){
			c_value = 55;
		}else{
			c_value = 130;
		}
		punit->Jog(75, 160000, c_value);
		std::cout << "[ ピッチキャリブレーション ] ピッチユニットの駆動開始" << std::endl;

		int main_counter = 0;
		while(true){
			//初期移動
			if(main_counter == 0){
				bowing->SetInitialMoveFlag(true);
			}else{
				bowing->SetInitialMoveFlag(false);
			}
			//弦選択
			if(currentString == "G"){
				bowing->SetStringPair(0,0);
				bowing->SetPlayerString(0);

			}else if(currentString == "D"){
				bowing->SetStringPair(2,2);
				bowing->SetPlayerString(2);

			}else if(currentString == "A"){

				bowing->SetStringPair(4,4);
				bowing->SetPlayerString(4);

			}else if(currentString == "E"){

				bowing->SetStringPair(6,6);
				bowing->SetPlayerString(6);

			}
			std::cout << "A";
			bowing->SetPressureVector(pressure);
			std::cout << "B";
			bowing->SetPositionVector(bowposition);
			std::cout << "C";
			bowing->Prepare();
			std::cout << "D";
			//ボーイング実行（ロックしない）
			bowing->Start();
			std::cout << "[ ピッチキャリブレーション ] # " << main_counter << " セット 標準ボーイング開始．CPS解析を開始します．" << std::endl;

			//// 周波数計測
			int sound_counter = 0;
			double min_fzero = 100;
			double max_fzero = 0;
			while(true){
				std::cout << 1;
				//解析結果コピー取得
				DWORD stime = GetTickCount();
				std::cout << 2;
				QVector<FrequencyDomainProfile*> fdp = recorder->FrequencyDomainAnalyze(1024, 0);//終端から（最新）の1024*20サンプル（約0.2秒分）
				std::cout << 3;
				//FZEROの取得
				double fzero = fdp.at(fdp.size()-1)->F0;
				std::cout << 4;
				//現在位置の取得
				double pos = punit->GetCurrentPosition();
				std::cout << 5;
				//記録
				if(fzero != 0){
					//ゼロスキップ
					scale[pos] = fzero;
				}
				//範囲を記録
				if(fzero < min_fzero){
					min_fzero = fzero;
				}
				if(max_fzero < fzero){
					max_fzero = fzero;
				}
				//解析結果の削除
				for(int i=0;i<fdp.size();i++) delete fdp[i];
				std::cout << 6;

				DWORD etime = GetTickCount();
				double timecost = etime - stime;				
				//デフウェイト
				Sleep(100 - timecost); // 0.1秒ごとに測定
				sound_counter++;
				if(main_counter == 0){
					if(sound_counter > 65){// 7秒間連続計測（先頭2秒のイニシャルムーブ）
						break;
					}
				}else{
					if(sound_counter > 45){// 5秒間連続計測
						break;
					}
				}
			}
			std::cout << "[ ピッチキャリブレーション ] # " << main_counter << " 最小F0: " << min_fzero << ", 最大F0: " << max_fzero << " を記録しました．セット終了します．" << std::endl;

			main_counter++;
			if(main_counter > 30){
				//およそ140秒で70%を超えて終了
				break;
			}
		}
		std::cout << "[ ピッチキャリブレーション ] 測定終了．" << std::endl;
		
		//後処理
		QMap<QString, double> rscale;
		if(currentString == "G"){
			rscale = pg;
		}else if(currentString == "D"){
			rscale = pd;
		}else if(currentString == "E"){
			rscale = pe;
		}else if(currentString == "A"){
			rscale = pa;
		}

		QMapIterator<QString, double> i(rscale);
		while(i.hasNext()){
			i.next();
			//参照スケールに対して
			currentPitch = i.key();  //ピッチ名
			double frequency = i.value(); //ピッチの参照周波数
			std::cout << "[ ピッチキャリブレーション ] " << currentPitch.toStdString() << " （参照周波数: " << frequency << "Hz）の最近傍測定点を算出中..." << std::endl;

			double mindistance = 100; // 計測スケールと参照スケールの最小距離（周波数）
			double nearestpos  = 0;   // 最近傍計測点

			QMapIterator<double, double> j(scale);
			double m_pos = 0;
			double m_frequency = 0;
			double m_distance = 0;
			double minfreq = 0;
			while(j.hasNext()){
				j.next();
				//計測スケールに対して
				m_pos = j.key();         //計測点
				m_frequency = j.value(); //計測周波数
	
				m_distance = fabs((double)(frequency-m_frequency));
				if(m_distance < mindistance){
					//最近傍点を見つける
					mindistance = m_distance; 
					minfreq = m_frequency;
					nearestpos = m_pos;
				}
			}
			std::cout << "[ ピッチキャリブレーション ] 計測点: " << nearestpos << ", 計測周波数: " << minfreq  << " を "<< mindistance << " Hz 差で，最近傍点として確定．" << std::endl;

			//最近傍点を全自動キャリブレーション結果として保存

			//既存データに存在すれば更新，無ければ追加
			int update_target = -1; // 既存データのインデックス
			for(int k=0;k<positions.size();k++){
				CalibratedPositions pos = positions[k];
				if(pos.stringName == currentString && 
					pos.pitchName == currentPitch && 
					pos.nb_pitchUnit == nb_pitchUnit){
						update_target = k;
						break;
				}
			}
			if(update_target == -1){
				//既存データにないので追加
				std::cout << "[ ピッチキャリブレーション ] 新規追加します．" << std::endl;
				CalibratedPositions pos;
				pos.id           = positions.size()+1; // ID番号
				pos.stringName   = currentString;      // 弦名
				pos.pitchName    = currentPitch;	   // ピッチ名
				pos.nb_pitchUnit = nb_pitchUnit;       // 軸番号
				pos.z_value      = nearestpos;		   // Z軸位置
				pos.c_value      = c_value;			   // C軸位置
				//追加
				positions.append(pos);					
			}else{
				//既存データを更新
				std::cout << "[ ピッチキャリブレーション ] 更新します．" << std::endl;
				positions[update_target].z_value = nearestpos;
				positions[update_target].c_value = c_value;
			}
		}
		std::cout << "[ ピッチキャリブレーション ] 軸キャリブレーションを終了しました．保存します．" << std::endl;

		//ピッチキャリブレーション結果をセーブ
		Save();

	}else if(operationMode == 2){
		//単音

		//ターゲット周波数の確認




	}else{
		Error::Critical(0, QString("[ Pitch Calibration ] Invalid Operation Mode."));
	}

	delete punit;
}