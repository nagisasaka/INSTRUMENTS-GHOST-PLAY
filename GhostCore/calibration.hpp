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

#ifndef __CALIBRATION_HPP__
#define __CALIBRATION_HPP__

#include <QObject>
#include <QSettings>
#include <QList>
#include <QThread>

#include <iostream>

class CalibrationTab;

/*!
 * \brief
 * �L�����u���[�V�����N���X
 * ���ʒu�i��0,��1�j�L�����u���[�V�����ƁC���K�i���e���V�����j�L�����u���[�V���������s
 * 
 * \see
 * calibration.cpp | vecmath.hpp
 */
class Calibration : public QThread
{
	Q_OBJECT

	friend class CalibrationTab;

signals:
	void UpdateCalibratedPosition();
	void CalibrationEnd();

public slots:

	/*!
	 * \brief
	 * �e���̉^�]�I���V�O�i�����ڑ������X���b�g
	 *
	 * \param uid
	 * ��UID
	 *
	 */
	void Repeat(int uid);

public:

	/*!
	 * \brief
	 * �Ώی��i�V�j
	 * 
	 */
	enum{
		STRING_1 ,
		STRING_12,
		STRING_2 ,
		STRING_23,
		STRING_3 ,
		STRING_34,
		STRING_4
	};

	/*!
	 * \brief
	 * ��0,1,2�̊֌W���܂Ƃ߂ċL������\����
	 * 
	 * \remarks
	 * ��0,1�̃|�W�V�����ɂ���Ă͎�2��[0,1]�Ŏg���؂邱�Ƃ͂ł����C�S�|�ł̃{�[�C���O�͂ł��Ȃ��C
	 * �܂�u�@�\��̑S�|�v�́C[0,1]���C���Ȃ萧������邱�Ƃɒ��ӂ��邱�ƁD
	 * 
	 */
	typedef struct PositionsStruct
	{
		//��2�n�_�i�{�[�C���O�J�n�_�j�ł̎�0,1,2�̈ʒu
		QList<double> start;
		//��2�I�_�i�{�[�C���O�I���_�j�ł̎�0,1,2�̈ʒu
		QList<double> end;
	}Positions;//��1,2,3,4�i4�j �������́C��1,1-2,2,2-3,3,3-4,4�i7�j

	/*!
	 * \brief
	 * �L�����u���[�V�����N���X�̃R���X�g���N�^�D�Z�[�u�����L�����u���[�V�����֘A����S�ă��[�h����D
	 * 
	 */
	Calibration();

	~Calibration(){}

	/*!
	 * \brief
	 * �L�����u���[�V���������ݒ�����[�h����D�L�����u���[�V�����͖�����s����邱�Ƃ��D�܂����D
	 * 
	 * \remarks
	 * INI�t�@�C���̓����\�������ʁDPosition�\���̂�metatype�ɋL�ڂ���QVariant�Ŏ�舵����悤�ɂ���Ȃǌ��ʂ���
	 * �ǂ��������D
	 * 
	 * \see
	 * Save() | Calibration()
	 */
	void Load();

	/*!
	 * \brief
	 * �L�����u���[�V���������ݒ�̃Z�[�u�D�f�X�g���N�^�Ŏ����I�ɌĂ΂��D
	 * 
	 * \remarks
	 * INI�t�@�C���̓����\�������ʁDPosition�\���̂�metatype�ɋL�ڂ���QVariant�Ŏ�舵����悤�ɂ���Ȃǌ��ʂ���
	 * �ǂ��������D
	 * 
	 * \see
	 * Load() | ~Calibration()
	 */
	void Save();
	
	/*!
	 * \brief
	 * �ʒu�L�����u���[�V����
	 * 
	 * \param nb_string
	 * ���ԍ�[1,7]. 0�X�^�[�g�ł͂Ȃ����Ƃɒ��ӁD�L�����u���[�V�������s�Ώۃt�@�C���D
	 *
	 * \throws <exception class>
	 * Description of criteria for throwing this exception.
	 *  
	 * \remarks
	 * ��2�i�^�|���j�ɂ��ẮC�L�����u���[�V�����ł��Ȃ��̂ŏ����l�����̂܂ܗ��p���邱�ƂɂȂ�D
	 * �����l�ŏ\�����ӂ��Ėڎ��ɂ��l��^���Ȃ���΂Ȃ�Ȃ��D
	 * ����͋@�\�݌v��̉��P���ڂł���C���炩�̃G���h�Z���T�[�̒ǉ����]�܂����ƍl������D
	 * 
	 */
	void Start(int nb_string);

	//�蓮��~
	void Stop();

