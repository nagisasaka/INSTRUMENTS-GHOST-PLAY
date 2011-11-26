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

#ifndef _SERIAL_COM_H_
#define _SERIAL_COM_H_
//
// �����I�ȃV���A���ʐM�͊��ˑ��ł��邪�A�ʐM��̃f�[�^�����͊��Ɉˑ����Ȃ��B�V���A���ʐM�N���X��
// ���b�v���āA�f�[�^�����Ǝ����𕪗�����B��������I���ŃX�e���Z���T�[�v���g�R���Ɉˑ���������������
// �s���ׁA�ėp�I�ł͂Ȃ��_�ɒ��ӁBOpen()�֐��ȊO�͔ėp�I�ȒP�X���b�h�V���A���ʐM�N���X�Ƃ��ė��p�\�B
//
#include <string>
#include <iostream>

#ifdef WIN32
#include "winserialcom.hpp"
#else if MACOSX
#include "macserialcom.hpp" //������
#endif

using namespace std;

class StellarSerialCom{
public:
	StellarSerialCom(){
		serialCom = new SerialCom();
	}

	//�V���A���ʐM�@����ڑ��@��������I�����������邱��
	bool Open(void){
		return serialCom->Open();
	}

	//�V���A���ʐM�@���M
	int Send(string str){
		return serialCom->Send(str);
	}

	//�V���A���ʐM�@��M
	int Recv(string& str){
		return serialCom->Recv(str);
	}

	//�V���A���ʐM�@��M�o�C�g�����w�肵����M
	int Recv(string& str, int length){
		return serialCom->Recv(str,length);
	}

	//�V���A���ʐM�@�G���[�擾
	bool GetError(string& str){
		return serialCom->GetError(str);
	}

	//�V���A���ʐM�@����ؒf
	bool Close(void){
		return serialCom->Close();
	}

private:
	SerialCom* serialCom;
};

#endif