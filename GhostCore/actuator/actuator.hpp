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

#ifndef _ACTUATOR_HPP_
#define _ACTUATOR_HPP_

#include <QObject>
#include <QVector>

class Controller;

typedef struct ActuatorStatusStruct
{
	//����p�����[�^�̓��͏��
	int Parameter;
	//������
	int Movement;
	//�ʒu
	double Position;
	//�ʒu�i���f�[�^�G�f�o�b�O�p�j
	int PositionRaw;
	//�G���[���
	int Error;

} ActuatorStatus;

class Actuator : public QObject
{
	Q_OBJECT

	friend class Controller;

public:

	//����p�����[�^���͏��
	enum { 
		   WAITING,         //����p�����[�^��t�҂�
		   PREPARED         //����p�����[�^��t�����C�ړ������v�Z����
	};

	//������
	enum {
		   STOP,            //��~��
		   ACCL,			//������
		   RUNNING,			//�����^����
		   DECL,			//������
		   ADJUSTING	    //�Ò蓮�쒆	
	};

	static char* ParamString(int p)
	{
		if(p == Actuator::STOP) return "��~��";
		else if(p == Actuator::ACCL) return "������";
		else if(p == Actuator::DECL) return "������";
		else if(p == Actuator::RUNNING) return "�����^����";
		else if(p == Actuator::ADJUSTING) return "�Ò蓮�쒆";
		else return "����`";
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// ����t���[
	//
	/////////////////////////////////////////////////////////////////////////

	// �I�u�W�F�N�g�̃A�b�v�f�[�g
	virtual void Update() = 0;

	// �������i�f�t�H���g�����j
	void Init()
	{
		//�f�t�H���g�����ł͉��q���z�[���|�W�V�����ֈړ�������
		SetHome();
	}

	/////////////////////////////////////////////////////////////////////////
	//
	// �ړ�����
	//
	/////////////////////////////////////////////////////////////////////////	

	// ������~
	virtual void Stop() = 0;

	// �ً}��~
	virtual void EmergencyStop() = 0;

	// �n�[�h�E�F�A�I��
	virtual void Close() = 0;

	// �w������ł̈ړ��g�����U�N�V�����J�n����
	virtual void Commit() = 0;

	/////////////////////////////////////////////////////////////////////////
	//
	// �ړ������ݒ�
	//
	/////////////////////////////////////////////////////////////////////////

	//���[�h�ݒ�C���[�h�ԍ������ۂ̃A�N�`���G�[�^�Ɉˑ�����D
	virtual void SetMode(int mode) = 0;

	// �ړ�����W�w��i0-100�j
	virtual void SetPosition(double targetPos) = 0;

	// �ړ�����W�w��i0-100�j
	virtual void SetPositionVector(QVector<double>& targetPos) = 0;

	// �ړ����Ԑݒ�i�~���b�j
	virtual void SetDuration(double time_ms) = 0;

	// �ړ����Ԑݒ�i�����E�������Ԋ����j
	virtual void SetDuration(double time, double accl_ratio, double decl_ratio) = 0;

	// �ړ����x�ݒ�
	//
	// ... v-t �n���K�v��������������� ... ���G�ɂȂ邾���Ŗ��p�̒������H
	//
	
	// �ړ������m��Ə����v�Z
	virtual void Prepare() = 0;

	/////////////////////////////////////////////////////////////////////////
	//
	// �A�N�Z�T
	//
	/////////////////////////////////////////////////////////////////////////

	// UIDaa
	int GetUID(){ return uid; }

	// ����X�e�[�^�X�i�f�t�H���g�����j
	ActuatorStatus GetStatus() { return status; }

protected:
	Actuator(int _uid) : uid(_uid) {}
	Actuator(int nb_axis, int maxpulse, int _uid);

	virtual ~Actuator(){}

	// �z�[���|�W�V�����ֈړ��D Init() �ŌĂяo�����D�O���璼�ڌĂ΂Ȃ����Ƃɂ����D
	virtual void SetHome() = 0;

	//�A�N�`���G�[�^ID
	int uid;

	//�A�N�`���G�[�^�X�e�[�^�X
	ActuatorStatus status;	
};

#endif