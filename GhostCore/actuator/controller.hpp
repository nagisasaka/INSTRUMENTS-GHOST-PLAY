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

#ifndef _CONTROLLER_HPP_
#define _CONTROLLER_HPP_

#include <QVector>
#include <QThread>

#include "actuator.hpp"
#include "mlinear/mlinear.hpp"
#include "mlinear/ppci7443.h"
#include "../audio/stringRecorder.hpp"
#include "servo/servo.hpp"
#include "servo/phidget21.h"
#include "iai/IAICtrler.hpp"

#include <iostream>

class Clock;

#pragma comment(lib, "lib/PPCI7443.lib")
#pragma comment(lib, "lib/phidget21.lib")

/*!
 * \brief
 * PPCI7443�p���T�[�{�[�h��IAI�V���A���ʐM�v���O������
 * ��ʂŒ��ۉ�����N���X�i�V���O���g���j

 * �S�ẴA�N�`���G�[�^�͂��̃N���X��ʂ��đ��삷��D
 * 
 * \see
 * actuator.hpp | mlinear.hpp
 */
class Controller : public QThread
{
	friend class HardTimer;

private:

	Controller(Controller*){}

	Controller();

	/*!
	 * \brief
	 * ���䉺�̊e���̐���X�V���s��
	 * 
	 */
	void Update();

	//�J�[�h����
	I16 totalCard;

	//�R���g���[���̃J�����g�X�e�[�^�X
	I16 retCode;

	//�e�����^�]���䒆�ł��邩�ǂ���
	bool inOperation;

	//instance
	static Controller* instance;

	//�R���g���[���[�̎��X�V�Ԋu[ms]
	static double cycle;

	//�_�~�[��
	static const unsigned int dummyAxis;

	//�N���b�N
	Clock *clock;

	//�X�g�����O���R�[�_�[
	StringRecorder *recorder;

	//�A�h�o���X�g�T�[�{�n���h��
	CPhidgetAdvancedServoHandle servoHandle;

	//�A�h�o���X�g�T�[�{�R���g���[�����\��
	void display_properties(CPhidgetAdvancedServoHandle phid);

protected:
	//����^�C�}�[
	void run();

public:

	/*!
	 * \brief
	 * �V���O���g���̃C���X�^���X�ւ̃A�N�Z�T
	 * 
	 * \returns
	 * Controller �C���X�^���X
	 * 
	 * \throws <const char*>
	 * �p���T�[��������Ȃ����߃��j�A�����������ł��܂���C���j�A������ݒ�t�@�C����������܂���.
	 *
	 * �p���T�[�������I�ɑ��݂��Ȃ��C7443.ini �����s�t�@�C���Ɠ����f�B���N�g���ɑ��݂��Ȃ��ꍇ�ɗ�O�D
	 * 
	 */
	static Controller* GetInstance()
	{
		if(!instance){
			std::cout << "�R���g���[���[���N�����܂��D" << std::endl; 
			instance = new Controller();
		}
		return instance;
	}

	static double GetCycle(){ return cycle; }

	Clock* GetClock(){ return clock; }

	StringRecorder* GetStringRecorder() { return recorder; }

	//������
	void Init();

	//�z�[���|�W�V�����֖߂�
	void SetHome();

	/*!
	 * \brief
	 * �R���g���[���[�𓮍�J�n������
	 * 
	 * \remarks
	 * ���ݎ����̓^�C�}�[���쓮���āC�e���̃X�e�[�^�X���X�V����X���b�h���X�^�[�g������
	 * 
	 * \see
	 * Update() | run();
	 */
	void Start()
	{
		std::cout << "�R���g���[���[���J�n���܂���" << std::endl;
		inOperation = true;
		start();
	}

