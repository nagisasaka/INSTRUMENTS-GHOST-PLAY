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

#ifndef _MLINEAR_HPP_
#define _MLINEAR_HPP_

#include "../actuator.hpp"
#include "ppci7443.h"
#include "7443err.h"
#include "../../error.hpp"

#include <QObject>
#include <QVector>

#include <iostream>

class Controller;

/*!
 * \brief
 * 磁気リニアアクチュエーターの制御クラス
 * 
 * \see
 * actuator.hpp|7443err.h|type_def.h
 */
class MLinear : public Actuator
{
	Q_OBJECT

	friend class Controller;

signals:
	/*!
	 * \brief
	 * 指定された動作が終了した時に送出されるシグナル．キャリブレーション時の荒い運転等
	 * 制御にシビアなタイミングが求められない場合にのみ利用する．基本的には，参考用とし
	 * てのみ用いる，参考用シグナル．
	 *
	 * \param int
	 * 軸UID
	 *
	 */
	void moveComplete(int);

public:
	
	/*!
	 * \brief
	 * 制御モード
	 *
	 * JOGモードはデフォルトで，あらゆる運転動作に適切に滑らかに加減速が設定される（等躍度運動；S字加速動作）
	 * CONTモードはトリプルバッファリングの連続動作モードで，微小区間での等速運動の連続
	 */
	enum
	{
		JOG ,	   // ジョグ
		CONTINUOUS // 連続動作
	};

	/*!
	 * \brief
	 * 制御モードのセッター
	 * 
	 * \param pos
	 * 制御モード
	 *
	 */
	void SetMode(int _mode);

	/*!
	 * \brief
	 * アップデート. 一回の処理は必ず一サイクル内に収まる必要があることに注意．
	 * 
	 */
	void Update();

	/**
	 * \brief
	 * ステータスをアップデートする（軸ステータスをパルサーに毎回問い合わせる）
	 *
	 * (1)アンプで軸動作させる場合は，パルサーは軸動作を監視できない．
	 * (2)パルサーで軸動作させる場合は，アンプも軸動作がわかる．
	 * → この2つは統一的に扱って，この関数内で隠蔽することにする
	 *
	 */
	void UpdateStatus();
	
	/**
	 * \brief
	 * 位置をアップデートする（コマンドポジションをパルサーに毎回問い合わせる）
	 *
	 */
	void UpdatePosition();

	/*!
	 * \brief
	 * 軸の最終準備，Commit() する前に各条件の最終計算他を行う．
	 * 
	 * \throws <exception class>
	 * 移動に必要な情報が足りません．
	 * 
	 * Commit() できない状態で Prepare() を呼ぶと発生する．プログラムミス．
	 * 
	 */
	void Prepare();

	/*!
	 * \brief
	 * 軸同期移動命令
	 * 
	 * \throws <const char*>
	 * 移動パラメータ未入力，移動失敗（移動中移動命令は受理できません）
	 * 
	 * 軸同期移動命令は，移動中移動命令を受理しない．
	 *
	 */
	void Commit();

	/*!
	 * \brief
	 * 連続位置指定データをセットする
	 * 
	 * \param pos
	 * 連続位置指定データ[0,100]
	 * 
	 * \remarks
	 * CONT_POSITIONモードで動作することに留意
	 * 
	 */
	void SetPositionVector(QVector<double>& pos);

	/*!
	 * \brief
	 * 連続速度指定データをセットする
	 * 
	 * \param vel
	 * 連続速度指定データ
	 * 
	 * \remarks
	 * CONT_VELOCITYモードで動作することに留意
	 * 
	 */
	void SetVelocityVector(QVector<double>& vel);

	/*!
	 * \brief
	 * 移動先位置を[0,100]で指定する．現在位置はコマンドポジションを所得することで得る．
	 * 
	 * \param targetPos
	 * 移動先位置．[0,100]で指定．0.1でも平均100パルス以上残っているので，桁落ちに注意せよ．
	 * 
	 * \throws <const char*>
	 * コマンドポジションを取得できません
	 * 
	 * 通常は発生しないと想定される．
	 * 
	 * \remarks
	 * targetPos は [0,100] の100分率形式で入力させるが，0.1%でも平均100パルス以上残っているため，上位の演算での桁落ち
	 * に注意すること．
	 * 
	 */
	void SetPosition(double targetPos);

