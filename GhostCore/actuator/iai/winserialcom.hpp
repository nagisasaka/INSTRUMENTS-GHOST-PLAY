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
// ウィンドウズ環境におけるシリアル通信クラス
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
		//ポートがオープン済の場合クローズする
		if (m_hComm != NULL) {
			CloseHandle(m_hComm);
		}
		DCB dcb;
		COMMTIMEOUTS pcto;

		wchar_t *p = L"\\\\.\\COM4";

		cout << "Searching Servo";
	
		m_hComm = CreateFile(p, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(m_hComm == INVALID_HANDLE_VALUE){
			//そもそも開けなかった場合
			return false;
		}else{
			string out;
			
			//DCBを設定
			GetCommState(m_hComm, &dcb);	// DCB を取得
			//dcb.BaudRate = 38400;           // baudrateの既定
			dcb.BaudRate = 115200;           // baudrateの既定
			dcb.ByteSize = 8;			    // バイトサイズ
			dcb.Parity = NOPARITY;			// パリティ(使用しない,他にはNOPARITY, EVENPARITY,ODDPARITY)
			dcb.fParity = FALSE;		       // パリティを使用するか
			dcb.StopBits = ONESTOPBIT;	    // ストップビット
			dcb.fOutxCtsFlow = FALSE;	    // 送信時に、CTS を監視するかどうかを
			dcb.fOutxDsrFlow = FALSE;	    // 送信時に、DSR を監視するかどうかを
			dcb.fDsrSensitivity = FALSE;    // DSR がOFFの間は受信データを無視するか
			SetCommState(m_hComm, &dcb);	// DCB を設定

			//タイムアウトを設定 ver1.01
			GetCommTimeouts(m_hComm,&pcto);
			pcto.WriteTotalTimeoutConstant   = 1000;
			pcto.WriteTotalTimeoutMultiplier = 1;
			SetCommTimeouts(m_hComm,&pcto);

			return true;
		}
		
	}


	int Send(string str){
		// COMポートが開いているかチェック
		if (m_hComm == NULL) {
			//error = "COMポートが開かれていません";
			error = "COM port closed";
			cerr << error << endl;
			return 0;
		}

		DWORD	dwWrite;    // COMポートに送ったバイト数
		WriteFile(m_hComm, str.c_str(), (DWORD)str.length(), &dwWrite, NULL);
		//この上のですでに移動開始しているため，これ以下の受信云々でどのくらい時間が遅れても問題ないだろうと思う
		Sleep(10);	
		return (int)dwWrite;
	}

	int Recv(string& str){
		error = "";
		int ncc = 0;    //1/100秒ステップで刻まれる無通信カウンタ

		COMSTAT ComStat;//COMSTAT構造体
		DWORD dwCount;  //COMSTAT構造体に記録された受信バイト数
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
				ReadFile(m_hComm, outBuf, dwCount, &dwRead, NULL); // 受信
				outBuf[dwRead] = '\0'; // 文字列の最後にふたをする．

				if (dwRead != 0) {
					break;
				} else {
					Sleep(10);
					continue;
				}
			} else { // データが来てなければまつ
				if(ncc > 100){
					//およそ1秒の待ち時間
					//error = "センサーとの通信がタイムアウトしました";
					error = "Timed out for sensor";
					cerr << error << endl;
					break;
				}
				ncc++;
				Sleep(10);//およそ1/100秒まつ
				continue;
			}
		}
		str = string(outBuf);
		Sleep(20);
		return (int)dwRead;
	}
	
	int Recv(string& str, unsigned int length);

	// シリアルポートを閉じる。開かれていない場合は必ずtrueが返る。
	bool Close(){
		bool result = true;
		if(m_hComm != NULL){
			result = CloseHandle(m_hComm);
			m_hComm = NULL;
		}
		return result;
	}

	// 現在のエラー状況を確認する　GetLastError()ではないことに注意
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