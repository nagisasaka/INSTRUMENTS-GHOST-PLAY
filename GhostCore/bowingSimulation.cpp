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

#include "bowingSimulation.hpp"
#include "bowing.hpp"
#include "ghostcore.hpp"
#include "actuator/controller.hpp"
#include "play/base.hpp"
#include "play/basegeometry.hpp"
#include "vecmath.hpp"

#include <iostream>
#include <QPainter>

BowingSimulation::BowingSimulation(GhostCore *_parent) : parent(_parent)
{
	//ボーイングシミュレーションクラスがボーイングクラスを持つ
	bowing = new Bowing();
	startString = 1;
	endString = 1;
	playerString = 0;
	initialMove = false;
	animationCounter = 0;
	animationStart = false;
	prepared = false;

	param.damp = 0.8;
	param.tate_shift = 1.2;
	param.yoko_shift = 2.0;

	//弓の弾性変形を描画する
	drawBowElasticDeformation = true;
}

BowingSimulation::~BowingSimulation()
{
	delete bowing;
}

void BowingSimulation::Start()
{
	animationStart = true;
}

void BowingSimulation::SetPressureVector(QVector<double> _pressureVector)
{
	pressureVector = _pressureVector;
	bowing->SetPressureVector(pressureVector);
}

void BowingSimulation::SetPositionVector(QVector<double> _positionVector)
{
	positionVector = _positionVector;
	bowing->SetPositionVector(positionVector);
}

void BowingSimulation::Prepare()
{
	bowing->Prepare();
	axis0 = bowing->base->axis0;
	axis1 = bowing->base->axis1;
	axis2 = bowing->positionVector;
	animationCounter = 0;
	prepared = true;
}

void BowingSimulation::SetStringPair(int _startString, int _endString)
{
	startString = _startString;
	endString = _endString;
	bowing->SetStringPair(_startString, _endString);
}

void BowingSimulation::SetPlayerString(int _playerString)
{
	playerString = _playerString;
	bowing->SetPlayerString(_playerString);
}


