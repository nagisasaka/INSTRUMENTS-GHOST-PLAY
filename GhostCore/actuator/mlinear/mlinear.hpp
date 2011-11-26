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

#ifndef _MLINEAR_HPP_
#define _MLINEAR_HPP_

#include "../actuator.hpp"
#include "ppci7443.h"
#include "7443err.h"
#include "../../error.hpp"

#include <QObject>
#include <QVector>

#include <iostream>

class Controller;

/*!
 * \brief
 * ���C���j�A�A�N�`���G�[�^�[�̐���N���X
 * 
 * \see
 * actuator.hpp|7443err.h|type_def.h
 */
class MLinear : public Actuator
{
	Q_OBJECT

	friend class Controller;

signals:
	/*!
	 * \brief
	 * �w�肳�ꂽ���삪�I���������ɑ��o�����V�O�i���D�L�����u���[�V�������̍r���^�]��
	 * ����ɃV�r�A�ȃ^�C�~���O�����߂��Ȃ��ꍇ�ɂ̂ݗ��p����D��{�I�ɂ́C�Q�l�p�Ƃ�
	 * �Ă̂ݗp����C�Q�l�p�V�O�i���D
	 *
	 * \param int
	 * ��UID
	 *
	 */
	void moveComplete(int);

public:
	
	/*!
	 * \brief
	 * ���䃂�[�h
	 *
	 * JOG���[�h�̓f�t�H���g�ŁC������^�]����ɓK�؂Ɋ��炩�ɉ��������ݒ肳���i�����x�^���GS����������j
	 * CONT���[�h�̓g���v���o�b�t�@�����O�̘A�����샂�[�h�ŁC������Ԃł̓����^���̘A��
	 */
	enum
	{
		JOG ,	   // �W���O
		CONTINUOUS // �A������
	};

	/*!
	 * \brief
	 * ���䃂�[�h�̃Z�b�^�[
	 * 
	 * \param pos
	 * ���䃂�[�h
	 *
	 */
	void SetMode(int _mode);

	/*!
	 * \brief
	 * �A�b�v�f�[�g. ���̏����͕K����T�C�N�����Ɏ��܂�K�v�����邱�Ƃɒ��ӁD
	 * 
	 */
	void Update();

	/**
	 * \brief
	 * �X�e�[�^�X���A�b�v�f�[�g����i���X�e�[�^�X���p���T�[�ɖ���₢���킹��j
	 *
	 * (1)�A���v�Ŏ����삳����ꍇ�́C�p���T�[�͎�������Ď��ł��Ȃ��D
	 * (2)�p���T�[�Ŏ����삳����ꍇ�́C�A���v�������삪�킩��D
	 * �� ����2�͓���I�Ɉ����āC���̊֐����ŉB�����邱�Ƃɂ���
	 *
	 */
	void UpdateStatus();
	
	/**
	 * \brief
	 * �ʒu���A�b�v�f�[�g����i�R�}���h�|�W�V�������p���T�[�ɖ���₢���킹��j
	 *
	 */
	void UpdatePosition();

	/*!
	 * \brief
	 * ���̍ŏI�����CCommit() ����O�Ɋe�����̍ŏI�v�Z�����s���D
	 * 
	 * \throws <exception class>
	 * �ړ��ɕK�v�ȏ�񂪑���܂���D
	 * 
	 * Commit() �ł��Ȃ���Ԃ� Prepare() ���ĂԂƔ�������D�v���O�����~�X�D
	 * 
	 */
	void Prepare();

	/*!
	 * \brief
	 * �������ړ�����
	 * 
	 * \throws <const char*>
	 * �ړ��p�����[�^�����́C�ړ����s�i�ړ����ړ����߂͎󗝂ł��܂���j
	 * 
	 * �������ړ����߂́C�ړ����ړ����߂��󗝂��Ȃ��D
	 *
	 */
	void Commit();

	/*!
	 * \brief
	 * �A���ʒu�w��f�[�^���Z�b�g����
	 * 
	 * \param pos
	 * �A���ʒu�w��f�[�^[0,100]
	 * 
	 * \remarks
	 * CONT_POSITION���[�h�œ��삷�邱�Ƃɗ���
	 * 
	 */
	void SetPositionVector(QVector<double>& pos);

	/*!
	 * \brief
	 * �A�����x�w��f�[�^���Z�b�g����
	 * 
	 * \param vel
	 * �A�����x�w��f�[�^
	 * 
	 * \remarks
	 * CONT_VELOCITY���[�h�œ��삷�邱�Ƃɗ���
	 * 
	 */
	void SetVelocityVector(QVector<double>& vel);

