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

#include "servo.hpp"
#include "../../actuator/controller.hpp"

#include <iostream>
#include <cstdlib>

Servo::Servo(CPhidgetAdvancedServoHandle *_servo, int _index)
{
	servo = _servo;
	index = _index;
	engaged = false;
	curr = 0;
	flagStart = false;
	mode = Servo::JOG;		 // �W���O���[�h�ŏ�����
	to = 0;

	//�g���C�X�e�[�g�|�W�V�����̏������i�ޔ��ׂ͗荇�������̓��m�̍�����90�x�ȏ�ɂȂ�悤�Ɂj
	if(index == 0){

		//100����60�ŃR���^�N�g�C180�őޔ��i�O��80�x�ޔ��j
		homePosition    = 110;
		readyPosition   = 110; 
		contactPosition = 55;

	}else if(index == 1){

		//80����120�ŃR���^�N�g�C90�őޔ��i����10�x�ޔ��j
		homePosition    = 70;
		readyPosition   = 70;
		contactPosition = 140;

	}else{

		std::cout << "[ SERVO ] �T�[�{ID���s���ł� : " << index << std::endl;

	}

	//�����s���O�I�t
	CPhidgetAdvancedServo_setSpeedRampingOn(*servo, index, 0);

	//���������ɃG���Q�[�W���Ă��܂�
	Engage(true);

	//�����s���O����ꍇ�͈ȉ��̃R�[�h�ő��x�E�����x���������肷��
	//Set up some initial acceleration and velocity values
	//double minAccel, maxVel;
	//CPhidgetAdvancedServo_getAccelerationMin(servo, index, &minAccel);
	//CPhidgetAdvancedServo_setAcceleration(servo, index, minAccel*2);
	//CPhidgetAdvancedServo_getVelocityMax(servo, index, &maxVel);
	//CPhidgetAdvancedServo_setVelocityLimit(servo, index, maxVel/2);
}


Servo::~Servo()
{
	Close();
}


/*!
 * \brief
 * �G���Q�[�W�i�T�[�{���[�^�[�ɓd�͂���������j
 * 
 * \param flag
 * true : �d�͋����J�n
 * false: �d�͋�����~
 * 
 * \remarks
 * ���܂蒷���ԓd�͋��������܂܂ɂ��Ȃ����ƁD
 * 
 */
void Servo::Engage(bool flag)
{
	if(flag){		
		//�ʓd����
		CPhidgetAdvancedServo_setEngaged(*servo, index, 1);
		engaged = true;
	}else{
		//�d���J�b�g����
		CPhidgetAdvancedServo_setEngaged(*servo, index, 0);
		engaged = false;
	}
}


/*!
 * \brief
 * �z�[���|�W�V�����ֈړ�
 * 
 * \remarks
 * ��p���얽�߂�����킯�ł͂Ȃ��̂Œ���
 *
 */
void Servo::SetHome()
{
	CPhidgetAdvancedServo_setPosition (*servo, index, homePosition);
	curr = 0;
}


/*!
 * \brief
 * ���f�B�|�W�V�����ֈړ�
 * 
 */
void Servo::SetReady()
{
	CPhidgetAdvancedServo_setPosition (*servo, index, readyPosition);
}


/*!
 * \brief
 * �R���^�N�g�|�W�V�����ֈړ�
 * 
 */
void Servo::SetContact()
{
	CPhidgetAdvancedServo_setPosition (*servo, index, contactPosition);
}


/*!
 * \brief
 * �T�[�{���[�^���N���[�Y����
 * 
 */
void Servo::Close()
{
	Engage(false);
}


/*!
 * \brief
 * ���݈ʒu���擾����
 * 
 * \param index
 * �T�[�{���[�^�[�ԍ�
 * 
 * \returns
 * ���݈ʒu
 * 
 */
double Servo::GetPosition()
{
	//���݈ʒu
	double curr_pos = 0;
	//���݈ʒu���擾
	CPhidgetAdvancedServo_getPosition(*servo, index, &curr_pos);
	//���݈ʒu��Ԃ�
	return curr_pos;
}


