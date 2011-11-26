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
// 物理的なシリアル通信は環境依存であるが、通信後のデータ処理は環境に依存しない。シリアル通信クラスを
// ラップして、データ処理と実装を分離する。自動回線選択でステラセンサープロトコルに依存した発見処理を
// 行う為、汎用的ではない点に注意。Open()関数以外は汎用的な単スレッドシリアル通信クラスとして利用可能。
//
#include <string>
#include <iostream>

#ifdef WIN32
#include "winserialcom.hpp"
#else if MACOSX
#include "macserialcom.hpp" //未実装
#endif

using namespace std;

class StellarSerialCom{
public:
	StellarSerialCom(){
		serialCom = new SerialCom();
	}

	//シリアル通信　回線接続　自動回線選択を実装すること
	bool Open(void){
		return serialCom->Open();
	}

	//シリアル通信　送信
	int Send(string str){
		return serialCom->Send(str);
	}

	//シリアル通信　受信
	int Recv(string& str){
		return serialCom->Recv(str);
	}

	//シリアル通信　受信バイト数を指定した受信
	int Recv(string& str, int length){
		return serialCom->Recv(str,length);
	}

	//シリアル通信　エラー取得
	bool GetError(string& str){
		return serialCom->GetError(str);
	}

	//シリアル通信　回線切断
	bool Close(void){
		return serialCom->Close();
	}

private:
	SerialCom* serialCom;
};

#endif