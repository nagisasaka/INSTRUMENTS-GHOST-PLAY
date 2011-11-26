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

#include "playSimulationTab.hpp"
#include "../ghostcore.hpp"
#include "../bowingSimulation.hpp"
#include "../actuator/controller.hpp"
#include "../pitchunitSimulation.hpp"
#include "../play/pitchunit.hpp"


PlaySimulationTab::PlaySimulationTab(GhostCore *_parent) : parent(_parent)
{
	animationStep = 0;

	bowingSimulation = new BowingSimulation(parent);
	bowingSimulation->SetDrawBowElasticDeformation(false);
	parent->ui.simulationMainPanel_2->setBackgroundRole(QPalette::Dark);
	parent->ui.simulationMainPanel_2->setWidget(bowingSimulation);

	pitchunitSimulation = new PitchunitSimulation(parent);
	parent->ui.pitchMainPanel->setBackgroundRole(QPalette::Dark);
	parent->ui.pitchMainPanel->setWidget(pitchunitSimulation);

	//�V�~�����[�V�����J�n
	connect(parent->ui.simulationPlayStart_2, SIGNAL(pressed()), this, SLOT(Start()));
	connect(parent->ui.simulationPlayStop_2, SIGNAL(pressed()), this, SLOT(Stop()));

	//UI�A��
	connect(parent->ui.simulationCurrentTimeSlider, SIGNAL(sliderMoved(int)), this, SLOT(SliderMoved(int)));

	//�A�h�z�b�N�ʒu�ύX���� FIXME
	connect(parent->ui.damp_2, SIGNAL(valueChanged(double)), bowingSimulation, SLOT(SetDAmp(double)));
	connect(parent->ui.tate_shift_2, SIGNAL(valueChanged(double)), bowingSimulation, SLOT(SetTateShift(double)));
	connect(parent->ui.yoko_shift_2, SIGNAL(valueChanged(double)), bowingSimulation, SLOT(SetYokoShift(double)));

	connect(parent->ui.damp_3, SIGNAL(valueChanged(double)), pitchunitSimulation, SLOT(SetDAmp(double)));
	connect(parent->ui.tate_shift_3, SIGNAL(valueChanged(double)), pitchunitSimulation, SLOT(SetTateShift(double)));
	connect(parent->ui.yoko_shift_3, SIGNAL(valueChanged(double)), pitchunitSimulation, SLOT(SetYokoShift(double)));

}


PlaySimulationTab::~PlaySimulationTab()
{
	delete bowingSimulation;
	delete pitchunitSimulation;
}


void PlaySimulationTab::SliderMoved(int pos)
{
	std::cout << "[ PLAYSIMULATION ] �V�~�����[�V������~" << std::endl;

	//�\���X�V
	parent->ui.simulationCurrentTime->setText(QString("%1 ms").arg(pos));

	//�V�~�����[�V������~
	Stop();

	//�Y���R�}�\��
	Snapshot(pos);
}


void PlaySimulationTab::Update()
{
	QApplication::processEvents();

	bowingSimulation->Update();
	pitchunitSimulation->Update();

	if(bowingSimulation->animationStart){
		animationStep = bowingSimulation->animationCounter;
		parent->ui.simulationCurrentTimeSlider->setSliderPosition(animationStep*Controller::GetCycle());
		parent->ui.simulationCurrentTime->setText(QString("%1 ms").arg(animationStep*Controller::GetCycle()));
	}
}


void PlaySimulationTab::Snapshot(int pos)
{
	//�X�e�b�v��
	int step = pos / Controller::GetCycle();
	animationStep = step;

	//�V�~�����[�^�𓖊Y�X�e�b�v��
	bowingSimulation->animationCounter = step;

	pitchunitSimulation->animationCounter = step;
}


void PlaySimulationTab::Start()
{
	Controller *control = Controller::GetInstance();
	bowingSimulation->axis0 = ((MLinear*)(control->axis[0]))->GetPositionVector();
	bowingSimulation->axis1 = ((MLinear*)(control->axis[1]))->GetPositionVector();
	bowingSimulation->axis2 = ((MLinear*)(control->axis[2]))->GetPositionVector();

	PitchUnit *pu0 = new PitchUnit(0); // �������ĉE
	PitchUnit *pu1 = new PitchUnit(1); // �������č��i�݌v��E���ɂ͂����Ȃ��j

	//PU0�Ɋւ���
	pitchunitSimulation->axis0 = ((MLinear*)(control->axis[pu0->GetNbAxis()]))->GetPositionVector();
	pitchunitSimulation->stringSelectSequence0 = control->iaic[pu0->GetNbIaic()-1]->GetControlVector();
	pitchunitSimulation->pushpullSequence0 = control->servo[pu0->GetNbServo()]->GetControlVector();

	//PU1�Ɋւ���
	pitchunitSimulation->axis1 = ((MLinear*)(control->axis[pu1->GetNbAxis()]))->GetPositionVector();
	pitchunitSimulation->stringSelectSequence1 = control->iaic[pu1->GetNbIaic()-1]->GetControlVector();
	pitchunitSimulation->pushpullSequence1 = control->servo[pu1->GetNbServo()]->GetControlVector();

	delete pu0;
	delete pu1;

	bowingSimulation->prepared = true;
	bowingSimulation->animationStart = true;

	pitchunitSimulation->prepared = true;
	pitchunitSimulation->animationStart = true;
}


void PlaySimulationTab::Stop()
{
	bowingSimulation->animationStart = false;
	pitchunitSimulation->animationStart = false;
}