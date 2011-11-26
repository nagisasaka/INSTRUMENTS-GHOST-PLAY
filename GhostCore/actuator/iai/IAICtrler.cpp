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

#include "IAICtrler.hpp"
#include <string>
#include <iostream>


IAICtrler::IAICtrler(StellarSerialCom *_scc, int _nb_axis) : scc(_scc) , nb_axis(_nb_axis)
{
	//�V���A���ʐM�N���X�̓I�[�v������Ă���O��

	//�p�����[�^�ݒ�
	Init();
	//�P�ɃX���[�v
	Sleep(500);
	//�T�[�{�I��
	OnServo();

	//���̑�������
	curr = 0;
	flagStart = false;
	controlVectorSize = false;
	mode = IAICtrler::JOG; // �W���O���[�h�ŏ�����
}


IAICtrler::~IAICtrler()
{
}


void IAICtrler::Close()
{
}


bool IAICtrler::Init()
{
	//�V���A���ʐM�N���X�̓I�[�v������Ă���O��
	string out;

	//disable PIO
	scc->Send(":01050427FF00D0\r\n");
	scc->Recv(out);
	scc->Send(":02050427FF00CF\r\n");
	scc->Recv(out);

	//clear Alarm
	scc->Send(":01050407FF00F0\r\n");
	scc->Recv(out);
	scc->Send(":010504070000EF\r\n");
	scc->Recv(out);
	scc->Send(":02050407FF00EF\r\n");
	scc->Recv(out);
	scc->Send(":020504070000EE\r\n");
	scc->Recv(out);
	return true; 
}


bool IAICtrler::OnServo()
{
	//Servo ON
	string out;
	switch(nb_axis){
		case 1:
			scc->Send(":01060D001000DC\r\n");
			scc->Recv(out);
			//Sleep(1000); 
			break;
		case 2:
			scc->Send(":02060D001000DB\r\n");
			scc->Recv(out);
			//Sleep(1000);
			break;
	}
	return true;
}


bool IAICtrler::ReturnOrigin()
{
	//Origin Return
	string out;
	switch(nb_axis){
		case 1:
			scc->Send(":01060D001010CC\r\n");
			scc->Recv(out);
			//Sleep(5000); // ���̎��̌��_���A�Ɏ��Ԃ��|����̂Ŏb��I�ɏȗ�
			break;
		case 2:
			scc->Send(":02060D001010CB\r\n");
			scc->Recv(out);
			//Sleep(5000); // ���̎��̌��_���A�Ɏ��Ԃ��|����̂Ŏb��I�ɏȗ�
			break;
	}

	//�J�����g�ʒu���N���A
	curr = 0;
	return true;
}

/*!
 * \brief
 * ���l�ړ�����
 * http://www.iai-robot.co.jp/download/tashakiki/pdf/SERIAL-COMMUNICATION_MODBUS(MJ0162-3B).pdf
 * 5.6.1 �ߎQ��
 *
 * \param Destination
 * �ڕW�ʒu�w��[0.01mm]�i��: 500 = 5mm�j
 * 
 * \param Accuracy
 * �ʒu���ߕ��w��[0.01mm]�i��: 10 = 0.1mm�j
 * 
 * \param Vel
 * ���x�w��[0.01mm/sec]�i��: 6000 = 60mm/sec = 6mm / 0.1�b(10step����), �@��ɂ���čő�l�͈قȂ�D���s�@���12000���炢�܂ł͓������������ɂ��➑̐U������������������Ȃ��j
 * 
 * \param Acceleration
 * �����x�w��[0.01G]
 * 
 * \returns
 * ����X�e�[�^�X
 * 
 * \remarks
 * private �����ɂ��邩������Ȃ�
 *
 */
bool IAICtrler::Move(int Destination, int Accuracy, int Vel, int Acceleration)
{
	char command[255];
	string out;

	for(int index=0;index<254;index++)command[index]=0x00;

	sprintf(command,":%02X1099000009120000%04X0000%04X0000%04X%04X00000000", nb_axis, Destination, Accuracy, Vel, Acceleration);
	int sum=0;
	char hex[2];
	int hex_val;
	for(int i=0;i<25;i++){
		strncpy(hex,&(command[1+i*2]),2);
		sscanf(hex,"%X",&hex_val);
		sum += hex_val;
	}
	int diff=0x10000-sum;
	char diff_hex[5];
	sprintf(diff_hex,"%04X",diff);
	char chksum[5];
	strncpy(chksum,&(diff_hex[2]),3);
	chksum[2]=0x0D;
	chksum[3]=0x0A;
	chksum[4]=0x00;
	strcat(command,chksum);

	string str;
	str=command;

	//scc->Send(str);
	//scc->Recv(out);
	printf("send int=%d\n",scc->Send(str));
	printf("recv int=%d\n",scc->Recv(out));
	std::cout << "str:" << str << std::endl;

	return true;
}

void IAICtrler::Evacuate()
{
	int position = 0;
	if(nb_axis == 1){
		//position = 1430; //G��
		position = 2150;   //���S�ޔ����
	}else if(nb_axis == 2){
		position = 1750; //E��
	}

	int accuracy = 10; // 0.1mm
	int vel  = 4000;   // 40mm/sec
	int accl = 30;     // 0.3G
	string str;
	if(!scc->GetError(str)){
		Move(position, accuracy, vel, accl);	
	}else{
		std::cout << "[ IAICTRLER ] SCC �G���[" << std::endl;
	}

}

