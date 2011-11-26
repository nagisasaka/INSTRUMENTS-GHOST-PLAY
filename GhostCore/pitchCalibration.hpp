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

#ifndef __PITCHCALIBRATION_HPP__
#define __PITCHCALIBRATION_HPP__

#include "actuator/controller.hpp"
#include "audio/stringRecorder.hpp"
#include "audio/frequencyDomainProfile.hpp"
#include "audio/timeDomainProfile.hpp"

#include <QVector>
#include <QString>
#include <QThread>

class PitchCalibrationTab;

class PitchCalibration : public QThread
{
	friend class PitchCalibrationTab;

public:

	typedef struct CalibratedPositionsStrcut
	{
		int id; // ID
		QString stringName; // ����[G,D,A,E]
		QString pitchName;  // �s�b�`��
		int nb_pitchUnit;	// �s�b�`���j�b�g�ԍ�
		double z_value;		// Z���̒l�i���K���蕔�厲�j
		double c_value;		// C���̒l�i�T�[�{���[�^�[�̉�]���j

	}CalibratedPositions;

	PitchCalibration(int nb_pitchUnit);

	~PitchCalibration();

	void Save();

	void Save(double z_value, double c_value);

	void Load();

	void SetStringName(QString name){ currentString = name; }

	void SetPitchName(QString name){ currentPitch = name; }

	void Start(QString stringName);

	void Start(QString stringName, QString pitchName);

	QVector<PitchCalibration::CalibratedPositions> GetCalibratedPositions(){ return positions; }

	QString GetCurrentString(){ return currentString; }

	QString GetCurrentPitch(){ return currentPitch; }

	double GetRefFrequency()
	{
		if(currentString == "G"){
			return pg[currentPitch];
		}else if(currentString == "D"){
			return pd[currentPitch];
		}else if(currentString == "E"){
			return pe[currentPitch];
		}else if(currentString == "F"){
			return pa[currentPitch];
		}else{
			return 0;
		}
	}

protected:

	void run();

	void MakeStandardBowing();

	//�J�����g�̃L�����u���[�V�����Ώی�
	QString currentString;

	//�J�����g�̃L�����u���[�V�����Ώۉ��K
	QString currentPitch;

	//�L�����u���[�V�����Ώۃs�b�`���j�b�g�ԍ�
	int nb_pitchUnit;

	//�P�����C���S�̂̎������s���i���[�h�j
	int operationMode;

	//�L�����u���[�V�����σf�[�^
	QVector<PitchCalibration::CalibratedPositions> positions;

	QMap<QString, double> pg; // G����̃|�W�V����
	QMap<QString, double> pd; // D����̃|�W�V����
	QMap<QString, double> pe; // E����̃|�W�V����
	QMap<QString, double> pa; // A����̃|�W�V����

	//�L�����u���[�V�����p�{�[�C���O�f�[�^
	//�|��
	QVector<double> pressure;
	//�|��
	QVector<double> bowposition;
};


#endif