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

#ifndef __PLAY_TAB_HPP__
#define __PLAY_TAB_HPP__

#include <iostream>
#include <QObject>

class GhostCore;
class Bowing;
class PitchUnit;
class PlayManager;

class PlayTab : public QObject
{
	Q_OBJECT

public:

	PlayTab(GhostCore *parent);

	~PlayTab();

	void Update();

private slots:

	//演奏データ保存ファイルを開く
	void OpenFile();

	//保存する
	void Save();

	//個別保存する
	void Apply();

	//次の画面に行く
	void Next();

	//前の画面に行く
	void Back();

	//すべて演奏
	void PlayAll();

	//画面から演奏
	void PlayFromThisScreen();

	//この画面まで演奏
	void PlayTillThisScreen();

	//個別演奏
	void Play();

	//プリペア
	void PrepareAll();

	//この画面からプリペア
	void PrepareFromThisScreen();

	//この画面までプリペア
	void PrepareTillThisScreen();

	//初期移動
	void InitialMove1();
	void InitialMove2();

	//弓圧データ変更
	void PressureDataChanged();

	//弓速データ変更
	void PositionDataChanged();

private:

	//プリペア時共通処理
	void PrepareCommon();

	//インデックスの変更時処理
	void IndexChange();

	//オブジェクトから設定ペインを作る
	void SetupPane(Bowing *bow, PitchUnit *pu, int nbPane);
	void ClearPane(int nbPane);
	
	//設定ペインからボーイングオブジェクトを作る
	Bowing* GenerateBowing(int nbPane);

	//設定ペインからピッチユニットオブジェクトを作る
	PitchUnit* GeneratePitchUnit(int nbPane);

	GhostCore* parent;

	QString fileName;

	//プレイマネージャ
	PlayManager *playManager;

	//現在の画面インデックス（もっとも左の番号）
	int index;

	//登録済みの演奏ユニット数（入力済みではなく登録済み）
	int nbPlayUnit;

	//総演奏時間
	int playTime;
};

#endif