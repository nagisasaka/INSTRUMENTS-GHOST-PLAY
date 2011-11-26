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

#include "JogTab.hpp"
#include "ghostcore.hpp"
#include "actuator/controller.hpp"

JogTab::JogTab(GhostCore* _parent) : parent(_parent)
{
	//ボタン系
	connect(parent->ui.ax0_jogmove,SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));
	connect(parent->ui.ax1_jogmove,SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));
	connect(parent->ui.ax2_jogmove,SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));

	//ジョグ
	connect(parent->ui.ax0_left_1, SIGNAL(pressed()), this, SLOT(jogMove()));
	connect(parent->ui.ax0_right_1, SIGNAL(pressed()), this, SLOT(jogMove()));
	connect(parent->ui.ax1_left_1, SIGNAL(pressed()), this, SLOT(jogMove()));
	connect(parent->ui.ax1_right_1, SIGNAL(pressed()), this, SLOT(jogMove()));
	connect(parent->ui.ax2_left_1, SIGNAL(pressed()), this, SLOT(jogMove()));
	connect(parent->ui.ax2_right_1, SIGNAL(pressed()), this, SLOT(jogMove()));

	//軸同期運転
	connect(parent->ui.axall_jogsyncbutton, SIGNAL(pressed()), this, SLOT(jogMoveAll()));

	//値連動系
	connect(parent->ui.ax0_jogslider, SIGNAL(valueChanged(int)), this, SLOT(uiCoupling()));
	connect(parent->ui.ax1_jogslider, SIGNAL(valueChanged(int)), this, SLOT(uiCoupling()));
	connect(parent->ui.ax2_jogslider, SIGNAL(valueChanged(int)), this, SLOT(uiCoupling()));
	connect(parent->ui.ax0_jogspinbox, SIGNAL(valueChanged(double)), this, SLOT(uiCoupling()));
	connect(parent->ui.ax1_jogspinbox, SIGNAL(valueChanged(double)), this, SLOT(uiCoupling()));
	connect(parent->ui.ax2_jogspinbox, SIGNAL(valueChanged(double)), this, SLOT(uiCoupling()));

	//拡大縮小
	connect(parent->ui.opg_smallbutton, SIGNAL(pressed()), parent->ui.operationGraph, SLOT(zoomout()));
	connect(parent->ui.opg_largebutton, SIGNAL(pressed()), parent->ui.operationGraph, SLOT(zoomin()));

	//エリアスクロール
	connect(parent->ui.operationgraphhslider, SIGNAL(valueChanged(int)), parent->ui.operationGraph, SLOT(horizontalScroll(int)));
	connect(parent->ui.operationgraphvslider, SIGNAL(valueChanged(int)), parent->ui.operationGraph, SLOT(verticalScroll(int)));
	connect(parent->ui.operationGraph, SIGNAL(AdjustScrollBar(double,double,double,double)), this, SLOT(AdjustScrollBar(double,double,double,double)));

	//変換と運転
	connect(parent->ui.opg_duration, SIGNAL(valueChanged(double)), parent->ui.operationGraph, SLOT(setDuration(double)));
	connect(parent->ui.opg_convert, SIGNAL(pressed()), parent->ui.operationGraph, SLOT(convert()));
	parent->ui.operationGraph->setDuration(parent->ui.opg_duration->value());

	//運転開始
	connect(parent->ui.opg_start, SIGNAL(pressed()), this, SLOT(Start()));
}

void JogTab::MoveFinished()
{
	Controller *control = Controller::GetInstance();
	parent->ui.ax0_jogspinbox->setValue(control->axis[0]->GetStatus().Position);
	parent->ui.ax1_jogspinbox->setValue(control->axis[1]->GetStatus().Position);
	parent->ui.ax2_jogspinbox->setValue(control->axis[2]->GetStatus().Position);
}

