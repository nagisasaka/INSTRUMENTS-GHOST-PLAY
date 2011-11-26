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

#include "controller.hpp"
#include "constants.hpp"
#include "../clock.hpp"

//シングルトンインスタンス
Controller* Controller::instance = 0;

//更新サイクル
double Controller::cycle = 10.;

//ダミー軸
const unsigned int Controller::dummyAxis = 7;

Controller::Controller()
{
	//// PCI7443制御ボードを初期化

	//ハードウェア初期化
	retCode = _7443_initial(&totalCard);
	if(totalCard == 0){
		throw "パルサーが見つからないためリニア軸を初期化できません.";
	}

	//動作設定ロード
	retCode = _7443_config_from_file((char*)Constants::INI_FILE_1.toStdString().c_str());
	if(retCode){
		throw "リニア軸動作設定ファイルが見つかりません.";
	}

	//機械バージョンの表示
	U16 hardwareInfo = 0;
	U16 softwareInfo = 0;
	U16 driverInfo = 0;
	retCode = _7443_version_info(0,&hardwareInfo,&softwareInfo,&driverInfo);
	std::cout << "[ ハードウェア情報 ] " << hardwareInfo << ", ";
	for(int i=0;i<sizeof(unsigned short)*8;i++){
		printf("%d", (hardwareInfo >> i) & 0x01);
	}
	std::cout << std::endl;
	std::cout << "[ ソフトウェア情報 ] " << softwareInfo << ", ";
	for(int i=0;i<sizeof(unsigned short)*8;i++){
		printf("%d", (softwareInfo >> i) & 0x01);
	}
	std::cout << std::endl;
	std::cout << "[ ドライバー情報   ] " << driverInfo << ", ";
	for(int i=0;i<sizeof(unsigned short)*8;i++){
		printf("%d", (driverInfo >> i) & 0x01);
	}
	std::cout << std::endl;

	//複数軸トランザクションの準備
	for(int i=0;i<totalCard*4;i++){
		//連続直線補間モードをオフに（位置決め補完モードに）
		_7443_set_line_move_mode(i,0);
		//補間速度モードをオフに
		_7443_set_axis_option(i,0);
	}

	//// Phidgets アドバンストサーボコントローラを初期化

	const char *err;

	//サーボオブジェクトを構築
	CPhidgetAdvancedServo_create(&servoHandle);

	//各種のハンドラを設定
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)servoHandle, Servo::AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)servoHandle, Servo::DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)servoHandle, Servo::ErrorHandler, NULL);
	CPhidgetAdvancedServo_set_OnPositionChange_Handler(servoHandle, Servo::PositionChangeHandler, NULL);

	//デバイスへの接続開始
	CPhidget_open((CPhidgetHandle)servoHandle, -1);

	//デバイスへの接続待ち
	int result = 0;
	printf("[ CONTROLLER ] サーボモーターの接続待ち...（別プロセスは立ち上がっていませんか？）");
	if((result = CPhidget_waitForAttachment((CPhidgetHandle)servoHandle, 10000)))
	{
		CPhidget_getErrorDescription(result, &err);
		printf("[ CONTROLLER ] サーボモーター接続待ち中にエラー: %s\n", err);
		Error::Critical(0, QObject::tr("[ CONTROLLER ] サーボモーター接続待ち中にエラー."));
	}

	//サーボモーターのプロパティを表示する
	display_properties(servoHandle);

	//イベントデータを受信する
	printf("[ CONTROLLER ] サーボモーター情報読み込み中.....\n");
	
	//display current motor position
	double curr_pos;
	if(CPhidgetAdvancedServo_getPosition(servoHandle, 0, &curr_pos) == EPHIDGET_OK){
		printf("モーター: 0 > 現在位置: %f\n", curr_pos);
	}
	
	std::cout << "[ CONTROLLER ] クロック管理構築..." << std::endl;
	//クロック管理クラス
	clock = new Clock(Controller::cycle);

	std::cout << "[ CONTROLLER ] 軸初期化開始..." << std::endl;
	//制御下にある全軸の初期化
	//リニアアクチュエータ
	axis.push_back(new MLinear(0, 17000, 0)); //運弓基部
	axis.push_back(new MLinear(1, 17000, 1)); //運弓基部
	axis.push_back(new MLinear(2, 43000, 2)); //運弓部
	axis.push_back(new MLinear(6, 17000, 3)); //音階決定部
	axis.push_back(new MLinear(4, 17000, 4));
		
	//IAIサーボ（暫定的にこのフレームに乗せていない）
	//axis.push_back(new IAIServo(5)); //未実装
	//axis.push_back(new IAIServo(6)); //未実装

	//// 音階決定部サーボ
	std::cout << "[ CONTROLLER ] サーボモーター初期化" << std::endl;
	servo.push_back(new Servo(&servoHandle, 0));
	servo.push_back(new Servo(&servoHandle, 1));

	std::cout << "[ CONTROLLER ] シリアル通信を構築" << std::endl;
	//// シリアル通信クラスを先に構築
	scc = new StellarSerialCom();
	bool success = scc->Open();
	if(!success){
		std::cout << "[ CONTROLLER ] シリアルポートを開けません．シリアルポート番号は変わっていませんか？ポート自動探索はサポートしていません！" << std::endl;
		Error::Critical(0, QObject::tr("[ CONTROLLER ] シリアルポートを開けません．シリアルポート番号は変わっていませんか？ポート自動探索はサポートしていません！"));
	}

	std::cout << "[ CONTROLLER ] IAIリニアアクチュエータ開始" << std::endl;
	//// IAIアクチュエータ
	iaic.push_back(new IAICtrler(scc, 1)); // 軸番号は1スタートであることに注意
	iaic.push_back(new IAICtrler(scc, 2));

	//運転開始フラグを立てる

	//// ストリングレコーダー
	std::cout << "[ CONTROLLER ] ストリングレコーダー構築" << std::endl;
	recorder = new StringRecorder();
}


