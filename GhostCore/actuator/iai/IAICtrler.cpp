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
	//シリアル通信クラスはオープンされている前提

	//パラメータ設定
	Init();
	//単にスリープ
	Sleep(500);
	//サーボオン
	OnServo();

	//その他初期化
	curr = 0;
	flagStart = false;
	controlVectorSize = false;
	mode = IAICtrler::JOG; // ジョグモードで初期化
}


IAICtrler::~IAICtrler()
{
}


void IAICtrler::Close()
{
}


bool IAICtrler::Init()
{
	//シリアル通信クラスはオープンされている前提
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
			//Sleep(5000); // 他の軸の原点復帰に時間が掛かるので暫定的に省略
			break;
		case 2:
			scc->Send(":02060D001010CB\r\n");
			scc->Recv(out);
			//Sleep(5000); // 他の軸の原点復帰に時間が掛かるので暫定的に省略
			break;
	}

	//カレント位置をクリア
	curr = 0;
	return true;
}

/*!
 * \brief
 * 直値移動命令
 * http://www.iai-robot.co.jp/download/tashakiki/pdf/SERIAL-COMMUNICATION_MODBUS(MJ0162-3B).pdf
 * 5.6.1 節参照
 *
 * \param Destination
 * 目標位置指定[0.01mm]（例: 500 = 5mm）
 * 
 * \param Accuracy
 * 位置決め幅指定[0.01mm]（例: 10 = 0.1mm）
 * 
 * \param Vel
 * 速度指定[0.01mm/sec]（例: 6000 = 60mm/sec = 6mm / 0.1秒(10step相当), 機種によって最大値は異なる．現行機種で12000くらいまでは動くが加原則による筐体振動が厳しいかもしれない）
 * 
 * \param Acceleration
 * 加速度指定[0.01G]
 * 
 * \returns
 * 動作ステータス
 * 
 * \remarks
 * private 扱いにするかもしれない
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
		//position = 1430; //G線
		position = 2150;   //完全退避状態
	}else if(nb_axis == 2){
		position = 1750; //E線
	}

	int accuracy = 10; // 0.1mm
	int vel  = 4000;   // 40mm/sec
	int accl = 30;     // 0.3G
	string str;
	if(!scc->GetError(str)){
		Move(position, accuracy, vel, accl);	
	}else{
		std::cout << "[ IAICTRLER ] SCC エラー" << std::endl;
	}

}

void IAICtrler::GoToCurrentString()
{
	GoToString(currentStringName);
}

/*!
 * \brief
 * 直値移動命令を四段階にステップ化する
 *
 * 該当ストリングに対して，各軸の絶対位置をハードコード（ここだけキャリブレーションしないのでハードコードになる）
 * 
 * \param stringName
 * 直値移動命令のステップ名（わかりやすさのために弦名としているが，単にプリディファイン位置の別名なので注意）
 * 
 * \remarks
 * 意味的には，単に，制御ベクタの下で４段階のプリディファイン位置に移動するものである．他のアクチュエータでは，controller系にポインタを委譲しない，
 * 別のマニピュレータクラスが使われるが，ここの場合は，キャリブレーションがいらないほど，ごく単純なので，４段階のプリディファイン位置として，基本
 * 制御クラスにハードコードしてしまうという意図．
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
			//設計上到達不能
			position = -1;
			std::cout << "[ IAICTRLER ] #" << nb_axis << " could not reached to pre-defined position E" << std::endl;
		}else{
			std::cout << "[ IAICTRLER ] Invalid pre-defined position" << std::endl;
		}
		
	}else if(nb_axis == 2){

		if(stringName == "G"){
			//原点
			position = 0; // 厳しい
		}else if(stringName == "D"){
			//6mm
			position = 600;
		}else if(stringName == "A"){
			//12mm
			position = 1200;
		}else if(stringName == "E"){
			//17mm // 厳しい
			position = 1700;
		}else{
			std::cout << "[ IAICTRLER ] Invalid pre-defined position" << std::endl;
		}

	}

	//直値移動（シリアル通信による移動命令なので移動開始タイミングが遅れることが予想される）
	int accuracy = 10; // 0.1mm
	int vel  = 8000;   // 80mm/sec
	int accl = 30;     // 0.3G
	Move(position, accuracy, vel, accl);	
}


void IAICtrler::run()
{
	GoToCurrentString();

	//ここですぐ終了してしまうのが問題のような気がする
	Sleep(500);
}


void IAICtrler::Prepare()
{
	//// エラーチェック ////

	curr = 0;
}


void IAICtrler::Commit()
{
	std::cout << "[ IAICTRLER ] コミット ";
	if(mode == IAICtrler::JOG){
		std::cout << "ジョグモード" << std::endl;
		//現在セットされている軸へ単発移動する
		start();
	}else if(mode == IAICtrler::PREDEFINED){
		std::cout << "プリディファインドモード 制御ベクトルサイズ: " << controlVectorSize << std::endl;
		//制御ベクトルによる移動
		flagStart = true;
	}else{
		std::cout << "[ IAICTRLER ] 不正な動作モードです : " << mode << std::endl;
	}
}


void IAICtrler::Update()
{
	if(flagStart){

		if(curr == 0){
			//最初に関しては初期移動する
			if(controlVector[curr] == 0){
				currentStringName = "G";
			}else if(controlVector[curr] == 1){
				currentStringName = "D";
			}else if(controlVector[curr] == 2){
				currentStringName = "A";
			}else if(controlVector[curr] == 3){
				currentStringName = "E";
			}
			//初期移動する
			//マルチスレッド
			//start();
			//シングルスレッド
			GoToString(currentStringName);

		}else{
			if(controlVector[curr-1] != controlVector[curr]){
				//前回と異なる場合のみ
				if(controlVector[curr] == 0){
					currentStringName = "G";
				}else if(controlVector[curr] == 1){
					currentStringName = "D";
				}else if(controlVector[curr] == 2){
					currentStringName = "A";
				}else if(controlVector[curr] == 3){
					currentStringName = "E";
				}
				//移弦移動する
				//マルチスレッド
				//start();
				//シングルスレッド
				GoToString(currentStringName);
			}
		}

		//終了条件判定
		curr++;
		if(curr >= controlVectorSize){
			std::cout << "[ IAICTRLER ] プリディファインドモード終了" << std::endl;
			curr = 0;
			flagStart = false;
		}

	}
}


