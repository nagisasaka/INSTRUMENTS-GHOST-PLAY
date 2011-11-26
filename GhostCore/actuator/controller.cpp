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

#include "controller.hpp"
#include "constants.hpp"
#include "../clock.hpp"

//�V���O���g���C���X�^���X
Controller* Controller::instance = 0;

//�X�V�T�C�N��
double Controller::cycle = 10.;

//�_�~�[��
const unsigned int Controller::dummyAxis = 7;

Controller::Controller()
{
	//// PCI7443����{�[�h��������

	//�n�[�h�E�F�A������
	retCode = _7443_initial(&totalCard);
	if(totalCard == 0){
		throw "�p���T�[��������Ȃ����߃��j�A�����������ł��܂���.";
	}

	//����ݒ胍�[�h
	retCode = _7443_config_from_file((char*)Constants::INI_FILE_1.toStdString().c_str());
	if(retCode){
		throw "���j�A������ݒ�t�@�C����������܂���.";
	}

	//�@�B�o�[�W�����̕\��
	U16 hardwareInfo = 0;
	U16 softwareInfo = 0;
	U16 driverInfo = 0;
	retCode = _7443_version_info(0,&hardwareInfo,&softwareInfo,&driverInfo);
	std::cout << "[ �n�[�h�E�F�A��� ] " << hardwareInfo << ", ";
	for(int i=0;i<sizeof(unsigned short)*8;i++){
		printf("%d", (hardwareInfo >> i) & 0x01);
	}
	std::cout << std::endl;
	std::cout << "[ �\�t�g�E�F�A��� ] " << softwareInfo << ", ";
	for(int i=0;i<sizeof(unsigned short)*8;i++){
		printf("%d", (softwareInfo >> i) & 0x01);
	}
	std::cout << std::endl;
	std::cout << "[ �h���C�o�[���   ] " << driverInfo << ", ";
	for(int i=0;i<sizeof(unsigned short)*8;i++){
		printf("%d", (driverInfo >> i) & 0x01);
	}
	std::cout << std::endl;

	//�������g�����U�N�V�����̏���
	for(int i=0;i<totalCard*4;i++){
		//�A��������ԃ��[�h���I�t�Ɂi�ʒu���ߕ⊮���[�h�Ɂj
		_7443_set_line_move_mode(i,0);
		//��ԑ��x���[�h���I�t��
		_7443_set_axis_option(i,0);
	}

	//// Phidgets �A�h�o���X�g�T�[�{�R���g���[����������

	const char *err;

	//�T�[�{�I�u�W�F�N�g���\�z
	CPhidgetAdvancedServo_create(&servoHandle);

	//�e��̃n���h����ݒ�
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)servoHandle, Servo::AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)servoHandle, Servo::DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)servoHandle, Servo::ErrorHandler, NULL);
	CPhidgetAdvancedServo_set_OnPositionChange_Handler(servoHandle, Servo::PositionChangeHandler, NULL);

	//�f�o�C�X�ւ̐ڑ��J�n
	CPhidget_open((CPhidgetHandle)servoHandle, -1);

	//�f�o�C�X�ւ̐ڑ��҂�
	int result = 0;
	printf("[ CONTROLLER ] �T�[�{���[�^�[�̐ڑ��҂�...�i�ʃv���Z�X�͗����オ���Ă��܂��񂩁H�j");
	if((result = CPhidget_waitForAttachment((CPhidgetHandle)servoHandle, 10000)))
	{
		CPhidget_getErrorDescription(result, &err);
		printf("[ CONTROLLER ] �T�[�{���[�^�[�ڑ��҂����ɃG���[: %s\n", err);
		Error::Critical(0, QObject::tr("[ CONTROLLER ] �T�[�{���[�^�[�ڑ��҂����ɃG���[."));
	}

	//�T�[�{���[�^�[�̃v���p�e�B��\������
	display_properties(servoHandle);

	//�C�x���g�f�[�^����M����
	printf("[ CONTROLLER ] �T�[�{���[�^�[���ǂݍ��ݒ�.....\n");
	
	//display current motor position
	double curr_pos;
	if(CPhidgetAdvancedServo_getPosition(servoHandle, 0, &curr_pos) == EPHIDGET_OK){
		printf("���[�^�[: 0 > ���݈ʒu: %f\n", curr_pos);
	}
	
	std::cout << "[ CONTROLLER ] �N���b�N�Ǘ��\�z..." << std::endl;
	//�N���b�N�Ǘ��N���X
	clock = new Clock(Controller::cycle);

	std::cout << "[ CONTROLLER ] ���������J�n..." << std::endl;
	//���䉺�ɂ���S���̏�����
	//���j�A�A�N�`���G�[�^
	axis.push_back(new MLinear(0, 17000, 0)); //�^�|�
	axis.push_back(new MLinear(1, 17000, 1)); //�^�|�
	axis.push_back(new MLinear(2, 43000, 2)); //�^�|��
	axis.push_back(new MLinear(6, 17000, 3)); //���K���蕔
	axis.push_back(new MLinear(4, 17000, 4));
		
	//IAI�T�[�{�i�b��I�ɂ��̃t���[���ɏ悹�Ă��Ȃ��j
	//axis.push_back(new IAIServo(5)); //������
	//axis.push_back(new IAIServo(6)); //������

	//// ���K���蕔�T�[�{
	std::cout << "[ CONTROLLER ] �T�[�{���[�^�[������" << std::endl;
	servo.push_back(new Servo(&servoHandle, 0));
	servo.push_back(new Servo(&servoHandle, 1));

	std::cout << "[ CONTROLLER ] �V���A���ʐM���\�z" << std::endl;
	//// �V���A���ʐM�N���X���ɍ\�z
	scc = new StellarSerialCom();
	bool success = scc->Open();
	if(!success){
		std::cout << "[ CONTROLLER ] �V���A���|�[�g���J���܂���D�V���A���|�[�g�ԍ��͕ς���Ă��܂��񂩁H�|�[�g�����T���̓T�|�[�g���Ă��܂���I" << std::endl;
		Error::Critical(0, QObject::tr("[ CONTROLLER ] �V���A���|�[�g���J���܂���D�V���A���|�[�g�ԍ��͕ς���Ă��܂��񂩁H�|�[�g�����T���̓T�|�[�g���Ă��܂���I"));
	}

	std::cout << "[ CONTROLLER ] IAI���j�A�A�N�`���G�[�^�J�n" << std::endl;
	//// IAI�A�N�`���G�[�^
	iaic.push_back(new IAICtrler(scc, 1)); // ���ԍ���1�X�^�[�g�ł��邱�Ƃɒ���
	iaic.push_back(new IAICtrler(scc, 2));

	//�^�]�J�n�t���O�𗧂Ă�

	//// �X�g�����O���R�[�_�[
	std::cout << "[ CONTROLLER ] �X�g�����O���R�[�_�[�\�z" << std::endl;
	recorder = new StringRecorder();
}


