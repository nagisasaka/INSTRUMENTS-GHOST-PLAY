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

#include "calibration.hpp"
#include "actuator/controller.hpp"
#include "actuator/mlinear/mlinear.hpp"
#include "constants.hpp"
#include "audio/audioBuffer.hpp"
#include "audio/timeDomainProfile.hpp"
#include "vecmath.hpp"


#include <QCoreApplication>

Calibration::Calibration()
{
	//// 初期化
	std::cout << "[ CALIBRATION ] キャリブレーションクラス初期化" << std::endl;

	//初期位置のクリア
	for(int i=0;i<7;++i){ // [1,4]弦, 各弦間
		Positions pos;
		pos.start << 0 << 0 << 0; //ボーイング開始点 軸0,1,2 の位置をクリア
		pos.end   << 0 << 0 << 0; //ボーイング終了点 軸0,1,2 の位置をクリア
		//初期位置のクリア
		initialPositions << pos;
	}

	//キャリブレーション済位置のクリア
	for(int i=0;i<7;i++){ // [1,4]弦，各弦間
		Positions pos;
		pos.start << 0 << 0 << 0; // ボーイング開始点 軸0,1,2 の位置を全てゼロクリア
		pos.end   << 0 << 0 << 0; // ボーイング終了点 軸0,1,2 の位置を全てゼロクリア
		//キャリブレーション済位置のクリア
		positions << pos;
	}

	//手動停止フラグ
	manualStopFlag = false;

	//デバッグ
	_debug = 0;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  ロード関連
//
//////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief
 * 設定のロード
 * 
 * \see
 * LoadInitialPosition() | LoadCalibratedPosition()
 */
void Calibration::Load()
{
	std::cout << "[ CALIBRATION ] キャリブレーション設定ロード" << std::endl;
	//初期位置のロード
	LoadInitialPosition();
	//キャリブレーション済位置のロード
	LoadCalibratedPosition();
}

/*!
 * \brief
 * 初期位置をロードする．
 *
 */
void Calibration::LoadInitialPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);

	//// 配列開始
	int size = settings.beginReadArray("Calibration/Positions/Initial");

	//各弦[1,4]
	if(size != 7){
		Error::Warning(0, QObject::tr("キャリブレーション初期位置設定数が不正です（1）: %1").arg(size));
	}

	//各弦について
	for(int nb_string=0;nb_string<size;++nb_string){
		settings.setArrayIndex(nb_string);
		Positions pos;
		//ボーイング開始点での軸0,1,2の位置
		pos.start << settings.value("start0").toDouble();
		pos.start << settings.value("start1").toDouble();
		pos.start << settings.value("start2").toDouble();
		//ボーイング終了点での軸0,1,2の位置
		pos.end << settings.value("end0").toDouble();
		pos.end << settings.value("end1").toDouble();
		pos.end << settings.value("end2").toDouble();
		//初期位置
		initialPositions[nb_string] = pos;
	}

	//配列終了
	settings.endArray();
}

/*!
 * \brief
 * キャリブレーション済位置をロードする
 * 
 */
void Calibration::LoadCalibratedPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);
	int size = settings.beginReadArray("Calibration/Positions/Calibrated");

	//キャリブレーション済位置
	if(size != 7){
		Error::Warning(0,QObject::tr("キャリブレーションポジション設定数が不正です（2）:%1").arg(size));
	}

	//各位置について
	for(int i=0;i<size;++i){
		settings.setArrayIndex(i);
		Positions pos;
		pos.start << settings.value("start0").toDouble();
		pos.start << settings.value("start1").toDouble();
		pos.start << settings.value("start2").toDouble();
		pos.end   << settings.value("end0").toDouble();
		pos.end   << settings.value("end1").toDouble();
		pos.end   << settings.value("end2").toDouble();
		//キャリブレーション済位置
		positions[i] = pos;
	}
	settings.endArray();
}

//////////////////////////////////////////////////////////////////////////////////
//
//  セーブ関連
//
//////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief
 * 設定のセーブ
 * 
 * \see
 * SaveInitialPosition() | SaveCalibratedPosition()
 */
void Calibration::Save()
{
	std::cout << "[ CALIBRATION ] キャリブレーション設定セーブ" << std::endl;
	//初期位置のセーブ
	SaveInitialPosition();
	//キャリブレーション済位置のセーブ
	SaveCalibratedPosition();
}

/*!
 * \brief
 * 初期位置を保存する
 * 
 */
void Calibration::SaveInitialPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);

	//// 配列開始
	settings.beginWriteArray("Calibration/Positions/Initial");
	int size = initialPositions.size();

	//各弦（初期位置なので，[1,4]弦）
	if(size != 7){
		Error::Warning(0, QObject::tr("キャリブレーション初期位置設定数が不正です（3）:%1").arg(size));
	}

	//各弦について
	for(int nb_string=0;nb_string<size;++nb_string){
		settings.setArrayIndex(nb_string);
		//ボーイング開始点
		settings.setValue("start0", initialPositions[nb_string].start[0]); //軸0の位置
		settings.setValue("start1", initialPositions[nb_string].start[1]); //軸1の位置
		settings.setValue("start2", initialPositions[nb_string].start[2]); //軸2の位置
		//ボーイング終了点
		settings.setValue("end0", initialPositions[nb_string].end[0]);     //軸0の位置
		settings.setValue("end1", initialPositions[nb_string].end[1]);     //軸1の位置
		settings.setValue("end2", initialPositions[nb_string].end[2]);     //軸2の位置
	}

	//// 配列終了
	settings.endArray();
}

