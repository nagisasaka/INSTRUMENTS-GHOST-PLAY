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

#ifndef __BOWING_HPP__
#define __BOWING_HPP__

#include <QThread>
#include <QVector>

class Base;
class BowingTab;
class BowingSimulation;

class Bowing : public QThread
{
	friend class PlayTab;
	friend class BowingTab;
	friend class BowingSimulation;

public:

	Bowing();

	Bowing(QStringList initData);

	~Bowing();

	QString toString();

	void SetInitialMoveFlag(bool t){ initialMove = t; }

	bool GetInitialMoveFlag(){ return initialMove; }

	int Prepare();

	void Start();

	void SetPressureVector(QVector<double> pressureVector);

	void SetPositionVector(QVector<double> positionVector);

	QVector<double> GetPressureVector(){ return pressureVector; }

	QVector<double> GetPositionVector(){ return positionVector; }

	void SetStringPair(int _startString, int _endString){ startString = _startString; endString = _endString; }

	void SetPlayerString(int _playerString){ playerString = _playerString; }

	int GetStartString(){ return startString; }

	int GetEndString(){ return endString; }

	int GetPlayerString(){ return playerString; }

	QVector<double> GetPositionVector(int nb_axis);

	bool isPrepared(){ return prepared; }

	int GetPlayTime(){ return playTime; }

protected:

	void run();

private:

	//ボウイングクラスの中でベースを持つ
	Base *base;

	int startString;

	int endString;

	int playerString;

	bool initialMove;

	//圧力ベクタ
	QVector<double> pressureVector;

	//弓速ベクタ（これらの制御ベクタは各弦位置と対応して初めて意味を持つ）
	QVector<double> positionVector;

	//プリペア済みフラグ
	bool prepared;

	//演奏時間
	int playTime;
};

#endif