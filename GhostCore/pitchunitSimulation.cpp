#include "pitchunitSimulation.hpp"
#include "ghostcore.hpp"
#include "actuator/controller.hpp"
#include "play/baseGeometry.hpp"
#include "vecmath.hpp"

#include <iostream>
#include <QPainter>


PitchunitSimulation::PitchunitSimulation(GhostCore* _parent) : parent(_parent)
{
	animationCounter = 0;
	animationStart = false;
	prepared = false;

	geometry = new BaseGeometry();

	param.damp = 0.5;
	param.tate_shift = 0.9;
	param.yoko_shift = 2.0;
}

PitchunitSimulation::~PitchunitSimulation()
{

}

void PitchunitSimulation::paintEvent(QPaintEvent *event)
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

	/*
	if(nb_axis == 1){

		if(currentStringName == "G"){ //0
			//14.3mm
			position = 1430;		
		}else if(currentStringName == "D"){ //1
			//7.5mm
			position = 750;
		}else if(currentStringName == "A"){ //2
			//原点
			position = 0;
		}else if(currentStringName == "E"){ //3
			//設計上到達不能
			position = -1;
			std::cout << "[ IAICTRLER ] #" << nb_axis << " could not reached to pre-defined position E" << std::endl;
		}else{
			std::cout << "[ IAICTRLER ] Invalid pre-defined position" << std::endl;
		}
		
	}else if(nb_axis == 2){

		if(currentStringName == "G"){
			//原点
			position = 0;		
		}else if(currentStringName == "D"){
			//6.3mm
			position = 630;
		}else if(currentStringName == "A"){
			//12.88mm
			position = 1288;
		}else if(currentStringName == "E"){
			//18mm
			position = 1800;
		}else{
			std::cout << "[ IAICTRLER ] Invalid pre-defined position" << std::endl;
		}

	}
	*/

	double dl = -170. / damp;
	double ul = -300. / damp;

	//タイヤ幅
	int size = 8;

	//アニメーションを実行しているとき
	if(animationStart){
		animationCounter++;	
		if(animationCounter == axis0.size()){
			animationCounter = axis0.size()-1;//最後の要素に固定
			animationStart = false;
		}
		if(animationCounter % 100 == 0) std::cout << "..";
	}

	//リミット
	if(animationCounter >= axis0.size()){
		animationCounter = axis0.size()-1;
	}

	if(prepared){

		double p0p = axis0.at(animationCounter); //[0,100] %
		double p1p = axis1.at(animationCounter); //[0,100] %
		double p0m = geometry->ConvertPercentToMM(p0p, 3) / damp;
		double p1m = geometry->ConvertPercentToMM(p1p, 4) / damp;

		//向かって右のピッチユニット（0番）
		int s0 = stringSelectSequence0.at(animationCounter); //[0,1,2,3]
		double c0 = 0;
		if(s0 == 0) c0 = -10./damp;
		else if(s0 == 1) c0 = -5./damp;
		else if(s0 == 2) c0 = 5./damp;
		else if(s0 == 3) c0 = 10./damp;
		else Error::Critical(0, QObject::tr("[ PITCHUNIT SIMULATOR ] Invalid stringSelectSequence0 at %1 : %2").arg(animationCounter).arg(s0));

		double f0 = pushpullSequence0.at(animationCounter);
		if(f0 == 0){
			painter.setPen(Qt::blue);			
		}else if(f0 == 1){
			painter.setPen(Qt::red);
		}else{
			Error::Critical(0, QObject::tr("[ PITCHUNIT SIMULATOR ] Invalid puspullSequence0 at %1 : %2").arg(animationCounter).arg(f0));
		}
		painter.drawLine(
			QPoint(c0+pad_yoko, painter.viewport().top()),
			QPoint(c0+pad_yoko, painter.viewport().bottom())
		);

		//向かって左のピッチユニット
		int s1 = stringSelectSequence1.at(animationCounter); //[0,1,2] ※ 3は設計上不可
		double c1 = 0;
		if(s1 == 0) c1 = -10./damp;
		else if(s1 == 1) c1 = -5./damp;
		else if(s1 == 2) c1 = 5./damp;
		else Error::Critical(0, QObject::tr("[ PITCHUNIT SIMULATOR ] Invalid stringSelectSequence1 at %1 : %2").arg(animationCounter).arg(s1));

		double f1 = pushpullSequence1.at(animationCounter);
		if(f1 == 0){
			painter.setPen(Qt::blue);			
		}else if(f1 == 1){
			painter.setPen(Qt::red);
		}else{
			Error::Critical(0, QObject::tr("[ PITCHUNIT SIMULATOR ] Invalid puspullSequence1 at %1 : %2").arg(animationCounter).arg(f1));
		}
		painter.drawLine(
			QPoint(c1+pad_yoko, painter.viewport().top()),
			QPoint(c1+pad_yoko, painter.viewport().bottom())
		);

		//P0
		QPoint p0 = QPoint(pad_yoko+c0, ul+p0m+pad_tate);

		//P1
		QPoint p1 = QPoint(pad_yoko+c1, ul+p1m+pad_tate);

		//押下点
		painter.setPen(Qt::magenta);
		painter.drawEllipse(p0,size,size);
		painter.drawEllipse(p1,size,size);
	}

	//固定的描画

	painter.setPen(Qt::yellow);
	//上限
	painter.drawLine(
		QPoint(painter.viewport().left(), ul+pad_tate),
		QPoint(painter.viewport().right(),ul+pad_tate)
	);

	painter.drawLine(
		QPoint(painter.viewport().left(), dl+pad_tate),
		QPoint(painter.viewport().right(),dl+pad_tate)
	);


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
