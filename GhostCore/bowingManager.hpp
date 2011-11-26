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

#ifndef __BOWING_MANAGER_HPP__
#define __BOWING_MANAGER_HPP__

#include "bowing.hpp"

#include <QObject>
#include <QString>
#include <QVector>

class BowingManager : public QObject
{
	Q_OBJECT

public:

	BowingManager();

	~BowingManager();

	//�����t����X�e�b�v����Ԃ�
	int GetPlayTime(){ return axis[0].size(); }

	//�{�[�C���O�P�ʂ�ǉ�����
	void AddBowing(Bowing *bow);

	//�{�[�C���O�P�ʂ��㏑������
	void UpdateBowing(Bowing *bow, int index);

	//�{�[�C���O�P�ʂ�Ԃ�
	Bowing* GetBowing(int index);

	//�{�[�C���O�P�ʂ��폜����
	void DelBowing(int index);

	//�v���y�A
	int Prepare();

	//�͈͎w��v���y�A
	int Prepare(int start, int end);

	//���s
	void Commit();

	//�����ړ�
	void InitialMove();

private:

	//�{�[�C���O�}�l�[�W���N���X���Ǘ�����{�[�C���O�N���X
	QVector<Bowing*> bowingVector;

	//�{�[�C���O�}�l�[�W���N���X���Ǘ�����{�[�C���O�N���X�����e����x�N�^
	
	//��0,1,2
	QVector<double> axis[3];
};


#endif