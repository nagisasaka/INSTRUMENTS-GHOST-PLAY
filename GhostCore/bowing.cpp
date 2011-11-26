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
	//ボーイングクラスの中でのベース
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

	//データからの復帰
	if(init.size() != 4){
		Error::Critical(0, QObject::tr("[ BOWING ] Invalid initial data: %1").arg(init.size()));
	}

	QString line0 = init[0]; //フォーマット指定

	//諸々のパラメータ指定
	QStringList line1_p = init[1].split(",");
	startString  = line1_p[0].toInt();
	endString    = line1_p[1].toInt();
	playerString = line1_p[2].toInt();

	//圧力ベクタ指定
	QStringList line2_p = init[2].split(",");
	for(int i=0;i<line2_p.size();i++){
		pressureVector.append(line2_p[i].toDouble());
	}
	SetPressureVector(pressureVector);

	//弓速（位置）ベクタ指定
	QStringList line3_p = init[3].split(",");
	for(int i=0;i<line3_p.size();i++){
		positionVector.append(line3_p[i].toDouble());
	}
	SetPositionVector(positionVector);

}

QString Bowing::toString()
{
	//ボーイングクラスの情報を保存用に文字列にして返す
	QString writer = QString("BOWING DATA\n");
	writer.append(QString("%1,%2,%3\n").arg(startString).arg(endString).arg(playerString));
	//プレッシャーベクタ
	for(int i=0;i<pressureVector.size()-1;i++){
		writer.append(QString("%1,").arg(pressureVector[i]));
	}
	writer.append(QString("%1\n").arg(pressureVector[pressureVector.size()-1]));
	//ポジションベクタ
	for(int i=0;i<positionVector.size()-1;i++){
		writer.append(QString("%1,").arg(positionVector[i]));
	}
	writer.append(QString("%1\n").arg(positionVector[positionVector.size()-1]));

	//4行
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
	//ボーイングクラスとベースクラスが圧力ベクタを持つ
	pressureVector = _pressureVector;
	base->SetPressureVector(pressureVector);

	//弓圧ベクタが更新されたらプリペアが必要
	prepared = false;
}

void Bowing::SetPositionVector(QVector<double> _positionVector)
{
	//ボーイングクラスのみが位置ベクタを持つ
	positionVector = _positionVector;
	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->SetMode(MLinear::CONTINUOUS);
	((MLinear*)(control->axis[2]))->SetPositionVector(positionVector);

	//位置ベクタが更新されたらプリペアが必要
	prepared = false;
}

int Bowing::Prepare()
{
	//ベースクラスをプリペアする

	//開始弦・終了弦・演奏弦の関係より，弓圧ベクタを軸0，軸1の制御ベクタに変換する．
	base->SetStringPair(startString, endString);
	base->SetPlayerString(playerString);
	int bowingTime = base->Prepare();

	//弓速ベクタを単に軸2にセットする，連続値接続チェックは不要．
	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->SetMode(MLinear::CONTINUOUS);
	((MLinear*)(control->axis[2]))->Prepare();

	//プリペア終了
	prepared = true;

	//演奏時間を記録
	playTime = bowingTime;

	//ボーイングに必要な時間を返す
	return bowingTime;
}

QVector<double> Bowing::GetPositionVector(int nb_axis)
{
	if(nb_axis == 0){
		//軸0の制御ベクトル
		return base->axis0;
	}else if(nb_axis == 1){
		//軸1の制御ベクトル
		return base->axis1;
	}else if(nb_axis == 2){
		//軸2の制御ベクトル
		return positionVector;
	}else{
		QVector<double> dummy;
		Error::Critical(0, QObject::tr("GetPositionVector(): Invalid nb_axis=%1").arg(nb_axis));
		return dummy; // never reached here
	}
}

void Bowing::run()
{
	std::cout << "[ ボーイング実行 ] " << std::endl;
	if(initialMove){
		//// アドホックな実装だが，最初のみなので妥協 ////
		std::cout << "[ 運弓試験 ] 初期移動実行します" << std::endl;
		base->InitialMove(startString);
		Sleep(3000);

		//// 再プリペアが必要 ////
		SetPositionVector(positionVector);
		SetPressureVector(pressureVector);
		Prepare();
	}

	//弓速側
	Controller *control = Controller::GetInstance();
	((MLinear*)(control->axis[2]))->Commit();
	//弓圧側
	base->Commit();
}