void Controller::Init()
{
	//std::cout << "[ CONTROLLER ] ストリングレコーダー終了" << std::endl;
	//recorder->EndRecord();
	if(recorder->isRecording()){
		recorder->EndRecord();
	}

	std::cout << "[ CONTROLLER ] 全軸ホームポジションへ移動開始..." << std::endl;

	//サーボモーターホームを優先
	for(int i=0;i<servo.size();i++){
		servo.at(i)->Init();
	}

	//IAIアクチュエータの機構退避を優先する（機構がジャムると過電流の前に51番偏差エラーで緊急停止する．軸3,4にはブレーキ機構がないのでそのまま落下する危険）
	for(int i=0;i<iaic.size();i++){
		iaic.at(i)->Evacuate();
	}
	//機構退避はシーケンシャルに行う．
	Sleep(500);

	//ホーム位置への移動
	for(int i=0;i<axis.size();i++){
		axis.at(i)->Init();
	}

	//// IAIアクチュエータはアブソリュートモードなので原点復帰の必要はない（単に原点復帰すると機構がジャムるので時間差が必要） ////
	//IAIアクチュエータ原点復帰
	//for(int i=0;i<iaic.size();i++){
	//	iaic.at(i)->ReturnOrigin();
	//}

	//すべての移動終了まで待つ
	while(true){
		std::cout << "[ CONTROLLER ] 移動状態確認... " << std::endl;
		bool isMoving = false;
		HardTimer::Wait(1000);
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Update();
			ActuatorStatus status = axis.at(i)->GetStatus();
			if(status.Movement != Actuator::STOP){
				std::cout << "軸 #" << axis.at(i)->GetUID() << " 移動中" << std::endl;
				isMoving = true;
			}else{
				std::cout << "軸 #" << axis.at(i)->GetUID() << " 停止中" << std::endl;
			}
		}
		if(!isMoving){
			std::cout << "[ CONTROLLER ] 全軸ホームポジションへ移動完了．" << std::endl;
			break;
		}
	}

	std::cout << "[ CONTROLLER ] ストリングレコーダー開始" << std::endl;
	recorder->StartRecord();
}

