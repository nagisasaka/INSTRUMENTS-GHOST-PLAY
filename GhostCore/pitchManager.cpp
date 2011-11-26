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

#include "pitchManager.hpp"
#include "error.hpp"
#include "actuator/controller.hpp"


PitchManager::PitchManager()
{
	NbPitchUnit = 2;
}


PitchManager::~PitchManager()
{
	//保持しているピッチユニットを削除
	for(int i=0;i<puVector.size();i++){
		delete puVector[i];
	}
}


void PitchManager::AddPitch(PitchUnit *pu)
{

	//// ここにエラーチェックコードを実装しても良い ////

	puVector.append(pu);
}


void PitchManager::DelPitch(int index)
{
	if(index >= puVector.size()){
		std::cout << "[ ピッチマネージャ ] 削除インデックスが登録ベクタサイズを超えています." << std::endl;
		Error::Critical(0, QObject::tr("[ PitchManager ] Invalid delete index"));
	}

	//削除
	puVector.remove(index);	
}


void PitchManager::UpdatePitch(PitchUnit *pu, int index)
{
	if(index >= puVector.size()){
		std::cout << "[ ピッチマネージャ ] 更新インデックスが登録ベクタサイズを超えています." << std::endl;
		Error::Critical(0, QObject::tr("[ PitchManager ] Invalid update index"));
	}

	//追加
	puVector.insert(index,pu);

	//削除
	puVector.remove(index+1);	
}


PitchUnit* PitchManager::GetPitch(int index)
{
	if(index >= puVector.size()){
		std::cout << "[ ピッチマネージャ ] 取得インデックスが登録ベクタサイズを超えています." << std::endl;
		Error::Critical(0, QObject::tr("[ PitchManager ] Invalid get index"));
	}

	return puVector.at(index);
}


void PitchManager::InitialMove()
{
	//ピッチマネージャのイニシャルムーブはどんな時もゼロ点（最初に時間が音階決定遷移時間が足りないときは，ボーイング側で動かさないことでカバー）

	int duration = 2000;
	Controller *control = Controller::GetInstance();	
	for(int i=0;i<NbPitchUnit;i++){

		//基本制御クラスとの接続関係を見るために一時構築
		PitchUnit *pu = new PitchUnit(i);

		//主軸の基本制御クラスを初期移動
		control->axis[pu->GetNbAxis()]->SetMode(MLinear::JOG);
		control->axis[pu->GetNbAxis()]->SetPosition(0);
		control->axis[pu->GetNbAxis()]->SetDuration(duration);
		control->axis[pu->GetNbAxis()]->Prepare();
		control->axis[pu->GetNbAxis()]->Commit();

		//第二軸の基本制御クラスを初期移動
		if(stringSelectSequence[i][0] == 0){
			control->iaic[pu->GetNbIaic()-1]->GoToString("G");
		}else if(stringSelectSequence[i][0] == 1){
			control->iaic[pu->GetNbIaic()-1]->GoToString("D");
		}else if(stringSelectSequence[i][0] == 2){
			control->iaic[pu->GetNbIaic()-1]->GoToString("A");
		}else if(stringSelectSequence[i][0] == 3){
			control->iaic[pu->GetNbIaic()-1]->GoToString("E");
		}

		//第三軸の基本制御クラスをコミット
		control->servo[pu->GetNbServo()]->SetReady();

		delete pu;
	}

	//シーケンシャルに待つ
	Sleep(duration+500);
}


/*!
 * \brief
 * 登録されている全ピッチユニットオブジェクトについてプリペアを実行する
 * 
 * \returns
 * プリペアで確定したピッチユニットの動作ステップ
 * 
 */
int PitchManager::Prepare()
{
	int start = 0;
	int end = puVector.size();

	return Prepare(start,end);
}


