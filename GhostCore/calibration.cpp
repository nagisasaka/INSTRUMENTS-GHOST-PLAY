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

#include "calibration.hpp"
#include "actuator/controller.hpp"
#include "actuator/mlinear/mlinear.hpp"
#include "constants.hpp"
#include "audio/audioBuffer.hpp"
#include "audio/timeDomainProfile.hpp"
#include "vecmath.hpp"


#include <QCoreApplication>

Calibration::Calibration()
{
	//// ������
	std::cout << "[ CALIBRATION ] �L�����u���[�V�����N���X������" << std::endl;

	//�����ʒu�̃N���A
	for(int i=0;i<7;++i){ // [1,4]��, �e����
		Positions pos;
		pos.start << 0 << 0 << 0; //�{�[�C���O�J�n�_ ��0,1,2 �̈ʒu���N���A
		pos.end   << 0 << 0 << 0; //�{�[�C���O�I���_ ��0,1,2 �̈ʒu���N���A
		//�����ʒu�̃N���A
		initialPositions << pos;
	}

	//�L�����u���[�V�����ψʒu�̃N���A
	for(int i=0;i<7;i++){ // [1,4]���C�e����
		Positions pos;
		pos.start << 0 << 0 << 0; // �{�[�C���O�J�n�_ ��0,1,2 �̈ʒu��S�ă[���N���A
		pos.end   << 0 << 0 << 0; // �{�[�C���O�I���_ ��0,1,2 �̈ʒu��S�ă[���N���A
		//�L�����u���[�V�����ψʒu�̃N���A
		positions << pos;
	}

	//�蓮��~�t���O
	manualStopFlag = false;

	//�f�o�b�O
	_debug = 0;
}

//////////////////////////////////////////////////////////////////////////////////
//
//  ���[�h�֘A
//
//////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief
 * �ݒ�̃��[�h
 * 
 * \see
 * LoadInitialPosition() | LoadCalibratedPosition()
 */
void Calibration::Load()
{
	std::cout << "[ CALIBRATION ] �L�����u���[�V�����ݒ胍�[�h" << std::endl;
	//�����ʒu�̃��[�h
	LoadInitialPosition();
	//�L�����u���[�V�����ψʒu�̃��[�h
	LoadCalibratedPosition();
}

/*!
 * \brief
 * �����ʒu�����[�h����D
 *
 */
void Calibration::LoadInitialPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);

	//// �z��J�n
	int size = settings.beginReadArray("Calibration/Positions/Initial");

	//�e��[1,4]
	if(size != 7){
		Error::Warning(0, QObject::tr("�L�����u���[�V���������ʒu�ݒ萔���s���ł��i1�j: %1").arg(size));
	}

	//�e���ɂ���
	for(int nb_string=0;nb_string<size;++nb_string){
		settings.setArrayIndex(nb_string);
		Positions pos;
		//�{�[�C���O�J�n�_�ł̎�0,1,2�̈ʒu
		pos.start << settings.value("start0").toDouble();
		pos.start << settings.value("start1").toDouble();
		pos.start << settings.value("start2").toDouble();
		//�{�[�C���O�I���_�ł̎�0,1,2�̈ʒu
		pos.end << settings.value("end0").toDouble();
		pos.end << settings.value("end1").toDouble();
		pos.end << settings.value("end2").toDouble();
		//�����ʒu
		initialPositions[nb_string] = pos;
	}

	//�z��I��
	settings.endArray();
}

/*!
 * \brief
 * �L�����u���[�V�����ψʒu�����[�h����
 * 
 */
void Calibration::LoadCalibratedPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);
	int size = settings.beginReadArray("Calibration/Positions/Calibrated");

	//�L�����u���[�V�����ψʒu
	if(size != 7){
		Error::Warning(0,QObject::tr("�L�����u���[�V�����|�W�V�����ݒ萔���s���ł��i2�j:%1").arg(size));
	}

	//�e�ʒu�ɂ���
	for(int i=0;i<size;++i){
		settings.setArrayIndex(i);
		Positions pos;
		pos.start << settings.value("start0").toDouble();
		pos.start << settings.value("start1").toDouble();
		pos.start << settings.value("start2").toDouble();
		pos.end   << settings.value("end0").toDouble();
		pos.end   << settings.value("end1").toDouble();
		pos.end   << settings.value("end2").toDouble();
		//�L�����u���[�V�����ψʒu
		positions[i] = pos;
	}
	settings.endArray();
}

