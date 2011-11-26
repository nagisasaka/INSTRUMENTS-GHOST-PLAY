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

#ifndef __CALIBRATION_HPP__
#define __CALIBRATION_HPP__

#include <QObject>
#include <QSettings>
#include <QList>
#include <QThread>

#include <iostream>

class CalibrationTab;

/*!
 * \brief
 * キャリブレーションクラス
 * 軸位置（軸0,軸1）キャリブレーションと，音階（弦テンション）キャリブレーションを実行
 * 
 * \see
 * calibration.cpp | vecmath.hpp
 */
class Calibration : public QThread
{
	Q_OBJECT

	friend class CalibrationTab;

signals:
	void UpdateCalibratedPosition();
	void CalibrationEnd();

public slots:

	/*!
	 * \brief
	 * 各軸の運転終了シグナルが接続されるスロット
	 *
	 * \param uid
	 * 軸UID
	 *
	 */
	void Repeat(int uid);

public:

	/*!
	 * \brief
	 * 対象弦（７個）
	 * 
	 */
	enum{
		STRING_1 ,
		STRING_12,
		STRING_2 ,
		STRING_23,
		STRING_3 ,
		STRING_34,
		STRING_4
	};

	/*!
	 * \brief
	 * 軸0,1,2の関係をまとめて記憶する構造体
	 * 
	 * \remarks
	 * 軸0,1のポジションによっては軸2を[0,1]で使い切ることはできず，全弓でのボーイングはできない，
	 * つまり「機構上の全弓」は，[0,1]より，かなり制限されることに注意すること．
	 * 
	 */
	typedef struct PositionsStruct
	{
		//軸2始点（ボーイング開始点）での軸0,1,2の位置
		QList<double> start;
		//軸2終点（ボーイング終了点）での軸0,1,2の位置
		QList<double> end;
	}Positions;//弦1,2,3,4（4個） もしくは，弦1,1-2,2,2-3,3,3-4,4（7個）

	/*!
	 * \brief
	 * キャリブレーションクラスのコンストラクタ．セーブしたキャリブレーション関連情報を全てロードする．
	 * 
	 */
	Calibration();

	~Calibration(){}

	/*!
	 * \brief
	 * キャリブレーション初期設定をロードする．キャリブレーションは毎回実行されることが好ましい．
	 * 
	 * \remarks
	 * INIファイルの内部構造が無駄．Position構造体をmetatypeに記載してQVariantで取り扱えるようにするなど見通しを
	 * 良くしたい．
	 * 
	 * \see
	 * Save() | Calibration()
	 */
	void Load();

	/*!
	 * \brief
	 * キャリブレーション初期設定のセーブ．デストラクタで自動的に呼ばれる．
	 * 
	 * \remarks
	 * INIファイルの内部構造が無駄．Position構造体をmetatypeに記載してQVariantで取り扱えるようにするなど見通しを
	 * 良くしたい．
	 * 
	 * \see
	 * Load() | ~Calibration()
	 */
	void Save();
	
	/*!
	 * \brief
	 * 位置キャリブレーション
	 * 
	 * \param nb_string
	 * 弦番号[1,7]. 0スタートではないことに注意．キャリブレーション実行対象ファイル．
	 *
	 * \throws <exception class>
	 * Description of criteria for throwing this exception.
	 *  
	 * \remarks
	 * 軸2（運弓軸）については，キャリブレーションできないので初期値をそのまま利用することになる．
	 * 初期値で十分注意して目視による値を与えなければならない．
	 * これは機構設計上の改善項目であり，何らかのエンドセンサーの追加が望ましいと考えられる．
	 * 
	 */
	void Start(int nb_string);

	//手動停止
	void Stop();

	/*!
	 * \brief
	 * 初期位置をセットする
	 * 
	 * \param nb_string
	 * 弦番号[1,4]．0スタートではないことに注意．初期位置はUIと結合されている．
	 * 
	 * \remarks
	 * 弦番号[1,4]で指定．0スタートではないことに注意．
	 * 
	 */
	void SetInitialPosition(int nb_string, Calibration::Positions position)
	{
		initialPositions[nb_string-1] = position;
	}

	/*!
	 * \brief
	 * キャリブレーション済位置をセットする
	 * 
	 * \param nb_string
	 * 弦番号[1,7]．0スタートではないことに注意．キャリブレーション済位置は基本的に自動キャリブレーション
	 * によって，calibrationクラス内で設定されるが，キャリブレーション結果に対して微調整を加える場合は，
	 * UIと結合されていたほうが利便性があるか．
	 * 
	 * \remarks
	 * 弦番号[1,7]は，それぞれ1,1-2,2,2-3,3,3-4,4に対応．
	 * 
	 */
	void SetCalibratedPosition(int nb_string, Calibration::Positions position)
	{
		positions[nb_string-1] = position;
	}

	/*!
	 * \brief
	 * 指定された弦についての初期位置を取得
	 * 
	 * \param nb_string
	 * 指定弦[1,4]
	 * 
	 * \returns
	 * 指定された弦についての初期位置（start:軸0,1,2 end:軸0,1,2）
	 * 
	 * \remarks
	 * startとendで軸0,1については同じ値であることに注意，軸2についてのみボーイング始点・終点指定．
	 *
	 */
	Calibration::Positions GetInitialPositions(int nb_string)
	{
		return initialPositions[nb_string-1];
	}

	/*!
	 * \brief
	 * 全弦についての初期位置を取得
	 * 
	 * \returns
	 * 全弦についての初期位置
	 * 
	 */
	QList<Calibration::Positions> GetInitialPositions()
	{
		return initialPositions; 
	}

	/*!
	 * \brief
	 * 指定された弦についてのキャリブレーション済位置を取得
	 * 
	 * \param nb_string
	 * 指定弦[1,7]
	 * 
	 * \returns
	 * 指定された弦についてのキャリブレーション済位置（start:軸0,1,2,end:軸0,1,2）
	 * 
	 * \remarks
	 * startとendで軸0,1についてはキャリブレーションにより調整される．軸2については初期位置指定のまま．
	 * 
	 */
	Calibration::Positions GetCalibratedPositions(int nb_string)
	{
		return positions[nb_string-1];
	}

	/*!
	 * \brief
	 * 全弦についてキャリブレーション済位置を取得
	 * 
	 * \returns
	 * 全弦についてのキャリブレーション済位置
	 * 
	 */
	QList<Calibration::Positions> GetCalibratedPositions()
	{ 
		return positions; 
	}

protected:

	void run();
	int nbString;

private:

	void SoundCheck();

	//初期位置をセーブする
	void SaveInitialPosition();

	//キャリブレーション済位置をロードする
	void SaveCalibratedPosition();

	//初期位置をロードする
	void LoadInitialPosition();

	//キャリブレーション済位置をロードする．
	void LoadCalibratedPosition();

	// キャリブレーション済位置の配列の要素数
	//
	// 0    1      2    3      4    5      6
	// 軸0, 軸0-1, 軸1, 軸1-2, 軸2, 軸2-3, 軸3
	// 0           1           2           3
	// 各キャリブレーション対象の軸0,1,2の始点と終点
	QList<Calibration::Positions> positions;
	// 同初期位置
	QList<Calibration::Positions> initialPositions;

	//キャリブレーション手動停止フラグ
	bool manualStopFlag;

	//デバッグ
	int _debug;

};

#endif