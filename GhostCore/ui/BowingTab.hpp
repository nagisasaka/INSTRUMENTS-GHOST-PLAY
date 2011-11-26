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

#ifndef __BOWING_TAB_HPP__
#define __BOWING_TAB_HPP__

#include <QObject>

class Bowing;
class BowingSimulation;
class GhostCore;

class BowingTab : public QObject
{
	Q_OBJECT
public:

	BowingTab(GhostCore *parent);

	~BowingTab();

	void Update();

private slots:

	void Start();

private:

	GhostCore *parent;

	Bowing *bowing;

	BowingSimulation *bowingSimulation;

	bool simulationFlag;
};

#endif