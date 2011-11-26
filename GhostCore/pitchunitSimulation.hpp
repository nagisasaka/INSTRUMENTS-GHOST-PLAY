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

#ifndef __PITCHUNIT_SIMULATION_HPP__
#define __PTTCHUNIT_SIMULATION_HPP__


#include <QWidget>

class GhostCore;
class PitchUnit;
class BaseGeometry;

class PitchunitSimulation : public QWidget
{
	Q_OBJECT
	
	friend class PlaySimulationTab;

public slots:

	void SetDAmp(double damp){ param.damp = damp; }

	void SetTateShift(double tate_shift){ param.tate_shift = tate_shift; }

	void SetYokoShift(double yoko_shift){ param.yoko_shift = yoko_shift; }

public:

	PitchunitSimulation(GhostCore *parent);

	~PitchunitSimulation();

	void Update(){ update(); } 

protected:

	void paintEvent(QPaintEvent *event);

private:

	BaseGeometry *geometry;

	typedef struct ScreenParamStruct{
		double damp;
		double tate_shift;
		double yoko_shift;
	}ScreenParam;

	ScreenParam param;

	GhostCore *parent;

	QVector<double> axis0;

	QVector<double> axis1;

	QVector<int> stringSelectSequence0;

	QVector<int> stringSelectSequence1;

	QVector<int> pushpullSequence0;

	QVector<int> pushpullSequence1;

	int animationCounter;

	bool animationStart;

	bool prepared;
};


#endif