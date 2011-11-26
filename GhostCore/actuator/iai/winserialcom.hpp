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

#ifndef _WINSERIALCOM_HPP_
#define _WINSERIALCOM_HPP_

#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

//
// �E�B���h�E�Y���ɂ�����V���A���ʐM�N���X
//
class SerialCom{
	friend class StellarSerialCom;
private:
	SerialCom(){
		m_hComm = NULL;
		error = "";
	}

	~SerialCom(){
		Close();
		error = "";
	}

	bool Open(){
		error = "";
		//�|�[�g���I�[�v���ς̏ꍇ�N���[�Y����
		if (m_hComm != NULL) {
			CloseHandle(m_hComm);
		}
		DCB dcb;
		COMMTIMEOUTS pcto;

		wchar_t *p = L"\\\\.\\COM4";

		cout << "Searching Servo";
	
		m_hComm = CreateFile(p, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(m_hComm == INVALID_HANDLE_VALUE){
			//���������J���Ȃ������ꍇ
			return false;
		}else{
			string out;
			
			//DCB��ݒ�
			GetCommState(m_hComm, &dcb);	// DCB ���擾
			//dcb.BaudRate = 38400;           // baudrate�̊���
			dcb.BaudRate = 115200;           // baudrate�̊���
			dcb.ByteSize = 8;			    // �o�C�g�T�C�Y
			dcb.Parity = NOPARITY;			// �p���e�B(�g�p���Ȃ�,���ɂ�NOPARITY, EVENPARITY,ODDPARITY)
			dcb.fParity = FALSE;		       // �p���e�B���g�p���邩
			dcb.StopBits = ONESTOPBIT;	    // �X�g�b�v�r�b�g
			dcb.fOutxCtsFlow = FALSE;	    // ���M���ɁACTS ���Ď����邩�ǂ�����
			dcb.fOutxDsrFlow = FALSE;	    // ���M���ɁADSR ���Ď����邩�ǂ�����
			dcb.fDsrSensitivity = FALSE;    // DSR ��OFF�̊Ԃ͎�M�f�[�^�𖳎����邩
			SetCommState(m_hComm, &dcb);	// DCB ��ݒ�

			//�^�C���A�E�g��ݒ� ver1.01
			GetCommTimeouts(m_hComm,&pcto);
			pcto.WriteTotalTimeoutConstant   = 1000;
			pcto.WriteTotalTimeoutMultiplier = 1;
			SetCommTimeouts(m_hComm,&pcto);

			return true;
		}
		
	}


	int Send(string str){
		// COM�|�[�g���J���Ă��邩�`�F�b�N
		if (m_hComm == NULL) {
			//error = "COM�|�[�g���J����Ă��܂���";
			error = "COM port closed";
			cerr << error << endl;
			return 0;
		}

		DWORD	dwWrite;    // COM�|�[�g�ɑ������o�C�g��
		WriteFile(m_hComm, str.c_str(), (DWORD)str.length(), &dwWrite, NULL);
		//���̏�̂ł��łɈړ��J�n���Ă��邽�߁C����ȉ��̎�M�]�X�łǂ̂��炢���Ԃ��x��Ă����Ȃ����낤�Ǝv��
		Sleep(10);	
		return (int)dwWrite;
	}

	int Recv(string& str){
		error = "";
		int ncc = 0;    //1/100�b�X�e�b�v�ō��܂�閳�ʐM�J�E���^

		COMSTAT ComStat;//COMSTAT�\����
		DWORD dwCount;  //COMSTAT�\���̂ɋL�^���ꂽ��M�o�C�g��
		DWORD dwErrors;
		DWORD dwRead;
		char outBuf[256];
		while(true){
			ClearCommError(m_hComm, &dwErrors, &ComStat);
			dwCount = ComStat.cbInQue;

			if (dwCount != 0) {
				cout << ".." << endl;
				if (dwCount >= 256) {
					dwCount = 255;
				}
				ZeroMemory(outBuf, 256);
				ReadFile(m_hComm, outBuf, dwCount, &dwRead, NULL); // ��M
				outBuf[dwRead] = '\0'; // ������̍Ō�ɂӂ�������D

				if (dwRead != 0) {
					break;
				} else {
					Sleep(10);
					continue;
				}
			} else { // �f�[�^�����ĂȂ���΂܂�
				if(ncc > 100){
					//���悻1�b�̑҂�����
					//error = "�Z���T�[�Ƃ̒ʐM���^�C���A�E�g���܂���";
					error = "Timed out for sensor";
					cerr << error << endl;
					break;
				}
				ncc++;
				Sleep(10);//���悻1/100�b�܂�
				continue;
			}
		}
		str = string(outBuf);
		Sleep(20);
		return (int)dwRead;
	}
	
	int Recv(string& str, unsigned int length);

	// �V���A���|�[�g�����B�J����Ă��Ȃ��ꍇ�͕K��true���Ԃ�B
	bool Close(){
		bool result = true;
		if(m_hComm != NULL){
			result = CloseHandle(m_hComm);
			m_hComm = NULL;
		}
		return result;
	}

	// ���݂̃G���[�󋵂��m�F����@GetLastError()�ł͂Ȃ����Ƃɒ���
	bool GetError(string& str){
		if(error == ""){
			str = "";
			return false;
		}else{
			str = error;
			return true;
		}
	}

	HANDLE m_hComm;
	string error;
};

#endif