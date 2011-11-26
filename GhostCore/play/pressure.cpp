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

#include "pressure.hpp"
#include "../actuator/controller.hpp"
#include "../calibration.hpp"
#include "../vecmath.hpp"
#include "baseGeometry.hpp"
#include "../error.hpp"

const double Pressure::MaxMM = 5.0;//最大押し込み幅[mm]

Pressure::Pressure()
{
	geometry = new BaseGeometry();
	calibration = new Calibration();
	calibration->Load();
}

Pressure::~Pressure()
{
	delete calibration;
	delete geometry;
}


/*!
 * \brief
 * [0,100]正規化表現を実際の押し込み幅に変換して返す．上位層では正規化表現を利用しているので，実値は，ベースジオメトリでの計算のみに使用．	
 * 
 * \param percent
 * [0,100]正規化表現．上位層で利用している．
 * 
 * \returns
 * 実際の押し込み幅[mm]
 * 
 */
double Pressure::ConvertToMM(double percent)
{
	return (percent/100.)*MaxMM;
}


/*!
 * \brief
 * 圧力付加対象弦を指定する
 * 
 * \param string
 * 弦番号[1,7]
 * 
 * 
 */
void Pressure::SetPlayerString(int _playerString)
{	
	//圧力付加対象弦
	playerString = _playerString;
}


/*!
 * \brief
 * 圧力ベクタを指定する
 * 
 * \param v
 * 圧力ベクタ系列
 * 
 */
void Pressure::SetPressureVector(QVector<double> p)
{
	pressureVector = p;
}


/*!
 * \brief
 * 初期位置・終了位置を指定する
 * 
 * \param _startString
 * 初期位置[1,3,5]
 * 
 * \param _endString
 * 終了位置[1,3,5]
 * 
 */
void Pressure::SetStringPair(int _startString, int _endString)
{
	startString = _startString;
	endString = _endString;
}


/*!
 * \brief
 * 左側弦を演奏しながら左側に一段階移弦する（新汎用実装；ほぼ安定な演奏中移弦）
 * OK 
 *
 * \param nb_string
 * 全位置[0,1,2,3,4,5,6]
 *
 * \remarks
 * 残余半径のステップ割は，着弦対象弦への残余半径減少スピードが遅すぎ，弓圧変化速度が速い場合に，ベースが逆に動く副作用がある．これは明らかに非効率．
 * 従って，最後の変曲点までは平行条件（もしくは二円接触条件）により平行付圧して，最後の変曲点以降で残余半径を一次で減少させる．
 * 
 */