void Controller::Init()
{
	//std::cout << "[ CONTROLLER ] �X�g�����O���R�[�_�[�I��" << std::endl;
	//recorder->EndRecord();
	if(recorder->isRecording()){
		recorder->EndRecord();
	}

	std::cout << "[ CONTROLLER ] �S���z�[���|�W�V�����ֈړ��J�n..." << std::endl;

	//�T�[�{���[�^�[�z�[����D��
	for(int i=0;i<servo.size();i++){
		servo.at(i)->Init();
	}

	//IAI�A�N�`���G�[�^�̋@�\�ޔ���D�悷��i�@�\���W������Ɖߓd���̑O��51�ԕ΍��G���[�ŋً}��~����D��3,4�ɂ̓u���[�L�@�\���Ȃ��̂ł��̂܂ܗ�������댯�j
	for(int i=0;i<iaic.size();i++){
		iaic.at(i)->Evacuate();
	}
	//�@�\�ޔ��̓V�[�P���V�����ɍs���D
	Sleep(500);

	//�z�[���ʒu�ւ̈ړ�
	for(int i=0;i<axis.size();i++){
		axis.at(i)->Init();
	}

	//// IAI�A�N�`���G�[�^�̓A�u�\�����[�g���[�h�Ȃ̂Ō��_���A�̕K�v�͂Ȃ��i�P�Ɍ��_���A����Ƌ@�\���W������̂Ŏ��ԍ����K�v�j ////
	//IAI�A�N�`���G�[�^���_���A
	//for(int i=0;i<iaic.size();i++){
	//	iaic.at(i)->ReturnOrigin();
	//}

	//���ׂĂ̈ړ��I���܂ő҂�
	while(true){
		std::cout << "[ CONTROLLER ] �ړ���Ԋm�F... " << std::endl;
		bool isMoving = false;
		HardTimer::Wait(1000);
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Update();
			ActuatorStatus status = axis.at(i)->GetStatus();
			if(status.Movement != Actuator::STOP){
				std::cout << "�� #" << axis.at(i)->GetUID() << " �ړ���" << std::endl;
				isMoving = true;
			}else{
				std::cout << "�� #" << axis.at(i)->GetUID() << " ��~��" << std::endl;
			}
		}
		if(!isMoving){
			std::cout << "[ CONTROLLER ] �S���z�[���|�W�V�����ֈړ������D" << std::endl;
			break;
		}
	}

	std::cout << "[ CONTROLLER ] �X�g�����O���R�[�_�[�J�n" << std::endl;
	recorder->StartRecord();
}

