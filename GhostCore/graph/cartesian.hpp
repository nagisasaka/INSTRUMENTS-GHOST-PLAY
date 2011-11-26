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

#ifndef __CARTESIAN_HPP__
#define __CARTESIAN_HPP__

#include <QPoint>

/*!
 * \brief
 * ÉOÉâÉtÇÃç¿ïWån
 * 
 * \see
 * bezierWidget.hpp
 */
class Cartesian
{
public:
	Cartesian(QPointF _origin, double _x_max_val, double _y_max_val) : origin(_origin),
		x_max_val(_x_max_val),
		y_max_val(_y_max_val)
	{
		Calc();
	}
	void SetOrigin(QPointF _origin){
		origin = _origin;
		Calc();
	}
	void Scale(double scale)
	{
		y_max_val *= scale;
		x_max_val *= scale;
		Calc();
	}
	void ScaleX(double scale){
		x_max_val *= scale;
		Calc();
	}
	void ScaleY(double scale){
		y_max_val *= scale;
		Calc();
	}
	QPointF GetOrigin(){ return origin; }
	QPointF GetYMax(){ return y_max; }
	QPointF GetYMin(){ return y_min; }
	QPointF GetXMax(){ return x_max; }
	double GetYMaxValue(){ return y_max_val; }
	double GetXMaxValue(){ return x_max_val; }
private:
	void Calc()
	{	
		x_max  = origin + QPointF(x_max_val, 0 );
		y_max  = origin + QPointF(0  ,y_max_val);
		y_min  = origin - QPointF(0  ,y_max_val);
	}
	QPointF mirror(const QPointF &center, const QPointF &pos)
	{
		QPointF dif = pos - center;
		return center - dif;		
	}
	QPointF origin;
	QPointF y_max;
	QPointF y_min;
	QPointF x_max;
	double y_max_val;
	double x_max_val;
};

#endif