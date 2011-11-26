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

#ifndef __PLAY_MANAGER_HPP__
#define __PLAY_MANAGER_HPP__

#include "bowingManager.hpp"
#include "pitchManager.hpp"
#include "play/pitchunit.hpp"

class PlayTab;

class PlayManager : public QObject
{
	Q_OBJECT

	friend class PlayTab;

public:

	PlayManager();

	~PlayManager();

	int GetNbPlayUnit(){ return nbPlayUnit; }

	//初期移動
	void InitialMove1();
	void InitialMove2();

	//セーブ
	void Save(QString filename);

	//ロード
	int Load(QString filename);

	//演奏単位を登録する
	void RegisterPlay(Bowing *bowing, PitchUnit *pu, int index);

	//演奏単位を削除する
	void DeletePlay(int index);
	
	//登録全演奏単位をプリペア
	int Prepare();

	//範囲指定プリペア
	int Prepare(int start, int end);

	void Commit();

private:

	//演奏単位を更新する
	void UpdatePlay(Bowing *bowing, PitchUnit *pu, int index);

	//演奏単位を追加する
	void AddPlay(Bowing *bowing, PitchUnit *pu);

	//ボーイングマネージャ
	BowingManager *bowingManager;

	//ピッチマネージャ
	PitchManager *pitchManager;

	//保持している演奏単位数
	int nbPlayUnit;
};

#endif