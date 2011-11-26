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

#include "playManager.hpp"
#include "error.hpp"

#include <QFile>
#include <QTextStream>

#include <iostream>

PlayManager::PlayManager()
{
	bowingManager = new BowingManager();
	pitchManager = new PitchManager();

	nbPlayUnit = 0;
}

PlayManager::~PlayManager()
{
	delete bowingManager;
	delete pitchManager;
}


void PlayManager::Save(QString filename)
{
	//マネージャクラスに登録されているオブジェクトのtoString()をまとめて発行してまとめて返すtoString()関数を
	//実装しても良いが，bowingとpitchを交互にファイル中に書き出したほうがreadableであるため，暫定的に実装しない

	//全演奏単位数を保存
	QString writer = QString();
	for(int i=0;i<nbPlayUnit;i++){
		QString writer_bowing = bowingManager->GetBowing(i)->toString();
		QString writer_pitch  = pitchManager->GetPitch(i)->toString();
		writer.append(writer_bowing);
		writer.append(writer_pitch);
	}

	//ファイルに書き落とし
	QFile file(filename);
	if(!file.open(QFile::WriteOnly|QFile::Truncate)){
		Error::Critical(0, QObject::tr("[ PLAYMANAGER ] Save(), could not open save file :").append(filename));
	}

	QTextStream out(&file);
	out << writer;

	std::cout << "[ プレイマネージャ ] 保存しました: " << filename.toStdString() << std::endl;
}


int PlayManager::Load(QString filename)
{
	//ファイルから演奏単位を追加していく

	//既存のマネージャクラスを削除
	delete bowingManager;
	delete pitchManager;

	//新しいマネージャクラスを構築
	bowingManager = new BowingManager();
	pitchManager = new PitchManager();

	//初期化により登録演奏単位数もゼロへ
	nbPlayUnit = 0;

	//ファイルオープン
	QFile file(filename);
	if(!file.open(QFile::ReadOnly)){
		Error::Critical(0, QObject::tr("[ PLAYMANAGER ] Load(), could not open save file:").append(filename));

	}

	//ボーイング要素の初期化用データ
	QStringList bowingInitData;
	//ピッチユニットの初期化用データ
	QStringList pitchInitData;
	//ラインカウンタ
	int lineCounter = 0;
	//ストリームオープン
	QTextStream in(&file);
	while(!in.atEnd()){
		//各行について
		QString line = in.readLine(); // \nがトリムされることに注意
		if(lineCounter < 4){
			//0,1,2,3
			bowingInitData.append(line);
		}else if(lineCounter == 4){
			//4
			pitchInitData.append(line);
		}else if(lineCounter == 5){
			//5		
			pitchInitData.append(line);

			//データが集まった

			//オブジェクト構築
			Bowing *bow = new Bowing(bowingInitData);
			PitchUnit *pu = new PitchUnit(pitchInitData);

			//このクラスへ登録
			AddPlay(bow,pu);

			//次へ
			lineCounter = 0;
			bowingInitData.clear();
			pitchInitData.clear();
			continue;
		}
		lineCounter++;		
	}

	std::cout << "[ プレイマネージャ ] " << nbPlayUnit << "演奏単位 ロードしました: " << filename.toStdString() << std::endl;
	return nbPlayUnit;
}


void PlayManager::RegisterPlay(Bowing *bowing, PitchUnit *pu, int index)
{
	if(index == nbPlayUnit){

		AddPlay(bowing, pu);

	}else if(index < nbPlayUnit){

		UpdatePlay(bowing, pu, index);

	}else{

		//UI上の問題で中飛びしようとしてしまった
		Error::Warning(0, QObject::tr("登録インデックスが連続していません"));
		return;

	}
}


void PlayManager::UpdatePlay(Bowing *bowing, PitchUnit *pu, int index)
{
	//演奏単位を更新する
	std::cout << "[ プレイマネージャ ] #" << index << " を更新します." << std::endl;

	int playTime = bowing->Prepare();
	pu->SetPlayTime(playTime);
	pitchManager->UpdatePitch(pu,index);
	bowingManager->UpdateBowing(bowing,index);
}


void PlayManager::DeletePlay(int index)
{
	//演奏単位を削除する
	std::cout << "[ プレイマネージャ ] #" << index << " を削除します." << std::endl;

	//ピッチ単位を削除
	pitchManager->DelPitch(index);
	//ボーイング単位を削除
	bowingManager->DelBowing(index);

	//登録演奏単位数を減らす
	nbPlayUnit--;
}


/*!
 * \brief
 * 演奏単位を追加する．
 * 
 * \param bowing
 * ボーイングクラス
 * 
 * \param pu
 * ピッチユニットクラス
 * 
 */
void PlayManager::AddPlay(Bowing *bowing, PitchUnit *pu)
{
	//* 演奏単位を追加する *
	//ボーイングは制御時間全部の制御値が含まれているので無処理で良いが，ピッチユニットのタイミングを
	//ボーイングのタイミングと合わせんだ形で制御時間全部を埋め尽くす制御値を生成する方針．
	std::cout << "[ プレイマネージャ ] 追加します．" << std::endl;

	//演奏時間（ステップ）
	int playTime = bowing->Prepare();

	//ピッチマネージャに追加
	pu->SetPlayTime(playTime);  //時間を指定
	pitchManager->AddPitch(pu); //ピッチマネージャに追加

	//ボーイングマネージャに追加
	bowingManager->AddBowing(bowing);

	//登録演奏単位数をインクリメント
	nbPlayUnit++;
}

int PlayManager::Prepare()
{
	std::cout << "[ プレイマネージャ ] プリペア．" << std::endl;
	int playTime = bowingManager->Prepare();
	pitchManager->Prepare();

	//演奏時間を返す
	return playTime;
}

int PlayManager::Prepare(int start, int end)
{
	std::cout << "[ プレイマネージャ ] 範囲指定プリペア．" << std::endl;
	int playTime = bowingManager->Prepare(start, end);
	pitchManager->Prepare(start, end);

	//演奏時間を返す
	return playTime;
}

void PlayManager::Commit()
{
	std::cout << "[ プレイマネージャ ] コミット．" << std::endl;
	bowingManager->Commit();
	pitchManager->Commit();
}

void PlayManager::InitialMove1()
{
	bowingManager->InitialMove();
	//pitchManager->InitialMove();
}

void PlayManager::InitialMove2()
{
	//bowingManager->InitialMove();
	pitchManager->InitialMove();
}
