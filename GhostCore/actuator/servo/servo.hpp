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

#ifndef __SERVO_HPP__
#define __SERVO_HPP__

#include "../../error.hpp"
#include "phidget21.h"

#include <QThread>

#include <iostream>

/// トリステートにせよ！


class Servo : public QThread
{
	friend class Controller;

public:

	//制御モード
	enum
	{
		JOG ,		// ジョグモード
		PREDEFINED  // 定義済み移動モード
	};

	//ポジション取得
	double GetPosition();

	//電流値取得
	double GetCurrent();

	//ホームポジションへ移動
	void SetHome();

	//レディポジションへ移動
	void SetReady();

	//コンタクトポジションへ移動
	void SetContact();

	//プリペア
	void Prepare();

	//コミット
	void Commit();

	//ポジション設定　ジョグ用
	void SetPosition(double pos);

	//エンゲージされているか
	bool isEngaged(){ return engaged; }

	//サーボモーターにエンゲージ
	void Engage(bool flag);

	//ポジションの取得
	double GetHomePosition(){ return homePosition; }
	
	//ポジションの取得
	double GetReadyPosition(){ return readyPosition; }

	//ポジションの取得
	double GetContactPosition(){ return contactPosition; }

	//モード決定
	void SetMode(int i){ mode = i; }

	//モード取得
	int GetMode(){ return mode; }

	QVector<int> GetControlVector(){ return controlVector; }

	void SetControlVector(QVector<int> _controlVector){ controlVector = _controlVector; controlVectorSize = controlVector.size(); }

protected:

	void run();

private:

	Servo(CPhidgetAdvancedServoHandle *_servo, int _index);

	~Servo();

	//初期化
	void Init(){ SetHome(); }

	//クローズ
	void Close();

	//インデックス
	int index;

	//アドバンストサーボハンドル
	CPhidgetAdvancedServoHandle *servo;

	//エンゲージされているか
	bool engaged;

	//自由位置
	double position;

	//アップデート
	void Update();

	//// トライステートポジション　////

	//ホームポジション
	double homePosition;

	//準備位置
	double readyPosition;

	//弦押下位置
	double contactPosition;

	//制御ベクトル
	QVector<int> controlVector;

	//制御ベクトルサイズ
	int controlVectorSize;

	//モード
	int mode;

	//動作開始フラグ
	bool flagStart;

	//カレント位置
	int curr;

	//プリディファインドモードでの移動先定義済位置
	int to;

	//// 以下コールバック関数 ////

	/*!
	 * \brief
	 * コールバック関数（接続時）
	 * 
	 * \param ADVSERVO
	 * アドバンストサーボハンドル
	 * 
	 * \param userptr
	 * ユーザーポインタ
	 * 
	 * \returns
	 * 無視
	 * 
	 */
	static int __stdcall AttachHandler(CPhidgetHandle ADVSERVO, void *userptr)	
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (ADVSERVO, &name);
		CPhidget_getSerialNumber(ADVSERVO, &serialNo);
		printf("%s %10d 接続されました．\n", name, serialNo);
		return 0;
	}

	/*!
	 * \brief
	 * コールバック関数（接続解除時）
	 * 
	 * \param ADVSERVO
	 * アドバンストサーボハンドル
	 * 
	 * \param userptr
	 * ユーザーポインタ
	 * 
	 * \returns
	 * 無視
	 * 
	 */
	static int __stdcall DetachHandler(CPhidgetHandle ADVSERVO, void *userptr)
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (ADVSERVO, &name);
		CPhidget_getSerialNumber(ADVSERVO, &serialNo);
		printf("%s %10d 接続が解除されました．\n", name, serialNo);
		Error::Critical(0, QObject::tr("サーボモーターの接続が解除されました．"));
		return 0;
	}

	/*!
	 * \brief
	 * コールバック関数（エラー時）
	 * 
	 * \param ADVSERVO
	 * アドバンストサーボハンドル
	 * 
	 * \param userptr
	 * ユーザーポインタ
	 * 
	 * \returns
	 * 無視
	 * 
	 */
	static int __stdcall ErrorHandler(CPhidgetHandle ADVSERVO, void *userptr, int ErrorCode, const char *Description)
	{
		printf("[ SERVO ] エラーハンドラ: %d - %s\n", ErrorCode, Description);
		return 0;
	}

	/*!
	 * \brief
	 * コールバック関数（位置変更時）
	 * 
	 * \param ADVSERVO
	 * アドバンストサーボハンドル
	 * 
	 * \param userptr
	 * ユーザーポインタ
	 * 
	 * \returns
	 * 無視
	 * 
	 */
	static int __stdcall PositionChangeHandler(CPhidgetAdvancedServoHandle ADVSERVO, void *usrptr, int Index, double Value)
	{
		printf("モータ駆動中: %d > 現在位置: %f\n", Index, Value);
		return 0;
	}
};

#endif