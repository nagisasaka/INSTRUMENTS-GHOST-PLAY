#include "CalibrationTab.hpp"
#include "GhostCore.hpp"
#include "constants.hpp"
#include "../calibration.hpp"
#include "../actuator/controller.hpp"
#include "../audio/stringRecorder.hpp"

#include <QScrollArea>

CalibrationTab::CalibrationTab(GhostCore *_parent) : parent(_parent)
{
	calibration = new Calibration();

	//// �t�@�C���Ǘ�
	//���[�h
	connect(parent->ui.calibrationFileLoad, SIGNAL(pressed()), this, SLOT(Load()));
	//�Z�[�u
	connect(parent->ui.calibrationFileSave, SIGNAL(pressed()), this, SLOT(Save()));

	//// ���݈ʒu�̎�荞��
	connect(parent->ui.calibrationInitialAxis0GetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));
	connect(parent->ui.calibrationInitialAxis1GetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));
	connect(parent->ui.calibrationInitialAxis2StartGetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));
	connect(parent->ui.calibrationInitialAxis2EndGetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));

	//// �����ʒu

	//// �I�����ω�
	connect(parent->ui.calibrationInitialStringSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectInitialPosition()));
	//// �l�ω�
	connect(parent->ui.calibrationInitialAxis0, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));
	connect(parent->ui.calibrationInitialAxis1, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));
	connect(parent->ui.calibrationInitialAxis2Start, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));
	connect(parent->ui.calibrationInitialAxis2End, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));

	//// �L�����u���[�V�����ψʒu

	//// �I�����ω�
	connect(parent->ui.calibrationStringSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectCalibratedPosition()));
	//// �l�ω�
	connect(parent->ui.calibrationAxis0, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));
	connect(parent->ui.calibrationAxis1, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));
	connect(parent->ui.calibrationAxis2Start, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));
	connect(parent->ui.calibrationAxis2End, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));

	//// �L�����u���[�V������̈ʒu�𔽉f
	connect(calibration, SIGNAL(UpdateCalibratedPosition()), this, SLOT(SelectCalibratedPosition()));

	//// �L�����u���[�V�������s
	connect(parent->ui.calibrationStart,SIGNAL(pressed()), this, SLOT(Start()));
	connect(parent->ui.calibrationAllStart,SIGNAL(pressed()), this, SLOT(StartAll()));
	connect(parent->ui.calibrationAllStop,SIGNAL(pressed()), this, SLOT(StopAll()));

	//�`�F�C�����s��ڑ�
	connect(calibration, SIGNAL(CalibrationEnd()), this, SLOT(StartAll()));

	//StartAll���Őڑ��i�s��j
	//connect(calibration, SIGNAL(CalibrationEnd()), this, SLOT(StartAll()));

	//// �L�����u���[�V������~�i�l�Ԃ̎����C�Z���T�[�ɂ�鎩����~�O�Ɏ蓮��~����ꍇ�j
	connect(parent->ui.calibrationStop, SIGNAL(pressed()), this, SLOT(Stop()));

	//���[�h
	Load();

	//// �����_�����O�G���A
	r1 = new RenderArea(parent->ui.calibrationSoundAmplitudeDisplay);
	parent->ui.calibrationSoundAmplitudeDisplay->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundAmplitudeDisplay->setWidget(r1);

	r2 = new RenderArea2(parent->ui.calibrationSoundCPSDisplay);
	parent->ui.calibrationSoundCPSDisplay->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundCPSDisplay->setWidget(r2);

	r3 = new RenderArea3(parent->ui.calibrationSoundFFTDisplay);
	parent->ui.calibrationSoundFFTDisplay->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundFFTDisplay->setWidget(r3);

	//������
	currentNbString = 0;
}

CalibrationTab::~CalibrationTab()
{
}

void CalibrationTab::Update()
{
	//�������Ǌ����Ă���q�E�B�W�F�b�g�ւ̑��B
	r1->Update();
	r2->Update();
	r3->Update();
}

void CalibrationTab::SelectInitialPosition()
{
	//// �w�茷�ԍ�
	int currentIndex = parent->ui.calibrationInitialStringSelect->currentIndex();
	//��
	int nb_string = currentIndex + 1;

	//// �|�W�V�����Z�b�g
	Calibration::Positions pos = calibration->GetInitialPositions(nb_string);

	parent->ui.calibrationInitialAxis0->setValue(pos.start[0]);
	parent->ui.calibrationInitialAxis1->setValue(pos.start[1]);
	parent->ui.calibrationInitialAxis2Start->setValue(pos.start[2]);
	parent->ui.calibrationInitialAxis2End->setValue(pos.end[2]);
}

void CalibrationTab::SelectCalibratedPosition()
{
	//std::cout << "SelectCalibratedPosition()" << std::endl;
	//// �w�茷�ԍ�
	int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	//���i���������Ԃ��܂ށj
	int nb_string = currentIndex + 1;

	Calibration::Positions pos = calibration->GetCalibratedPositions(nb_string);

	parent->ui.calibrationAxis0->setValue(pos.start[0]);
	parent->ui.calibrationAxis1->setValue(pos.start[1]);
	parent->ui.calibrationAxis2Start->setValue(pos.start[2]);
	parent->ui.calibrationAxis2End->setValue(pos.end[2]);
}