void Pressure::ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	if(nb_string == 0){
		//左側弦が存在しない
		Error::Critical(0, QObject::tr("[ PRESSURE ] Invalid nb_string: %1").arg(nb_string));
	}

	if(nb_string % 2 == 0){

		//// 単弦位置 ////	

		//左側弦は両弦位置，演奏弦と移弦先が異なるので問題が難しい．最後の変曲点までは平行条件で平行付圧して，最後の変曲点以降で残余半径を一気に落とす

		//移弦先位置
		int left_string = nb_string - 1;

		//弓の弾性変形の補正係数（最大押込み幅の調整によって変化する）
		//double reformCoef = 0.5;

		//移弦先位置では着弦（弓圧ゼロ），演奏弦は残余弓圧が存在する場合があるが，許容できない仕様（和音の片弦だけを強く弾くことはできない）
		//つまり，単弦位置から両弦位置へ演奏中移弦する場合は，残余弓圧がゼロであることが必須．
		if(pressureVector[pressureVector.size()-1] != 0){
			Error::Critical(0, QObject::tr("[ PRESSURE ] 単弦位置から両弦位置への演奏中移弦では，残余弓圧はゼロでなければなりません．"));
		}
		
		//最後の変曲点を探す
		int lastLocalMaxIndex = 0;
		double maxValue = 0;
		for(int i=0;i<pressureVector.size()-1;i++){
			if(pressureVector[i] < pressureVector[i+1]){
				lastLocalMaxIndex = i+1;
				maxValue = pressureVector[i+1];
			}
			if(maxValue == pressureVector[i]){
				lastLocalMaxIndex = i;
			}
		}

		//平行移動条件で単弦単純与圧
		for(int i=0;i<lastLocalMaxIndex;i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

		//平行移動条件での単弦単純与圧終了時点での残余半径を求める
		double lastResidue = geometry->residualRadius2(pos0,pos1,left_string-1);
		double residueByStep = lastResidue / (double)(pressureVector.size()-(lastLocalMaxIndex+1));

		//二円接触条件で与圧
		int lineCounter = 0;
		for(int i=lastLocalMaxIndex; i<pressureVector.size();i++){

			//共通接線条件下，相対的に左弦を演奏する
			//右側；オリジナル押込み
			double deltaL0 = ConvertToMM(pressureVector[i]);
			//左側；
			double deltaL1 = residueByStep*lineCounter - lastResidue;	
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, left_string);
			axis0.append(pos0);
			axis1.append(pos1);
			lineCounter++;

		}

	}else{

		//// 両弦位置 ////

		//左側弦は単弦位置，演奏弦と移弦先が同じなので問題は比較的簡単．

		//両弦位置からdeltaL1与圧し，平行条件でdeltaL1を与えるpos0とpos1に対して残余半径を求め，deltaL0を0.5mmずつ平行条件に近づくように追い込むステップを
		//平行条件に一致するまで繰り返し，平行条件に一致した後は，残りの弓圧ベクトルを平行条件与圧で消化する．
		
		//両弦位置を構成する左側弦
		int left_string = nb_string - 1;
		//両弦位置を構成する右側弦
		int right_string = nb_string + 1;
		
		//1ステップで追い込んでよい最大幅（おおまかな値）
		double step = 5.0 / (double)pressureVector.size();

		int parallelTransIndex = 0;
		//pressureVectorが連続していれば前の残余弓圧と今回の初期弓圧は当然に一致する．ここで特段の配慮をする必要はない．
		for(int i=0;i<pressureVector.size();i++){
			//共通接線条件下，相対的に左弦を演奏する

			//左側；オリジナル押込み deltaL1
			double deltaL1 = ConvertToMM(pressureVector[i]);

			//平行条件与圧でdeltaL1を与えるpos0_refとpos1_refは？
			double pos0_ref = 0;
			double pos1_ref = 0;
			geometry->deltaL(deltaL1, pos0_ref, pos1_ref, left_string);


			//平行条件での右弦への距離は
			double residue = geometry->residualRadius2(pos0_ref,pos1_ref,right_string)*(-1.);

			//右側への押込み距離を決定する
			double deltaL0 = 0;

			if(i != 0){

				//現在の直線pos0,pos1と右弦との距離は？
				double curr_residue = geometry->residualRadius2(pos0,pos1,right_string)*(-1.); //正で返る→負へ

				//平行条件との距離の差は？
				double curr_diff = residue - curr_residue;

				//収束判定（ここは少し微妙）
				if(qAbs(curr_diff) < 0.2){ // 0.5mm / 10msec
					//十分収束しているので一気に平行条件へ落とす

					deltaL0 = residue;		//追い込み完了
					parallelTransIndex = i; //平行条件へ与圧する

				}else{

					//まだ追い込める				
					if(curr_diff < 0){
						//左弦への距離が内側
						deltaL0 = (-1.)*step*i;
					}else{
						//左弦の距離が外側（内側へ追い込んでいく）
						deltaL0 = (+1.)*step*i;
					}
				}

			}

			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
			if(parallelTransIndex != 0){
				break;
			}

		}

		if(parallelTransIndex == 0){
			Error::Critical(0,QObject::tr("[ PRESSURE ] 両弦位置からの右方向演奏中移弦制御が収束しませんでした."));
		}
		for(int i=parallelTransIndex+1;i<pressureVector.size();i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, left_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}			
	}
}


/*!
 * \brief
 * 右側弦を演奏しながら右側に一段階移弦する（新汎用実装；ほぼ安定な演奏中移弦）
 * OK 
 *
 * \param nb_string
 * 全位置[0,1,2,3,4,5,6]
 *
 * \remarks
 * 残余半径のステップ割は，着弦対象弦への残余半径減少スピードが遅すぎ，弓圧変化速度が速い場合に，ベースが逆に動く副作用がある．これは明らかに非効率．
 * 従って，最後の変曲点までは平行条件（もしくは二円接触条件）により平行付圧して，最後の変曲点以降で残余半径を一次で減少させる．
 * 
 */
