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

#ifndef _ACTUATOR_HPP_
#define _ACTUATOR_HPP_

#include <QObject>
#include <QVector>

class Controller;

typedef struct ActuatorStatusStruct
{
	//動作パラメータの入力状態
	int Parameter;
	//動作状態
	int Movement;
	//位置
	double Position;
	//位置（生データ；デバッグ用）
	int PositionRaw;
	//エラー状態
	int Error;

} ActuatorStatus;

class Actuator : public QObject
{
	Q_OBJECT

	friend class Controller;

public:

	//動作パラメータ入力状態
	enum { 
		   WAITING,         //動作パラメータ受付待ち
		   PREPARED         //動作パラメータ受付完了，移動準備計算完了
	};

	//動作状態
	enum {
		   STOP,            //停止中
		   ACCL,			//加速中
		   RUNNING,			//等速運動中
		   DECL,			//減速中
		   ADJUSTING	    //静定動作中	
	};

	static char* ParamString(int p)
	{
		if(p == Actuator::STOP) return "停止中";
		else if(p == Actuator::ACCL) return "加速中";
		else if(p == Actuator::DECL) return "減速中";
		else if(p == Actuator::RUNNING) return "等速運動中";
		else if(p == Actuator::ADJUSTING) return "静定動作中";
		else return "未定義";
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// 制御フロー
	//
	/////////////////////////////////////////////////////////////////////////

	// オブジェクトのアップデート
	virtual void Update() = 0;

	// 初期化（デフォルト実装）
	void Init()
	{
		//デフォルト実装では可動子をホームポジションへ移動させる
		SetHome();
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// 移動制御
	//
	/////////////////////////////////////////////////////////////////////////	

	// 減速停止
	virtual void Stop() = 0;

	// 緊急停止
	virtual void EmergencyStop() = 0;

	// ハードウェア終了
	virtual void Close() = 0;

	// 指定条件での移動トランザクション開始命令
	virtual void Commit() = 0;

	/////////////////////////////////////////////////////////////////////////
	//
	// 移動条件設定
	//
	/////////////////////////////////////////////////////////////////////////

	//モード設定，モード番号他実際のアクチュエータに依存する．
	virtual void SetMode(int mode) = 0;

	// 移動先座標指定（0-100）
	virtual void SetPosition(double targetPos) = 0;

	// 移動先座標指定（0-100）
	virtual void SetPositionVector(QVector<double>& targetPos) = 0;

	// 移動時間設定（ミリ秒）
	virtual void SetDuration(double time_ms) = 0;

	// 移動時間設定（加速・減速時間割合）
	virtual void SetDuration(double time, double accl_ratio, double decl_ratio) = 0;

	// 移動速度設定
	//
	// ... v-t 系も必要だったら実装せよ ... 複雑になるだけで無用の長物か？
	//
	
	// 移動条件確定と準備計算
	virtual void Prepare() = 0;

	/////////////////////////////////////////////////////////////////////////
	//
	// アクセサ
	//
	/////////////////////////////////////////////////////////////////////////

	// UIDaa
	int GetUID(){ return uid; }

	// 動作ステータス（デフォルト実装）
	ActuatorStatus GetStatus() { return status; }

protected:
	Actuator(int _uid) : uid(_uid) {}
	Actuator(int nb_axis, int maxpulse, int _uid);

	virtual ~Actuator(){}

	// ホームポジションへ移動． Init() で呼び出される．外から直接呼ばないことにした．
	virtual void SetHome() = 0;

	//アクチュエータID
	int uid;

	//アクチュエータステータス
	ActuatorStatus status;	
};

#endif