/*!
 * \brief
 * �d���l��Ԃ�
 * 
 * \returns
 * �d���l
 * 
 * \remarks
 * Current�͓d���̈Ӗ��D���̊֐��ł́u���݂́v�̈Ӗ��ŗ��p����Ă���C�d���̈Ӗ��ŗ��p����Ă���̂͂��̊֐��݂̂ł��邱�Ƃɒ��ӁD
 * 
 */
double Servo::GetCurrent()
{
	//���݈ʒu
	double current = 0;
	//�d���l���擾
	CPhidgetAdvancedServo_getCurrent(*servo, index, &current);
	//�d���l��Ԃ�
	return current;
}


/*!
 * \brief
 * �|�W�V�����ݒ�@�W���O�p
 * 
 * \param pos
 * �|�W�V����
 * 
 */
void Servo::SetPosition(double pos)
{
	position = pos;
}


void Servo::Update()
{
	if(flagStart){

		if(curr == 0){
			//�����ړ��̂ݓ���

			if(controlVector[curr] == 1){
				to = 1;
			}else{
				to = 0;
			}
			start();

		}else{

			if(controlVector[curr-1] != controlVector[curr]){

				if(controlVector[curr] == 1){
					to = 1;
				}else{
					to = 0;
				}
				start();
	
			}
		}

		//�I����������
		curr++;
		if(curr >= controlVectorSize){
			std::cout << "[ SERVO ] �v���f�B�t�@�C���h���[�h�I��" << std::endl;
			curr = 0;
			flagStart = false;
		}
		
	}
}


void Servo::Prepare()
{
	//// �G���[�`�F�b�N ////
	
	curr = 0;
}


void Servo::Commit()
{
	std::cout << "[ SERVO ] �R�~�b�g ";
	if(mode == Servo::JOG){
		std::cout << "�W���O���[�h" << std::endl;
		//�W���O����̎��́C�P���G���Q�[�W����
		start();
	}else if(mode == Servo::PREDEFINED){
		std::cout << "�v���f�B�t�@�C���h���[�h ����x�N�g���T�C�Y: " << controlVectorSize << std::endl;
		//�v���f�B�t�@�C���h����̎��͐���x�N�g���ɂ�鐧��Ɉڂ�
		flagStart = true;
	}
}


void Servo::run()
{
	if(mode == Servo::JOG){

		//�W���O���[�h�̏ꍇ�͒P���ŖړI�ʒu�ֈړ�
		CPhidgetAdvancedServo_setPosition (*servo, index, position);

	}else if(mode == Servo::PREDEFINED){

		//�v���f�B�t�@�C���h���[�h�̏ꍇ�͐���x�N�g���ɏ]���Ă��ꂼ��ړ�
		if(to == 0){
			SetReady();
		}else if(to == 1){
			SetContact();
		}else if(to == 2){
			SetHome();
		}

	}

	//// �G���Q�[�W

	/*
	if(mode == Servo::JOG){
		std::cout << "[ �T�[�{���[�^�[����X���b�h ] ���[�^ID : " << index << " > �G���Q�[�W�i�W���O�j" << std::endl;
		Engage(true);
	}else if(mode == Servo::KNOCK){
		std::cout << "[ �T�[�{���[�^�[����X���b�h ] ���[�^ID : " << index << " > �G���Q�[�W�i�d���فj" << std::endl;
		CPhidgetAdvancedServo_setPosition (*servo, index, knockPosition);
	}
	*/

	//// �f�B�X�G���Q�[�W����͍s��Ȃ�

	//�d���ٓI���쏈�����ԁi0.5�b�j
	//Sleep(500);
	//
	//�f�B�X�G���Q�[�W
	//std::cout << "�f�B�X�G���Q�[�W" << std::endl;
	//CPhidgetAdvancedServo_setEngaged(*servo, index, 0);
}