void Controller::SetHome()
{
	std::cout << "[ CONTROLLER ] �S���z�[���|�W�V�����ֈړ��J�n..." << std::endl;

	//������
	for(int i=0;i<servo.size();i++){
		servo.at(i)->Init();
	}
	//�@�\�ޔ�
	for(int i=0;i<iaic.size();i++){
		iaic.at(i)->Evacuate();
	}
	//�@�\�ޔ��̓V�[�P���V�����ɗD�悷��D
	Sleep(500);

	//�z�[���ʒu�ւ̈ړ�
	for(int i=0;i<axis.size();i++){
		axis.at(i)->Init();
	}

	//// IAI�A�N�`���G�[�^�̓A�u�\�����[�g���[�h�Ȃ̂Ō��_���A�̕K�v�͂Ȃ��i�P�Ɍ��_���A����Ƌ@�\���W������̂Ŏ��ԍ����K�v�j ////
	//IAI�A�N�`���G�[�^���_���A
	//for(int i=0;i<iaic.size();i++){
	//	iaic.at(i)->ReturnOrigin();
	//}

	//���ׂĂ̈ړ��I���܂ő҂i�T�[�{���͓d���ٓI����̈ב҂K�v�͖����j
	while(true){
		std::cout << "[ CONTROLLER ] �ړ���Ԋm�F... " << std::endl;
		bool isMoving = false;
		HardTimer::Wait(1000);
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Update();
			ActuatorStatus status = axis.at(i)->GetStatus();
			if(status.Movement != Actuator::STOP){
				std::cout << "�� #" << axis.at(i)->GetUID() << " �ړ���" << std::endl;
				isMoving = true;
			}else{
				std::cout << "�� #" << axis.at(i)->GetUID() << " ��~��" << std::endl;
			}
		}
		if(!isMoving){
			std::cout << "[ CONTROLLER ] �S���z�[���|�W�V�����ֈړ������D" << std::endl;
			break;
		}
	}
}


/*!
 * \brief
 * �v���p�e�B��\������
 * 
 * \param phid
 * �n���h��
 * 
 */
