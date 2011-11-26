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

#ifndef __IGEN_HPP__
#define __IGEN_HPP__

#include <QVector>

class BaseGeometry;
class Calibration;

/*!
 * \brief
 * 移弦管理クラスの実装．
 * 
 * 演奏中移弦はこのクラスでは管理しない．初期位置間（残余弓圧無し）の移弦のみを管理する．
 * 
 * \remarks
 * 演奏中移弦はこのクラスでは管理しないことに注意．
 * 
 */
class Igen
{
	friend class Base;

private:

	Igen();

	~Igen();

	//移弦の初期位置と終了位置を設定
	void SetStringPair(int _startString, int _endString);

	//演奏弦を指定
	void SetPlayerString(int _playerString);

	//プリペア
	void Prepare(QVector<double> &axis0, QVector<double> &axis1);

	//左へ移弦
	void ToLeft(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//左へ移弦（新実装）
	void ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int nb_string);

	//右へ移弦
	void ToRight(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//右へ移弦
	void ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int nb_string);

	//初期位置
	int startString;

	//終了位置
	int endString;

	//演奏弦
	int playerString;

	//ベースジオメトリ
	BaseGeometry *geometry;

	//キャリブレーションクラス
	Calibration *calibration;

	//移弦の実行時間（一段階あたり）[ms]
	static const int DurationTime;
};

#endif