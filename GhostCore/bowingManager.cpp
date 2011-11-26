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

#include "bowingManager.hpp"
#include "error.hpp"
#include "actuator/controller.hpp"

BowingManager::BowingManager()
{

}

BowingManager::~BowingManager()
{
	//�o�^����Ă���S�Ẵ{�[�C���O�N���X���폜����
	for(int i=0;i<bowingVector.size();i++){
		delete bowingVector[i];
	}
}

void BowingManager::InitialMove()
{
	//�v���y�A��Ɏ��s����邱�Ƃ��K�{

	//�v���y�A���ꂽ�ŏ��̎��ʒu�ֈړ�
	int duration = 2000;
	Controller *control = Controller::GetInstance();
	for(int i=0;i<3;i++){
		control->axis[i]->SetMode(MLinear::JOG);
		control->axis[i]->SetPosition(axis[i].at(0));
		control->axis[i]->SetDuration(duration);
		control->axis[i]->Prepare();
	}
	for(int i=0;i<3;i++){
		control->axis[i]->Commit();
	}

	//���ڂɑ҂�
	Sleep(duration+500);
}


void BowingManager::AddBowing(Bowing *bow)
{

	//// �G���[�`�F�b�N�������ɓ����Ă��ǂ� ////

	bowingVector.append(bow);
}


void BowingManager::DelBowing(int index)
{
	if(index >= bowingVector.size()){
		std::cout << "[ �{�[�C���O�}�l�[�W�� ] �폜�C���f�b�N�X���o�^�x�N�^�T�C�Y�𒴂��Ă��܂�." << std::endl;
		Error::Critical(0, QObject::tr("[ BowingManager ] Invalid delete index"));
	}

	//�폜
	bowingVector.remove(index);
}


void BowingManager::UpdateBowing(Bowing *bow, int index)
{
	if(index >= bowingVector.size()){
		std::cout << "[ �{�[�C���O�}�l�[�W�� ] �X�V�C���f�b�N�X���o�^�x�N�^�T�C�Y�𒴂��Ă��܂�." << std::endl;
		Error::Critical(0, QObject::tr("[ BowingManager ] Invalid update index"));
	}

	//index�Ԗڂɒǉ�
	bowingVector.insert(index, bow);

	//�����o���ꂽindex+1�Ԗڂ��폜
	bowingVector.remove(index+1);
}


Bowing* BowingManager::GetBowing(int index)
{
	if(index >= bowingVector.size()){
		std::cout << "[ �{�[�C���O�}�l�[�W�� ] �擾�C���f�b�N�X���o�^�x�N�^�T�C�Y�𒴂��Ă��܂�." << std::endl;
		Error::Critical(0, QObject::tr("[ BowingManager ] Invalid get index"));
	}

	return bowingVector.at(index);
}


int BowingManager::Prepare()
{
	int start = 0;
	int end = bowingVector.size();

	return Prepare(start,end);
}



/*!
 * \brief
 * �o�^����Ă���S�Ẵ{�[�C���O�P�ʂɂ��ăo���f�[�V�������Ȃ���C�e���ɂ��āC�ЂƂ���x�N�g���֌�������D
 * 
 * \returns
 * ���t�X�e�b�v��
 * 
 */
int BowingManager::Prepare(int start, int end)
{
	//�S�{�[�C���O�P�ʂ��v���y�A���Đ���x�N�g�����������ăo���f�[�V��������

	//�ŏ��̃{�[�C���O�P�ʂ͓��ʈ���
	Bowing *bow = bowingVector.at(start);
	//�v���y�A�ς�O�񂷂邱�Ƃ͒����ɔ����Ă���̂ŁC�����ł��ꉞ�v���y�A���Ă����i�����炭�O�i�K�ŉ��x���v���y�A����Ă��邾�낤���G�����Ă��̂悤�ȏ璷�v���y�A�͖��ʂ����j
	bow->Prepare();
	//�e���̐���l���擾
	for(int i=0;i<3;i++){
		//�����ŏ㏑�����Ă���̂Ŏ��O�N���A�͕K�v�Ȃ���
		axis[i] = bow->GetPositionVector(i);
	}
	if(axis[0].size() != axis[1].size()){
		std::cout << "[ �{�[�C���O�}�l�[�W�� ] 0��, ��0�Ǝ�1�̐���x�N�g��������v���܂���: axis0.size() = " << axis[0].size() << ", axis1.size() = " << axis[1].size() << std::endl;
		Error::Critical(0, QObject::tr("[ �{�[�C���O�}�l�[�W�� ] ��0�Ǝ�1�̐���x�N�g���l����v���܂���D"));
	}

	//��Ԗڂ̃{�[�C���O�P�ʂ���ǉ��������Ă���
	for(int i=start+1;i<end;i++){
		Bowing *bow = bowingVector.at(i);
		bow->Prepare();
		//�e���̐���x�N�g���ɒǉ�����
		for(int k=0;k<3;k++){
			QVector<double> temp = bow->GetPositionVector(k);
			if(temp[0] != axis[k][axis[k].size()-1]){
				//�ڑ��l�̘A���`�F�b�N
				std::cout << "[ �{�[�C���O�}�l�[�W�� ] ��" << k << ", "<< i << "�Ԗڂ�" << i-1 << "�Ԗڂ̍Ō�ƍŏ��̐���l����v���܂���." << std::endl;
				std::cout << "temp[0,1,2] = " << temp[0] << "," << temp[1] << "," << temp[2] << std::endl;
				std::cout << "axis[" << k << "][" << axis[k].size()-3 << "," << axis[k].size()-2 << "," <<  axis[k].size()-1 << "] = " <<  axis[k][axis[k].size()-3] << "," << axis[k][axis[k].size()-2] << ", " << axis[k][axis[k].size()-1] << std::endl;
				Error::Critical(0, QObject::tr("[ BowingManager ] Last position value of #%1 and first position value of #%2 is not the same").arg(i-1).arg(i));
			}
			for(int j=0;j<temp.size();j++){
				axis[k].append(temp.at(j));
			}
		}
		//�T�C�Y�`�F�b�N
		if(axis[0].size() != axis[1].size()){
			std::cout << "[ �{�[�C���}�l�[�W�� ] " << i << "��, ��0�Ǝ�1�̐���x�N�g��������v���܂���: axis0.size() = " << axis[0].size() << ", axis1.size() = " << axis[1].size() << std::endl;
			Error::Critical(0, QObject::tr("[ BowingManager ] axis0.size() != axis1.size() error"));
		}
	}

	//���������e���̐���x�N�g�����Z�b�g�E�v���y�A
	Controller *control = Controller::GetInstance();
	for(int i=0;i<3;i++){
		control->axis[i]->SetMode(MLinear::CONTINUOUS);
		control->axis[i]->SetPositionVector(axis[i]);
		control->axis[i]->Prepare();
	}

	//�T�C�Y��Ԃ�
	return axis[0].size();
}

/*!
 * \brief
 * �o�^����Ă���S�Ẵ{�[�C���O�P�ʂɂ��ĉ��t�����s����
 * 
 */
void BowingManager::Commit()
{
	std::cout << "[ �{�[�C���O�}�l�[�W�� ] �R�~�b�g." << std::endl;
	Controller *control = Controller::GetInstance();
	for(int i=0;i<3;i++){
		control->axis[i]->Commit();
	}
}

