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

#ifndef __BASE_GEOMETRY_HPP__
#define __BASE_GEOMETRY_HPP__

#include "vecmath.hpp"
#include "calibration.hpp"
#include "../bowingSimulation.hpp"

#include <QPoint>

class BaseGeometry
{
	friend class BowingSimulation;
	friend class PitchunitSimulation;

public:

	//各弦位置での左右弦に対するマージン
	typedef struct StringMarginStruct{
		double leftMargin;
		double rightMargin;
	}StringMargin;

	BaseGeometry();

	~BaseGeometry();

	void Init();

	//共通接線を求める
	void commonTangent(double deltaL0, double deltaL1, double &pos0, double &pos1, int nb_string);

	//残余半径を求める
	double residualRadius(double pos0, double pos1, int nb_string);

	//残余半径を求める
	double residualRadius2(double pos0, double pos1, int nb_string);

	//ストリングマージン構造体
	StringMargin margin[7];

	//QPoint型からPoint型への変換
	static Point QPointToPoint(QPoint point){ return Point(point.x(),point.y()); }

	//Point型からQPoint型への変換
	static QPoint PointToQPoint(Point point){ return QPoint(point.real(), point.imag()); }

	//【OBSOLETE】指定弦に対してΔLを与える2点P0,P1のY座標を返す
	void deltaL(double deltaL, double &pos0, double &pos1, int nb_string);

	//【OBSOLETE】自弦位置か両弦位置かを判定する（自弦位置0, 両弦位置1）
	int positionType(double pos0, double pos1, int nb_string);

private:

	//キャリブレーションクラス
	Calibration *calibration;

	//弦平面座標を求める
	Point CalcStringPosition(int nb_string);

	//初期半径を求める
	double initialRadius(double pos0, double pos1, int nb_string);

	//初期半径を求める
	double initialRadius2(double pos0, double pos1, int nb_string);

	//回転
	Point rot(Point p, double theta);

	//%単位をmm単位に変換する
	double ConvertPercentToMM(double percent, int nb_axis);

	//mm単位を%単位に変換する
	double ConvertMMToPercent(double mm, int nb_axis);

	//各弦の平面座標（mm単位）
	Point origin[4];

	//各弦位置のベースライン直線の単位法線ベクトル（全自弦位置＋全両弦位置で7本）；方向は，ベースラインから弦方向であることに注意
	Point unitnormal[7];

	//// 機構パラメータ（実測値）

	static const double c;

	static const double L;
};

#endif