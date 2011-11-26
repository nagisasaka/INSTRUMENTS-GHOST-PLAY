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

#ifndef _CONTROLLER_HPP_
#define _CONTROLLER_HPP_

#include <QVector>
#include <QThread>

#include "actuator.hpp"
#include "mlinear/mlinear.hpp"
#include "mlinear/ppci7443.h"
#include "../audio/stringRecorder.hpp"
#include "servo/servo.hpp"
#include "servo/phidget21.h"
#include "iai/IAICtrler.hpp"

#include <iostream>

class Clock;

#pragma comment(lib, "lib/PPCI7443.lib")
#pragma comment(lib, "lib/phidget21.lib")

/*!
 * \brief
 * PPCI7443パルサーボードとIAIシリアル通信プログラムを
 * 上位で抽象化するクラス（シングルトン）

 * 全てのアクチュエータはこのクラスを通じて操作する．
 * 
 * \see
 * actuator.hpp | mlinear.hpp
 */
class Controller : public QThread
{
	friend class HardTimer;

private:

	Controller(Controller*){}

	Controller();

	/*!
	 * \brief
	 * 制御下の各軸の制御更新を行う
	 * 
	 */
	void Update();

	//カード枚数
	I16 totalCard;

	//コントローラのカレントステータス
	I16 retCode;

	//各軸が運転制御中であるかどうか
	bool inOperation;

	//instance
	static Controller* instance;

	//コントローラーの軸更新間隔[ms]
	static double cycle;

	//ダミー軸
	static const unsigned int dummyAxis;

	//クロック
	Clock *clock;

	//ストリングレコーダー
	StringRecorder *recorder;

	//アドバンストサーボハンドル
	CPhidgetAdvancedServoHandle servoHandle;

	//アドバンストサーボコントローラ情報表示
	void display_properties(CPhidgetAdvancedServoHandle phid);

protected:
	//制御タイマー
	void run();

public:

	/*!
	 * \brief
	 * シングルトンのインスタンスへのアクセサ
	 * 
	 * \returns
	 * Controller インスタンス
	 * 
	 * \throws <const char*>
	 * パルサーが見つからないためリニア軸を初期化できません，リニア軸動作設定ファイルが見つかりません.
	 *
	 * パルサーが物理的に存在しない，7443.ini が実行ファイルと同じディレクトリに存在しない場合に例外．
	 * 
	 */
	static Controller* GetInstance()
	{
		if(!instance){
			std::cout << "コントローラーを起動します．" << std::endl; 
			instance = new Controller();
		}
		return instance;
	}

	static double GetCycle(){ return cycle; }

	Clock* GetClock(){ return clock; }

	StringRecorder* GetStringRecorder() { return recorder; }

	//初期化
	void Init();

	//ホームポジションへ戻し
	void SetHome();

	/*!
	 * \brief
	 * コントローラーを動作開始させる
	 * 
	 * \remarks
	 * 現在実装はタイマーを駆動して，各軸のステータスを更新するスレッドをスタートさせる
	 * 
	 * \see
	 * Update() | run();
	 */
	void Start()
	{
		std::cout << "コントローラーを開始しました" << std::endl;
		inOperation = true;
		start();
	}

	/*!
	 * \brief
	 * コントローラーを動作終了させる．
	 * 
	 * \remarks
	 * 現在実装はタイマースレッドの無限ループを終了させる．
     *
	 * 同時に，非監視状態で各軸が動作しているのは好ましくないので，各軸の動作を停止させる．
	 *
	 * \see
	 * run()
	 * 
	 */
	void Stop()
	{
		std::cout << "コントローラーを停止しました" << std::endl;
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Stop();
		}		
		inOperation = false;
	}

	/*!
	 * \brief
	 * コントローラーの終了処理．各軸の終了を行う．
	 * 
	 */
	void Close()
	{
		std::cout << "コントローラーを終了しました．" << std::endl;

		//// 各常駐スレッド終了
		
		//メイン制御ループ終了
		inOperation = false;

		//ストリングレコーダー終了
		recorder->flagRecording = false;

		//                                          //
		// その他のスレッドがあればここで終了させる //
		//                                          //

		//最大残留時間分待つ
		Sleep(100);

		//軸終了
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Close();
			delete axis.at(i);
			axis[i] = 0;
		}
		_7443_close();

		//サーボモーター終了
		for(int i=0;i<servo.size();i++){
			servo.at(i)->Close();
			delete servo.at(i);
			servo[i] = 0;
		}

		//IAIアクチュエータ終了
		for(int i=0;i<iaic.size();i++){
			iaic.at(i)->Close();
			delete iaic.at(i);
			iaic[i] = 0;
		}

		//シリアル通信クローズ
		scc->Close();

		//Phidgetアドバンストサーボコントローラ クローズ
		CPhidget_close((CPhidgetHandle)servoHandle);
		CPhidget_delete((CPhidgetHandle)servoHandle);
	}

	//各軸が運転中であるかどうかを返す
	bool isInOperation(){ return inOperation; };

	//制御下にあるアクチュエータ
	QVector<Actuator*> axis;

	//制御下にあるサーボモータアクチュエータ
	QVector<Servo*> servo;

	//制御下にあるIAIアクチュエータ
	QVector<IAICtrler*> iaic;

	//シリアル通信制御クラス（IAICtrlerから委譲）
	StellarSerialCom *scc;

};

/*!
 * \brief
 * ハードウェアタイマー
 * 
 * \remarks
 * ダミー軸番号に注意してください！接続されるリニアモーターの数を変更した場合，ダミー軸番号を変更しないと
 * 適切に動作しません！
 * 
 */
class HardTimer
{
public:

	//ハードウェアクロックによるウェイト（ミリ秒）
	static void Wait(unsigned long time_ms)
	{
		int ret = _7443_delay_time(Controller::dummyAxis, (unsigned long)time_ms);
		if(ret){
			//ソフトウェアタイマーで代替（少なくともSleep()は使わないように…）
			Sleep((DWORD)time_ms);
			std::cout << "ハードウェアタイマーが利用できませんでした（エラーコード:"<< ret << "）" << std::endl;
			std::cout << "ソフトウェアタイマーで代用するためタイマー精度が低下している可能性があります" << std::endl;
		}
	}

	//ソフトウェアクロックによるウェイト（ミリ秒）；実装をここで入れ替えることができるようにこの関数を使ってください！
	static void Wait2(unsigned long time_ms)
	{
		//なんとなくで良いとき
		Sleep(time_ms);
	}
};

#endif