//////////////////////////////////////////////////////////////////////////////////
//
//  �Z�[�u�֘A
//
//////////////////////////////////////////////////////////////////////////////////

/*!
 * \brief
 * �ݒ�̃Z�[�u
 * 
 * \see
 * SaveInitialPosition() | SaveCalibratedPosition()
 */
void Calibration::Save()
{
	std::cout << "[ CALIBRATION ] �L�����u���[�V�����ݒ�Z�[�u" << std::endl;
	//�����ʒu�̃Z�[�u
	SaveInitialPosition();
	//�L�����u���[�V�����ψʒu�̃Z�[�u
	SaveCalibratedPosition();
}

/*!
 * \brief
 * �����ʒu��ۑ�����
 * 
 */
void Calibration::SaveInitialPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);

	//// �z��J�n
	settings.beginWriteArray("Calibration/Positions/Initial");
	int size = initialPositions.size();

	//�e���i�����ʒu�Ȃ̂ŁC[1,4]���j
	if(size != 7){
		Error::Warning(0, QObject::tr("�L�����u���[�V���������ʒu�ݒ萔���s���ł��i3�j:%1").arg(size));
	}

	//�e���ɂ���
	for(int nb_string=0;nb_string<size;++nb_string){
		settings.setArrayIndex(nb_string);
		//�{�[�C���O�J�n�_
		settings.setValue("start0", initialPositions[nb_string].start[0]); //��0�̈ʒu
		settings.setValue("start1", initialPositions[nb_string].start[1]); //��1�̈ʒu
		settings.setValue("start2", initialPositions[nb_string].start[2]); //��2�̈ʒu
		//�{�[�C���O�I���_
		settings.setValue("end0", initialPositions[nb_string].end[0]);     //��0�̈ʒu
		settings.setValue("end1", initialPositions[nb_string].end[1]);     //��1�̈ʒu
		settings.setValue("end2", initialPositions[nb_string].end[2]);     //��2�̈ʒu
	}

	//// �z��I��
	settings.endArray();
}

/*!
 * \brief
 * �L�����u���[�V�������ꂽ�l��ۑ�����
 * 
 */
void Calibration::SaveCalibratedPosition()
{
	QSettings settings(Constants::INI_FILE_2, QSettings::IniFormat);

	//// �z��J�n
	settings.beginWriteArray("Calibration/Positions/Calibrated");
	int size = positions.size();

	//�L�����u���[�V�����ψʒu�i1,2,3,4,1-2,2-3,3-4 �Ȃ̂� 7���ځj
	if(size != 7){
		Error::Warning(0,QObject::tr("�L�����u���[�V�����ψʒu�ݒ萔���s���ł��i4�j:%1").arg(size));
	}

	//�e�ʒu�ɂ���
	for(int nb_string=0;nb_string<size;++nb_string){
		settings.setArrayIndex(nb_string);
		//�{�[�C���O�J�n�_
		settings.setValue("start0", positions[nb_string].start[0]);
		settings.setValue("start1", positions[nb_string].start[1]);
		settings.setValue("start2", positions[nb_string].start[2]);
		//�{�[�C���O�I���_
		settings.setValue("end0", positions[nb_string].end[0]);
		settings.setValue("end1", positions[nb_string].end[1]);
		settings.setValue("end2", positions[nb_string].end[2]);
	}
	
	//// �z��I��
	settings.endArray();
}

void Calibration::Start(int nb_string)
{
	nbString = nb_string;
	manualStopFlag = false;
	start();
}

void Calibration::Stop()
{
	manualStopFlag = true;
}

void Calibration::SoundCheck()
{
	/*
	control->axis[4]->SetMode(MLinear::CONTINUOUS);
	QVector<double> tousoku;
	//�u���b�N���b�N�V���[�g�I
	//10msec x 1% ���� -> �Г�1�b�A����2�b

	//3%�������O���1�b�Ԃ�8��i���̎��� 0.125�b/�Г�0.0625�b=62.5msec�j	
	for(int k=0;k<10;k++){
		int turn = 6;
		for(int i=0;i<turn;i++){
			tousoku << (double)i/3.0;
		}
		for(int i=turn;i>=0;i--){
			tousoku << (double)i/3.0;
		}
	}

	control->axis[4]->SetPositionVector(tousoku);
	control->axis[4]->Prepare();
	control->axis[4]->Commit();

	return;
	*/

}

