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

#ifndef __SERVO_HPP__
#define __SERVO_HPP__

#include "../../error.hpp"
#include "phidget21.h"

#include <QThread>

#include <iostream>

/// �g���X�e�[�g�ɂ���I


class Servo : public QThread
{
	friend class Controller;

public:

	//���䃂�[�h
	enum
	{
		JOG ,		// �W���O���[�h
		PREDEFINED  // ��`�ς݈ړ����[�h
	};

	//�|�W�V�����擾
	double GetPosition();

	//�d���l�擾
	double GetCurrent();

	//�z�[���|�W�V�����ֈړ�
	void SetHome();

	//���f�B�|�W�V�����ֈړ�
	void SetReady();

	//�R���^�N�g�|�W�V�����ֈړ�
	void SetContact();

	//�v���y�A
	void Prepare();

	//�R�~�b�g
	void Commit();

	//�|�W�V�����ݒ�@�W���O�p
	void SetPosition(double pos);

	//�G���Q�[�W����Ă��邩
	bool isEngaged(){ return engaged; }

	//�T�[�{���[�^�[�ɃG���Q�[�W
	void Engage(bool flag);

	//�|�W�V�����̎擾
	double GetHomePosition(){ return homePosition; }
	
	//�|�W�V�����̎擾
	double GetReadyPosition(){ return readyPosition; }

	//�|�W�V�����̎擾
	double GetContactPosition(){ return contactPosition; }

	//���[�h����
	void SetMode(int i){ mode = i; }

	//���[�h�擾
	int GetMode(){ return mode; }

	QVector<int> GetControlVector(){ return controlVector; }

	void SetControlVector(QVector<int> _controlVector){ controlVector = _controlVector; controlVectorSize = controlVector.size(); }

protected:

	void run();

private:

	Servo(CPhidgetAdvancedServoHandle *_servo, int _index);

	~Servo();

	//������
	void Init(){ SetHome(); }

	//�N���[�Y
	void Close();

	//�C���f�b�N�X
	int index;

	//�A�h�o���X�g�T�[�{�n���h��
	CPhidgetAdvancedServoHandle *servo;

	//�G���Q�[�W����Ă��邩
	bool engaged;

	//���R�ʒu
	double position;

	//�A�b�v�f�[�g
	void Update();

	//// �g���C�X�e�[�g�|�W�V�����@////

	//�z�[���|�W�V����
	double homePosition;

	//�����ʒu
	double readyPosition;

	//�������ʒu
	double contactPosition;

	//����x�N�g��
	QVector<int> controlVector;

	//����x�N�g���T�C�Y
	int controlVectorSize;

	//���[�h
	int mode;

	//����J�n�t���O
	bool flagStart;

	//�J�����g�ʒu
	int curr;

	//�v���f�B�t�@�C���h���[�h�ł̈ړ����`�ψʒu
	int to;

	//// �ȉ��R�[���o�b�N�֐� ////

	/*!
	 * \brief
	 * �R�[���o�b�N�֐��i�ڑ����j
	 * 
	 * \param ADVSERVO
	 * �A�h�o���X�g�T�[�{�n���h��
	 * 
	 * \param userptr
	 * ���[�U�[�|�C���^
	 * 
	 * \returns
	 * ����
	 * 
	 */
	static int __stdcall AttachHandler(CPhidgetHandle ADVSERVO, void *userptr)	
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (ADVSERVO, &name);
		CPhidget_getSerialNumber(ADVSERVO, &serialNo);
		printf("%s %10d �ڑ�����܂����D\n", name, serialNo);
		return 0;
	}

	/*!
	 * \brief
	 * �R�[���o�b�N�֐��i�ڑ��������j
	 * 
	 * \param ADVSERVO
	 * �A�h�o���X�g�T�[�{�n���h��
	 * 
	 * \param userptr
	 * ���[�U�[�|�C���^
	 * 
	 * \returns
	 * ����
	 * 
	 */
	static int __stdcall DetachHandler(CPhidgetHandle ADVSERVO, void *userptr)
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (ADVSERVO, &name);
		CPhidget_getSerialNumber(ADVSERVO, &serialNo);
		printf("%s %10d �ڑ�����������܂����D\n", name, serialNo);
		Error::Critical(0, QObject::tr("�T�[�{���[�^�[�̐ڑ�����������܂����D"));
		return 0;
	}

	/*!
	 * \brief
	 * �R�[���o�b�N�֐��i�G���[���j
	 * 
	 * \param ADVSERVO
	 * �A�h�o���X�g�T�[�{�n���h��
	 * 
	 * \param userptr
	 * ���[�U�[�|�C���^
	 * 
	 * \returns
	 * ����
	 * 
	 */
	static int __stdcall ErrorHandler(CPhidgetHandle ADVSERVO, void *userptr, int ErrorCode, const char *Description)
	{
		printf("[ SERVO ] �G���[�n���h��: %d - %s\n", ErrorCode, Description);
		return 0;
	}

	/*!
	 * \brief
	 * �R�[���o�b�N�֐��i�ʒu�ύX���j
	 * 
	 * \param ADVSERVO
	 * �A�h�o���X�g�T�[�{�n���h��
	 * 
	 * \param userptr
	 * ���[�U�[�|�C���^
	 * 
	 * \returns
	 * ����
	 * 
	 */
	static int __stdcall PositionChangeHandler(CPhidgetAdvancedServoHandle ADVSERVO, void *usrptr, int Index, double Value)
	{
		printf("���[�^�쓮��: %d > ���݈ʒu: %f\n", Index, Value);
		return 0;
	}
};

#endif