/*!
 * \brief
 * キャリブレーションされた値を保存する
 * 
 */
void Calibration::SaveCalibratedPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);

	//// 配列開始
	settings.beginWriteArray("Calibration/Positions/Calibrated");
	int size = positions.size();

	//キャリブレーション済位置（1,2,3,4,1-2,2-3,3-4 なので 7項目）
	if(size != 7){
		Error::Warning(0,QObject::tr("キャリブレーション済位置設定数が不正です（4）:%1").arg(size));
	}

	//各位置について
	for(int nb_string=0;nb_string<size;++nb_string){
		settings.setArrayIndex(nb_string);
		//ボーイング開始点
		settings.setValue("start0", positions[nb_string].start[0]);
		settings.setValue("start1", positions[nb_string].start[1]);
		settings.setValue("start2", positions[nb_string].start[2]);
		//ボーイング終了点
		settings.setValue("end0", positions[nb_string].end[0]);
		settings.setValue("end1", positions[nb_string].end[1]);
		settings.setValue("end2", positions[nb_string].end[2]);
	}
	
	//// 配列終了
	settings.endArray();
}

void Calibration::Start(int nb_string)
{
	nbString = nb_string;
	manualStopFlag = false;
	start();
}

void Calibration::Stop()
{
	manualStopFlag = true;
}

void Calibration::SoundCheck()
{
	/*
	control->axis[4]->SetMode(MLinear::CONTINUOUS);
	QVector<double> tousoku;
	//ブラックロックシュート！
	//10msec x 1% ずつ -> 片道1秒、往復2秒

	//3%分だけ前後を1秒間に8回（一回の時間 0.125秒/片道0.0625秒=62.5msec）	
	for(int k=0;k<10;k++){
		int turn = 6;
		for(int i=0;i<turn;i++){
			tousoku << (double)i/3.0;
		}
		for(int i=turn;i>=0;i--){
			tousoku << (double)i/3.0;
		}
	}

	control->axis[4]->SetPositionVector(tousoku);
	control->axis[4]->Prepare();
	control->axis[4]->Commit();

	return;
	*/

}