	/*!
	 * 移動先位置を[0,100]で指定する．現在位置は第二引数で指定する
	 * 
	 * \param targetPos
	 * 移動先位置．[0,100]で指定．0.1でも平均100パルス以上残っているので，桁落ちに注意せよ．
	 * 
	 * \param fromPos
	 * 移動前位置（現在位置）．[0,100]で指定．
	 * 
	 * \remarks
	 * 連続動作モードをリアルタイム処理する場合に利用することができる．
	 * 連続動作モードを事前計算する場合は利用する必要はない．
	 * 
	 */
	void SetPosition(double targetPos, double fromPos);

	/*!
	 * \brief
	 * 移動時間指定（ユーティリティ関数）
	 * 
	 * \param time_ms
	 * 移動時間（ミリ秒）
	 */
	void SetDuration(double time_ms);

	/*!
	 * \brief
	 * 移動時間，加減速時間（割合）指定
	 * 
	 * \param time_ms
	 * 合計移動時間（ミリ秒）
	 * 
	 * \param accl_ratio
	 * 合計移動時間中の加速時間の割合[0,1]
	 * 
	 * \param decl_ratio
	 * 合計移動時間中の減速時間の割合[0,1]
	 * 
	 * \throws <const char*>
	 * 加速・減速時間割合の合計が1を超えています
	 * 
	 */
	void SetDuration(double time_ms, double accl_ratio, double decl_ratio);

	/*!
	 * \brief
	 * 制御パラメーター初期化
	 * 
	 */
	void ParameterInit();

	/*!
	 * \brief
	 * 減速停止
	 * 
	 * \remarks
	 * 減速停止に失敗した場合は自動的に緊急停止が試行される．
	 * 
	 * \see
	 * EmergencyStop()
	 *
	 */
	void Stop()
	{
		//１秒間で減速停止
		retCode = _7443_sd_stop(nb_axis,1);
		if(!retCode){
			std::cout << "軸 #" << uid << " 減速停止．" << std::endl;
		}else{
			std::cout << "軸 #" << uid << " 減速停止失敗（" << retCode << "），緊急停止実行．" << std::endl;
			EmergencyStop();
		}
	}

	/*!
	 * \brief
	 * 緊急停止
	 *
	 * \throws <const char*>
	 * 緊急停止正常実行
	 *
	 * \remarks
	 * 緊急停止に失敗した場合は，コントローラーを強制終了し，プログラム自体をexit(1)してしまう．
	 * 緊急停止に成功した場合も，例外を送出する．この段階では，プログラムを継続動作させることは
	 * 想定しておらず，可能な限り速やかにプログラムを終了することを目的とする．
	 * 
	 */
	void EmergencyStop()
	{		
		retCode = _7443_emg_stop(nb_axis);
		if(!retCode){
			Error::Critical(0, QObject::tr("軸 #%1 緊急停止．").arg(uid));
		}else{
			std::cout << "軸 #" << uid << " 緊急停止失敗（"<< retCode << "）．コントローラを強制シャットダウンします．" << std::endl;
			Close();
			_7443_close();
			//throw "緊急停止失敗，コントローラ強制シャットダウン実行．";
			Error::Critical(0, QObject::tr("軸 #%1 緊急停止失敗，コントローラ強制シャットダウン実行．").arg(uid));
		}
	}


	/*!
	 * \brief
	 * ホームポジションへ移動する
	 * 
	 * \throws <const char*>
	 * コマンドポジションクリア失敗，SVON信号立上失敗，SVON信号立下失敗．
	 * 
	 * 通常は発生しないと想定される．
	 *
	 * \remarks
	 * 原点復帰はパルサーの機能は利用せず，SVON信号の立下りで，アンプ側で動作する．
	 * あらゆる原点復帰の設定はアンプ側で為されることに注意．
	 * 
	 */
	void SetHome();

	// ハードウェア終了
	void Close()
	{
		std::cout << "軸 #" << uid << " 終了" << std::endl;

		//パラメータ初期化
		ParameterInit();

		//軸ソフトリミット解除
		retCode = _7443_disable_soft_limit(nb_axis);
	}

