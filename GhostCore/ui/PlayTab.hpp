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

#ifndef __PLAY_TAB_HPP__
#define __PLAY_TAB_HPP__

#include <iostream>
#include <QObject>

class GhostCore;
class Bowing;
class PitchUnit;
class PlayManager;

class PlayTab : public QObject
{
	Q_OBJECT

public:

	PlayTab(GhostCore *parent);

	~PlayTab();

	void Update();

private slots:

	//���t�f�[�^�ۑ��t�@�C�����J��
	void OpenFile();

	//�ۑ�����
	void Save();

	//�ʕۑ�����
	void Apply();

	//���̉�ʂɍs��
	void Next();

	//�O�̉�ʂɍs��
	void Back();

	//���ׂĉ��t
	void PlayAll();

	//��ʂ��牉�t
	void PlayFromThisScreen();

	//���̉�ʂ܂ŉ��t
	void PlayTillThisScreen();

	//�ʉ��t
	void Play();

	//�v���y�A
	void PrepareAll();

	//���̉�ʂ���v���y�A
	void PrepareFromThisScreen();

	//���̉�ʂ܂Ńv���y�A
	void PrepareTillThisScreen();

	//�����ړ�
	void InitialMove1();
	void InitialMove2();

	//�|���f�[�^�ύX
	void PressureDataChanged();

	//�|���f�[�^�ύX
	void PositionDataChanged();

private:

	//�v���y�A�����ʏ���
	void PrepareCommon();

	//�C���f�b�N�X�̕ύX������
	void IndexChange();

	//�I�u�W�F�N�g����ݒ�y�C�������
	void SetupPane(Bowing *bow, PitchUnit *pu, int nbPane);
	void ClearPane(int nbPane);
	
	//�ݒ�y�C������{�[�C���O�I�u�W�F�N�g�����
	Bowing* GenerateBowing(int nbPane);

	//�ݒ�y�C������s�b�`���j�b�g�I�u�W�F�N�g�����
	PitchUnit* GeneratePitchUnit(int nbPane);

	GhostCore* parent;

	QString fileName;

	//�v���C�}�l�[�W��
	PlayManager *playManager;

	//���݂̉�ʃC���f�b�N�X�i�����Ƃ����̔ԍ��j
	int index;

	//�o�^�ς݂̉��t���j�b�g���i���͍ς݂ł͂Ȃ��o�^�ς݁j
	int nbPlayUnit;

	//�����t����
	int playTime;
};

#endif