void Pressure::ToRightSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{

	double pos0 = 0;
	double pos1 = 0;

	if(nb_string == 6){
		//右側弦が存在しない
		Error::Critical(0, QObject::tr("[ PRESSURE ] Invalid nb_string: %1 ( Right side string does not exists! )").arg(nb_string));
	}

	if(nb_string % 2 == 0){

		//// 単弦位置 ////	

		//右側弦は両弦位置，最後の変曲点までは平行与圧し，最後の変曲点以降で残余半径を一気に減少させる

		//移弦先位置
		int right_string = nb_string + 1;

		//弓の弾性変形の補正係数（最大押込み幅の調整によって変化する）
		//double reformCoef = 0.5;

		//移弦先位置では着弦（弓圧ゼロ），演奏弦は残余弓圧が存在する場合があるが，許容できない仕様（和音の片弦だけを強く弾くことはできない）
		//つまり，単弦位置から両弦位置へ演奏中移弦する場合は，残余弓圧がゼロであることが必須．
		if(pressureVector[pressureVector.size()-1] != 0){
			Error::Critical(0, QObject::tr("[ PRESSURE ] 単弦位置から両弦位置への演奏中移弦では，残余弓圧はゼロでなければなりません（右側移弦）"));
		}
		
		//最後の変曲点を探す
		int lastLocalMaxIndex = 0;
		double maxValue = 0;
		for(int i=0;i<pressureVector.size()-1;i++){
			if(pressureVector[i] < pressureVector[i+1]){
				lastLocalMaxIndex = i+1;
				maxValue = pressureVector[i+1];
			}
			if(maxValue == pressureVector[i]){
				lastLocalMaxIndex = i;
			}
		}

		//平行移動条件で単弦単純与圧
		for(int i=0;i<lastLocalMaxIndex;i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

		//平行移動条件での単弦単純与圧終了時点での残余半径を求める
		double lastResidue = geometry->residualRadius2(pos0,pos1,right_string+1); // 正の値で返る ... 
		double residueByStep = lastResidue / (double)(pressureVector.size()-(lastLocalMaxIndex+1));

		//二円接触条件で与圧
		int lineCounter = 0;
		for(int i=lastLocalMaxIndex; i<pressureVector.size();i++){

			//共通接線条件下，相対的に左弦を演奏する

			//右側；
			double deltaL0 = residueByStep*lineCounter - lastResidue; // マイナス値→ゼロへ OK	
			//左側；オリジナル押込み
			double deltaL1 = ConvertToMM(pressureVector[i]);

			//右弦位置で両円接触条件で展開
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, right_string);
			axis0.append(pos0);
			axis1.append(pos1);
			lineCounter++;

			if(i == pressureVector.size()-1){
				std::cout << std::endl;
			}

		}

	}else{

		//// 両弦位置 ////

		//右側弦は単弦位置，演奏弦と移弦先が同じなので問題は比較的簡単．

		//両弦位置からdeltaL1与圧し，平行条件でdeltaL1を与えるpos0とpos1に対して残余半径を求め，deltaL0を0.5mmずつ平行条件に近づくように追い込むステップを
		//平行条件に一致するまで繰り返し，平行条件に一致した後は，残りの弓圧ベクトルを平行条件与圧で消化する．
		
		//両弦位置を構成する左側弦
		int left_string = nb_string - 1;
		//両弦位置を構成する右側弦
		int right_string = nb_string + 1;
		
		//1ステップで追い込んでよい最大幅（おおまかな値）
		double step = 5.0 / (double)pressureVector.size();

		int parallelTransIndex = 0;
		//pressureVectorが連続していれば前の残余弓圧と今回の初期弓圧は当然に一致する．ここで特段の配慮をする必要はない．
		for(int i=0;i<pressureVector.size();i++){
			//共通接線条件下，相対的に右弦を演奏する

			//右側；オリジナル押込み deltaL0
			double deltaL0 = ConvertToMM(pressureVector[i]);

			//平行条件でdeltaL0を与えるpos0とpos1は？
			double pos0_ref = 0;
			double pos1_ref = 0;
			geometry->deltaL(deltaL0, pos0_ref, pos1_ref, right_string);

			//直線pos0_ref-pos1_refと左弦への距離は（これが現在のdeltaL0で平行条件に至るための距離）
			double residue = geometry->residualRadius2(pos0_ref,pos1_ref,left_string)*(-1.); // 正で返る→負へ

			//左側への押込み距離を決定する
			double deltaL1 = 0;

			if(i != 0){
				//現在の直線pos0,pos1と左弦との距離は？
				double curr_residue = geometry->residualRadius2(pos0,pos1,left_string)*(-1.); //正で返る→負へ

				//平行条件との距離の差は？
				double curr_diff = residue - curr_residue;

				//収束判定（ここは少し微妙）
				if(qAbs(curr_diff) < 0.2){ // 0.5mm / 10msec
					//十分収束しているので一気に平行条件へ落とす

					deltaL1 = residue;		//追い込み完了
					parallelTransIndex = i; //平行条件へ与圧する

				}else{

					//まだ追い込める				
					if(curr_diff < 0){
						//左弦への距離が内側
						deltaL1 = (-1.)*step*i;
					}else{
						//左弦の距離が外側（内側へ追い込んでいく）
						deltaL1 = (+1.)*step*i;
					}

				}
			}

			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
			if(parallelTransIndex != 0){
				break;
			}
		}

		//平行与圧
		if(parallelTransIndex == 0){
			Error::Critical(0,QObject::tr("[ PRESSURE ] 両弦位置からの右方向演奏中移弦制御が収束しませんでした."));
		}
		for(int i=parallelTransIndex+1;i<pressureVector.size();i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, right_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}
				
	}

}


