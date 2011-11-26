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

#ifndef __BOWING_MANAGER_HPP__
#define __BOWING_MANAGER_HPP__

#include "bowing.hpp"

#include <QObject>
#include <QString>
#include <QVector>

class BowingManager : public QObject
{
	Q_OBJECT

public:

	BowingManager();

	~BowingManager();

	//総演奏制御ステップ数を返す
	int GetPlayTime(){ return axis[0].size(); }

	//ボーイング単位を追加する
	void AddBowing(Bowing *bow);

	//ボーイング単位を上書きする
	void UpdateBowing(Bowing *bow, int index);

	//ボーイング単位を返す
	Bowing* GetBowing(int index);

	//ボーイング単位を削除する
	void DelBowing(int index);

	//プリペア
	int Prepare();

	//範囲指定プリペア
	int Prepare(int start, int end);

	//実行
	void Commit();

	//初期移動
	void InitialMove();

private:

	//ボーイングマネージャクラスが管理するボーイングクラス
	QVector<Bowing*> bowingVector;

	//ボーイングマネージャクラスが管理するボーイングクラスが持つ各制御ベクタ
	
	//軸0,1,2
	QVector<double> axis[3];
};


#endif