void IAICtrler::GoToCurrentString()
{
	GoToString(currentStringName);
}

/*!
 * \brief
 * ���l�ړ����߂��l�i�K�ɃX�e�b�v������
 *
 * �Y���X�g�����O�ɑ΂��āC�e���̐�Έʒu���n�[�h�R�[�h�i���������L�����u���[�V�������Ȃ��̂Ńn�[�h�R�[�h�ɂȂ�j
 * 
 * \param stringName
 * ���l�ړ����߂̃X�e�b�v���i�킩��₷���̂��߂Ɍ����Ƃ��Ă��邪�C�P�Ƀv���f�B�t�@�C���ʒu�̕ʖ��Ȃ̂Œ��Ӂj
 * 
 * \remarks
 * �Ӗ��I�ɂ́C�P�ɁC����x�N�^�̉��łS�i�K�̃v���f�B�t�@�C���ʒu�Ɉړ�������̂ł���D���̃A�N�`���G�[�^�ł́Ccontroller�n�Ƀ|�C���^���Ϗ����Ȃ��C
 * �ʂ̃}�j�s�����[�^�N���X���g���邪�C�����̏ꍇ�́C�L�����u���[�V����������Ȃ��قǁC�����P���Ȃ̂ŁC�S�i�K�̃v���f�B�t�@�C���ʒu�Ƃ��āC��{
 * ����N���X�Ƀn�[�h�R�[�h���Ă��܂��Ƃ����Ӑ}�D
 * 
 */
void IAICtrler::GoToString(QString stringName)
{
	int position = 0;

	if(nb_axis == 1){

		if(stringName == "G"){
			//17mm
			position = 1700;		
		}else if(stringName == "D"){
			//12mm
			position = 1000;
		}else if(stringName == "A"){
			//4mm
			position = 400;
		}else if(stringName == "E"){
			//�݌v�㓞�B�s�\
			position = -1;
			std::cout << "[ IAICTRLER ] #" << nb_axis << " could not reached to pre-defined position E" << std::endl;
		}else{
			std::cout << "[ IAICTRLER ] Invalid pre-defined position" << std::endl;
		}
		
	}else if(nb_axis == 2){

		if(stringName == "G"){
			//���_
			position = 0; // ������
		}else if(stringName == "D"){
			//6mm
			position = 600;
		}else if(stringName == "A"){
			//12mm
			position = 1200;
		}else if(stringName == "E"){
			//17mm // ������
			position = 1700;
		}else{
			std::cout << "[ IAICTRLER ] Invalid pre-defined position" << std::endl;
		}

	}

	//���l�ړ��i�V���A���ʐM�ɂ��ړ����߂Ȃ̂ňړ��J�n�^�C�~���O���x��邱�Ƃ��\�z�����j
	int accuracy = 10; // 0.1mm
	int vel  = 8000;   // 80mm/sec
	int accl = 30;     // 0.3G
	Move(position, accuracy, vel, accl);	
}


void IAICtrler::run()
{
	GoToCurrentString();

	//�����ł����I�����Ă��܂��̂����̂悤�ȋC������
	Sleep(500);
}


void IAICtrler::Prepare()
{
	//// �G���[�`�F�b�N ////

	curr = 0;
}


void IAICtrler::Commit()
{
	std::cout << "[ IAICTRLER ] �R�~�b�g ";
	if(mode == IAICtrler::JOG){
		std::cout << "�W���O���[�h" << std::endl;
		//���݃Z�b�g����Ă��鎲�֒P���ړ�����
		start();
	}else if(mode == IAICtrler::PREDEFINED){
		std::cout << "�v���f�B�t�@�C���h���[�h ����x�N�g���T�C�Y: " << controlVectorSize << std::endl;
		//����x�N�g���ɂ��ړ�
		flagStart = true;
	}else{
		std::cout << "[ IAICTRLER ] �s���ȓ��샂�[�h�ł� : " << mode << std::endl;
	}
}


void IAICtrler::Update()
{
	if(flagStart){

		if(curr == 0){
			//�ŏ��Ɋւ��Ă͏����ړ�����
			if(controlVector[curr] == 0){
				currentStringName = "G";
			}else if(controlVector[curr] == 1){
				currentStringName = "D";
			}else if(controlVector[curr] == 2){
				currentStringName = "A";
			}else if(controlVector[curr] == 3){
				currentStringName = "E";
			}
			//�����ړ�����
			//�}���`�X���b�h
			//start();
			//�V���O���X���b�h
			GoToString(currentStringName);

		}else{
			if(controlVector[curr-1] != controlVector[curr]){
				//�O��ƈقȂ�ꍇ�̂�
				if(controlVector[curr] == 0){
					currentStringName = "G";
				}else if(controlVector[curr] == 1){
					currentStringName = "D";
				}else if(controlVector[curr] == 2){
					currentStringName = "A";
				}else if(controlVector[curr] == 3){
					currentStringName = "E";
				}
				//�ڌ��ړ�����
				//�}���`�X���b�h
				//start();
				//�V���O���X���b�h
				GoToString(currentStringName);
			}
		}

		//�I����������
		curr++;
		if(curr >= controlVectorSize){
			std::cout << "[ IAICTRLER ] �v���f�B�t�@�C���h���[�h�I��" << std::endl;
			curr = 0;
			flagStart = false;
		}

	}
}


