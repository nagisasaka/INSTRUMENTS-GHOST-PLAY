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

#include "winserialcom.hpp"

int SerialCom::Recv(string& str, unsigned int length){
	error = "";
	int ncc = 0;    //1/100�b�X�e�b�v�ō��܂�閳�ʐM�J�E���^

	COMSTAT ComStat;//COMSTAT�\����
	DWORD dwCount;  //COMSTAT�\���̂ɋL�^���ꂽ��M�o�C�g��
	DWORD dwErrors;
	DWORD dwRead;
	char outBuf[256];
	//2009/08/24
	ZeroMemory(outBuf, 256);
	while(true){
		ClearCommError(m_hComm, &dwErrors, &ComStat);
		if(dwErrors){
			//2009/8/24 IO�G���[���o��ǉ�
			error = "sensor_io_error";
			cerr << "I/O error for sensor: " << endl;
			str = error;
			return 0;
		}

		dwCount = ComStat.cbInQue;

		//�G���[����dwCount�́A����`�ɂȂ�...

		if (dwCount >= length) {   //�Œ��M������������w��B>= �Ƃ��Ă���̂̓G���[�ϐ����グ���
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
				error = "sensor_timeout";
				//�f�o�b�O�o��
				cerr << "Timed out for sensor: " << outBuf << " (" << dwRead << ")" << endl;
				//2009/08/24
				str = error;
				return 0;
				//break;
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