void CalibrationTab::UpdateInitialPosition()
{
	//// �w�茷�ԍ�
	int currentIndex = parent->ui.calibrationInitialStringSelect->currentIndex();
	//��
	int nb_string = currentIndex + 1;

	//// �|�W�V�����Z�b�g
	Calibration::Positions pos = calibration->GetInitialPositions(nb_string);

	QString obj = QObject::sender()->objectName();
	if(obj == "calibrationInitialAxis0"){
		pos.start[0] = parent->ui.calibrationInitialAxis0->value();
		pos.end[0] = pos.start[0];
	}else if(obj == "calibrationInitialAxis1"){
		pos.start[1] = parent->ui.calibrationInitialAxis1->value();
		pos.end[1] = pos.start[1];
	}else if(obj == "calibrationInitialAxis2Start"){
		pos.start[2] = parent->ui.calibrationInitialAxis2Start->value();
	}else if(obj == "calibrationInitialAxis2End"){
		pos.end[2] = parent->ui.calibrationInitialAxis2End->value();
	}

	//// �ۑ�
	calibration->SetInitialPosition(nb_string, pos);
}

void CalibrationTab::UpdateCalibratedPosition()
{
	/*
	int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	int nb_string = currentIndex + 1;
	Calibration::Positions pos = calibration->GetCalibratedPositions(nb_string);
	QString obj = QObject::sender()->objectName();
	if(obj == "calibrationAxis0"){
		pos.start[0] = parent->ui.calibrationInitialAxis0->value();
		pos.end[0] = pos.start[0];
	}else if(obj == "calibrationAxis1"){
		pos.start[1] = parent->ui.calibrationInitialAxis1->value();
		pos.end[1] = pos.start[1];
	}else if(obj == "calibrationAxis2Start"){
		pos.start[2] = parent->ui.calibrationAxis2Start->value();
	}else if(obj == "calibrationAxis2End"){
		pos.end[2] = parent->ui.calibrationAxis2End->value();
	}
	calibration->SetCalibratedPosition(nb_string, pos);
	*/
}

void CalibrationTab::GetCurrentPosition()
{
	//// �w�茷�ԍ�
	int currentIndex = parent->ui.calibrationInitialStringSelect->currentIndex();
	//��
	int nb_string = currentIndex + 1;

	//// �|�W�V�����Z�b�g
	Calibration::Positions pos = calibration->GetInitialPositions(nb_string);

	//// ���݈ʒu
	std::cout << "[ CALIBRATION TAB ] ���݈ʒu�̎�荞��" << std::endl;
	QString obj = QObject::sender()->objectName();
	Controller *control = Controller::GetInstance();
	if(obj == "calibrationInitialAxis0GetPosition"){
		//��0
		ActuatorStatus status = control->axis[0]->GetStatus();
		parent->ui.calibrationInitialAxis0->setValue(status.Position);
		//�ۑ�
		pos.start[0] = status.Position;
		pos.end[0] = status.Position;
	}else if(obj == "calibrationInitialAxis1GetPosition"){
		//��1
		ActuatorStatus status = control->axis[1]->GetStatus();
		parent->ui.calibrationInitialAxis1->setValue(status.Position);
		//�ۑ�
		pos.start[1] = status.Position;
		pos.end[1] = status.Position;
	}else if(obj == "calibrationInitialAxis2StartGetPosition"){
		//��2 �J�n�_
		ActuatorStatus status = control->axis[2]->GetStatus();
		parent->ui.calibrationInitialAxis2Start->setValue(status.Position);
		//�ۑ�
		pos.start[2] = status.Position;
	}else if(obj == "calibrationInitialAxis2EndGetPosition"){
		//��2 �I���_
		ActuatorStatus status = control->axis[2]->GetStatus();
		parent->ui.calibrationInitialAxis2End->setValue(status.Position);
		//�ۑ�
		pos.end[2] = status.Position;
	}

	//// �ۑ�
	calibration->SetInitialPosition(nb_string, pos);
}

void CalibrationTab::Load()
{
	//���[�h���s
	calibration->Load();

	//�ݒ�t�@�C������\���i�t�@�C������I���ł��Ȃ�INI�t�@�C���ł��j
	parent->ui.calibrationFileName->setText(Constants::INI_FILE_2);

	//UI������ԂɓK�p
	Calibration::Positions pos = calibration->GetInitialPositions(parent->ui.calibrationInitialStringSelect->currentIndex()+1);
	parent->ui.calibrationInitialAxis0->setValue(pos.start[0]);
	parent->ui.calibrationInitialAxis1->setValue(pos.start[1]);
	parent->ui.calibrationInitialAxis2Start->setValue(pos.start[2]);
	parent->ui.calibrationInitialAxis2End->setValue(pos.end[2]);
}

void CalibrationTab::Save()
{
	//�Z�[�u���s
	calibration->Save();
}

void CalibrationTab::Start()
{
	//// �w�茷�ԍ�
	int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	//��
	int nb_string = currentIndex + 1;

	calibration->Start(nb_string);

	//�`�F�C�����s��h��
	currentNbString = 7;
}

void CalibrationTab::StartAll()
{
	std::cout << "[ �L�����u���[�V�������s�Ώ� ] currentNbString=" << currentNbString << std::endl;

	//int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	//��
	//int nb_string = currentIndex + 1;
	if(currentNbString == 7){
		//�I��
		parent->ui.calibrationStart->setEnabled(true);
		currentNbString = 0;
		return;		
	}
	//GUI�̕ύX
	parent->ui.calibrationStringSelect->setCurrentIndex(currentNbString);
	parent->ui.calibrationInitialStringSelect->setCurrentIndex(currentNbString);
	parent->ui.calibrationStart->setEnabled(false);

	calibration->Start(currentNbString+1);
	//���̌��|�W�V������
	currentNbString++;
}

void CalibrationTab::StopAll()
{
	if(currentNbString != 0){
		//���̉�ŏI����
		currentNbString = 7;
	}
}

void CalibrationTab::Stop()
{
	//�蓮��~
	calibration->Stop();
}

