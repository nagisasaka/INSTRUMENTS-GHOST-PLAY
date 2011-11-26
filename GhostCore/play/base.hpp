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

#ifndef __BASE_HPP__
#define __BASE_HPP__

#include <QVector>

class Pressure;
class Igen;
class BaseGeometry;
class Calibration;

/*!
 * \brief
 * �^�|�x�[�X�N���X
 *
 * �^�|�Ɋւ����؂��Ǌ�����
 * 
 */
class Base
{
	friend class Bowing;
	friend class BowingSimulation;

public:

	Base();

	~Base();

	//���t�I�����ɐڂ��Ă���|�W�V�������w�肷��
	void SetStringPair(int start_string, int end_string);

	//���t�����w�肷��
	void SetPlayerString(int nb_string);
	
	//���t�����擾����
	int GetPlayerString(){ return PlayerString; }

	//�|���x�N�^�[���w�肷��(10msec��)
	void SetPressureVector(QVector<double> p);

	//�|���x�N�^�[���擾����(�Z�b�g���ɑ΂��ăT�C�Y�͕ς���Ă���\��������)
	QVector<double> GetPressureVector();

	//�v���y�A
	int Prepare();

	//�R�~�b�g
	void Commit();

	//�����ړ�
	void InitialMove(int nb_string);

private:

	//�|���Ǘ��N���X
	Pressure *pressure;

	//�ڌ��Ǘ��N���X�i�����j
	Igen *igen;

	//�x�[�X�W�I���g��
	BaseGeometry *geometry;

	//�L�����u���[�V����
	Calibration *calibration;

	//���t�J�n���ʒu
	int StartString;

	//���t��
	int PlayerString;

	//���t�I�����ڐG���ʒu
	int EndString;
	
	//�A�N�`���G�[�^����x�N�^
	//��0
	QVector<double> axis0;
	//��1
	QVector<double> axis1;
};

#endif