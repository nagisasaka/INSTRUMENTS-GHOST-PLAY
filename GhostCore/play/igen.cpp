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

#include "igen.hpp"
#include "baseGeometry.hpp"
#include "../error.hpp"
#include "../actuator/controller.hpp"
#include "../calibration.hpp"

//一段階移弦の所要時間（機械限界を考慮して設定すること）
//const int Igen::DurationTime = 100;//[ms]
const int Igen::DurationTime = 80;//[ms]

Igen::Igen()
{
	geometry = new BaseGeometry();
	calibration = new Calibration();
	calibration->Load();
	startString = 0;
	endString = 0;
}

Igen::~Igen()
{
	delete geometry;
	delete calibration;
}

/*!
 * \brief
 * 初期位置と終了位置を設定
 * 
 * \param _string1
 * 初期位置[0,6]
 * 
 * \param _string2
 * 終了位置[0,6]
 * 
 */
void Igen::SetStringPair(int _startString, int _endString)
{
	startString = _startString;
	endString = _endString;
}

void Igen::SetPlayerString(int _playerString)
{
	playerString = _playerString;
}


/*!
 * \brief
 * 右へ移弦（新実装）
 * 
 * \param nb_string
 * 移弦前ポジション
 * 
 * 
 */
void Igen::ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int _startString)
{

	//// axisに追加していくので現在のaxisの最終値がここの初期値（キャリブレーションは使わない）

	//初期状態の取得
	double initial_pos0 = axis0[axis0.size()-1];
	double initial_pos1 = axis1[axis1.size()-1];

	//1ステップの時間を取得
	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//移弦用に追加するベクタサイズ
	int size = (double)DurationTime / (double)cycle;

	//軸上位置
	double pos0 = 0;
	double pos1 = 0;

	//// 移弦ステップ分左に移弦する

	//ループ内で決定する
	int current_string = 0;
	int left_string = 0;
	int right_string = 0;

	//クリアランスはループ内で固定が良いと思われる
	double deltaL  = -5.0;
	double deltaL0 = deltaL;	//右弦に対するクリアランス（※注意※一単位右の両弦位置を使うので，着目弦が相対左弦になることに注意）
	double deltaL1 = deltaL;	//着目弦に対するクリアランス

	nbIgenStep = qAbs(nbIgenStep);

	for(int i=0;i<nbIgenStep;i++){

		//(1)最初に，自弦と右弦に対して一定のクリアランスを保つまで運動する．
		//(2)次に，右弦と，さらに右弦に対して一定のクリアランスを保つまで運動する．

		current_string = _startString + i;  //着目弦
		left_string = current_string - 1;   //左弦（current_string==0の時のみ左弦は存在しないが問題ない）
		right_string = current_string + 1;  //右弦

		if(current_string % 2 == 0){
			//現在着目している位置が単弦位置の場合；

			//ループの初期位置
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){
				//右側の次の単弦位置
				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, right_string+1)*(-1.);
				//着目弦
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, current_string)*(-1.);
			}else{
				//右側の次の単弦位置
				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, right_string+1)*(-1.);
				//着目弦
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, current_string)*(-1.);
			}
			
			double deltaL0_step = (deltaL0-curr_deltaL0) / (double)size;	//右弦ステップ
			double deltaL1_step = (deltaL1-curr_deltaL1) / (double)size;	//着目弦ステップ

			//右弦位置で両円接触条件にて展開
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step; //右弦ステップ
				curr_deltaL1 += deltaL1_step; //着目弦
				geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, right_string); //右弦位置で展開
				axis0.append(pos0);
				axis1.append(pos1);
			}

			curr_deltaL0 = 0;
			curr_deltaL1 = 0;

		}else{
			//現在着目している位置が両弦位置の場合；

			//ループの初期位置
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){
				//右弦単弦位置
				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, right_string)*(-1.);
				//左弦単弦位置
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, left_string)*(-1.);
			}else{
				//右側単弦位置
				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, right_string)*(-1.);
				//左側単弦位置
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, left_string)*(-1.);
			}

			//右弦に平行条件でdeltaL0を与える直線pos0_ref,pos1_refの，左弦との残余半径は？
			double pos0_ref = 0;
			double pos1_ref = 0;

			//右弦の平行条件でdeltaL0を与える直線pos0_ref,pos1_refを求める
			geometry->deltaL(deltaL0, pos0_ref, pos1_ref, right_string);

			//左弦との残余半径を求める
			double residue = geometry->residualRadius2(pos0_ref, pos1_ref, left_string)*(-1.);

			//ステップを求める
			double deltaL0_step = (deltaL0 - curr_deltaL0) / (double)size; //右弦ステップ
			double deltaL1_step = (residue - curr_deltaL1) / (double)size; //左弦ステップ

			//着目弦位置で両円接触条件にて展開
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step;
				curr_deltaL1 += deltaL1_step;
				geometry->commonTangent(curr_deltaL0,curr_deltaL1,pos0,pos1,current_string);//着目弦位置で展開
				axis0.append(pos0);
				axis1.append(pos1);
			}
			
			curr_deltaL0 = 0;
			curr_deltaL1 = 0;
		}

	}

	//このループを抜けた時点で，平行条件もしくは両円接触条件でクリアランスdeltaL1=deltaL0を保った状態となっているので
	//キャリブレーション位置まで平行移動する．

	current_string++;

	//最後の平行移動
	if(current_string % 2 == 0){

		//最終状態が単弦位置の場合は；
		double curr_deltaL = geometry->residualRadius2(pos0,pos1,current_string)*(-1.);
		double deltaL_step = curr_deltaL / (double)size;

		//着目弦位置で平行移動させる
		for(int i=0;i<size;i++){
			curr_deltaL -= deltaL_step; // 徐々にゼロへ
			geometry->deltaL(curr_deltaL, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}else{
		right_string = current_string + 1;
		left_string = current_string - 1;

		//最終状態が両弦位置の場合は；
		double curr_deltaL0 = geometry->residualRadius2(pos0,pos1,right_string)*(-1.);
		double curr_deltaL1 = geometry->residualRadius2(pos0,pos1,left_string)*(-1.);

		double deltaL0_step = curr_deltaL0 / (double)size;
		double deltaL1_step = curr_deltaL1 / (double)size;

		//着目弦位置で両円接触条件で移動させる
		for(int i=0;i<size;i++){
			curr_deltaL0 -= deltaL0_step;
			curr_deltaL1 -= deltaL1_step;
			geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}

	//これで最終着弦状態となった．
}


/*!
 * \brief
 * 左へ移弦（新実装）
 * 
 * \param nb_string
 * 移弦前ポジション
 * 
 * 
 */
void Igen::ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int _startString)
{

	//// axisに追加していくので現在のaxisの最終値がここの初期値（キャリブレーションは使わない）

	//初期状態の取得
	double initial_pos0 = axis0[axis0.size()-1];
	double initial_pos1 = axis1[axis1.size()-1];

	//1ステップの時間を取得
	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//移弦用に追加するベクタサイズ
	int size = (double)DurationTime / (double)cycle;

	//軸上位置
	double pos0 = 0;
	double pos1 = 0;

	//// 移弦ステップ分左に移弦する

	//ループ内で決定する
	int current_string = 0;
	int left_string = 0;
	int right_string = 0;

	//クリアランスはループ内で固定が良いと思われる
	double deltaL  = -5.0;
	double deltaL0 = deltaL;	//着目弦に対するクリアランス
	double deltaL1 = deltaL;	//左弦に対するクリアランス

	for(int i=0;i<nbIgenStep;i++){

		//(1)最初に，自弦と左弦に対して一定のクリアランスを保つまで運動する．
		//(2)次に，左弦とさらに左弦に対して一定のクリアランスを保つまで運動する．

		current_string = _startString - i;  //着目弦
		left_string = current_string - 1;   //左弦
		right_string = current_string + 1;  //右弦

		if(current_string % 2 == 0){
			//現在着目している位置が単弦位置の場合；

			//ループの初期位置
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){

				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, current_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, left_string-1)*(-1.);  //正の値で返る→負へ

			}else{
				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, current_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, left_string-1)*(-1.); //正の値で返る→負へ

			}

			double deltaL0_step = (deltaL0-curr_deltaL0) / (double)size;	//着目弦ステップ
			double deltaL1_step = (deltaL1-curr_deltaL1) / (double)size;	//左弦ステップ

			//左弦位置で両円接触条件にて展開
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step; //着目弦
				curr_deltaL1 += deltaL1_step; //左弦
				geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, left_string); //左弦位置で展開
				axis0.append(pos0);
				axis1.append(pos1);
			}

			curr_deltaL0 = 0;
			curr_deltaL1 = 0;

		}else{
			//現在着目している位置が両弦位置の場合；

			//ループの初期位置
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){

				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, right_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, left_string)*(-1.);

			}else{

				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, right_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, left_string)*(-1.);

			}

			//左弦に平行条件でdeltaL1を与える直線pos0_ref,pos1_refの，右弦との残余半径は？
			double pos0_ref = 0;
			double pos1_ref = 0;

			//左弦の平行条件でdeltaL1を与える直線pos0,pos1を求める
			geometry->deltaL(deltaL1, pos0_ref, pos1_ref, left_string);

			//右弦との残余半径を求める
			double residue = geometry->residualRadius2(pos0_ref, pos1_ref, right_string)*(-1.);

			//ステップを求める
			double deltaL0_step = (residue - curr_deltaL0) / (double)size; //右弦ステップ
			double deltaL1_step = (deltaL1 - curr_deltaL1) / (double)size; //左弦ステップ

			//着目弦位置で両円接触条件にて展開
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step;
				curr_deltaL1 += deltaL1_step;
				geometry->commonTangent(curr_deltaL0,curr_deltaL1,pos0,pos1,current_string);//着目弦位置で展開
				axis0.append(pos0);
				axis1.append(pos1);
			}
			
			curr_deltaL0 = 0;
			curr_deltaL1 = 0;
		}

	}

	//このループを抜けた時点で，平行条件もしくは両円接触条件でクリアランスdeltaL1=deltaL0を保った状態となっているので
	//キャリブレーション位置まで平行移動する．

	current_string--;

	//最後の平行移動
	if(current_string % 2 == 0){

		//最終状態が単弦位置の場合は；
		double curr_deltaL = geometry->residualRadius2(pos0,pos1,current_string)*(-1.);
		double deltaL_step = curr_deltaL / (double)size;

		//着目弦位置で平行移動させる
		for(int i=0;i<size;i++){
			curr_deltaL -= deltaL_step; // 徐々にゼロへ
			geometry->deltaL(curr_deltaL, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}else{
		right_string = current_string + 1;
		left_string = current_string - 1;

		//最終状態が両弦位置の場合は；
		double curr_deltaL0 = geometry->residualRadius2(pos0,pos1,right_string)*(-1.);
		double curr_deltaL1 = geometry->residualRadius2(pos0,pos1,left_string)*(-1.);

		double deltaL0_step = curr_deltaL0 / (double)size;
		double deltaL1_step = curr_deltaL1 / (double)size;

		//着目弦位置で両円接触条件で移動させる
		for(int i=0;i<size;i++){
			curr_deltaL0 -= deltaL0_step;
			curr_deltaL1 -= deltaL1_step;
			geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}

	//これで最終着弦状態となった．
}



/*!
 * \brief
 * 右へ移弦
 * 
 * \param nb_string
 * 移弦前ポジション
 * 
 * 
 */
void Igen::ToRight(QVector<double> &axis0, QVector<double> &axis1,int _startString)
{
	//キャリブレーション位置を使う（初期状態を計算する必要はない）
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions は[0,100]正規化されているためmm単位にはなっていないことに注意

	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//移弦用に追加するベクタサイズ
	int size = (double)DurationTime / (double)cycle;

	//軸上位置
	double pos0 = 0;
	double pos1 = 0;
	
	//残余半径計算対象弦
	int nb_string = 0;

	//シングル右側
	if(_startString == 1){//1-2弦，シングル右は2-3弦
		//残余半径計算対象弦は第3弦
		nb_string = 2;
	}else if(_startString == 3){//2-3弦，シングル右は3-4弦
		//残余半径計算対象弦は第4弦
		nb_string = 3;
	}else{
		//右はない
		Error::Critical(0, QObject::tr("ToRight()関数の第一引数が不正です"));
	}

	double radius = geometry->residualRadius(pos[_startString].start[0], pos[_startString].start[1], nb_string);//移弦先は第3弦
	//移弦に必要なΔLを求める
	double step_deltaL = radius/(double)size;
	//移弦を行う時間分の制御ベクトルを追加する
	for(int i=1;i<=size;i++){				
		double deltaL0 = (-radius)+step_deltaL*i; //相対右側
		double deltaL1 = 0;//ここは安全マージンを取るなら取っても良い // 相対左側
		//if(i == 50){
		//	cout << "debug";
		//}
		if(i == size){
			std::cout << "deltaL0=" << deltaL0 << "deltaL1=" << deltaL1 << " deltaL0 を厳密に0に落とします." << std::endl;
			deltaL0 = 0;
		}
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, _startString + 2);
		if(i == size){
			std::cout << "pos0=" << pos0 << "pos1=" << pos1 << std::endl;
		}
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * 左へ移弦（旧実装）
 * 
 * \param nb_string
 * 移弦前ポジション
 * 
 * 
 */
void Igen::ToLeft(QVector<double> &axis0, QVector<double> &axis1,int _startString)
{
	//キャリブレーション位置を使う（初期状態を計算する必要はない）
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions は[0,100]正規化されているためmm単位にはなっていないことに注意

	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//移弦用に追加するベクタサイズ
	int size = (double)DurationTime / (double)cycle;

	//軸上位置
	double pos0 = 0;
	double pos1 = 0;
	
	//残余半径計算対象弦
	int nb_string = 0;

	if(_startString == 3){//2-3弦，シングル左は1-2弦
		//残余半径計算対象弦は第1弦
		nb_string = 0;
	}else if(_startString == 5){//3-4弦，シングル左は2-3弦
		//残余半径計算対象弦は第2弦
		nb_string = 1;
	}else{
		//左はない
		Error::Critical(0, QObject::tr("ToLeft()関数の第一引数が不正です"));
	}

	double radius = geometry->residualRadius(pos[_startString].start[0], pos[_startString].start[1], nb_string);
	//移弦に必要なΔLを求める
	double step_deltaL = radius/(double)size;
	//移弦を行う時間分の制御ベクトルを追加する
	for(int i=1;i<=size;i++){				
		double deltaL0 = 0;	//ここは安全マージンを取るなら取っても良い //相対右側
		double deltaL1 = (-radius)+step_deltaL*i;					   //相対左側
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, _startString - 2);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * プリペアの実行．与えられた軸制御ベクターに移弦用のシーケンスをアペンドする．
 * 
 * \param axis0
 * 軸0の制御ベクタ（絶対位置％指定[0,100]）
 * 
 * \param axis1
 * 軸1の制御ベクタ（絶対位置％指定[0,100]）
 * 
 */
void Igen::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString == endString) return;

	//移弦ステップ数
	int nbIgenStep = startString - endString;

	if(startString % 2 == 0){
		//// 単弦位置

		if(!(startString - endString == 1 || startString - endString == -1)){

			//// 演奏中移弦されていない場合について，こちらで必要分移弦

			if(nbIgenStep < 0){
				//右側に移弦
				ToRightSide(axis0,axis1,nbIgenStep,startString);
			}else{
				//左側に移弦
				ToLeftSide(axis0,axis1,nbIgenStep,startString);
			}
		}

	}else{

		//// 両弦位置

		// 演奏中二段階移弦が可能だが，今は暫定未実装．

		if(startString == playerString){

			if(nbIgenStep < 0){
				ToRightSide(axis0,axis1,nbIgenStep,startString);
			}else{
				ToLeftSide(axis0,axis1,nbIgenStep,startString);
			}

		}else if(startString - playerString == 1){

			//相対左側弦演奏時，相対左弦へ演奏中移弦済である

			if(startString - endString == 1){
				//移弦は完了している
			}else{
				//移弦未完了
				nbIgenStep = playerString - endString; // 上書き
				if(nbIgenStep < 0){
					//移弦後位置が開始弦位置になる
					ToRightSide(axis0,axis1,nbIgenStep,playerString);
				}else{
					ToLeftSide(axis0,axis1,nbIgenStep,playerString);
				}
			}
			
		}else if(startString - playerString == -1){

			//相対右側弦演奏時，相対右弦に演奏中移弦済である

			if(startString - endString == -1){
				//移弦は完了している
			}else{
				//移弦未完了
				nbIgenStep = playerString - endString;
				if(nbIgenStep < 0){
					ToRightSide(axis0,axis1,nbIgenStep,playerString);
				}else{
					ToLeftSide(axis0,axis1,nbIgenStep,playerString);
				}
			}
		}
	}
}


/*!
 * \brief
 * プリペアの実行．与えられた軸制御ベクターに移弦用のシーケンスをアペンドする．
 * 
 * \param axis0
 * 軸0の制御ベクタ（絶対位置％指定[0,100]）
 * 
 * \param axis1
 * 軸1の制御ベクタ（絶対位置％指定[0,100]）
 * 
 */
/*
void Igen::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString == 1){//1-2(1)弦から
		if(endString == 1){//1-2(1)弦へ
			//移弦無し
		}else if(endString == 3){//2-3(3)弦へ
			if(playerString == 0){
				//1(0)弦
				ToRight(axis0,axis1,1);
			}else if(playerString == 1){
				//1-2(1)弦
				ToRight(axis0,axis1,1);
			}else if(playerString == 2){
				//2(2)弦
				//演奏しながら1-2(1)→2-3(3)へ移弦済
			}
		}else if(endString == 5){//3-4(5)弦へ
			if(playerString == 0){
				//1弦
				ToRight(axis0,axis1,1);
			}else if(playerString == 1){
				//1-2弦
				ToRight(axis0,axis1,1);
			}else if(playerString == 2){
				//2弦
				//演奏しながら1-2(1)→2-3(3)へ移弦済
			}
			//2-3(3)→3-4(5)へ移弦
			ToRight(axis0,axis1,3);
		}
	}else if(startString == 3){//2-3(3)から
		if(endString == 1){//1-2(1)弦へ
			if(playerString == 2){
				//2(2)弦
				//演奏しながら2-3(3)→1-2(1)へ移弦済
			}else if(playerString == 3){
				//2-3(3)弦
				ToLeft(axis0,axis1,3);
			}else if(playerString == 4){
				//3(4)弦
				ToLeft(axis0,axis1,3);
			}
		}else if(endString == 3){//2-3(3)弦へ
			//移弦無し
		}else if(endString == 5){//3-4(5)弦へ
			if(playerString == 2){
				//2(2)弦
				ToRight(axis0,axis1,3);
			}else if(playerString == 3){
				//2-3(3)弦
				ToRight(axis0,axis1,3);
			}else if(playerString == 4){
				//3(4)弦
				//演奏しながら2-3(3)→3-4(5)弦へ移弦済
			}
		}
	}else if(startString == 5){//3-4(5)弦から
		if(endString == 1){//1-2弦へ
			if(playerString == 4){
				//3弦
				//演奏しながら3-4(5)→2-3(3)へ移弦済
			}else if(playerString == 5){
				//3-4弦
				ToLeft(axis0,axis1,5);
			}else if(playerString == 6){
				//4弦
				ToLeft(axis0,axis1,5);
			}
			ToLeft(axis0,axis1,3);
		}else if(endString == 3){//2-3弦へ
			if(playerString == 4){
				//3(4)弦
				//演奏しながら3-4(5)→2-3(3)へ移弦済
			}else if(playerString == 5){
				//3-4(5)弦
				ToLeft(axis0,axis1,5);
			}else if(playerString == 6){
				//4(6)弦
				ToLeft(axis0,axis1,5);
			}
		}else if(endString == 5){//3-4(5)弦へ
			//移弦無し
		}
	}
}
*/