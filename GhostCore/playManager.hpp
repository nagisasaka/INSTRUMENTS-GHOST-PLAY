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

#ifndef __PLAY_MANAGER_HPP__
#define __PLAY_MANAGER_HPP__

#include "bowingManager.hpp"
#include "pitchManager.hpp"
#include "play/pitchunit.hpp"

class PlayTab;

class PlayManager : public QObject
{
	Q_OBJECT

	friend class PlayTab;

public:

	PlayManager();

	~PlayManager();

	int GetNbPlayUnit(){ return nbPlayUnit; }

	//�����ړ�
	void InitialMove1();
	void InitialMove2();

	//�Z�[�u
	void Save(QString filename);

	//���[�h
	int Load(QString filename);

	//���t�P�ʂ�o�^����
	void RegisterPlay(Bowing *bowing, PitchUnit *pu, int index);

	//���t�P�ʂ��폜����
	void DeletePlay(int index);
	
	//�o�^�S���t�P�ʂ��v���y�A
	int Prepare();

	//�͈͎w��v���y�A
	int Prepare(int start, int end);

	void Commit();

private:

	//���t�P�ʂ��X�V����
	void UpdatePlay(Bowing *bowing, PitchUnit *pu, int index);

	//���t�P�ʂ�ǉ�����
	void AddPlay(Bowing *bowing, PitchUnit *pu);

	//�{�[�C���O�}�l�[�W��
	BowingManager *bowingManager;

	//�s�b�`�}�l�[�W��
	PitchManager *pitchManager;

	//�ێ����Ă��鉉�t�P�ʐ�
	int nbPlayUnit;
};

#endif