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

#ifndef __BOWING_SIMULATION_HPP__
#define __BOWING_SIMULATION_HPP__

#include <QWidget>
#include <QVector>

class BowingTab;
class Bowing;
class GhostCore;
class playSimulationTab;

class BowingSimulation : public QWidget
{
	Q_OBJECT

	friend class BowingTab;
	friend class PlaySimulationTab;

public slots:

	void SetDAmp(double damp){ param.damp = damp; }

	void SetTateShift(double tate_shift){ param.tate_shift = tate_shift; }

	void SetYokoShift(double yoko_shift){ param.yoko_shift = yoko_shift; }

public:

	BowingSimulation(GhostCore *parent);

	~BowingSimulation();

	void SetInitialMoveFlag(bool t){ initialMove = t; }

	bool GetInitialMoveFlag(){ return initialMove; }

	void Prepare();

	void Start();

	void SetPressureVector(QVector<double> pressureVector);

	void SetPositionVector(QVector<double> positionVector);

	void SetStringPair(int _startString, int _endString);

	void SetPlayerString(int _playerString);

	void Update(){ update(); }

	bool GetDrawBowElasticDeformation(){ return drawBowElasticDeformation; }

	void SetDrawBowElasticDeformation(bool t){ drawBowElasticDeformation = t; }

protected:

	void paintEvent(QPaintEvent *event);

private:

	typedef struct ScreenParamStruct{
		double damp;
		double tate_shift;
		double yoko_shift;
	}ScreenParam;

	ScreenParam param;

	Bowing *bowing;

	int startString;

	int endString;

	int playerString;

	bool initialMove;

	QVector<double> pressureVector;

	QVector<double> positionVector;

	GhostCore *parent;

	QVector<double> axis0;

	QVector<double> axis1;

	QVector<double> axis2;

	int animationCounter;

	bool animationStart;

	bool prepared;

	bool drawBowElasticDeformation;
};

#endif