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

#include "pitchCalibrationTab.hpp"
#include "../pitchCalibration.hpp"
#include "../ghostcore.hpp"
#include "../actuator/controller.hpp"
#include "../actuator/servo/servo.hpp"
#include "../play/pitchunit.hpp"

//debug
//#include "../actuator/servo/servo.hpp"

PitchCalibrationTab::PitchCalibrationTab(GhostCore *_parent)
{
	parent = _parent;

	//ジョグ移動
	connect(parent->ui.ax3_jogMove,SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));
	connect(parent->ui.ax4_jogMove,SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));

	//弦押下
	//connect(parent->ui.ax3_jogDown, SIGNAL(pressed()), this, SLOT(jogDown()));
	//connect(parent->ui.ax4_jogDown, SIGNAL(pressed()), this, SLOT(jogDown()));

	//保存
	//connect(parent->ui.ax3_jogSave, SIGNAL(pressed()), this, SLOT(Save()));
	//connect(parent->ui.ax4_jogSave, SIGNAL(pressed()), this, SLOT(Save()));

	//動作終了時点
	Controller *control = Controller::GetInstance();
	connect(control->axis[3], SIGNAL(moveComplete(int)), this, SLOT(MoveFinished()));
	connect(control->axis[4], SIGNAL(moveComplete(int)), this, SLOT(MoveFinished()));

	//// サーボ（暫定）
	connect(parent->ui.sx3_jogMove, SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));
	connect(parent->ui.sx4_jogMove, SIGNAL(pressed()), this, SLOT(jogMoveByPosition()));

	//レンダリングエリア
	r2 = new RenderArea2(parent->ui.calibrationSoundCPSDisplay_2);
	parent->ui.calibrationSoundCPSDisplay_2->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundCPSDisplay_2->setWidget(r2);

	//接続
	connect(parent->ui.Calibration_G, SIGNAL(pressed()), this, SLOT(AutoCalibration()));
	connect(parent->ui.Calibration_D, SIGNAL(pressed()), this, SLOT(AutoCalibration()));
	connect(parent->ui.Calibration_E, SIGNAL(pressed()), this, SLOT(AutoCalibration()));
	connect(parent->ui.Calibration_A, SIGNAL(pressed()), this, SLOT(AutoCalibration()));

	connect(parent->ui.pitchCalibrationLoad, SIGNAL(pressed()), this, SLOT(Load()));
	connect(parent->ui.pitchCalibrationSave, SIGNAL(pressed()), this, SLOT(Save()));

	connect(parent->ui.S11, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S12, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S13, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S14, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S15, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S16, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S17, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S18, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S21, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S22, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S23, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S24, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S25, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S26, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S27, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S28, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S31, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S32, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S33, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S34, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S35, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S36, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S37, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S38, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S41, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S42, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S43, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S44, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S45, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S46, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S47, SIGNAL(pressed()), this, SLOT(EachCalibration()));
	connect(parent->ui.S48, SIGNAL(pressed()), this, SLOT(EachCalibration()));

	//それぞれのピッチユニットを構築（同一設定ファイルを使うので注意）
	pu0 = new PitchUnit(0);
	pu1 = new PitchUnit(1);

	pc0 = pu0->calibration;
	pc1 = pu1->calibration;

}


PitchCalibrationTab::~PitchCalibrationTab()
{
	delete pc0;
	delete pc1;
	delete r2;
}

void PitchCalibrationTab::Load()
{
	PitchCalibration *pc = 0;
	if(parent->ui.calibrationTargetUnit->currentIndex() == 0){
		pc = pc0;
	}else if(parent->ui.calibrationTargetUnit->currentIndex() == 1){
		pc = pc1;
	}
	pc->Load();
}

void PitchCalibrationTab::Save()
{
	PitchCalibration *pc = 0;
	double z_value = 0;
	double c_value = 0;
	if(parent->ui.calibrationTargetUnit->currentIndex() == 0){
		pc = pc0;
		z_value = parent->ui.ax3_jogSpinBox->value();
		c_value = parent->ui.sx3_jogSpinBox->value();
	}else if(parent->ui.calibrationTargetUnit->currentIndex() == 1){
		pc = pc1;
		z_value = parent->ui.ax4_jogSpinBox->value();
		c_value = parent->ui.sx4_jogSpinBox->value();
	}
	pc->Save(z_value, c_value);
}