	/*!
	 * \brief
	 * �����ʒu���Z�b�g����
	 * 
	 * \param nb_string
	 * ���ԍ�[1,4]�D0�X�^�[�g�ł͂Ȃ����Ƃɒ��ӁD�����ʒu��UI�ƌ�������Ă���D
	 * 
	 * \remarks
	 * ���ԍ�[1,4]�Ŏw��D0�X�^�[�g�ł͂Ȃ����Ƃɒ��ӁD
	 * 
	 */
	void SetInitialPosition(int nb_string, Calibration::Positions position)
	{
		initialPositions[nb_string-1] = position;
	}

	/*!
	 * \brief
	 * �L�����u���[�V�����ψʒu���Z�b�g����
	 * 
	 * \param nb_string
	 * ���ԍ�[1,7]�D0�X�^�[�g�ł͂Ȃ����Ƃɒ��ӁD�L�����u���[�V�����ψʒu�͊�{�I�Ɏ����L�����u���[�V����
	 * �ɂ���āCcalibration�N���X���Őݒ肳��邪�C�L�����u���[�V�������ʂɑ΂��Ĕ�������������ꍇ�́C
	 * UI�ƌ�������Ă����ق������֐������邩�D
	 * 
	 * \remarks
	 * ���ԍ�[1,7]�́C���ꂼ��1,1-2,2,2-3,3,3-4,4�ɑΉ��D
	 * 
	 */
	void SetCalibratedPosition(int nb_string, Calibration::Positions position)
	{
		positions[nb_string-1] = position;
	}

	/*!
	 * \brief
	 * �w�肳�ꂽ���ɂ��Ă̏����ʒu���擾
	 * 
	 * \param nb_string
	 * �w�茷[1,4]
	 * 
	 * \returns
	 * �w�肳�ꂽ���ɂ��Ă̏����ʒu�istart:��0,1,2 end:��0,1,2�j
	 * 
	 * \remarks
	 * start��end�Ŏ�0,1�ɂ��Ă͓����l�ł��邱�Ƃɒ��ӁC��2�ɂ��Ă̂݃{�[�C���O�n�_�E�I�_�w��D
	 *
	 */
	Calibration::Positions GetInitialPositions(int nb_string)
	{
		return initialPositions[nb_string-1];
	}

	/*!
	 * \brief
	 * �S���ɂ��Ă̏����ʒu���擾
	 * 
	 * \returns
	 * �S���ɂ��Ă̏����ʒu
	 * 
	 */
	QList<Calibration::Positions> GetInitialPositions()
	{
		return initialPositions; 
	}

	/*!
	 * \brief
	 * �w�肳�ꂽ���ɂ��ẴL�����u���[�V�����ψʒu���擾
	 * 
	 * \param nb_string
	 * �w�茷[1,7]
	 * 
	 * \returns
	 * �w�肳�ꂽ���ɂ��ẴL�����u���[�V�����ψʒu�istart:��0,1,2,end:��0,1,2�j
	 * 
	 * \remarks
	 * start��end�Ŏ�0,1�ɂ��Ă̓L�����u���[�V�����ɂ�蒲�������D��2�ɂ��Ă͏����ʒu�w��̂܂܁D
	 * 
	 */
	Calibration::Positions GetCalibratedPositions(int nb_string)
	{
		return positions[nb_string-1];
	}

	/*!
	 * \brief
	 * �S���ɂ��ăL�����u���[�V�����ψʒu���擾
	 * 
	 * \returns
	 * �S���ɂ��ẴL�����u���[�V�����ψʒu
	 * 
	 */
	QList<Calibration::Positions> GetCalibratedPositions()
	{ 
		return positions; 
	}

protected:

	void run();
	int nbString;

private:

	void SoundCheck();

	//�����ʒu���Z�[�u����
	void SaveInitialPosition();

	//�L�����u���[�V�����ψʒu�����[�h����
	void SaveCalibratedPosition();

	//�����ʒu�����[�h����
	void LoadInitialPosition();

	//�L�����u���[�V�����ψʒu�����[�h����D
	void LoadCalibratedPosition();

	// �L�����u���[�V�����ψʒu�̔z��̗v�f��
	//
	// 0    1      2    3      4    5      6
	// ��0, ��0-1, ��1, ��1-2, ��2, ��2-3, ��3
	// 0           1           2           3
	// �e�L�����u���[�V�����Ώۂ̎�0,1,2�̎n�_�ƏI�_
	QList<Calibration::Positions> positions;
	// �������ʒu
	QList<Calibration::Positions> initialPositions;

	//�L�����u���[�V�����蓮��~�t���O
	bool manualStopFlag;

	//�f�o�b�O
	int _debug;

};

#endif