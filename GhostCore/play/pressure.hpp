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

#ifndef __PRESSURE_HPP__
#define __PRESSURE_HPP__

#include <QVector>

class Calibration;
class BaseGeometry;

/*!
 * \brief
 * ボーイング時の弓圧管理クラス
 * 
 * 軸0と軸1の協調動作により、弓圧を決定することができるが、このクラスでは、弓圧を0%～100%までに正規化し
 * 軸0と軸1の直接管理を隠蔽する。
 * 
 */
class Pressure
{
	friend class Base;

private:

	Pressure();

	~Pressure();

	//開始位置・終了位置を指定する
	void SetStringPair(int _startString, int _endString);

	//圧力付加対象弦を設定する（演奏弦を指定する）
	void SetPlayerString(int _playerString);

	//圧力ベクタ（圧力系列）をセットする（圧力ベクタは[0,100]正規化，絶対値指定）
	void SetPressureVector(QVector<double> v);

	//パーセントをmm単位に変換する
	double ConvertToMM(double percent);

	//圧力付加用の軸0,1の制御ベクトルを計算する
	void Prepare(QVector<double> &axis0, QVector<double> &axis1);

	//// 単純演奏

	//右へ圧力付加
	void ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//両弦へ圧力付加
	void ToBothSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//左へ圧力付加
	void ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//// 移弦動作込演奏

	//右へ移弦しながら圧力付加
	void ToRightSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//左へ移弦しながら圧力付加
	void ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//単弦への平行移動条件下での圧力付加
	void SinglePressure(QVector<double> &axis0, QVector<double> &axis1, int nb_string);

	//圧力付加対象弦
	int playerString;

	//開始位置
	int startString;

	//終了位置
	int endString;

	//圧力系列
	QVector<double> pressureVector;

	//キャリブレーションクラス
	Calibration *calibration;

	//ベースジオメトリ
	BaseGeometry *geometry;

	//最大押し込み幅
	static const double MaxMM;
};

#endif