void JogTab::Start()
{
	QVector<double> positionVector;
	QMap<int,double> opg = parent->ui.operationGraph->GetOperationGraph();
	QMapIterator<int,double> i(opg);
	while(i.hasNext()){
		i.next();
		double val = i.value();
		if(val < 0) val = val*(-1.0);
		positionVector.append(val);
	}

	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->SetMode(MLinear::CONTINUOUS);
	((MLinear*)(control->axis[2]))->SetPositionVector(positionVector);
	((MLinear*)(control->axis[2]))->Prepare();
	((MLinear*)(control->axis[2]))->Commit();
	//クロックをリセット
	control->GetClock()->ResetCurrentClock();
	//クロックをスタート
	control->GetClock()->Start();
}

JogTab::~JogTab()
{
}

void JogTab::Init()
{
	Controller *control = Controller::GetInstance();
	parent->ui.ax0_jogspinbox_2->setValue(control->axis[0]->GetStatus().PositionRaw);
	parent->ui.ax1_jogspinbox_2->setValue(control->axis[1]->GetStatus().PositionRaw);
	parent->ui.ax2_jogspinbox_2->setValue(control->axis[2]->GetStatus().PositionRaw);
	parent->ui.ax0_jogspinbox->setValue(control->axis[0]->GetStatus().Position);
	parent->ui.ax1_jogspinbox->setValue(control->axis[1]->GetStatus().Position);
	parent->ui.ax2_jogspinbox->setValue(control->axis[2]->GetStatus().Position);

	//動作終了時点
	connect(control->axis[0], SIGNAL(moveComplete(int)), this, SLOT(MoveFinished()));
	connect(control->axis[1], SIGNAL(moveComplete(int)), this, SLOT(MoveFinished()));
	connect(control->axis[2], SIGNAL(moveComplete(int)), this, SLOT(MoveFinished()));
}

void JogTab::Update()
{
	//UI更新
	//Controller *control = Controller::GetInstance();
	//parent->ui.ax0_jogspinbox_2->setValue(control->axis[0]->GetStatus().PositionRaw);
	//parent->ui.ax1_jogspinbox_2->setValue(control->axis[1]->GetStatus().PositionRaw);
	//parent->ui.ax2_jogspinbox_2->setValue(control->axis[2]->GetStatus().PositionRaw);

	/*
	int h = 0;
	int m = 0;
	int s = 0;
	int ms = 0;
	control->GetClock()->ClockToTime(&h,&m,&s,&ms);

	QString timeString = QString("%1: %2: %3").arg(m).arg(s).arg(ms);

	parent->ui.opg_currenttime->setText(timeString);
	*/
}

void JogTab::jogMoveAll()
{
	try{
	Controller *control = Controller::GetInstance();
	if(parent->ui.ax0_jogsynccheck->isChecked()){
		MLinear *axis = (MLinear*)control->axis[0];
		axis->SetMode(MLinear::JOG);
		axis->SetPosition(parent->ui.ax0_jogspinbox->value());
		axis->SetDuration(1000.0, 0.2, 0.2);
		axis->Prepare();		
	}
	if(parent->ui.ax1_jogsynccheck->isChecked()){
		MLinear *axis = (MLinear*)control->axis[1];
		axis->SetMode(MLinear::JOG);
		axis->SetPosition(parent->ui.ax1_jogspinbox->value());
		axis->SetDuration(1000.0, 0.2, 0.2);
		axis->Prepare();		
	}
	if(parent->ui.ax2_jogsynccheck->isChecked()){
		MLinear *axis = (MLinear*)control->axis[2];
		axis->SetMode(MLinear::JOG);
		axis->SetPosition(parent->ui.ax2_jogspinbox->value());
		axis->SetDuration(1000.0, 0.2, 0.2);
		axis->Prepare();		
	}
	for(int i=0;i<control->axis.size();i++){
		control->axis.at(i)->Commit();
	}
	}catch(const char* str){
		std::cout << "[ 例外 ] " << str << std::endl;
	}
}