void Calibration::run()
{	
	/////////////////////////////////////////////////////////////////////
	//
	// 初期移動
	//
	/////////////////////////////////////////////////////////////////////

	Controller *control = Controller::GetInstance();
	//ロックでホーム位置へ
	control->SetHome();

	//オーディオ関連取得
	StringRecorder *recorder = control->GetStringRecorder();
	AudioBuffer *buffer = recorder->GetBuffer();

	//ストリングレコーダーチェック
	if(!recorder->isRecording()){
		Error::Critical(0, QObject::tr("string recorder is not activated"));
	}

	//初期移動
	for(int i=0;i<3;i++){
		//ジョグモードでよいかどうか？
		control->axis.at(i)->SetMode(MLinear::JOG); // ジョグモード
		double target = initialPositions[nbString-1].start[i]; // calib_step分だけ足し算する
		control->axis.at(i)->SetPosition(target); // 目標位置セット
		std::cout << "[ キャリブレーション初期移動 ]" << target << std::endl;
		control->axis.at(i)->SetDuration(1000, 0.2, 0.2); // 時間セット（1秒間で移動）
		control->axis.at(i)->Prepare(); 
	}

	//移動実行
	for(int i=0;i<3;i++){
		control->axis.at(i)->Commit();
	}

	//ジョグモード後のウェイト
	Sleep(1000);

	/////////////////////////////////////////////////////////////////////
	//
	// キャリブレーションループ開始
	//
	/////////////////////////////////////////////////////////////////////

	//移動設定（軸0,1,2について）
	int counter = 0;
	double calib_step = 0.05;
	double calib = 0;
	double calibrated_pos = 0;

	while(true)
	{
		//// 軸0,1についてループ移動開始 ////

		for(int i=0;i<2;i++){
			//ジョグモードでよいかどうか？
			control->axis.at(i)->SetMode(MLinear::JOG); // ジョグモード
			calibrated_pos = initialPositions[nbString-1].start[i] + calib; // calib_step分だけ足し算する
			control->axis.at(i)->SetPosition(calibrated_pos); // 目標位置セット
			std::cout << "[ キャリブレーション目標位置 ]" << calibrated_pos << std::endl;
			control->axis.at(i)->SetDuration(1000, 0.2, 0.2); // 時間セット(200ミリ秒で移動)
			control->axis.at(i)->Prepare(); 

			//逐次保存
			positions[nbString-1].start[i] = calibrated_pos;
			positions[nbString-1].end[i] = calibrated_pos;
			emit UpdateCalibratedPosition();
		}
		//逐次保存（する必要はないけれど，特に害もないのでここで保存する）
		positions[nbString-1].start[2] = initialPositions[nbString-1].start[2];
		positions[nbString-1].end[2] = initialPositions[nbString-1].end[2];

		//移動実行
		for(int i=0;i<2;i++){
			control->axis.at(i)->Commit();
		}

		//キャリブレーションステップ更新
		calib += calib_step;
		//カウンター更新
		counter++;
		//ウェイト
		Sleep(1000);

		//// 軸2 について運行開始 ////

		//運転終了シグナルの接続
		//connect(control->axis[2], SIGNAL(moveComplete(int)), this, SLOT(Repeat(int)));

		//軸2は指定幅で全弓使い切って往復運転開始
		control->axis[2]->SetMode(MLinear::CONTINUOUS);
		QVector<double> tousoku;

		//ブラックロックシュート！
		int axis2_start = initialPositions[nbString-1].start[2];
		int axis2_end   = initialPositions[nbString-1].end[2];
		std::cout << "start:" << axis2_start << ", end:" << axis2_end << std::endl;

		//10msec x 1% ずつ -> 片道1秒、往復2.4秒
		for(int i=axis2_start;i<axis2_end;i++){
			tousoku << i;
			std::cout << i << ",";
		}
		for(int i=0;i<40;i++){
			tousoku << axis2_end;
		}
		for(int i=axis2_end;i>=axis2_start;i--){
			tousoku << i;
		}
		control->axis[2]->SetPositionVector(tousoku);
		control->axis[2]->Prepare();

		//運転開始の直前にオーディオバッファーのカレントオフセットを確認
		double timeoffset_start = buffer->offset_msec();
		//運転実行
		control->axis[2]->Commit();
		//往復2秒（基準）なので大雑把に待つ; 一応待ち時間を計算する
		Sleep(2500*(axis2_end-axis2_start)/100.);
		//運転終了時のオーディオバッファーのカレントオフセットを確認
		double timeoffset_end = buffer->offset_msec();

		//二つのオフセットの間の値を表示する
		QVector<TimeDomainProfile*> tdp = recorder->TimeDomainAnalyze(0, timeoffset_start, timeoffset_end);
		std::cout << "***********************************************************" << std::endl;
		std::cout << "[ キャリブレーション ] オフセット値: " << timeoffset_start << " サブリングサイズ: " << tdp.size() << std::endl;
		double sum = 0;
		for(int i=0;i<tdp.size();i++){
			sum += tdp[i]->maxAmplitude;
		}
		double avg = sum/tdp.size();
		std::cout << "[ キャリブレーション ] 平均最大振幅: " << avg << std::endl;
		std::cout << "***********************************************************" << std::endl;

		if(avg >= 0.0025){
			std::cout << "[ キャリブレーション ] 終了..." << std::endl;
			break;
		}

		if(manualStopFlag){
			std::cout << "[ キャリブレーション ] 手動終了ボタンの押下による手動終了が実行されました．" << std::endl;
			break;
		}
	}
	//キャリブレーション終了
	Save();
	emit CalibrationEnd();
		
		/*
		//【下の形式だとエラーが出やすい】
		//減速時間ゼロで指定しているので，逆方向パルスでも打ち込まない限り
		//どうしても自然な減速時間が掛かってしまうため，減速時間＋位置決め
		//静定時間の間に次の命令を発行するとエラーになる．
		//位置決め静定時間よりも，最終パルス発行後の減速時間の方が実質的な
		//影響を及ぼしていると考えて妥当か．
		//moveComplete(int)シグナルを待っても，そのシグナルは結局パルサー
		//レベルなので，実際の終了はわからない設計になっている．もっとも
		//この点は，アンプからのフィードバック信号をパルサーの適当なポート
		//に入力すれば解決するのだが，あまり実用性が無いような気がして，
		//サボっている状態．
		control->axis[2]->SetMode(MLinear::JOG);
		control->axis[2]->SetPosition(100);
		control->axis[2]->SetDuration(1000);
		control->axis[2]->Prepare();
		control->axis[2]->Commit();
		Sleep(5000);
		control->axis[2]->SetMode(MLinear::JOG);
		control->axis[2]->SetPosition(1);
		control->axis[2]->SetDuration(1000);
		control->axis[2]->Prepare();
		control->axis[2]->Commit();
		*/
}

void Calibration::Repeat(int uid)
{
	std::cout << "[ REPEAT ] UID=" << uid << std::endl;
	if(uid == 2){
		Controller *control = Controller::GetInstance();
		control->axis[2]->SetMode(MLinear::CONTINUOUS);
		QVector<double> tousoku;
		for(int i=0;i<100;i++){
			tousoku << i;
		}
		for(int i=100;i>0;i--){
			tousoku << i;
		}
		control->axis[2]->SetPositionVector(tousoku);
		control->axis[2]->Prepare();
		control->axis[2]->Commit();

		//デバッグ
		_debug++;
		if(_debug == 1){
			disconnect(control->axis[2], SIGNAL(moveComplete(int)), this, SLOT(Repeat(int)));
		}
	}
}