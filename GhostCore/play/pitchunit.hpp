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

#ifndef __PITCH_UNIT_HPP__
#define __PITCH_UNIT_HPP__

#include <QThread>
#include <QString>
#include <QVector>

class PitchCalibration;

/*!
 * \brief
 * ���K����N���X
 *
 * ���K���蕔�i�s�b�`���j�b�g�j�̈�؂��Ǌ�
 * 
 */
class PitchUnit : public QThread
{
	friend class PitchCalibration;
	friend class PitchCalibrationTab;

public:

	PitchUnit(int nb_unit);

	PitchUnit(QStringList initData);

	~PitchUnit();

	//�ۑ��\�ȕ������Ԃ��i����String�̈Ӗ��͕�����D���ł͂Ȃ��j
	QString toString();

	//�f�t�H���g�[���i�f�t�H���g�w��̏ꍇ�͎��Ԃ���������邱�Ƃɒ��Ӂj�G�����Ŏw�肳�ꂽ���Ԃ܂Ńp�f�B���O�����
	void SetPlayTime(int _playTime){ prepared = false; playTime = _playTime; }

	//�f�t�H���g�I�t
	void SetVibrateFlag(bool t){ prepared = false; vibrateFlag = t; }

	//�f�t�H���g�I�t
	void SetRigenFlag(bool t){ prepared = false; rigenFlag = t; }

	void SetVibrateHz(double hz){ prepared = false; vibrateHz = hz; }

	void SetVibratePercent(double percent){ prepared = false; vibratePercent = percent; }

	int GetPlayTime(){ return playTime; }

	bool GetVibrateFlag(){ return vibrateFlag; }

	bool GetRigenFlag(){ return rigenFlag; }

	void SetTransitionTime(int step){ prepared = false; transitionTime = step; }

	int GetTransitionTime(){ return transitionTime; }

	double GetVibrateHz(){ return vibrateHz; }

	double GetVibratePercent(){ return vibratePercent; }

	//���䌷�ԍ��Ŏw��
	void SetString(int nb_string);

	//�����Ŏw��
	void SetStringName(QString name){ stringName = name; }

	void SetPitch(QString pitchName);

	void CalibrationReLoad();

	int Prepare();

	int Prepare(double lastPosition);

	void Commit();

	int GetNbUnit(){ return nb_unit; }

	QString GetStringName(){ return stringName; }

	int GetStringCode();

	QString GetPitchName(){ return pitchName; }

	double GetCurrentPosition();

	void SetContactDelay(double delay){ contactDelay = delay; }

	double GetContactDelay(){ return contactDelay; }

	//// �Ή����̓I�u�W�F�N�g�Ɉ�ӂȂ̂�bowing�N���X�̂悤�Ɉ����͖���

	//�厲�i�s�b�`����j�̐���x�N�g����Ԃ�
	QVector<double> GetPositionVector(){ return pitchSequence; }

	//��񎲁i���I���j�̐���x�N�g����Ԃ�
	QVector<int> GetStringSelectSequence(){ return stringSelectSequence; }

	//��O���i�������j�̐���x�N�g����Ԃ�
	QVector<int> GetPushpullSequence(){ return pushpullSequence; }

	//�Ή������擾
	int GetNbAxis(){ return nb_axis; }

	//�Ή��T�[�{ID���擾
	int GetNbServo(){ return nb_servo; }

	//IAIC���擾
	int GetNbIaic(){ return nb_iaic; }
	
	//�W���O����
	void Jog(double z_value, double z_time, double c_value);

	//�W���O����
	void JogGoToString(QString stringName);

	//�@�\�ޔ�
	void Evacuate();

	//�W���O����i�Z�b�g�z�[���j
	//void SetHome();

	//�v���y�A�ς��H
	bool isPrepared(){ return prepared; }

protected:

	void run();

private:

	//�r�u���[�g�V�[�P���X��ǉ����C�ǉ�����������Ԃ�
	int MakeVibrate(int togo);

	//���j�b�g�ԍ�
	int nb_unit;

	//���̃��j�b�g�����p���郊�j�A���ԍ�
	int nb_axis;

	//���̃��j�b�g�����p���鉟���ݎ��ԍ�
	int nb_servo;

	//���̃��j�b�g�����p����IAI�A�N�`���G�[�^�ԍ�
	int nb_iaic;

	//�ݒ肳�ꂽ����
	QString stringName;

	//�ݒ肳�ꂽ������
	QString pitchName;

	//�L�����u���[�V�����N���X
	PitchCalibration *calibration;

	//�r�u���[�g�t���O
	bool vibrateFlag;

	//�����J�ڃt���O
	bool rigenFlag;

	//�J�ڎ���
	int transitionTime;

	//�ێ�����
	int playTime;

	//�r�u���[�g���g��
	double vibrateHz;

	//�r�u���[�g���l��
	double vibratePercent;

	//�������i�R���^�N�g�j�J�n�܂ł̃f�B���C�i�X�e�b�v���j
	double contactDelay;

	//�������ׂ��^�[�Q�b�g
	double c_value;

	double z_value;

	//�厲�̐���x�N�g��
	QVector<double> pitchSequence;

	//��񎲂̐���x�N�g���i���I���j
	QVector<int> stringSelectSequence;

	//��O���̐���x�N�g���i�������j
	QVector<int> pushpullSequence;

	//�v���y�A�σt���O
	bool prepared;
};

#endif