/*!
 * \brief
 * 指定弦に対する全ポジション自動キャリブレーションを実行する
 * 
 */
void PitchCalibrationTab::AutoCalibration()
{
	QString obj = QObject::sender()->objectName();
	PitchCalibration *pc = 0;
	if(parent->ui.calibrationTargetUnit->currentIndex() == 0){
		pc = pc0;
	}else if(parent->ui.calibrationTargetUnit->currentIndex() == 1){
		pc = pc1;
	}

	if(obj == "Calibration_G"){
		pc->Start("G");
	}else if(obj == "Calibration_D"){
		pc->Start("D");
	}else if(obj == "Calibration_E"){
		pc->Start("E");
	}else if(obj == "Calibration_A"){
		pc->Start("A");
	}
}

void PitchCalibrationTab::EachCalibration()
{
	QString obj = QObject::sender()->objectName();
	PitchUnit *pu = 0;
	PitchCalibration *pc = 0;
	if(parent->ui.calibrationTargetUnit->currentIndex() == 0){
		pu = pu0;
		pc = pc0;
	}else if(parent->ui.calibrationTargetUnit->currentIndex() == 1){
		pu = pu1;
		pc = pc0;
	}

	//G線
	if(obj == "S11"){
		pu->SetString(0);
		pu->SetPitch("G");
	}else if(obj == "S12"){
		pu->SetString(0);
		pu->SetPitch("G#");
	}else if(obj == "S13"){
		pu->SetString(0);
		pu->SetPitch("A");
	}else if(obj == "S14"){
		pu->SetString(0);
		pu->SetPitch("A#");
	}else if(obj == "S15"){
		pu->SetString(0);
		pu->SetPitch("B");
	}else if(obj == "S16"){
		pu->SetString(0);
		pu->SetPitch("C");
	}else if(obj == "S17"){
		pu->SetString(0);
		pu->SetPitch("C#");
	}else if(obj == "S18"){
		pu->SetString(0);
		pu->SetPitch("D");
	}
	//D線
	if(obj == "S21"){
		pu->SetString(2);
		pu->SetPitch("D");
	}else if(obj == "S22"){
		pu->SetString(2);
		pu->SetPitch("D#");
	}else if(obj == "S23"){
		pu->SetString(2);
		pu->SetPitch("E");
	}else if(obj == "S24"){
		pu->SetString(2);
		pu->SetPitch("F");
	}else if(obj == "S25"){
		pu->SetString(2);
		pu->SetPitch("F#");
	}else if(obj == "S26"){
		pu->SetString(2);
		pu->SetPitch("G");
	}else if(obj == "S27"){
		pu->SetString(2);
		pu->SetPitch("G#");
	}else if(obj == "S28"){
		pu->SetString(2);
		pu->SetPitch("A");
	}
	//E線
	if(obj == "S31"){
		pu->SetString(4);
		pu->SetPitch("A");
	}else if(obj == "S32"){
		pu->SetString(4);
		pu->SetPitch("A#");
	}else if(obj == "S33"){
		pu->SetString(4);
		pu->SetPitch("B");
	}else if(obj == "S34"){
		pu->SetString(4);
		pu->SetPitch("C");
	}else if(obj == "S35"){
		pu->SetString(4);
		pu->SetPitch("C#");
	}else if(obj == "S36"){
		pu->SetString(4);
		pu->SetPitch("D");
	}else if(obj == "S37"){
		pu->SetString(4);
		pu->SetPitch("D#");
	}else if(obj == "S38"){
		pu->SetString(4);
		pu->SetPitch("E");
	}

	//A線
	if(obj == "S41"){
		pu->SetString(6);
		pu->SetPitch("E");
	}else if(obj == "S42"){
		pu->SetString(6);
		pu->SetPitch("F");
	}else if(obj == "S43"){
		pu->SetString(6);
		pu->SetPitch("F#");
	}else if(obj == "S44"){
		pu->SetString(6);
		pu->SetPitch("G");
	}else if(obj == "S45"){
		pu->SetString(6);
		pu->SetPitch("G#");
	}else if(obj == "S46"){
		pu->SetString(6);
		pu->SetPitch("A");
	}else if(obj == "S47"){
		pu->SetString(6);
		pu->SetPitch("A#");
	}else if(obj == "S48"){
		pu->SetString(6);
		pu->SetPitch("B");
	}

	pu->Prepare();
	pu->Commit();
}