void Controller::SetHome()
{
	std::cout << "[ CONTROLLER ] 全軸ホームポジションへ移動開始..." << std::endl;

	//初期化
	for(int i=0;i<servo.size();i++){
		servo.at(i)->Init();
	}
	//機構退避
	for(int i=0;i<iaic.size();i++){
		iaic.at(i)->Evacuate();
	}
	//機構退避はシーケンシャルに優先する．
	Sleep(500);

	//ホーム位置への移動
	for(int i=0;i<axis.size();i++){
		axis.at(i)->Init();
	}

	//// IAIアクチュエータはアブソリュートモードなので原点復帰の必要はない（単に原点復帰すると機構がジャムるので時間差が必要） ////
	//IAIアクチュエータ原点復帰
	//for(int i=0;i<iaic.size();i++){
	//	iaic.at(i)->ReturnOrigin();
	//}

	//すべての移動終了まで待つ（サーボ側は電磁弁的制御の為待つ必要は無い）
	while(true){
		std::cout << "[ CONTROLLER ] 移動状態確認... " << std::endl;
		bool isMoving = false;
		HardTimer::Wait(1000);
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Update();
			ActuatorStatus status = axis.at(i)->GetStatus();
			if(status.Movement != Actuator::STOP){
				std::cout << "軸 #" << axis.at(i)->GetUID() << " 移動中" << std::endl;
				isMoving = true;
			}else{
				std::cout << "軸 #" << axis.at(i)->GetUID() << " 停止中" << std::endl;
			}
		}
		if(!isMoving){
			std::cout << "[ CONTROLLER ] 全軸ホームポジションへ移動完了．" << std::endl;
			break;
		}
	}
}


/*!
 * \brief
 * プロパティを表示する
 * 
 * \param phid
 * ハンドル
 * 
 */
void Controller::display_properties(CPhidgetAdvancedServoHandle phid)
{
	int serialNo, version, numMotors;
	const char* ptr;

	//デバイスタイプ
	CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
	//シリアル番号
	CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
	//デバイスバージョン
	CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);
	//モーター数
	CPhidgetAdvancedServo_getMotorCount (phid, &numMotors);

	printf("[ CONTROLLER ] サーボモーター情報\n");
	printf("%s\n", ptr);
	printf("Serial Number: %10d\nVersion: %8d\n# Motors: %d\n", serialNo, version, numMotors);
}


/*!
 * \brief
 * 制御タイマー
 * 
 * \remarks
 * clock.hpp による仕組みから移行して，コントローラーの中で single shot timer を使う形とした．
 * 
 * \see
 * Separate items with the '|' character.
 */
void Controller::run()
{
	//時間測定用変数の初期化
	LARGE_INTEGER nFreq, nBefore, nAfter;
	memset(&nFreq,   0x00, sizeof nFreq);
	memset(&nBefore, 0x00, sizeof nBefore);
	memset(&nAfter,  0x00, sizeof nAfter);
	double dwTime = 0.;
	double swTime1 = 0.; // 合計のズレ時間
	double swTime2 = 0.; //      =
	int ret  = 0;
	double wait = 0;
	QueryPerformanceFrequency(&nFreq);
	if(nFreq.QuadPart == 0){
		Error::Critical(0, QObject::tr("システムが QueryPerformanceCounter() 関数をサポートしていません．"));
	}

	//オペレーション中
	while(inOperation){
		//処理開始時刻 ← 時刻0
		QueryPerformanceCounter(&nBefore);	
		//時間計測対象
		{
			//離散制御実行
			Update();
			//クロック管理保存（クロックが停止しているときは単に無視される）
			clock->IncrementCurrentClock();
		}
		//処理終了時刻
		QueryPerformanceCounter(&nAfter);
		//処理に要した時間
		dwTime = (double)((nAfter.QuadPart - nBefore.QuadPart) * 1000. / nFreq.QuadPart); // 0.03msec -> 1000発 = 30msec
		//今回の待ち時間（これが出来るほどの精度の良いタイマーがない）
		//wait = cycle; - dwTime;
		//まとめて補正
		swTime1 += dwTime;
		if(swTime1 >= 1.0){
			swTime1 = swTime1 - 1.0;
			wait = (int)cycle - 1; // int型にキャストした際に cycle - 1 になるように
		}else{
			wait = cycle;
		}
		
		//実際に待つ
		if(_7443_delay_time(dummyAxis, (int)wait)){
			Error::Critical(0, QObject::tr("ハードウェアタイマーが利用できませんでした．"));
		}
		//処理終了 ← 時刻cycle 		
	}
}

/*!
 * \brief
 * 制御下の各軸の制御更新を行う
 * 
 */
void Controller::Update()
{
	//主軸（軸更新）；遅延無し
	for(int i=0;i<axis.size();i++){
		if(axis.at(i) != 0) axis.at(i)->Update();
	}


	//第三軸（サーボ更新）；最も遅い可能性がある
	for(int i=0;i<servo.size();i++){
		if(servo.at(i) != 0) servo.at(i)->Update();
	}

	//第二軸（IAIC更新）；次に遅いと思われる
	for(int i=0;i<iaic.size();i++){
		if(iaic.at(i) != 0) iaic.at(i)->Update();
	}
}