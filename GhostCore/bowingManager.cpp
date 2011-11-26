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

#include "bowingManager.hpp"
#include "error.hpp"
#include "actuator/controller.hpp"

BowingManager::BowingManager()
{

}

BowingManager::~BowingManager()
{
	//登録されている全てのボーイングクラスを削除する
	for(int i=0;i<bowingVector.size();i++){
		delete bowingVector[i];
	}
}

void BowingManager::InitialMove()
{
	//プリペア後に実行されることが必須

	//プリペアされた最初の軸位置へ移動
	int duration = 2000;
	Controller *control = Controller::GetInstance();
	for(int i=0;i<3;i++){
		control->axis[i]->SetMode(MLinear::JOG);
		control->axis[i]->SetPosition(axis[i].at(0));
		control->axis[i]->SetDuration(duration);
		control->axis[i]->Prepare();
	}
	for(int i=0;i<3;i++){
		control->axis[i]->Commit();
	}

	//直接に待つ
	Sleep(duration+500);
}


void BowingManager::AddBowing(Bowing *bow)
{

	//// エラーチェックがここに入っても良い ////

	bowingVector.append(bow);
}


void BowingManager::DelBowing(int index)
{
	if(index >= bowingVector.size()){
		std::cout << "[ ボーイングマネージャ ] 削除インデックスが登録ベクタサイズを超えています." << std::endl;
		Error::Critical(0, QObject::tr("[ BowingManager ] Invalid delete index"));
	}

	//削除
	bowingVector.remove(index);
}


void BowingManager::UpdateBowing(Bowing *bow, int index)
{
	if(index >= bowingVector.size()){
		std::cout << "[ ボーイングマネージャ ] 更新インデックスが登録ベクタサイズを超えています." << std::endl;
		Error::Critical(0, QObject::tr("[ BowingManager ] Invalid update index"));
	}

	//index番目に追加
	bowingVector.insert(index, bow);

	//押し出されたindex+1番目を削除
	bowingVector.remove(index+1);
}


Bowing* BowingManager::GetBowing(int index)
{
	if(index >= bowingVector.size()){
		std::cout << "[ ボーイングマネージャ ] 取得インデックスが登録ベクタサイズを超えています." << std::endl;
		Error::Critical(0, QObject::tr("[ BowingManager ] Invalid get index"));
	}

	return bowingVector.at(index);
}


int BowingManager::Prepare()
{
	int start = 0;
	int end = bowingVector.size();

	return Prepare(start,end);
}



/*!
 * \brief
 * 登録されている全てのボーイング単位についてバリデーションしながら，各軸について，ひとつ制御ベクトルへ結合する．
 * 
 * \returns
 * 演奏ステップ数
 * 
 */
int BowingManager::Prepare(int start, int end)
{
	//全ボーイング単位をプリペアして制御ベクトルを結合してバリデーションする

	//最初のボーイング単位は特別扱い
	Bowing *bow = bowingVector.at(start);
	//プリペア済を前提することは直感に反しているので，ここでも一応プリペアしておく（おそらく前段階で何度かプリペアされているだろうが；そしてそのような冗長プリペアは無駄だが）
	bow->Prepare();
	//各軸の制御値を取得
	for(int i=0;i<3;i++){
		//ここで上書きしているので事前クリアは必要ないよ
		axis[i] = bow->GetPositionVector(i);
	}
	if(axis[0].size() != axis[1].size()){
		std::cout << "[ ボーイングマネージャ ] 0番, 軸0と軸1の制御ベクトル数が一致しません: axis0.size() = " << axis[0].size() << ", axis1.size() = " << axis[1].size() << std::endl;
		Error::Critical(0, QObject::tr("[ ボーイングマネージャ ] 軸0と軸1の制御ベクトル値が一致しません．"));
	}

	//二番目のボーイング単位から追加結合していく
	for(int i=start+1;i<end;i++){
		Bowing *bow = bowingVector.at(i);
		bow->Prepare();
		//各軸の制御ベクトルに追加結合
		for(int k=0;k<3;k++){
			QVector<double> temp = bow->GetPositionVector(k);
			if(temp[0] != axis[k][axis[k].size()-1]){
				//接続値の連続チェック
				std::cout << "[ ボーイングマネージャ ] 軸" << k << ", "<< i << "番目と" << i-1 << "番目の最後と最初の制御値が一致しません." << std::endl;
				std::cout << "temp[0,1,2] = " << temp[0] << "," << temp[1] << "," << temp[2] << std::endl;
				std::cout << "axis[" << k << "][" << axis[k].size()-3 << "," << axis[k].size()-2 << "," <<  axis[k].size()-1 << "] = " <<  axis[k][axis[k].size()-3] << "," << axis[k][axis[k].size()-2] << ", " << axis[k][axis[k].size()-1] << std::endl;
				Error::Critical(0, QObject::tr("[ BowingManager ] Last position value of #%1 and first position value of #%2 is not the same").arg(i-1).arg(i));
			}
			for(int j=0;j<temp.size();j++){
				axis[k].append(temp.at(j));
			}
		}
		//サイズチェック
		if(axis[0].size() != axis[1].size()){
			std::cout << "[ ボーインマネージャ ] " << i << "番, 軸0と軸1の制御ベクトル数が一致しません: axis0.size() = " << axis[0].size() << ", axis1.size() = " << axis[1].size() << std::endl;
			Error::Critical(0, QObject::tr("[ BowingManager ] axis0.size() != axis1.size() error"));
		}
	}

	//結合した各軸の制御ベクトルをセット・プリペア
	Controller *control = Controller::GetInstance();
	for(int i=0;i<3;i++){
		control->axis[i]->SetMode(MLinear::CONTINUOUS);
		control->axis[i]->SetPositionVector(axis[i]);
		control->axis[i]->Prepare();
	}

	//サイズを返す
	return axis[0].size();
}

/*!
 * \brief
 * 登録されている全てのボーイング単位について演奏を実行する
 * 
 */
void BowingManager::Commit()
{
	std::cout << "[ ボーイングマネージャ ] コミット." << std::endl;
	Controller *control = Controller::GetInstance();
	for(int i=0;i<3;i++){
		control->axis[i]->Commit();
	}
}

