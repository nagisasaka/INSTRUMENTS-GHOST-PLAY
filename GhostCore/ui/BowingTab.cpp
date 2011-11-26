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

#include "bowingTab.hpp"
#include "../bowing.hpp"
#include "../bowingSimulation.hpp"
#include "../ghostcore.hpp"

#include <QString>

BowingTab::BowingTab(GhostCore *_parent) : parent(_parent)
{
	//運弓実際
	bowing = new Bowing();

	//運弓シミュレーション
	bowingSimulation = new BowingSimulation(parent);

	parent->ui.simulationMainPanel->setBackgroundRole(QPalette::Dark);
	parent->ui.simulationMainPanel->setWidget(bowingSimulation);

	//このタブ
	connect(parent->ui.playStart, SIGNAL(pressed()), this, SLOT(Start()));

	//ピッチキャリブレーションタブについてもここで
	connect(parent->ui.bowingStart, SIGNAL(pressed()), this, SLOT(Start()));

	//シミュレーションボタン
	connect(parent->ui.simulationPlayStart, SIGNAL(pressed()), this, SLOT(Start()));

	//アドホック位置変更実装 FIXME
	connect(parent->ui.damp, SIGNAL(valueChanged(double)), bowingSimulation, SLOT(SetDAmp(double)));
	connect(parent->ui.tate_shift, SIGNAL(valueChanged(double)), bowingSimulation, SLOT(SetTateShift(double)));
	connect(parent->ui.yoko_shift, SIGNAL(valueChanged(double)), bowingSimulation, SLOT(SetYokoShift(double)));

	//シミュレーション関係
	simulationFlag = true; // デフォルトシミュレーション
}

BowingTab::~BowingTab()
{
	delete bowing;
	delete bowingSimulation;
}

void BowingTab::Update()
{
	QString pressureText = parent->ui.pressureData->toPlainText();
	QStringList p = pressureText.split("\n", QString::SkipEmptyParts);
	int msec = p.size() * 10; // msec
	parent->ui.bowingTime->setText(QString("%1").arg(msec));

	if(simulationFlag){
		bowingSimulation->Update();
	}
}

void BowingTab::Start()
{
	QString obj = QObject::sender()->objectName();

	if(obj == "simulationPlayStart"){
		
		//// シミュレーション時
		simulationFlag = true;

		if(parent->ui.simulationInitialMoveFlag->isChecked()){
			bowingSimulation->SetInitialMoveFlag(true);
		}else{
			bowingSimulation->SetInitialMoveFlag(false);
		}

		//演奏弦指定
		bowingSimulation->SetStringPair(parent->ui.StartString->value(), parent->ui.EndString->value());
		bowingSimulation->SetPlayerString(parent->ui.PlayerString->value());

		//位置ベクタ
		QString positionText = parent->ui.positionData->toPlainText();
		QStringList pos = positionText.split("\n",QString::SkipEmptyParts);
		QVector<double> position;
		for(int i=0;i<pos.size();i++){
			position.append(pos.at(i).toDouble());
		}
		bowingSimulation->SetPositionVector(position);
		
		//圧力ベクタ
		QString pressureText = parent->ui.pressureData->toPlainText();
		QStringList p = pressureText.split("\n", QString::SkipEmptyParts);
		QVector<double> pressure;
		for(int i=0;i<p.size();i++){
			pressure.append(p.at(i).toDouble());
		}
		bowingSimulation->SetPressureVector(pressure);

		bowingSimulation->Prepare();
		bowingSimulation->Start();

	}else if(obj == "playStart" || obj == "bowingStart"){

		//// 実動作時
		simulationFlag = false;

		if(parent->ui.initialMoveFlag->isChecked()){
			bowing->SetInitialMoveFlag(true);
		}else{
			bowing->SetInitialMoveFlag(false);
		}

		bowing->startString  = parent->ui.StartString->value();
		bowing->endString    = parent->ui.EndString->value();
		bowing->playerString = parent->ui.PlayerString->value();

		QString positionText = parent->ui.positionData->toPlainText();
		QStringList pos = positionText.split("\n",QString::SkipEmptyParts);
		QVector<double> position;
		for(int i=0;i<pos.size();i++){
			position.append(pos.at(i).toDouble());
		}
		bowing->SetPositionVector(position);
		
		QString pressureText = parent->ui.pressureData->toPlainText();
		QStringList p = pressureText.split("\n", QString::SkipEmptyParts);
		QVector<double> pressure;
		for(int i=0;i<p.size();i++){
			pressure.append(p.at(i).toDouble());
		}
		bowing->SetPressureVector(pressure);
		bowing->Prepare();
		bowing->Start();
	}
}