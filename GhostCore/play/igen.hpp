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

#ifndef __IGEN_HPP__
#define __IGEN_HPP__

#include <QVector>

class BaseGeometry;
class Calibration;

/*!
 * \brief
 * �ڌ��Ǘ��N���X�̎����D
 * 
 * ���t���ڌ��͂��̃N���X�ł͊Ǘ����Ȃ��D�����ʒu�ԁi�c�]�|�������j�̈ڌ��݂̂��Ǘ�����D
 * 
 * \remarks
 * ���t���ڌ��͂��̃N���X�ł͊Ǘ����Ȃ����Ƃɒ��ӁD
 * 
 */
class Igen
{
	friend class Base;

private:

	Igen();

	~Igen();

	//�ڌ��̏����ʒu�ƏI���ʒu��ݒ�
	void SetStringPair(int _startString, int _endString);

	//���t�����w��
	void SetPlayerString(int _playerString);

	//�v���y�A
	void Prepare(QVector<double> &axis0, QVector<double> &axis1);

	//���ֈڌ�
	void ToLeft(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//���ֈڌ��i�V�����j
	void ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int nb_string);

	//�E�ֈڌ�
	void ToRight(QVector<double> &axis0, QVector<double> &axis1,int nb_string);

	//�E�ֈڌ�
	void ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int nb_string);

	//�����ʒu
	int startString;

	//�I���ʒu
	int endString;

	//���t��
	int playerString;

	//�x�[�X�W�I���g��
	BaseGeometry *geometry;

	//�L�����u���[�V�����N���X
	Calibration *calibration;

	//�ڌ��̎��s���ԁi��i�K������j[ms]
	static const int DurationTime;
};

#endif