	/*!
	 * \brief
	 * �ړ���ʒu��[0,100]�Ŏw�肷��D���݈ʒu�̓R�}���h�|�W�V�������������邱�Ƃœ���D
	 * 
	 * \param targetPos
	 * �ړ���ʒu�D[0,100]�Ŏw��D0.1�ł�����100�p���X�ȏ�c���Ă���̂ŁC�������ɒ��ӂ���D
	 * 
	 * \throws <const char*>
	 * �R�}���h�|�W�V�������擾�ł��܂���
	 * 
	 * �ʏ�͔������Ȃ��Ƒz�肳���D
	 * 
	 * \remarks
	 * targetPos �� [0,100] ��100�����`���œ��͂����邪�C0.1%�ł�����100�p���X�ȏ�c���Ă��邽�߁C��ʂ̉��Z�ł̌�����
	 * �ɒ��ӂ��邱�ƁD
	 * 
	 */
	void SetPosition(double targetPos);

	/*!
	 * �ړ���ʒu��[0,100]�Ŏw�肷��D���݈ʒu�͑������Ŏw�肷��
	 * 
	 * \param targetPos
	 * �ړ���ʒu�D[0,100]�Ŏw��D0.1�ł�����100�p���X�ȏ�c���Ă���̂ŁC�������ɒ��ӂ���D
	 * 
	 * \param fromPos
	 * �ړ��O�ʒu�i���݈ʒu�j�D[0,100]�Ŏw��D
	 * 
	 * \remarks
	 * �A�����샂�[�h�����A���^�C����������ꍇ�ɗ��p���邱�Ƃ��ł���D
	 * �A�����샂�[�h�����O�v�Z����ꍇ�͗��p����K�v�͂Ȃ��D
	 * 
	 */
	void SetPosition(double targetPos, double fromPos);

	/*!
	 * \brief
	 * �ړ����Ԏw��i���[�e�B���e�B�֐��j
	 * 
	 * \param time_ms
	 * �ړ����ԁi�~���b�j
	 */
	void SetDuration(double time_ms);

	/*!
	 * \brief
	 * �ړ����ԁC���������ԁi�����j�w��
	 * 
	 * \param time_ms
	 * ���v�ړ����ԁi�~���b�j
	 * 
	 * \param accl_ratio
	 * ���v�ړ����Ԓ��̉������Ԃ̊���[0,1]
	 * 
	 * \param decl_ratio
	 * ���v�ړ����Ԓ��̌������Ԃ̊���[0,1]
	 * 
	 * \throws <const char*>
	 * �����E�������Ԋ����̍��v��1�𒴂��Ă��܂�
	 * 
	 */
	void SetDuration(double time_ms, double accl_ratio, double decl_ratio);

	/*!
	 * \brief
	 * ����p�����[�^�[������
	 * 
	 */
	void ParameterInit();

	/*!
	 * \brief
	 * ������~
	 * 
	 * \remarks
	 * ������~�Ɏ��s�����ꍇ�͎����I�ɋً}��~�����s�����D
	 * 
	 * \see
	 * EmergencyStop()
	 *
	 */
	void Stop()
	{
		//�P�b�ԂŌ�����~
		retCode = _7443_sd_stop(nb_axis,1);
		if(!retCode){
			std::cout << "�� #" << uid << " ������~�D" << std::endl;
		}else{
			std::cout << "�� #" << uid << " ������~���s�i" << retCode << "�j�C�ً}��~���s�D" << std::endl;
			EmergencyStop();
		}
	}

	/*!
	 * \brief
	 * �ً}��~
	 *
	 * \throws <const char*>
	 * �ً}��~������s
	 *
	 * \remarks
	 * �ً}��~�Ɏ��s�����ꍇ�́C�R���g���[���[�������I�����C�v���O�������̂�exit(1)���Ă��܂��D
	 * �ً}��~�ɐ��������ꍇ���C��O�𑗏o����D���̒i�K�ł́C�v���O�������p�����삳���邱�Ƃ�
	 * �z�肵�Ă��炸�C�\�Ȍ��葬�₩�Ƀv���O�������I�����邱�Ƃ�ړI�Ƃ���D
	 * 
	 */
	void EmergencyStop()
	{		
		retCode = _7443_emg_stop(nb_axis);
		if(!retCode){
			Error::Critical(0, QObject::tr("�� #%1 �ً}��~�D").arg(uid));
		}else{
			std::cout << "�� #" << uid << " �ً}��~���s�i"<< retCode << "�j�D�R���g���[���������V���b�g�_�E�����܂��D" << std::endl;
			Close();
			_7443_close();
			//throw "�ً}��~���s�C�R���g���[�������V���b�g�_�E�����s�D";
			Error::Critical(0, QObject::tr("�� #%1 �ً}��~���s�C�R���g���[�������V���b�g�_�E�����s�D").arg(uid));
		}
	}


	/*!
	 * \brief
	 * �z�[���|�W�V�����ֈړ�����
	 * 
	 * \throws <const char*>
	 * �R�}���h�|�W�V�����N���A���s�CSVON�M�����㎸�s�CSVON�M���������s�D
	 * 
	 * �ʏ�͔������Ȃ��Ƒz�肳���D
	 *
	 * \remarks
	 * ���_���A�̓p���T�[�̋@�\�͗��p�����CSVON�M���̗�����ŁC�A���v���œ��삷��D
	 * �����錴�_���A�̐ݒ�̓A���v���ňׂ���邱�Ƃɒ��ӁD
	 * 
	 */
	void SetHome();

