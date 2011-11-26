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

#include "playManager.hpp"
#include "error.hpp"

#include <QFile>
#include <QTextStream>

#include <iostream>

PlayManager::PlayManager()
{
	bowingManager = new BowingManager();
	pitchManager = new PitchManager();

	nbPlayUnit = 0;
}

PlayManager::~PlayManager()
{
	delete bowingManager;
	delete pitchManager;
}


void PlayManager::Save(QString filename)
{
	//�}�l�[�W���N���X�ɓo�^����Ă���I�u�W�F�N�g��toString()���܂Ƃ߂Ĕ��s���Ă܂Ƃ߂ĕԂ�toString()�֐���
	//�������Ă��ǂ����Cbowing��pitch�����݂Ƀt�@�C�����ɏ����o�����ق���readable�ł��邽�߁C�b��I�Ɏ������Ȃ�

	//�S���t�P�ʐ���ۑ�
	QString writer = QString();
	for(int i=0;i<nbPlayUnit;i++){
		QString writer_bowing = bowingManager->GetBowing(i)->toString();
		QString writer_pitch  = pitchManager->GetPitch(i)->toString();
		writer.append(writer_bowing);
		writer.append(writer_pitch);
	}

	//�t�@�C���ɏ������Ƃ�
	QFile file(filename);
	if(!file.open(QFile::WriteOnly|QFile::Truncate)){
		Error::Critical(0, QObject::tr("[ PLAYMANAGER ] Save(), could not open save file :").append(filename));
	}

	QTextStream out(&file);
	out << writer;

	std::cout << "[ �v���C�}�l�[�W�� ] �ۑ����܂���: " << filename.toStdString() << std::endl;
}


int PlayManager::Load(QString filename)
{
	//�t�@�C�����牉�t�P�ʂ�ǉ����Ă���

	//�����̃}�l�[�W���N���X���폜
	delete bowingManager;
	delete pitchManager;

	//�V�����}�l�[�W���N���X���\�z
	bowingManager = new BowingManager();
	pitchManager = new PitchManager();

	//�������ɂ��o�^���t�P�ʐ����[����
	nbPlayUnit = 0;

	//�t�@�C���I�[�v��
	QFile file(filename);
	if(!file.open(QFile::ReadOnly)){
		Error::Critical(0, QObject::tr("[ PLAYMANAGER ] Load(), could not open save file:").append(filename));

	}

	//�{�[�C���O�v�f�̏������p�f�[�^
	QStringList bowingInitData;
	//�s�b�`���j�b�g�̏������p�f�[�^
	QStringList pitchInitData;
	//���C���J�E���^
	int lineCounter = 0;
	//�X�g���[���I�[�v��
	QTextStream in(&file);
	while(!in.atEnd()){
		//�e�s�ɂ���
		QString line = in.readLine(); // \n���g��������邱�Ƃɒ���
		if(lineCounter < 4){
			//0,1,2,3
			bowingInitData.append(line);
		}else if(lineCounter == 4){
			//4
			pitchInitData.append(line);
		}else if(lineCounter == 5){
			//5		
			pitchInitData.append(line);

			//�f�[�^���W�܂���

			//�I�u�W�F�N�g�\�z
			Bowing *bow = new Bowing(bowingInitData);
			PitchUnit *pu = new PitchUnit(pitchInitData);

			//���̃N���X�֓o�^
			AddPlay(bow,pu);

			//����
			lineCounter = 0;
			bowingInitData.clear();
			pitchInitData.clear();
			continue;
		}
		lineCounter++;		
	}

	std::cout << "[ �v���C�}�l�[�W�� ] " << nbPlayUnit << "���t�P�� ���[�h���܂���: " << filename.toStdString() << std::endl;
	return nbPlayUnit;
}


void PlayManager::RegisterPlay(Bowing *bowing, PitchUnit *pu, int index)
{
	if(index == nbPlayUnit){

		AddPlay(bowing, pu);

	}else if(index < nbPlayUnit){

		UpdatePlay(bowing, pu, index);

	}else{

		//UI��̖��Œ���т��悤�Ƃ��Ă��܂���
		Error::Warning(0, QObject::tr("�o�^�C���f�b�N�X���A�����Ă��܂���"));
		return;

	}
}


void PlayManager::UpdatePlay(Bowing *bowing, PitchUnit *pu, int index)
{
	//���t�P�ʂ��X�V����
	std::cout << "[ �v���C�}�l�[�W�� ] #" << index << " ���X�V���܂�." << std::endl;

	int playTime = bowing->Prepare();
	pu->SetPlayTime(playTime);
	pitchManager->UpdatePitch(pu,index);
	bowingManager->UpdateBowing(bowing,index);
}


void PlayManager::DeletePlay(int index)
{
	//���t�P�ʂ��폜����
	std::cout << "[ �v���C�}�l�[�W�� ] #" << index << " ���폜���܂�." << std::endl;

	//�s�b�`�P�ʂ��폜
	pitchManager->DelPitch(index);
	//�{�[�C���O�P�ʂ��폜
	bowingManager->DelBowing(index);

	//�o�^���t�P�ʐ������炷
	nbPlayUnit--;
}


/*!
 * \brief
 * ���t�P�ʂ�ǉ�����D
 * 
 * \param bowing
 * �{�[�C���O�N���X
 * 
 * \param pu
 * �s�b�`���j�b�g�N���X
 * 
 */
void PlayManager::AddPlay(Bowing *bowing, PitchUnit *pu)
{
	//* ���t�P�ʂ�ǉ����� *
	//�{�[�C���O�͐��䎞�ԑS���̐���l���܂܂�Ă���̂Ŗ������ŗǂ����C�s�b�`���j�b�g�̃^�C�~���O��
	//�{�[�C���O�̃^�C�~���O�ƍ��킹�񂾌`�Ő��䎞�ԑS���𖄂ߐs��������l�𐶐�������j�D
	std::cout << "[ �v���C�}�l�[�W�� ] �ǉ����܂��D" << std::endl;

	//���t���ԁi�X�e�b�v�j
	int playTime = bowing->Prepare();

	//�s�b�`�}�l�[�W���ɒǉ�
	pu->SetPlayTime(playTime);  //���Ԃ��w��
	pitchManager->AddPitch(pu); //�s�b�`�}�l�[�W���ɒǉ�

	//�{�[�C���O�}�l�[�W���ɒǉ�
	bowingManager->AddBowing(bowing);

	//�o�^���t�P�ʐ����C���N�������g
	nbPlayUnit++;
}

int PlayManager::Prepare()
{
	std::cout << "[ �v���C�}�l�[�W�� ] �v���y�A�D" << std::endl;
	int playTime = bowingManager->Prepare();
	pitchManager->Prepare();

	//���t���Ԃ�Ԃ�
	return playTime;
}

int PlayManager::Prepare(int start, int end)
{
	std::cout << "[ �v���C�}�l�[�W�� ] �͈͎w��v���y�A�D" << std::endl;
	int playTime = bowingManager->Prepare(start, end);
	pitchManager->Prepare(start, end);

	//���t���Ԃ�Ԃ�
	return playTime;
}

void PlayManager::Commit()
{
	std::cout << "[ �v���C�}�l�[�W�� ] �R�~�b�g�D" << std::endl;
	bowingManager->Commit();
	pitchManager->Commit();
}

void PlayManager::InitialMove1()
{
	bowingManager->InitialMove();
	//pitchManager->InitialMove();
}

void PlayManager::InitialMove2()
{
	//bowingManager->InitialMove();
	pitchManager->InitialMove();
}