/*!
 * \brief
 * GUIアップデートタイマーでGUIアップデートを行う．
 * 
 */
void PitchCalibrationTab::Update()
{
	//UI更新
	Controller *control = Controller::GetInstance();
	//parent->ui.ax3_jogSpinBox_2->setValue(control->axis[3]->GetStatus().PositionRaw);
	//parent->ui.ax4_jogSpinBox_2->setValue(control->axis[4]->GetStatus().PositionRaw);

	//UI更新
	parent->ui.sx3_jogSpinBox_2->setValue(control->servo[0]->GetPosition());
	parent->ui.sx4_jogSpinBox_2->setValue(control->servo[1]->GetPosition());

	PitchCalibration *pc = 0;
	if(parent->ui.calibrationTargetUnit->currentIndex() == 0){
		pc = pc0;
	}else if(parent->ui.calibrationTargetUnit->currentIndex() == 1){
		pc = pc1;
	}
	parent->ui.currentTargetString->setText(pc->GetCurrentString());
	parent->ui.currentPitch->setText(pc->GetCurrentPitch());
	parent->ui.refFrequency->setText(QString("%1 Hz").arg(pc->GetRefFrequency()));

	//レンダリングエリア
	r2->Update();
	//ここは本来UI担当クラスから引っ張ってくる情報ではないが，アドホックに実装（設計上今後要修正）
	double fzero = r2->GetFZero();
	parent->ui.fzero->setText(QString("%1 Hz").arg(fzero));
}


/*!
 * \brief
 * JOG移動関数
 * 
 */
void PitchCalibrationTab::jogMoveByPosition()
{
	try{
		QString obj = QObject::sender()->objectName();
		Controller *control = Controller::GetInstance();
		double targetPos = 0.0;
		if(obj == "ax3_jogMove" || obj == "ax4_jogMove"){
			MLinear *axis = 0;
			if(obj == "ax3_jogMove"){
				axis = (MLinear*)control->axis.at(3);
				targetPos = parent->ui.ax3_jogSpinBox->value();
			}else if(obj == "ax4_jogMove"){
				axis = (MLinear*)control->axis.at(4);
				targetPos = parent->ui.ax4_jogSpinBox->value();
			}
			axis->SetMode(MLinear::JOG);
			axis->SetPosition(targetPos);
			axis->SetDuration(1000.0, 0.2, 0.2);
			axis->Prepare();
			axis->Commit();	
		}else{
			if(obj == "sx3_jogMove"){
				control->servo[0]->SetPosition(parent->ui.sx3_jogSpinBox->value());
				control->servo[0]->Prepare();
				control->servo[0]->Commit();
			}else if(obj == "sx4_jogMove"){
				control->servo[1]->SetPosition(parent->ui.sx4_jogSpinBox->value());
				control->servo[1]->Prepare();
				control->servo[1]->Commit();
			}

		}
	}catch(char *str){
		std::cout << "[ 例外 ]" << str << std::endl;
	}
}


void PitchCalibrationTab::MoveFinished()
{
	Controller *control = Controller::GetInstance();
	parent->ui.ax3_jogSpinBox->setValue(control->axis[3]->GetStatus().Position);
	parent->ui.ax4_jogSpinBox->setValue(control->axis[4]->GetStatus().Position);

	parent->ui.sx3_jogSpinBox->setValue(control->servo[0]->GetPosition());
	parent->ui.sx4_jogSpinBox->setValue(control->servo[1]->GetPosition());
}