void BowingSimulation::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	//横軸
	//画面サイズの確認
	QPoint tl = QPoint(painter.viewport().left(),  painter.viewport().top());
	QPoint tr = QPoint(painter.viewport().right(), painter.viewport().top());
	QPoint br = QPoint(painter.viewport().right(), painter.viewport().bottom());
	QPoint bl = QPoint(painter.viewport().left(),  painter.viewport().bottom());

	double damp = param.damp;
	double tate_shift = param.tate_shift;
	double yoko_shift = param.yoko_shift;

	//原点を画面中央にずらす為のパディング
	int pad_tate = (br.y()-tr.y())/tate_shift;
	int pad_yoko = (br.x()-bl.x())/yoko_shift;

	double c = bowing->base->geometry->c / damp;
	double L = bowing->base->geometry->L / damp;

	double s0x = bowing->base->geometry->origin[0].real() / damp*(-1);
	double s0y = bowing->base->geometry->origin[0].imag() / damp;
	double s1x = bowing->base->geometry->origin[1].real() / damp*(-1);
	double s1y = bowing->base->geometry->origin[1].imag() / damp;
	double s2x = bowing->base->geometry->origin[2].real() / damp*(-1);
	double s2y = bowing->base->geometry->origin[2].imag() / damp;
	double s3x = bowing->base->geometry->origin[3].real() / damp*(-1);
	double s3y = bowing->base->geometry->origin[3].imag() / damp;

	QPoint s0 = QPoint(s0x+pad_yoko,s0y+pad_tate);
	QPoint s1 = QPoint(s1x+pad_yoko,s1y+pad_tate);
	QPoint s2 = QPoint(s2x+pad_yoko,s2y+pad_tate);
	QPoint s3 = QPoint(s3x+pad_yoko,s3y+pad_tate);

	int size = 6;

	//アニメーションを実行しているとき
	if(animationStart){
		animationCounter++;	
		if(animationCounter == axis0.size()){
			animationCounter = axis0.size()-1;//最後の要素に固定
			animationStart = false;
		}
		if(animationCounter % 100 == 0) std::cout << "..";
	}

	if(animationCounter >= axis0.size()){
		animationCounter = axis0.size()-1;
	}

	if(prepared){
	
		double p0p = axis0.at(animationCounter); //[0,100] %
		double p1p = axis1.at(animationCounter); //[0,100] %
		double p2p = 0;
		if(animationCounter > axis2.size() - 1){
			p2p = axis2.at(axis2.size()-1);
		}else{
			p2p = axis2.at(animationCounter);
		}

		double p0m = bowing->base->geometry->ConvertPercentToMM(p0p, 0) / damp;
		double p1m = bowing->base->geometry->ConvertPercentToMM(p1p, 1) / damp;
		double p2m = bowing->base->geometry->ConvertPercentToMM(p2p, 2) / damp;

		//P0
		QPoint p0 = QPoint(pad_yoko-c, p0m+pad_tate);
		//P1
		QPoint p1 = QPoint(pad_yoko+c, p1m+pad_tate);

		//法線計算
		Point p0p1 =  BaseGeometry::QPointToPoint(p1)- BaseGeometry::QPointToPoint(p0);
		Point n0 = Point(0,0);
		Point n1 = Point(0,0);
		vecmath::unitnormal(p0p1, &n0, &n1);
		Point unitnormal = Point(0,0);
		if(n0.imag() > n1.imag()){
			unitnormal = n1;
		}else{
			unitnormal = n0;
		}
		Point normal = unitnormal * L;

		//弓上の二点を確定
		//P0'
		QPoint p0_dash = BaseGeometry::PointToQPoint( BaseGeometry::QPointToPoint(p0)+normal );
		//P1'
		QPoint p1_dash = BaseGeometry::PointToQPoint( BaseGeometry::QPointToPoint(p1)+normal );

		//弓の移動点を確定
		QPoint yumi_cp = BaseGeometry::PointToQPoint(vecmath::unit(p0p1) * p2m + BaseGeometry::QPointToPoint(p0_dash));

		//弓線分を確定
		//左（軸0側）440mm
		//右（軸1側）200mm
		double y_left = 440 / damp;
		double y_right= 200 / damp;
		QPoint yumi_lp = BaseGeometry::PointToQPoint(vecmath::unit(p0p1) * y_left * (-1.) + BaseGeometry::QPointToPoint(yumi_cp));
		QPoint yumi_rp = BaseGeometry::PointToQPoint(vecmath::unit(p0p1) * y_right+ BaseGeometry::QPointToPoint(yumi_cp));

		painter.setPen(Qt::magenta);
		painter.drawEllipse(yumi_cp,size,size);

		if(drawBowElasticDeformation){
			//弓の弾性変形を描画する
			if(playerString == 0){
				painter.drawLine(yumi_lp,s0);
				painter.drawLine(yumi_rp,s0);
			}else if(playerString == 1){
				painter.drawLine(yumi_lp,s1);
				painter.drawLine(s1,s0);
				painter.drawLine(yumi_rp,s0);
			}else if(playerString == 2){
				painter.drawLine(yumi_lp,s1);
				painter.drawLine(yumi_rp,s1);
			}else if(playerString == 3){
				painter.drawLine(yumi_lp,s2);
				painter.drawLine(s2,s1);
				painter.drawLine(yumi_rp,s1);
			}else if(playerString == 4){
				painter.drawLine(yumi_lp,s2);
				painter.drawLine(yumi_rp,s2);
			}else if(playerString == 5){
				painter.drawLine(yumi_lp,s3);
				painter.drawLine(s3,s2);
				painter.drawLine(yumi_rp,s2);
			}else if(playerString == 6){
				painter.drawLine(yumi_lp,s3);
				painter.drawLine(yumi_rp,s3);
			}
		}else{
			//弓の弾性変形を描画しない
			painter.drawLine(yumi_lp, yumi_rp);
		}
		
		painter.setPen(Qt::blue);
		painter.drawEllipse(p0_dash,size,size);
		painter.drawEllipse(p1_dash,size,size);
		painter.drawLine(p0_dash,p1_dash);

		painter.setPen(Qt::cyan);
		painter.drawLine(p0,p0_dash);
		painter.drawLine(p1,p1_dash);

		painter.setPen(Qt::blue);
		painter.drawEllipse(p0,size,size);
		painter.drawEllipse(p1,size,size);
		painter.drawLine(p0,p1);
	}

	//固定的描画

	//軸0, 軸1描画
	painter.setPen(Qt::red);
	painter.drawLine(
		QPoint(c+pad_yoko, painter.viewport().top()),
		QPoint(c+pad_yoko, painter.viewport().bottom())
	);
	painter.drawLine(
		QPoint(pad_yoko-c, painter.viewport().top()),
		QPoint(pad_yoko-c, painter.viewport().bottom())
	);

	//弦位置描画
	size = 6;
	painter.setPen(Qt::green);
	painter.drawEllipse((s0x+pad_yoko-size/2.)*(1.),s0y+pad_tate-size/2.,size,size);
	painter.drawPoint((s0x+pad_yoko)*(1),s0y+pad_tate);

	painter.drawEllipse((s1x+pad_yoko-size/2.)*(1.),s1y+pad_tate-size/2.,size,size);
	painter.drawPoint((s1x+pad_yoko)*(1),s1y+pad_tate);

	painter.drawEllipse((s2x+pad_yoko-size/2.)*(1.),s2y+pad_tate-size/2.,size,size);
	painter.drawPoint((s2x+pad_yoko)*(1),s2y+pad_tate);

	painter.drawEllipse((s3x+pad_yoko-size/2.)*(1.),s3y+pad_tate-size/2.,size,size);
	painter.drawPoint((s3x+pad_yoko)*(1.),s3y+pad_tate);


	//X軸
	painter.setPen(Qt::gray);
	painter.drawLine(		
		QPoint(
			painter.viewport().left(), 
			painter.viewport().bottom()/tate_shift
		),		
		QPoint(
			painter.viewport().right(),
			painter.viewport().bottom()/tate_shift
		)
	);

	//Y軸
	painter.drawLine(
		QPoint(
			painter.viewport().right()/yoko_shift,
			painter.viewport().top()
		),
		QPoint(
			painter.viewport().right()/yoko_shift,
			painter.viewport().bottom()
		)
	);

	//原点
	painter.setPen(Qt::red);
	painter.drawPoint(
		QPoint(0+ pad_yoko, 0+ pad_tate)	
	);
}