/*!
 * \brief
 * 登録されているピッチユニットオブジェクトの一部をスライスしてプリペアする
 * 
 * \param start
 * スライス開始インデックス
 * 
 * \param end
 * スライス終了インデックス（この値未満；forループ内で「<」 で評価される）
 * 
 * \returns
 * プリペアで確定したピッチユニットの動作ステップ
 * 
 */
int PitchManager::Prepare(int start, int end)
{
	if(puVector.size() < end){
		Error::Warning(0, QObject::tr("[ ピッチマネージャ ] プリペア範囲指定が登録データ数を超えています．"));
		return 0;
	}

	//// 各軸ごとに穴の無いように埋めていく．

	//カレントのピッチユニットに対して，次のピッチユニットの番号を見て；
	//(1)同一の場合は，該当ピッチユニットは通常通りプリペア．それ以外のピッチユニットは前回終了値を使ってパディング．
	//(2)異なる場合は，該当ピッチユニットは通常通りプリペア．次のピッチユニットに該当するピッチユニットはカレントのプレイタイム以内に初期移動シーケンスを追加してパディング．
	//   それ以外のピッチユニットは，前回終了値を使ってパディング（軸3,4しか制御下に無い場合は「それ以外」は存在しない）．




	//// 各制御軸の値をクリア

	//主軸群；ピッチ決定
	axis[0].clear();  //軸3クリア #0ピッチユニット
	axis[1].clear();  //軸4クリア #1ピッチユニット

	//第二軸群；弦選択
	stringSelectSequence[0].clear(); //#0ピッチユニット
	stringSelectSequence[1].clear(); //#1ピッチユニット

	//第三軸群；弦押下
	pushpullSequence[0].clear(); //#0ピッチユニット
	pushpullSequence[1].clear(); //#1ピッチユニット

	//// エンドインデックスを計算（puVectorのループでひとつ先を見ているので，puVectorの最後の要素だけ注意）
	int endIndex = end;
	if(end == puVector.size()){
		endIndex = puVector.size() - 1;
	}

	//ピッチユニットで現在利用されていないユニットについても，制御ベクタのギャップがあってはならないので，
	//それを埋める必要があるのと，次の演奏に備えて，事前移動させておくことをここで行わなければならない．
	//
	//その際に，制御ベクタサイズは，どんなときでも常に一定になっていることがチェック条件になる．

	for(int i=start;i<endIndex;i++){

		//カレントのピッチユニット
		PitchUnit *puCurr = puVector.at(i);

		//次のピッチユニット
		PitchUnit *puNext = puVector.at(i+1);
		
		//該当ピッチユニットについては通常通りプリペア
		if(axis[puCurr->GetNbUnit()].size() == 0){
			//最初の最初
			puCurr->Prepare(0);
		}else{
			puCurr->Prepare(axis[puCurr->GetNbUnit()][axis[puCurr->GetNbUnit()].size()-1]);
		}

		//主軸
		QVector<double> temp = puCurr->GetPositionVector();
		for(int k=0;k<temp.size();k++){
			axis[puCurr->GetNbUnit()].append(temp[k]);
		}

		//第二軸（弦選択）
		QVector<int> temp2 = puCurr->GetStringSelectSequence();
		for(int k=0;k<temp2.size();k++){
			stringSelectSequence[puCurr->GetNbUnit()].append(temp2[k]);
		}

		//第三軸（弦押下）
		QVector<int> temp3 = puCurr->GetPushpullSequence();
		for(int k=0;k<temp3.size();k++){
			pushpullSequence[puCurr->GetNbUnit()].append(temp3[k]);
		}
				
		//// 非該当ピッチユニットについては，次のピッチユニットの番号と，次の選択弦により，動作が異なる

		if(puCurr->GetNbUnit() == puNext->GetNbUnit()){

			//// カレントのピッチユニットと次のピッチユニットが同一の場合；

			for(int j=0;j<NbPitchUnit;j++){
				
				if(puCurr->GetNbUnit() == j){

					//// カレントのピッチユニットに関しては既に処理済なので何もしない

					//// 次のピッチユニットに関して，演奏弦が異なる場合には，このユニットが移弦することになるのでコンタクトディレイを仕込んでおく

					if(puCurr->GetStringName() != puNext->GetStringName()){
						////演奏弦が異なれば；
						puNext->SetContactDelay(20); // 100ステップのコンタクトディレイを仕込んでおく 40step = 200msec = 0.4秒
					}
					
				}else{

					////カレントのピッチユニットと，次のピッチユニット以外は，動作させない．
					
					//主軸に対しては，カレントの演奏時間分を，前回終了値でパディングする
					int playTime = puCurr->GetPlayTime();		  //カレントの演奏時間
					double lastValue = 0;
					if(axis[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = axis[j][axis[j].size()-1]; //前回終了値
					}
					for(int h=0;h<playTime;h++){			   //カレントの演奏時間分，前回終了値でパディング．
						axis[j].append(lastValue);
					}

					//第二軸（弦選択）に対しては，カレントの演奏時間分を，前回終了値で埋める
					if(stringSelectSequence[j].size() == 0){
						//※ここは注意せよ※
						if(j == 0) lastValue = 2;     // 0番ピッチユニットの初期位置はA線
						else if(j == 1) lastValue = 1;// 1番ピッチユニットの初期位置はD線
					}else{
						lastValue = stringSelectSequence[j][stringSelectSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						stringSelectSequence[j].append(lastValue);
					}

					//第三軸（弦押下）に対しては，カレントの演奏時間分を，前回終了値で埋める
					if(pushpullSequence[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = pushpullSequence[j][pushpullSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						pushpullSequence[j].append(lastValue);
					}

				}

			}
		
		}else{

			//// カレントのピッチユニットと次のピッチユニットが異なる場合

			for(int j=0; j<NbPitchUnit; j++){

				if(puCurr->GetNbUnit() == j){

					//// カレントの該当ピッチユニットは，処理済なので何もしなくて良い

				}else if(puNext->GetNbUnit() == j){

					//// 次の該当ピッチユニットについては，ここで移弦させておく

					//カレントの演奏時間
					int playTime = puCurr->GetPlayTime();

					//仮想ピッチユニットを構築
					PitchUnit *virtual_pu = new PitchUnit(j); 

					virtual_pu->SetPlayTime(playTime);
					virtual_pu->SetStringName(puNext->GetStringName()); //次の弦
					virtual_pu->SetPitch(puNext->GetPitchName());       //次の音階
					//コンタクトディレイに関して選択
					//virtual_pu->SetContactDelay(playTime*0.5);	 // playTimeの半分のコンタクトディレイ

					////※ここのコンタクトディレイは余裕がある．先に主軸を動かすのでコンタクトディレイは300msecを超えて，さらに大きく

					virtual_pu->SetContactDelay(80);//0.8sec				 // 100ステップのコンタクトディレイ- 100ステップ = 1秒 50ステップ=0.5秒 10ステップ=100mec = 0.1秒

					//トランザクションタイムに関して選択
					//virtual_pu->SetTransitionTime(playTime * 0.5); // カレントの演奏時間の50%で移動終了を目指す
					if(axis[j].size()==0){
						virtual_pu->Prepare(0);
					}else{
						virtual_pu->Prepare(axis[j][axis[j].size()-1]);
					}

					//主軸
					QVector<double> temp = virtual_pu->GetPositionVector();
					for(int k=0;k<temp.size();k++){
						axis[j].append(temp[k]);
					}

					//第二軸（弦選択）

					QVector<int> temp2 = virtual_pu->GetStringSelectSequence();
					int margin = 30;

					if(temp2.size() < margin){
						for(int k=0;k<temp2.size();k++){
							stringSelectSequence[j].append(temp2[k]);
						}
						std::cout << "演奏時間が" << margin << "ステップより短い状況での移弦操作は難しいかもしれません．" << std::endl;
					}else{

						//カレントのピッチユニットと同軸に入っていく場合
						//→先に主軸を優先する（ピッチが異なるのでジャムらない）
						//if(puNext->GetStringCode() <= puCurr->GetStringCode()){

							//最初のmargin msecまでは前回の自分の弦番号を投入（puCurrは他人なので注意！）
							int lastString = stringSelectSequence[j][stringSelectSequence[j].size()-1];
							for(int k=0;k<margin;k++){
								stringSelectSequence[j].append(lastString);
							}
							//n msecから最後まで新しい弦番号を投入
							for(int k=margin;k<temp2.size();k++){
								stringSelectSequence[j].append(temp2[k]);
							}

						//}else{

							//カレントのピッチユニットと同軸から出て行く場合
							//→先に第二軸を優先する（軸が異なるのでジャムらない）

							//for(int k=0;k<temp2.size();k++){
							//	stringSelectSequence[j].append(temp2[k]);
							//}

							//先に第二軸を動かしたいが，かなりトリッキーな実装になるため，あきらめる！

						//}
					}

					//第三軸（弦押下）
					QVector<int> temp3 = virtual_pu->GetPushpullSequence();
					for(int k=0;k<temp3.size();k++){
						pushpullSequence[j].append(temp3[k]);
					}

					//仮想ピッチユニットを削除
					delete virtual_pu;

				}else{

					//// カレントのピッチユニットと次のピッチユニットが異なる場合で，そのどちらにも該当しないピッチユニットに関しては，動かさない
					
					//主軸に対しては，カレントの演奏時間分を，前回終了値でパディングする
					int playTime = puCurr->GetPlayTime();		  //カレントの演奏時間
					double lastValue = 0;
					if(axis[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = axis[j][axis[j].size()-1]; //前回終了値
					}
					for(int h=0;h<playTime;h++){				  //カレントの演奏時間分，前回終了値でパディング．
						axis[j].append(lastValue);
					}

					//第二軸（弦選択）に対しては，カレントの演奏時間分を，前回終了値で埋める
					if(stringSelectSequence[j].size() == 0){
						//※ここは注意せよ※
						if(j == 0) lastValue = 2;     // 0番ピッチユニットの初期位置はA線
						else if(j == 1) lastValue = 1;// 1番ピッチユニットの初期位置はD線
					}else{
						lastValue = stringSelectSequence[j][stringSelectSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						stringSelectSequence[j].append(lastValue);
					}

					//第三軸（弦押下）に対しては，カレントの演奏時間分を，前回終了値で埋める
					if(pushpullSequence[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = pushpullSequence[j][pushpullSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						pushpullSequence[j].append(lastValue);
					}									
				}
			}
		}

		//この時点で，登録全ての軸の制御値のサイズが同一でなければならない．
		for(int k=0;k<NbPitchUnit-1;k++){
			if(axis[k].size() != axis[k+1].size()){
				//エラー
				std::cout << "[ ピッチマネージャ ] i 番目 : " << i << "番目" << std::endl;
				std::cout << "[ ピッチマネージャ ] ピッチユニット間制御値のサイズが異なります: k, k+1: " << k << ", " << k+1 << " | " << axis[k].size() << "," << axis[k+1].size() << std::endl;
				Error::Critical(0, QObject::tr("[ PitchManager ] control vector size error"));
			}
		}
	}


	if(end == puVector.size()){
		//puVector一杯一杯使うときは，最後のひとつについて処理しないといけない

		//最後のひとつについて処理

		//カレントのピッチユニット
		PitchUnit *puCurr = puVector[puVector.size()-1];

		//該当ピッチユニットについては通常通りプリペア
		puCurr->Prepare(axis[puCurr->GetNbUnit()][axis[puCurr->GetNbUnit()].size()-1]);

		//主軸
		QVector<double> temp = puCurr->GetPositionVector();
		for(int k=0;k<temp.size();k++){
			axis[puCurr->GetNbUnit()].append(temp[k]);
		}

		//第二軸
		QVector<int> temp2 = puCurr->GetStringSelectSequence();
		for(int k=0;k<temp2.size();k++){
			stringSelectSequence[puCurr->GetNbUnit()].append(temp2[k]);
		}

		//第三軸
		QVector<int> temp3 = puCurr->GetPushpullSequence();
		for(int k=0;k<temp3.size();k++){
			pushpullSequence[puCurr->GetNbUnit()].append(temp3[k]);
		}

		//それ以外のピッチユニットについては単にパディング（これは別に無くても構わない）
		for(int j=0;j<NbPitchUnit;j++){
			if(puCurr->GetNbUnit() != j){
				//カレントのピッチユニット以外は；

				//主軸に対しては，カレントの演奏時間分を，前回終了値でパディングする
				int playTime = puCurr->GetPlayTime();		  //カレントの演奏時間
				double lastValue = axis[j][axis[j].size()-1]; //前回終了値
				for(int h=0;h<playTime;h++){				  //カレントの演奏時間分，前回終了値でパディング．
					axis[j].append(lastValue);
				}

				//第二軸（弦選択）に対しては，カレントの演奏時間分を，前回終了値で埋める
				lastValue = stringSelectSequence[j][stringSelectSequence[j].size()-1];
				for(int h=0;h<playTime;h++){
					stringSelectSequence[j].append(lastValue);
				}

				//第三軸（弦押下）に対しては，カレントの演奏時間分を，前回終了値で埋める
				lastValue = pushpullSequence[j][pushpullSequence[j].size()-1];
				for(int h=0;h<playTime;h++){
					pushpullSequence[j].append(lastValue);
				}									
			}
		}

	}

	//結合した各軸の制御ベクトルをセット・プリペア
	Controller *control = Controller::GetInstance();
	
	for(int i=0;i<NbPitchUnit;i++){

		//基本制御クラスとの接続関係を見るために一時構築
		PitchUnit *pu = new PitchUnit(i);

		//主軸の基本制御クラスをプリペア
		control->axis[pu->GetNbAxis()]->SetMode(MLinear::CONTINUOUS);
		control->axis[pu->GetNbAxis()]->SetPositionVector(axis[i]);
		control->axis[pu->GetNbAxis()]->Prepare();

		//第二軸の基本制御クラスをプリペア
		control->iaic[pu->GetNbIaic()-1]->SetMode(IAICtrler::PREDEFINED);
		control->iaic[pu->GetNbIaic()-1]->SetControlVector(stringSelectSequence[i]);
		control->iaic[pu->GetNbIaic()-1]->Prepare();

		//第三軸の基本制御クラスをプリペア
		control->servo[pu->GetNbServo()]->SetMode(Servo::PREDEFINED);
		control->servo[pu->GetNbServo()]->SetControlVector(pushpullSequence[i]);
		control->servo[pu->GetNbServo()]->Prepare();

		delete pu;
	}

	return axis[0].size();
}


void PitchManager::Commit()
{
	std::cout << "[ ピッチマネージャ ] コミット．" << std::endl;

	Controller *control = Controller::GetInstance();	
	for(int i=0;i<NbPitchUnit;i++){

		//基本制御クラスとの接続関係を見るために一時構築
		PitchUnit *pu = new PitchUnit(i);

		//主軸の基本制御クラスをコミット
		control->axis[pu->GetNbAxis()]->Commit();

		//第二軸の基本制御クラスをコミット
		control->iaic[pu->GetNbIaic()-1]->Commit();

		//第三軸の基本制御クラスをコミット
		control->servo[pu->GetNbServo()]->Commit();

		delete pu;
	}

}