/*!
 * \brief
 * 単純平行移動条件による弓圧付加関数（両弦圧力付加と事実上同じ動きになる）
 * 
 * \param axis0
 * 軸0の制御ベクトル
 * 
 * \param axis1
 * 軸1の制御ベクトル
 * 
 * \param nb_string
 * 弦位置番号
 * 
 */
void Pressure::SinglePressure(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	for(int i=0;i<pressureVector.size();i++){
		double dL = ConvertToMM(pressureVector[i]);
		geometry->deltaL(dL,pos0,pos1,nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * 両弦を演奏する
 * 
 * \param nb_string
 * 両弦位置を指定[1,3,5]
 * 
 */
void Pressure::ToBothSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;
	for(int i=0;i<pressureVector.size();i++){
		//共通接線条件下，両弦を演奏する（deltaL0:変化，deltaL1:変化（等率））
		double deltaL0 = ConvertToMM(pressureVector[i]); //右側
		double deltaL1 = deltaL0;						 //左側
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * 両弦位置から相対的に向かって左側弦を演奏する（新実装；弓の弾性変形を考慮して，両弦を押し込む実装）
 * OK
 *
 * \param nb_string
 * 両弦位置を指定[1,3,5]
 *
 * \remarks
 * 弓の弾性変形を厳密に計算するためには，軸2の位置が必要になり，軸0,1と軸2の独立性が失われるため近似する．
 * 
 */
void Pressure::ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;
	//pressureVectorが連続していれば前の残余弓圧と今回の初期弓圧は当然に一致する．ここで特段の配慮をする必要はない．
	for(int i=0;i<pressureVector.size();i++){
		//共通接線条件下，相対的に左弦を演奏する（deltaL0:一定，deltaL1:変化）
		//double deltaL0 = 0;									    //右側 ; オリジナル
		double deltaL0 = ConvertToMM(pressureVector[i])*0.5;	//右側 ; 弾性変形考慮によりn掛けで同一方向へ押し込む（係数は「最大押込幅」の調整によって変わると思われる）
		double deltaL1 = ConvertToMM(pressureVector[i]);		//左側
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * 右側弦を演奏する（新実装；弓の弾性変形を考慮して，両弦を押し込む実装）
 * OK
 *
 * \param nb_string
 * 両弦位置を指定[1,3,5]
 *
 * \remarks
 * TODO
 *
 */
void Pressure::ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;
	for(int i=0;i<pressureVector.size();i++){
		//共通接線条件下，相対的に右弦を演奏する（deltaL0:変化，deltaL1:一定）
		double deltaL0 = ConvertToMM(pressureVector[i]);	 //右側
		double deltaL1 = ConvertToMM(pressureVector[i])*0.5; //左側 ... 弓の弾性変形考慮0.5掛けでこちらも押し込む
		//double deltaL1 = 0;									 //左側 ... オリジナル
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}



/*!
 * \brief
 * 圧力ベクタをプリペアする（開始弦位置と終了弦位置に制限のない新実装）
 * 0	1弦
 * 1	1-2弦
 * 2	2弦
 * 3	2-3弦
 * 4	3弦
 * 5	3-4弦
 * 6	4弦
 * 
 * \param axis0
 * 軸0の制御ベクタ（appendされます）
 * 
 * \param axis1
 * 軸1の制御ベクタ（appendされます）
 * 
 */
void Pressure::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString % 2 == 0){
		//開始弦が単弦位置
		if(startString != playerString){
			//開始弦と演奏弦は同一弦でなければならない
			Error::Critical(0, QObject::tr("開始弦と演奏弦は同一弦でなければなりません: %1 != %2").arg(startString).arg(playerString));
		}

		if(startString - endString == 1){
			//左側弦へ演奏中移弦
			ToLeftSideWithIgen(axis0, axis1, startString);
		}else if(startString - endString == -1){
			//右側弦へ演奏中移弦
			ToRightSideWithIgen(axis0, axis1, startString);
		}else{
			//演奏中移弦せず平行与圧（遠隔弦の場合，演奏中移弦した後，その方向へ一段階演奏中移弦しても良いが，移弦クラスで演奏後に移弦する設計とする）
			SinglePressure(axis0, axis1, startString);
		}


	}else{
		//開始弦が両弦位置
		if(!(startString == playerString || startString - playerString == 1 || startString - playerString == -1)){
			//開始弦と演奏弦の関係はこの通り
			Error::Critical(0, QObject::tr("両弦位置では，演奏弦は同一位置か両弦位置に含まれる左右の単弦である必要があります: %1, %2").arg(startString).arg(playerString));
		}

		if(startString == playerString){

			//終了弦に関わらず両円接触条件与圧
			ToBothSide(axis0,axis1,startString);

		}else if(startString - playerString == 1){

			//左側弦演奏時

			if(endString == startString){
				//元の位置に戻る場合のみ
				ToLeftSide(axis0,axis1,startString);
			}else{
				//それ以外は全て演奏弦先に移弦させる（右遠隔弦の場合，その方向へ戻してもよいが，移弦クラスで演奏後に移弦する設計とする）
				ToLeftSideWithIgen(axis0,axis1,startString);
			}

		}else if(startString - playerString == -1){

			//右側弦演奏時

			if(endString == startString){
				//元の位置に戻る場合のみ
				ToRightSide(axis0,axis1,startString);
			}else{
				//それ以外は全て演奏弦先に移弦させる（右遠隔弦の場合，その方向へ戻してもよいが，移弦クラスで演奏後に移弦する設計とする）
				ToRightSideWithIgen(axis0,axis1,startString);
			}

		}

	}

}



/*!
 * \brief
 * 左側弦を演奏しながら左側に一段階移弦する（新実装；残余半径のステップ割）
 * 
 * \param nb_string
 * 全位置[0,1,2,3,4,5,6]
 * 
 */
/*
void Pressure::ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	//初期状態を計算する
	//Controller *controller = Controller::GetInstance();
	//ActuatorStatus status0 = controller->axis[0]->GetStatus();
	//ActuatorStatus status1 = controller->axis[1]->GetStatus();

	//【設計指針】
	//初期状態は，キャリブレーションされた初期弦位置であることに注意．弓圧が掛かっている状態からの遷移を
	//サポートしていないことに注意．つまり，各ボーイング単位で必ず弓圧力はゼロに落ちていることが前提．
	//
	//初期状態を引き継ぎたい場合は，Bowing::Prepare()の引数をpitchunitのように追加して，Base::Prepare()にも
	//追加，最後にPressure::Prepare()に追加して，この関数まで引っ張ってくる．
	//そして，その状態からの残余半径を再計算する（geometry->margin[]はキャリブレーション位置からのマージンなので使わない）

	if(nb_string == 0){
		Error::Critical(0,QObject::tr("ToLeftSideWithIgen(): Invalid nb_string: %1").arg(nb_string));
	}

	//移弦するための残余半径
	double radius = 0;

	//もともとシングル位置の場合
	if(nb_string == 2){
		//2弦シングル位置から，2弦を演奏しながら，1-2弦位置へ（1弦に着地）
		radius = geometry->margin[2].leftMargin; // 2弦シングル位置での左側マージン
	}else if(nb_string == 4){
		//3弦シングル位置から，3弦を演奏しながら，2-3弦位置へ（2弦に着地）
		radius = geometry->margin[4].leftMargin; // 3弦シングル位置での左側マージン
	}else if(nb_string == 6){
		//4弦シングル位置から，4弦を演奏しながら，3-4弦位置へ（3弦に着地）
		radius = geometry->margin[6].leftMargin; // 4弦シングル位置での左側マージン
	}

	//もともと両弦位置の場合
	if(nb_string == 1){
		//1-2弦位置から1弦を演奏しながら，1弦シングル位置へ
		radius = geometry->margin[0].rightMargin; // 1弦シングル位置での右側マージン
	}else if(nb_string == 3){
		//2-3弦位置から2弦を演奏しながら，2弦シングル位置へ
		radius = geometry->margin[2].rightMargin; // 1つ左の弦位置の右マージン
	}else if(nb_string == 5){
		//3-4弦位置から3弦を演奏しながら，3弦シングル位置へ
		radius = geometry->margin[4].rightMargin;
	}

	//移弦先弦への残余半径を検索
	//double radius = geometry->residualRadius(status0.Position, status1.Position, to_string);
	//移弦に必要なΔLを求める
	double step_deltaL = radius/(double)pressureVector.size();

	for(int i=0;i<pressureVector.size();i++){

		//共通接線条件下，2弦を演奏する
		double deltaL0 = 0; //右側
		double deltaL1 = 0; //左側
		if(nb_string % 2 == 0){//2,4,6

			//もともとシングル位置の場合

			deltaL0 = ConvertToMM(pressureVector[i]);	//右側（左側へ移動するので，もともと左側へ一段階移弦した位置に対しての右側は弓圧付加対象
			deltaL1 = (-radius)+step_deltaL*i;			//左側（こちらは着弦対象）

			//共通接線条件下
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string-1); //左側へ一段階移弦した初期位置で計算

		}else{
			//もともと両弦位置の場合
			deltaL0 = (-radius)+step_deltaL*i;			//右側（着弦対象）
			deltaL1 = ConvertToMM(pressureVector[i]);	//左側（弓圧付加対象）

			//共通接線条件下
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string); //指定位置で計算
		}
		axis0.append(pos0);
		axis1.append(pos1);
	}
}
*/

/*!
 * \brief
 * 左側弦を演奏しながら左側に一段階移弦する（旧汎用実装）
 * 
 * \param nb_string
 * 両弦位置を指定[1,3,5]
 * 
 */
/*
void Pressure::ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	//初期状態を計算する
	Controller *controller = Controller::GetInstance();
	ActuatorStatus status0 = controller->axis[0]->GetStatus();
	ActuatorStatus status1 = controller->axis[1]->GetStatus();
	
	int to_string = 0;
	if(nb_string == 1){
		Error::Critical(0,QObject::tr("ToRightSideWithIgen() の指定が不正"));
	}else if(nb_string == 3){
		//2-3弦位置から2弦を演奏しながら1-2弦位置へ（第1弦に着地）
		to_string = 0;
	}else if(nb_string == 5){
		//3-4弦位置から3弦を演奏しながら2-3弦位置へ（第2弦に着地）
		to_string = 1;
	}

	//移弦先弦への残余半径を検索
	double radius = geometry->residualRadius(status0.Position, status1.Position, to_string);
	//移弦に必要なΔLを求める
	double step_deltaL = radius/(double)pressureVector.size();

	for(int i=0;i<pressureVector.size();i++){

		//共通接線条件下，2弦を演奏する
		double deltaL0 = ConvertToMM(pressureVector[i]);	//右側
		double deltaL1 = (-radius)+step_deltaL*i;					//左側
		//共通接線条件下
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string-2); //左側へ一段階移弦した初期位置で計算
		axis0.append(pos0);
		axis1.append(pos1);
	}
}
*/


/*!
 * \brief
 * 圧力ベクタをプリペアする（旧一般化実装）
 * StartString[1,3,5], EndString[1,3,5]
 * 0	1弦
 * 1	1-2弦
 * 2	2弦
 * 3	2-3弦
 * 4	3弦
 * 5	3-4弦
 * 6	4弦
 * 
 * \param axis0
 * 軸0の制御ベクタ（appendされます）
 * 
 * \param axis1
 * 軸1の制御ベクタ（appendされます）
 * 
 */
/*
void Pressure::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString == 1){
		//1-2弦両弦位置から
		if(endString == 1){ 
			//1-2弦両弦位置へ（移弦無し）
			if(playerString == 0){
				//1弦を演奏
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 1){
				//1-2弦両弦を演奏
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 2){
				//2弦を演奏
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 3){ // ... endString == 5 と等しい
			//2-3弦両弦位置へ
			if(playerString == 0){
				//1弦を演奏する
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 1){
				//1-2両弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 2){
				//2弦を演奏する（演奏しながら2-3弦両弦位置へ移弦）
				ToRightSideWithIgen(axis0,axis1,startString);
			}
		}else if(endString == 5){// ... endString == 3 と等しい
			//3-4弦両弦位置へ
			if(playerString == 0){
				//1弦を演奏する
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 1){
				//1-2両弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 2){
				//2弦を演奏する（演奏しながら2-3弦両弦位置へ移弦）→その後3-4弦両弦位置へ移弦
				ToRightSideWithIgen(axis0,axis1,startString);
			}
		}
	}else if(startString == 3){
		//2-3弦両弦位置から
		if(endString == 1){
			//1-2弦両弦位置へ
			if(playerString == 2){
				//2弦を演奏する（演奏しながら1-2弦両弦位置へ移弦）
				ToLeftSideWithIgen(axis0,axis1,startString);
			}else if(playerString == 3){
				//2-3弦両弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 4){
				//3弦を演奏する
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 3){
			//2-3弦両弦位置へ（移弦無し）
			if(playerString == 2){
				//2弦を演奏する
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 3){
				//2-3弦両弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 4){
				//3弦を演奏する
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 5){
			//3-4弦両弦位置へ
			if(playerString == 2){
				//2弦を演奏する
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 3){
				//2-3弦両弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 4){
				//3弦を演奏する（演奏しながら3-4弦両弦位置へ移弦）
				ToRightSideWithIgen(axis0,axis1,startString);
			}
		}
	}else if(startString == 5){
		//3-4弦両弦位置から
		if(endString == 1){ // ... endString == 3 と等しい
			//1-2弦両弦位置へ
			if(playerString == 4){
				//3弦を演奏する（演奏しながら2-3弦両弦位置へ移弦）→その後1-2弦両弦位置へ移弦
				ToLeftSideWithIgen(axis0,axis1,startString);
			}else if(playerString == 5){
				//3-4弦両弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 6){
				//4弦を演奏する
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 3){// ... endString == 1 と等しい
			//2-3弦両弦位置へ
			if(playerString == 4){
				//3弦を演奏する（演奏しながら2-3弦両弦位置へ移弦）
				ToLeftSideWithIgen(axis0,axis1,startString);
			}else if(playerString == 5){
				//3-4弦を演奏する
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 6){
				//4弦を演奏する
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 5){
			//3-4弦両弦位置へ（移弦無し）
			if(playerString == 4){
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 5){
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 6){
				ToRightSide(axis0,axis1,startString);
			}
		}
	}		
}
*/


/*!
 * \brief
 * 右側弦を演奏しながら右側に一段階移弦する（旧汎用実装）
 * 
 * \param nb_string
 * 両弦位置を指定[1,3,5]
 * 
 */
/*
void Pressure::ToRightSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	//初期状態を計算する
	Controller *controller = Controller::GetInstance();
	ActuatorStatus status0 = controller->axis[0]->GetStatus();
	ActuatorStatus status1 = controller->axis[1]->GetStatus();
	
	int to_string = 0;
	if(nb_string == 1){
		//1-2弦位置から2弦を演奏しながら2-3弦位置へ（第3弦に着地）
		to_string = 2;
	}else if(nb_string == 3){
		//2-3弦位置から3弦を演奏しながら3-4弦位置へ（第4弦に着地）
		to_string = 3;
	}else if(nb_string == 5){
		Error::Critical(0,QObject::tr("ToRightSideWithIgen() の指定が不正"));
	}

	//移弦先弦への残余半径を検索
	double radius = geometry->residualRadius(status0.Position, status1.Position, to_string);
	//移弦に必要なΔLを求める
	double step_deltaL = radius/(double)pressureVector.size();

	for(int i=0;i<pressureVector.size();i++){

		//共通接線条件下，2弦を演奏する: deltaL0一定，deltaL1変化
		double deltaL0 = (-radius)+step_deltaL*i;					//右側
		double deltaL1 = ConvertToMM(pressureVector[i]);	//左側
		//共通接線条件下
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string+2); //右側へ一段階移弦した初期位置で計算
		axis0.append(pos0);
		axis1.append(pos1);
	}
}
*/

