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

#ifndef __BASE_HPP__
#define __BASE_HPP__

#include <QVector>

class Pressure;
class Igen;
class BaseGeometry;
class Calibration;

/*!
 * \brief
 * 運弓ベースクラス
 *
 * 運弓に関する一切を管轄する
 * 
 */
class Base
{
	friend class Bowing;
	friend class BowingSimulation;

public:

	Base();

	~Base();

	//演奏終了時に接しているポジションを指定する
	void SetStringPair(int start_string, int end_string);

	//演奏弦を指定する
	void SetPlayerString(int nb_string);
	
	//演奏弦を取得する
	int GetPlayerString(){ return PlayerString; }

	//弓圧ベクターを指定する(10msec毎)
	void SetPressureVector(QVector<double> p);

	//弓圧ベクターを取得する(セット時に対してサイズは変わっている可能性がある)
	QVector<double> GetPressureVector();

	//プリペア
	int Prepare();

	//コミット
	void Commit();

	//初期移動
	void InitialMove(int nb_string);

private:

	//弓圧管理クラス
	Pressure *pressure;

	//移弦管理クラス（自動）
	Igen *igen;

	//ベースジオメトリ
	BaseGeometry *geometry;

	//キャリブレーション
	Calibration *calibration;

	//演奏開始時位置
	int StartString;

	//演奏弦
	int PlayerString;

	//演奏終了時接触弦位置
	int EndString;
	
	//アクチュエータ制御ベクタ
	//軸0
	QVector<double> axis0;
	//軸1
	QVector<double> axis1;
};

#endif