void JogTab::jogMove()
{
	try{
	Controller *control = Controller::GetInstance();
	QString obj = QObject::sender()->objectName();
	MLinear *axis = 0;
	double currentPos = 0;
	//ジョグ運転幅
	double jogWidth = 1.0;
	double targetPos = 0;
	if(obj == "ax0_left_1"){
		axis = (MLinear*)control->axis[0];
		currentPos = parent->ui.ax0_jogspinbox->value();
		targetPos = currentPos - jogWidth;
		parent->ui.ax0_jogspinbox->setValue(targetPos);
	}else if(obj == "ax0_right_1"){
		axis = (MLinear*)control->axis[0];
		currentPos = parent->ui.ax0_jogspinbox->value();
		targetPos = currentPos + jogWidth;
		parent->ui.ax0_jogspinbox->setValue(targetPos);
	}else if(obj == "ax1_left_1"){
		axis = (MLinear*)control->axis[1];
		currentPos = parent->ui.ax1_jogspinbox->value();
		targetPos = currentPos - jogWidth;
		parent->ui.ax1_jogspinbox->setValue(targetPos);
	}else if(obj == "ax1_right_1"){
		axis = (MLinear*)control->axis[1];
		currentPos = parent->ui.ax1_jogspinbox->value();
		targetPos = currentPos + jogWidth;
		parent->ui.ax1_jogspinbox->setValue(targetPos);
	}else if(obj == "ax2_left_1"){
		axis = (MLinear*)control->axis[2];
		currentPos = parent->ui.ax2_jogspinbox->value();
		targetPos = currentPos - jogWidth;
		parent->ui.ax2_jogspinbox->setValue(targetPos);
	}else if(obj == "ax2_right_1"){
		axis = (MLinear*)control->axis[2];
		currentPos = parent->ui.ax2_jogspinbox->value();
		targetPos = currentPos + jogWidth;
		parent->ui.ax2_jogspinbox->setValue(targetPos);
	}
	axis->SetMode(MLinear::JOG);
	axis->SetDuration(200.0, 0.2, 0.2);
	axis->SetPosition(targetPos);
	axis->Prepare();
	axis->Commit();
	}catch(const char *str){
		std::cout << "[ 例外 ] " << str << std::endl;
	}
}

void JogTab::jogMoveByPosition()
{
	try{
	QString obj = QObject::sender()->objectName();
	Controller *control = Controller::GetInstance();
	MLinear *axis = 0;
	double targetPos = 0.0;
	if(obj == "ax0_jogmove"){
		axis = (MLinear*)control->axis.at(0);
		targetPos = parent->ui.ax0_jogspinbox->value();
	}else if(obj == "ax1_jogmove"){
		axis = (MLinear*)control->axis.at(1);
		targetPos = parent->ui.ax1_jogspinbox->value();
	}else if(obj == "ax2_jogmove"){
		axis = (MLinear*)control->axis.at(2);
		targetPos = parent->ui.ax2_jogspinbox->value();
	}
	axis->SetPosition(targetPos);
	axis->SetDuration(1000.0, 0.2, 0.2);
	axis->Prepare();
	axis->Commit();	
	}catch(char *str){
		std::cout << "[ 例外 ]" << str << std::endl;
	}
}

void JogTab::uiCoupling()
{
	QString obj = QObject::sender()->objectName();
	if(obj == "ax0_jogslider"){
		parent->ui.ax0_jogspinbox->setValue(parent->ui.ax0_jogslider->value());
	}else if(obj == "ax1_jogslider"){
		parent->ui.ax1_jogspinbox->setValue(parent->ui.ax1_jogslider->value());		
	}else if(obj == "ax2_jogslider"){
		parent->ui.ax2_jogspinbox->setValue(parent->ui.ax2_jogslider->value());
	}else if(obj == "ax0_jogspinbox"){
		parent->ui.ax0_jogslider->setValue(parent->ui.ax0_jogspinbox->value());	
	}else if(obj == "ax1_jogspinbox"){
		parent->ui.ax1_jogslider->setValue(parent->ui.ax1_jogspinbox->value());	
	}else if(obj == "ax2_jogspinbox"){
		parent->ui.ax2_jogslider->setValue(parent->ui.ax2_jogspinbox->value());	
	}
}

void JogTab::AdjustScrollBar(double v,double h,double vc, double hc)
{
	parent->ui.operationgraphvslider->setMaximum((int)v);
	parent->ui.operationgraphhslider->setMaximum((int)h);
	parent->ui.operationgraphvslider->setValue((int)vc);
	//水平スクロールバーの現在位置はここで変更する必要はない
	//parent->ui.operationgraphhslider->setValue((int)hc); // 不要
}