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

#include "bowing.hpp"
#include "play/base.hpp"
#include "actuator/controller.hpp"

#include <iostream>

Bowing::Bowing()
{
	//�{�[�C���O�N���X�̒��ł̃x�[�X
	base = new Base();
	startString = 1;
	endString = 1;
	playerString = 0;
	initialMove = false;
	prepared = false;
}

Bowing::Bowing(QStringList init)
{
	base = new Base();
	initialMove = false;
	prepared = false;

	//�f�[�^����̕��A
	if(init.size() != 4){
		Error::Critical(0, QObject::tr("[ BOWING ] Invalid initial data: %1").arg(init.size()));
	}

	QString line0 = init[0]; //�t�H�[�}�b�g�w��

	//���X�̃p�����[�^�w��
	QStringList line1_p = init[1].split(",");
	startString  = line1_p[0].toInt();
	endString    = line1_p[1].toInt();
	playerString = line1_p[2].toInt();

	//���̓x�N�^�w��
	QStringList line2_p = init[2].split(",");
	for(int i=0;i<line2_p.size();i++){
		pressureVector.append(line2_p[i].toDouble());
	}
	SetPressureVector(pressureVector);

	//�|���i�ʒu�j�x�N�^�w��
	QStringList line3_p = init[3].split(",");
	for(int i=0;i<line3_p.size();i++){
		positionVector.append(line3_p[i].toDouble());
	}
	SetPositionVector(positionVector);

}

QString Bowing::toString()
{
	//�{�[�C���O�N���X�̏���ۑ��p�ɕ�����ɂ��ĕԂ�
	QString writer = QString("BOWING DATA\n");
	writer.append(QString("%1,%2,%3\n").arg(startString).arg(endString).arg(playerString));
	//�v���b�V���[�x�N�^
	for(int i=0;i<pressureVector.size()-1;i++){
		writer.append(QString("%1,").arg(pressureVector[i]));
	}
	writer.append(QString("%1\n").arg(pressureVector[pressureVector.size()-1]));
	//�|�W�V�����x�N�^
	for(int i=0;i<positionVector.size()-1;i++){
		writer.append(QString("%1,").arg(positionVector[i]));
	}
	writer.append(QString("%1\n").arg(positionVector[positionVector.size()-1]));

	//4�s
	return writer;
}

Bowing::~Bowing()
{
	delete base;
}

void Bowing::Start()
{
	start();
}

void Bowing::SetPressureVector(QVector<double> _pressureVector)
{
	//�{�[�C���O�N���X�ƃx�[�X�N���X�����̓x�N�^������
	pressureVector = _pressureVector;
	base->SetPressureVector(pressureVector);

	//�|���x�N�^���X�V���ꂽ��v���y�A���K�v
	prepared = false;
}

void Bowing::SetPositionVector(QVector<double> _positionVector)
{
	//�{�[�C���O�N���X�݂̂��ʒu�x�N�^������
	positionVector = _positionVector;
	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->SetMode(MLinear::CONTINUOUS);
	((MLinear*)(control->axis[2]))->SetPositionVector(positionVector);

	//�ʒu�x�N�^���X�V���ꂽ��v���y�A���K�v
	prepared = false;
}

int Bowing::Prepare()
{
	//�x�[�X�N���X���v���y�A����

	//�J�n���E�I�����E���t���̊֌W���C�|���x�N�^����0�C��1�̐���x�N�^�ɕϊ�����D
	base->SetStringPair(startString, endString);
	base->SetPlayerString(playerString);
	int bowingTime = base->Prepare();

	//�|���x�N�^��P�Ɏ�2�ɃZ�b�g����C�A���l�ڑ��`�F�b�N�͕s�v�D
	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->SetMode(MLinear::CONTINUOUS);
	((MLinear*)(control->axis[2]))->Prepare();

	//�v���y�A�I��
	prepared = true;

	//���t���Ԃ��L�^
	playTime = bowingTime;

	//�{�[�C���O�ɕK�v�Ȏ��Ԃ�Ԃ�
	return bowingTime;
}

QVector<double> Bowing::GetPositionVector(int nb_axis)
{
	if(nb_axis == 0){
		//��0�̐���x�N�g��
		return base->axis0;
	}else if(nb_axis == 1){
		//��1�̐���x�N�g��
		return base->axis1;
	}else if(nb_axis == 2){
		//��2�̐���x�N�g��
		return positionVector;
	}else{
		QVector<double> dummy;
		Error::Critical(0, QObject::tr("GetPositionVector(): Invalid nb_axis=%1").arg(nb_axis));
		return dummy; // never reached here
	}
}

void Bowing::run()
{
	std::cout << "[ �{�[�C���O���s ] " << std::endl;
	if(initialMove){
		//// �A�h�z�b�N�Ȏ��������C�ŏ��݂̂Ȃ̂őË� ////
		std::cout << "[ �^�|���� ] �����ړ����s���܂�" << std::endl;
		base->InitialMove(startString);
		Sleep(3000);

		//// �ăv���y�A���K�v ////
		SetPositionVector(positionVector);
		SetPressureVector(pressureVector);
		Prepare();
	}

	//�|����
	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->Commit();
	//�|����
	base->Commit();
}
