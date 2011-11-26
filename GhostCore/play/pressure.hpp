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

#ifndef __PRESSURE_HPP__
#define __PRESSURE_HPP__

#include <QVector>

class Calibration;
class BaseGeometry;

/*!
 * \brief
 * �{�[�C���O���̋|���Ǘ��N���X
 * 
 * ��0�Ǝ�1�̋�������ɂ��A�|�������肷�邱�Ƃ��ł��邪�A���̃N���X�ł́A�|����0%�`100%�܂łɐ��K����
 * ��0�Ǝ�1�̒��ڊǗ����B������B
 * 
 */
class Pressure
{
	friend class Base;

private:

	Pressure();

	~Pressure();

	//�J�n�ʒu�E�I���ʒu���w�肷��
	void SetStringPair(int _startString, int _endString);

	//���͕t���Ώی���ݒ肷��i���t�����w�肷��j
	void SetPlayerString(int _playerString);

	//���̓x�N�^�i���͌n��j���Z�b�g����i���̓x�N�^��[0,100]���K���C��Βl�w��j
	void SetPressureVector(QVector<double> v);

	//�p�[�Z���g��mm�P�ʂɕϊ�����
	double ConvertToMM(double percent);

	//���͕t���p�̎�0,1�̐���x�N�g�����v�Z����
	void Prepare(QVector<double> &axis0, QVector<double> &axis1);

	//// �P�����t

	//�E�ֈ��͕t��
	void ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//�����ֈ��͕t��
	void ToBothSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//���ֈ��͕t��
	void ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//// �ڌ����썞���t

	//�E�ֈڌ����Ȃ��爳�͕t��
	void ToRightSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//���ֈڌ����Ȃ��爳�͕t��
	void ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//�P���ւ̕��s�ړ��������ł̈��͕t��
	void SinglePressure(QVector<double> &axis0, QVector<double> &axis1, int nb_string);

	//���͕t���Ώی�
	int playerString;

	//�J�n�ʒu
	int startString;

	//�I���ʒu
	int endString;

	//���͌n��
	QVector<double> pressureVector;

	//�L�����u���[�V�����N���X
	Calibration *calibration;

	//�x�[�X�W�I���g��
	BaseGeometry *geometry;

	//�ő剟�����ݕ�
	static const double MaxMM;
};

#endif