	// �n�[�h�E�F�A�I��
	void Close()
	{
		std::cout << "�� #" << uid << " �I��" << std::endl;

		//�p�����[�^������
		ParameterInit();

		//���\�t�g���~�b�g����
		retCode = _7443_disable_soft_limit(nb_axis);
	}

	//// �A�N�Z�T
	
	int GetMaxPulse(){ return max_pulse; }
	int GetPulseByMeter() { return PulseByMeter; }

	QVector<double> GetPositionVector(){ return positionVector;	}
	
protected:

	MLinear(int _nb_axis, int _max_pulse, int _uid) : nb_axis(_nb_axis), max_pulse(_max_pulse), Actuator(_uid)
	{
		//�@�B�p�����[�^�ݒ�
		PulseByMeter = 100000; // 10^5 pulse = 1 meter �݌v�m�[�g p4 �i��j27000�p���X->270mm 10^2 �p���X 1mm(10^-3m) : 100�p���X1mm 1�p���X 100����1mm�i10^-5m�j=10�}�C�N�����[�g��  

		//�X�e�[�^�X�N���A
		status.Parameter = Actuator::WAITING;
		status.Movement = Actuator::STOP;
		status.Error = 0;

		//�p�����[�^������
		ParameterInit();

		//7443��̃\�t�g���~�b�g�ݒ�iSHDA02�̃\�t�g���~�b�g�Ɠ������D�\�t�g���~�b�g�̓�d���j		
		retCode = _7443_set_soft_limit(nb_axis, max_pulse, 0);
		retCode = _7443_enable_soft_limit(nb_axis, 2); // 1: �ً}��~, 2: ������~�i�̗p�j p168

		//���[�h�ݒ�
		mode = JOG;

		//���̑�������
		size = 0;
		curr = 0;
		flagContinuousOperationStart = false;
		flagJogOperationStart = false;
		flagUpdating = false;
	}

	~MLinear()
	{
		Close();
	}

	/*!
	 * \brief
	 * �A�������ݒ肷��D�A������́C�ʒu�w��E���x�w��̕ʂ�����̂�
	 * �����ł̓��[�h���m�肳���邱�Ƃ͏o���Ȃ��D
	 * 
	 * \param t
	 * true : �A�������ݒ�
	 * false: �A�����������
	 * 
	 */
	void SetDiscreteMove(bool t);

	/*!
	 * \brief
	 * �A������f�[�^�iD-V�j
	 *
	 * ���x��pps�P�ʂŕK�����̒l�C�����͑��΃p���X��[-max_pulse, +max_pulse]
	 * 
	 * \remarks
	 * ���̃f�[�^�́C�ʒu�w�肪��Έʒu�ɂȂ��Ă��Ȃ����߁C�l�Ԃ����Ă���������Ȃ��߁C����
	 * �N���X���ŁC��Ύw��f�[�^����ϊ����āC�p���T�[�n�[�h�E�F�A�ɍ����ɑ��荞�ނ��߂�
	 * ���O�v�Z�p�Ƃ��ėp��������́D
	 */
	typedef struct ContinuousControlDataStruct{
		int velocity; // pps;   [0, �ő呬�x]
		int distance; // pulse; [-max_pulse, +max_pulse]
	}ContinuousControlData;

	//7443���ԍ�
	int nb_axis;

	//�ő�p���X��
	int max_pulse;

	//�p���X�𑜓x
	int PulseByMeter;

	//���̃J�����g�X�e�[�^�X
	I16 retCode;

	//// �W���O����p
	//�����x��������p�����[�^�G���[�U�[�K�C�h4.1.4�@�݌v�m�[�g2�y�[�W
	int Dist;   //�C���N�������^�����[�h�ɂ��ړ�����
	int StrVel; //�ړ��J�n���x
	int MaxVel; //�ő呬�x
	double Tacc;//�ᑬ���獂���ւ̉������ԁi�b�j
	double Trun;//�����^�����ԁi�b�j
	double Tdec;//��������ᑬ�ւ̉������ԁi�b�j
	int SVacc;  //�������Ԃ̓����x���B���x�ipps�j
	int SVdec;  //�������Ԃ̓����x���B���x�ipps�j

	//���Ԏw��p�̃p�����[�^
	int Duration; //�ړ��Ɋ|���鎞�ԁi�~���b�j
	
	//���ꂼ��f�[�^���w�肳��Ă��邩�ǂ���
	bool flagPosition;
	bool flagDuration;
	bool flagVelocity;

	//// �W���O����p
	bool flagJogOperationStart;


	//// �A������p
	//�A������w��f�[�^
	QVector<ContinuousControlData> continuousControlData;
	//�A������ʒu
	int curr;
	//�A������x�N�g���T�C�Y
	int size;
	//�A������J�n�t���O
	bool flagContinuousOperationStart;
	//�A�b�v�f�[�g�������t���O
	bool flagUpdating;

	//// �^�]���샂�[�h�i�W���O���삩�A�����삩�j
	int mode;

	//// �f�o�b�O
	bool _debug;

	QVector<double> positionVector;
};

#endif