void Calibration::run()
{	
	/////////////////////////////////////////////////////////////////////
	//
	// �����ړ�
	//
	/////////////////////////////////////////////////////////////////////

	Controller *control = Controller::GetInstance();
	//���b�N�Ńz�[���ʒu��
	control->SetHome();

	//�I�[�f�B�I�֘A�擾
	StringRecorder *recorder = control->GetStringRecorder();
	AudioBuffer *buffer = recorder->GetBuffer();

	//�X�g�����O���R�[�_�[�`�F�b�N
	if(!recorder->isRecording()){
		Error::Critical(0, QObject::tr("string recorder is not activated"));
	}

	//�����ړ�
	for(int i=0;i<3;i++){
		//�W���O���[�h�ł悢���ǂ����H
		control->axis.at(i)->SetMode(MLinear::JOG); // �W���O���[�h
		double target = initialPositions[nbString-1].start[i]; // calib_step�����������Z����
		control->axis.at(i)->SetPosition(target); // �ڕW�ʒu�Z�b�g
		std::cout << "[ �L�����u���[�V���������ړ� ]" << target << std::endl;
		control->axis.at(i)->SetDuration(1000, 0.2, 0.2); // ���ԃZ�b�g�i1�b�Ԃňړ��j
		control->axis.at(i)->Prepare(); 
	}

	//�ړ����s
	for(int i=0;i<3;i++){
		control->axis.at(i)->Commit();
	}

	//�W���O���[�h��̃E�F�C�g
	Sleep(1000);

	/////////////////////////////////////////////////////////////////////
	//
	// �L�����u���[�V�������[�v�J�n
	//
	/////////////////////////////////////////////////////////////////////

	//�ړ��ݒ�i��0,1,2�ɂ��āj
	int counter = 0;
	double calib_step = 0.05;
	double calib = 0;
	double calibrated_pos = 0;

	while(true)
	{
		//// ��0,1�ɂ��ă��[�v�ړ��J�n ////

		for(int i=0;i<2;i++){
			//�W���O���[�h�ł悢���ǂ����H
			control->axis.at(i)->SetMode(MLinear::JOG); // �W���O���[�h
			calibrated_pos = initialPositions[nbString-1].start[i] + calib; // calib_step�����������Z����
			control->axis.at(i)->SetPosition(calibrated_pos); // �ڕW�ʒu�Z�b�g
			std::cout << "[ �L�����u���[�V�����ڕW�ʒu ]" << calibrated_pos << std::endl;
			control->axis.at(i)->SetDuration(1000, 0.2, 0.2); // ���ԃZ�b�g(200�~���b�ňړ�)
			control->axis.at(i)->Prepare(); 

			//�����ۑ�
			positions[nbString-1].start[i] = calibrated_pos;
			positions[nbString-1].end[i] = calibrated_pos;
			emit UpdateCalibratedPosition();
		}
		//�����ۑ��i����K�v�͂Ȃ�����ǁC���ɊQ���Ȃ��̂ł����ŕۑ�����j
		positions[nbString-1].start[2] = initialPositions[nbString-1].start[2];
		positions[nbString-1].end[2] = initialPositions[nbString-1].end[2];

		//�ړ����s
		for(int i=0;i<2;i++){
			control->axis.at(i)->Commit();
		}

		//�L�����u���[�V�����X�e�b�v�X�V
		calib += calib_step;
		//�J�E���^�[�X�V
		counter++;
		//�E�F�C�g
		Sleep(1000);

		//// ��2 �ɂ��ĉ^�s�J�n ////

		//�^�]�I���V�O�i���̐ڑ�
		//connect(control->axis[2], SIGNAL(moveComplete(int)), this, SLOT(Repeat(int)));

		//��2�͎w�蕝�őS�|�g���؂��ĉ����^�]�J�n
		control->axis[2]->SetMode(MLinear::CONTINUOUS);
		QVector<double> tousoku;

		//�u���b�N���b�N�V���[�g�I
		int axis2_start = initialPositions[nbString-1].start[2];
		int axis2_end   = initialPositions[nbString-1].end[2];
		std::cout << "start:" << axis2_start << ", end:" << axis2_end << std::endl;

		//10msec x 1% ���� -> �Г�1�b�A����2.4�b
		for(int i=axis2_start;i<axis2_end;i++){
			tousoku << i;
			std::cout << i << ",";
		}
		for(int i=0;i<40;i++){
			tousoku << axis2_end;
		}
		for(int i=axis2_end;i>=axis2_start;i--){
			tousoku << i;
		}
		control->axis[2]->SetPositionVector(tousoku);
		control->axis[2]->Prepare();

		//�^�]�J�n�̒��O�ɃI�[�f�B�I�o�b�t�@�[�̃J�����g�I�t�Z�b�g���m�F
		double timeoffset_start = buffer->offset_msec();
		//�^�]���s
		control->axis[2]->Commit();
		//����2�b�i��j�Ȃ̂ő�G�c�ɑ҂�; �ꉞ�҂����Ԃ��v�Z����
		Sleep(2500*(axis2_end-axis2_start)/100.);
		//�^�]�I�����̃I�[�f�B�I�o�b�t�@�[�̃J�����g�I�t�Z�b�g���m�F
		double timeoffset_end = buffer->offset_msec();

		//��̃I�t�Z�b�g�̊Ԃ̒l��\������
		QVector<TimeDomainProfile*> tdp = recorder->TimeDomainAnalyze(0, timeoffset_start, timeoffset_end);
		std::cout << "***********************************************************" << std::endl;
		std::cout << "[ �L�����u���[�V���� ] �I�t�Z�b�g�l: " << timeoffset_start << " �T�u�����O�T�C�Y: " << tdp.size() << std::endl;
		double sum = 0;
		for(int i=0;i<tdp.size();i++){
			sum += tdp[i]->maxAmplitude;
		}
		double avg = sum/tdp.size();
		std::cout << "[ �L�����u���[�V���� ] ���ύő�U��: " << avg << std::endl;
		std::cout << "***********************************************************" << std::endl;

		if(avg >= 0.0025){
			std::cout << "[ �L�����u���[�V���� ] �I��..." << std::endl;
			break;
		}

		if(manualStopFlag){
			std::cout << "[ �L�����u���[�V���� ] �蓮�I���{�^���̉����ɂ��蓮�I�������s����܂����D" << std::endl;
			break;
		}
	}
	//�L�����u���[�V�����I��
	Save();
	emit CalibrationEnd();
		
		/*
		//�y���̌`�����ƃG���[���o�₷���z
		//�������ԃ[���Ŏw�肵�Ă���̂ŁC�t�����p���X�ł��ł����܂Ȃ�����
		//�ǂ����Ă����R�Ȍ������Ԃ��|�����Ă��܂����߁C�������ԁ{�ʒu����
		//�Ò莞�Ԃ̊ԂɎ��̖��߂𔭍s����ƃG���[�ɂȂ�D
		//�ʒu���ߐÒ莞�Ԃ����C�ŏI�p���X���s��̌������Ԃ̕��������I��
		//�e�����y�ڂ��Ă���ƍl���đÓ����D
		//moveComplete(int)�V�O�i����҂��Ă��C���̃V�O�i���͌��ǃp���T�[
		//���x���Ȃ̂ŁC���ۂ̏I���͂킩��Ȃ��݌v�ɂȂ��Ă���D�����Ƃ�
		//���̓_�́C�A���v����̃t�B�[�h�o�b�N�M�����p���T�[�̓K���ȃ|�[�g
		//�ɓ��͂���Ή�������̂����C���܂���p���������悤�ȋC�����āC
		//�T�{���Ă����ԁD
		control->axis[2]->SetMode(MLinear::JOG);
		control->axis[2]->SetPosition(100);
		control->axis[2]->SetDuration(1000);
		control->axis[2]->Prepare();
		control->axis[2]->Commit();
		Sleep(5000);
		control->axis[2]->SetMode(MLinear::JOG);
		control->axis[2]->SetPosition(1);
		control->axis[2]->SetDuration(1000);
		control->axis[2]->Prepare();
		control->axis[2]->Commit();
		*/
}

void Calibration::Repeat(int uid)
{
	std::cout << "[ REPEAT ] UID=" << uid << std::endl;
	if(uid == 2){
		Controller *control = Controller::GetInstance();
		control->axis[2]->SetMode(MLinear::CONTINUOUS);
		QVector<double> tousoku;
		for(int i=0;i<100;i++){
			tousoku << i;
		}
		for(int i=100;i>0;i--){
			tousoku << i;
		}
		control->axis[2]->SetPositionVector(tousoku);
		control->axis[2]->Prepare();
		control->axis[2]->Commit();

		//�f�o�b�O
		_debug++;
		if(_debug == 1){
			disconnect(control->axis[2], SIGNAL(moveComplete(int)), this, SLOT(Repeat(int)));
		}
	}
}