	//// アクセサ
	
	int GetMaxPulse(){ return max_pulse; }
	int GetPulseByMeter() { return PulseByMeter; }

	QVector<double> GetPositionVector(){ return positionVector;	}
	
protected:

	MLinear(int _nb_axis, int _max_pulse, int _uid) : nb_axis(_nb_axis), max_pulse(_max_pulse), Actuator(_uid)
	{
		//機械パラメータ設定
		PulseByMeter = 100000; // 10^5 pulse = 1 meter 設計ノート p4 （例）27000パルス->270mm 10^2 パルス 1mm(10^-3m) : 100パルス1mm 1パルス 100分の1mm（10^-5m）=10マイクロメートル  

		//ステータスクリア
		status.Parameter = Actuator::WAITING;
		status.Movement = Actuator::STOP;
		status.Error = 0;

		//パラメータ初期化
		ParameterInit();

		//7443上のソフトリミット設定（SHDA02のソフトリミットと等しい．ソフトリミットの二重化）		
		retCode = _7443_set_soft_limit(nb_axis, max_pulse, 0);
		retCode = _7443_enable_soft_limit(nb_axis, 2); // 1: 緊急停止, 2: 減速停止（採用） p168

		//モード設定
		mode = JOG;

		//その他初期化
		size = 0;
		curr = 0;
		flagContinuousOperationStart = false;
		flagJogOperationStart = false;
		flagUpdating = false;
	}

	~MLinear()
	{
		Close();
	}

	/*!
	 * \brief
	 * 連続動作を設定する．連続動作は，位置指定・速度指定の別があるので
	 * ここではモードを確定させることは出来ない．
	 * 
	 * \param t
	 * true : 連続動作を設定
	 * false: 連続動作を解除
	 * 
	 */
	void SetDiscreteMove(bool t);

	/*!
	 * \brief
	 * 連続制御データ（D-V）
	 *
	 * 速度はpps単位で必ず正の値，距離は相対パルス数[-max_pulse, +max_pulse]
	 * 
	 * \remarks
	 * このデータは，位置指定が絶対位置になっていないため，人間が見ても理解困難なため，この
	 * クラス内で，絶対指定データから変換して，パルサーハードウェアに高速に送り込むための
	 * 事前計算用として用いられるもの．
	 */
	typedef struct ContinuousControlDataStruct{
		int velocity; // pps;   [0, 最大速度]
		int distance; // pulse; [-max_pulse, +max_pulse]
	}ContinuousControlData;

	//7443軸番号
	int nb_axis;

	//最大パルス数
	int max_pulse;

	//パルス解像度
	int PulseByMeter;

	//軸のカレントステータス
	I16 retCode;

	//// ジョグ動作用
	//等躍度加速動作パラメータ；ユーザーガイド4.1.4　設計ノート2ページ
	int Dist;   //インクリメンタルモードによる移動距離
	int StrVel; //移動開始速度
	int MaxVel; //最大速度
	double Tacc;//低速から高速への加速時間（秒）
	double Trun;//等速運動時間（秒）
	double Tdec;//高速から低速への加速時間（秒）
	int SVacc;  //加速時間の等躍度到達速度（pps）
	int SVdec;  //減速時間の等躍度到達速度（pps）

	//時間指定用のパラメータ
	int Duration; //移動に掛かる時間（ミリ秒）
	
	//それぞれデータが指定されているかどうか
	bool flagPosition;
	bool flagDuration;
	bool flagVelocity;

	//// ジョグ動作用
	bool flagJogOperationStart;


	//// 連続動作用
	//連続動作指定データ
	QVector<ContinuousControlData> continuousControlData;
	//連続動作位置
	int curr;
	//連続動作ベクトルサイズ
	int size;
	//連続動作開始フラグ
	bool flagContinuousOperationStart;
	//アップデート処理中フラグ
	bool flagUpdating;

	//// 運転動作モード（ジョグ動作か連続動作か）
	int mode;

	//// デバッグ
	bool _debug;

	QVector<double> positionVector;
};

#endif