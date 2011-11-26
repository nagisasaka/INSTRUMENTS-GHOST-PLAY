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

#ifndef __RENDER_AREA2_HPP__
#define __RENDER_AREA2_HPP__

#include <QWidget>
#include <QPixmap>
#include <QPaintEvent>

class RenderArea2 : public QWidget
{
	Q_OBJECT

public:

	RenderArea2(QWidget *parent = 0);

	void Update();

	//F0を返す（ここにあるのは設計上好ましくない；stringRecorderの再解析関数が設計ミスである；stringRecorderには安定性に疑問があるので再実装が望ましい）
	double GetFZero(){ return fzero; }

protected:

	void paintEvent(QPaintEvent *event);

	double fzero;

};

#endif