	/*!
	 * \brief
	 * �R���g���[���[�𓮍�I��������D
	 * 
	 * \remarks
	 * ���ݎ����̓^�C�}�[�X���b�h�̖������[�v���I��������D
     *
	 * �����ɁC��Ď���ԂŊe�������삵�Ă���͍̂D�܂����Ȃ��̂ŁC�e���̓�����~������D
	 *
	 * \see
	 * run()
	 * 
	 */
	void Stop()
	{
		std::cout << "�R���g���[���[���~���܂���" << std::endl;
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Stop();
		}		
		inOperation = false;
	}

	/*!
	 * \brief
	 * �R���g���[���[�̏I�������D�e���̏I�����s���D
	 * 
	 */
	void Close()
	{
		std::cout << "�R���g���[���[���I�����܂����D" << std::endl;

		//// �e�풓�X���b�h�I��
		
		//���C�����䃋�[�v�I��
		inOperation = false;

		//�X�g�����O���R�[�_�[�I��
		recorder->flagRecording = false;

		//                                          //
		// ���̑��̃X���b�h������΂����ŏI�������� //
		//                                          //

		//�ő�c�����ԕ��҂�
		Sleep(100);

		//���I��
		for(int i=0;i<axis.size();i++){
			axis.at(i)->Close();
			delete axis.at(i);
			axis[i] = 0;
		}
		_7443_close();

		//�T�[�{���[�^�[�I��
		for(int i=0;i<servo.size();i++){
			servo.at(i)->Close();
			delete servo.at(i);
			servo[i] = 0;
		}

		//IAI�A�N�`���G�[�^�I��
		for(int i=0;i<iaic.size();i++){
			iaic.at(i)->Close();
			delete iaic.at(i);
			iaic[i] = 0;
		}

		//�V���A���ʐM�N���[�Y
		scc->Close();

		//Phidget�A�h�o���X�g�T�[�{�R���g���[�� �N���[�Y
		CPhidget_close((CPhidgetHandle)servoHandle);
		CPhidget_delete((CPhidgetHandle)servoHandle);
	}

	//�e�����^�]���ł��邩�ǂ�����Ԃ�
	bool isInOperation(){ return inOperation; };

	//���䉺�ɂ���A�N�`���G�[�^
	QVector<Actuator*> axis;

	//���䉺�ɂ���T�[�{���[�^�A�N�`���G�[�^
	QVector<Servo*> servo;

	//���䉺�ɂ���IAI�A�N�`���G�[�^
	QVector<IAICtrler*> iaic;

	//�V���A���ʐM����N���X�iIAICtrler����Ϗ��j
	StellarSerialCom *scc;

};

/*!
 * \brief
 * �n�[�h�E�F�A�^�C�}�[
 * 
 * \remarks
 * �_�~�[���ԍ��ɒ��ӂ��Ă��������I�ڑ�����郊�j�A���[�^�[�̐���ύX�����ꍇ�C�_�~�[���ԍ���ύX���Ȃ���
 * �K�؂ɓ��삵�܂���I
 * 
 */
class HardTimer
{
public:

	//�n�[�h�E�F�A�N���b�N�ɂ��E�F�C�g�i�~���b�j
	static void Wait(unsigned long time_ms)
	{
		int ret = _7443_delay_time(Controller::dummyAxis, (unsigned long)time_ms);
		if(ret){
			//�\�t�g�E�F�A�^�C�}�[�ő�ցi���Ȃ��Ƃ�Sleep()�͎g��Ȃ��悤�Ɂc�j
			Sleep((DWORD)time_ms);
			std::cout << "�n�[�h�E�F�A�^�C�}�[�����p�ł��܂���ł����i�G���[�R�[�h:"<< ret << "�j" << std::endl;
			std::cout << "�\�t�g�E�F�A�^�C�}�[�ő�p���邽�߃^�C�}�[���x���ቺ���Ă���\��������܂�" << std::endl;
		}
	}

	//�\�t�g�E�F�A�N���b�N�ɂ��E�F�C�g�i�~���b�j�G�����������œ���ւ��邱�Ƃ��ł���悤�ɂ��̊֐����g���Ă��������I
	static void Wait2(unsigned long time_ms)
	{
		//�Ȃ�ƂȂ��ŗǂ��Ƃ�
		Sleep(time_ms);
	}
};

#endif