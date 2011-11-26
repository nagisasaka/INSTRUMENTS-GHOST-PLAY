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
	int ncc = 0;    //1/100秒ステップで刻まれる無通信カウンタ

	COMSTAT ComStat;//COMSTAT構造体
	DWORD dwCount;  //COMSTAT構造体に記録された受信バイト数
	DWORD dwErrors;
	DWORD dwRead;
	char outBuf[256];
	//2009/08/24
	ZeroMemory(outBuf, 256);
	while(true){
		ClearCommError(m_hComm, &dwErrors, &ComStat);
		if(dwErrors){
			//2009/8/24 IOエラー検出を追加
			error = "sensor_io_error";
			cerr << "I/O error for sensor: " << endl;
			str = error;
			return 0;
		}

		dwCount = ComStat.cbInQue;

		//エラー時にdwCountは、未定義になる...

		if (dwCount >= length) {   //最低受信数を引数から指定。>= としているのはエラー耐性を上げる為
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
				error = "sensor_timeout";
				//デバッグ出力
				cerr << "Timed out for sensor: " << outBuf << " (" << dwRead << ")" << endl;
				//2009/08/24
				str = error;
				return 0;
				//break;
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