void Controller::display_properties(CPhidgetAdvancedServoHandle phid)
{
	int serialNo, version, numMotors;
	const char* ptr;

	//�f�o�C�X�^�C�v
	CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
	//�V���A���ԍ�
	CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
	//�f�o�C�X�o�[�W����
	CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);
	//���[�^�[��
	CPhidgetAdvancedServo_getMotorCount (phid, &numMotors);

	printf("[ CONTROLLER ] �T�[�{���[�^�[���\n");
	printf("%s\n", ptr);
	printf("Serial Number: %10d\nVersion: %8d\n# Motors: %d\n", serialNo, version, numMotors);
}


/*!
 * \brief
 * ����^�C�}�[
 * 
 * \remarks
 * clock.hpp �ɂ��d�g�݂���ڍs���āC�R���g���[���[�̒��� single shot timer ���g���`�Ƃ����D
 * 
 * \see
 * Separate items with the '|' character.
 */
void Controller::run()
{
	//���ԑ���p�ϐ��̏�����
	LARGE_INTEGER nFreq, nBefore, nAfter;
	memset(&nFreq,   0x00, sizeof nFreq);
	memset(&nBefore, 0x00, sizeof nBefore);
	memset(&nAfter,  0x00, sizeof nAfter);
	double dwTime = 0.;
	double swTime1 = 0.; // ���v�̃Y������
	double swTime2 = 0.; //      =
	int ret  = 0;
	double wait = 0;
	QueryPerformanceFrequency(&nFreq);
	if(nFreq.QuadPart == 0){
		Error::Critical(0, QObject::tr("�V�X�e���� QueryPerformanceCounter() �֐����T�|�[�g���Ă��܂���D"));
	}

	//�I�y���[�V������
	while(inOperation){
		//�����J�n���� �� ����0
		QueryPerformanceCounter(&nBefore);	
		//���Ԍv���Ώ�
		{
			//���U������s
			Update();
			//�N���b�N�Ǘ��ۑ��i�N���b�N����~���Ă���Ƃ��͒P�ɖ��������j
			clock->IncrementCurrentClock();
		}
		//�����I������
		QueryPerformanceCounter(&nAfter);
		//�����ɗv��������
		dwTime = (double)((nAfter.QuadPart - nBefore.QuadPart) * 1000. / nFreq.QuadPart); // 0.03msec -> 1000�� = 30msec
		//����̑҂����ԁi���ꂪ�o����قǂ̐��x�̗ǂ��^�C�}�[���Ȃ��j
		//wait = cycle; - dwTime;
		//�܂Ƃ߂ĕ␳
		swTime1 += dwTime;
		if(swTime1 >= 1.0){
			swTime1 = swTime1 - 1.0;
			wait = (int)cycle - 1; // int�^�ɃL���X�g�����ۂ� cycle - 1 �ɂȂ�悤��
		}else{
			wait = cycle;
		}
		
		//���ۂɑ҂�
		if(_7443_delay_time(dummyAxis, (int)wait)){
			Error::Critical(0, QObject::tr("�n�[�h�E�F�A�^�C�}�[�����p�ł��܂���ł����D"));
		}
		//�����I�� �� ����cycle 		
	}
}

/*!
 * \brief
 * ���䉺�̊e���̐���X�V���s��
 * 
 */
void Controller::Update()
{
	//�厲�i���X�V�j�G�x������
	for(int i=0;i<axis.size();i++){
		if(axis.at(i) != 0) axis.at(i)->Update();
	}


	//��O���i�T�[�{�X�V�j�G�ł��x���\��������
	for(int i=0;i<servo.size();i++){
		if(servo.at(i) != 0) servo.at(i)->Update();
	}

	//��񎲁iIAIC�X�V�j�G���ɒx���Ǝv����
	for(int i=0;i<iaic.size();i++){
		if(iaic.at(i) != 0) iaic.at(i)->Update();
	}
}