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

#include "servo.hpp"
#include "../../actuator/controller.hpp"

#include <iostream>
#include <cstdlib>

Servo::Servo(CPhidgetAdvancedServoHandle *_servo, int _index)
{
	servo = _servo;
	index = _index;
	engaged = false;
	curr = 0;
	flagStart = false;
	mode = Servo::JOG;		 // ジョグモードで初期化
	to = 0;

	//トライステートポジションの初期化（退避は隣り合ったもの同士の差分が90度以上になるように）
	if(index == 0){

		//100から60でコンタクト，180で退避（外側80度退避）
		homePosition    = 110;
		readyPosition   = 110; 
		contactPosition = 55;

	}else if(index == 1){

		//80から120でコンタクト，90で退避（内側10度退避）
		homePosition    = 70;
		readyPosition   = 70;
		contactPosition = 140;

	}else{

		std::cout << "[ SERVO ] サーボIDが不正です : " << index << std::endl;

	}

	//ランピングオフ
	CPhidgetAdvancedServo_setSpeedRampingOn(*servo, index, 0);

	//初期化時にエンゲージしてしまう
	Engage(true);

	//ランピングする場合は以下のコードで速度・加速度制限を決定する
	//Set up some initial acceleration and velocity values
	//double minAccel, maxVel;
	//CPhidgetAdvancedServo_getAccelerationMin(servo, index, &minAccel);
	//CPhidgetAdvancedServo_setAcceleration(servo, index, minAccel*2);
	//CPhidgetAdvancedServo_getVelocityMax(servo, index, &maxVel);
	//CPhidgetAdvancedServo_setVelocityLimit(servo, index, maxVel/2);
}


Servo::~Servo()
{
	Close();
}


/*!
 * \brief
 * エンゲージ（サーボモーターに電力を供給する）
 * 
 * \param flag
 * true : 電力供給開始
 * false: 電力供給停止
 * 
 * \remarks
 * あまり長時間電力供給したままにしないこと．
 * 
 */
void Servo::Engage(bool flag)
{
	if(flag){		
		//通電せよ
		CPhidgetAdvancedServo_setEngaged(*servo, index, 1);
		engaged = true;
	}else{
		//電流カットせよ
		CPhidgetAdvancedServo_setEngaged(*servo, index, 0);
		engaged = false;
	}
}


/*!
 * \brief
 * ホームポジションへ移動
 * 
 * \remarks
 * 専用動作命令があるわけではないので注意
 *
 */
void Servo::SetHome()
{
	CPhidgetAdvancedServo_setPosition (*servo, index, homePosition);
	curr = 0;
}


/*!
 * \brief
 * レディポジションへ移動
 * 
 */
void Servo::SetReady()
{
	CPhidgetAdvancedServo_setPosition (*servo, index, readyPosition);
}


/*!
 * \brief
 * コンタクトポジションへ移動
 * 
 */
void Servo::SetContact()
{
	CPhidgetAdvancedServo_setPosition (*servo, index, contactPosition);
}


/*!
 * \brief
 * サーボモータをクローズする
 * 
 */
void Servo::Close()
{
	Engage(false);
}


/*!
 * \brief
 * 現在位置を取得する
 * 
 * \param index
 * サーボモーター番号
 * 
 * \returns
 * 現在位置
 * 
 */
double Servo::GetPosition()
{
	//現在位置
	double curr_pos = 0;
	//現在位置を取得
	CPhidgetAdvancedServo_getPosition(*servo, index, &curr_pos);
	//現在位置を返す
	return curr_pos;
}


/*!
 * \brief
 * 電流値を返す
 * 
 * \returns
 * 電流値
 * 
 * \remarks
 * Currentは電流の意味．他の関数では「現在の」の意味で利用されており，電流の意味で利用されているのはこの関数のみであることに注意．
 * 
 */
double Servo::GetCurrent()
{
	//現在位置
	double current = 0;
	//電流値を取得
	CPhidgetAdvancedServo_getCurrent(*servo, index, &current);
	//電流値を返す
	return current;
}


/*!
 * \brief
 * ポジション設定　ジョグ用
 * 
 * \param pos
 * ポジション
 * 
 */
void Servo::SetPosition(double pos)
{
	position = pos;
}


void Servo::Update()
{
	if(flagStart){

		if(curr == 0){
			//初期移動のみ特別

			if(controlVector[curr] == 1){
				to = 1;
			}else{
				to = 0;
			}
			start();

		}else{

			if(controlVector[curr-1] != controlVector[curr]){

				if(controlVector[curr] == 1){
					to = 1;
				}else{
					to = 0;
				}
				start();
	
			}
		}

		//終了条件判定
		curr++;
		if(curr >= controlVectorSize){
			std::cout << "[ SERVO ] プリディファインドモード終了" << std::endl;
			curr = 0;
			flagStart = false;
		}
		
	}
}


void Servo::Prepare()
{
	//// エラーチェック ////
	
	curr = 0;
}


void Servo::Commit()
{
	std::cout << "[ SERVO ] コミット ";
	if(mode == Servo::JOG){
		std::cout << "ジョグモード" << std::endl;
		//ジョグ動作の時は，単発エンゲージする
		start();
	}else if(mode == Servo::PREDEFINED){
		std::cout << "プリディファインドモード 制御ベクトルサイズ: " << controlVectorSize << std::endl;
		//プリディファインド動作の時は制御ベクトルによる制御に移る
		flagStart = true;
	}
}


void Servo::run()
{
	if(mode == Servo::JOG){

		//ジョグモードの場合は単発で目的位置へ移動
		CPhidgetAdvancedServo_setPosition (*servo, index, position);

	}else if(mode == Servo::PREDEFINED){

		//プリディファインドモードの場合は制御ベクトルに従ってそれぞれ移動
		if(to == 0){
			SetReady();
		}else if(to == 1){
			SetContact();
		}else if(to == 2){
			SetHome();
		}

	}

	//// エンゲージ

	/*
	if(mode == Servo::JOG){
		std::cout << "[ サーボモーター制御スレッド ] モータID : " << index << " > エンゲージ（ジョグ）" << std::endl;
		Engage(true);
	}else if(mode == Servo::KNOCK){
		std::cout << "[ サーボモーター制御スレッド ] モータID : " << index << " > エンゲージ（電磁弁）" << std::endl;
		CPhidgetAdvancedServo_setPosition (*servo, index, knockPosition);
	}
	*/

	//// ディスエンゲージ動作は行わない

	//電磁弁的動作処理時間（0.5秒）
	//Sleep(500);
	//
	//ディスエンゲージ
	//std::cout << "ディスエンゲージ" << std::endl;
	//CPhidgetAdvancedServo_setEngaged(*servo, index, 0);
}
