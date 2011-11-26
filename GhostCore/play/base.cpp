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

#include "base.hpp"
#include "../actuator/controller.hpp"
#include "../actuator/actuator.hpp"
#include "../vecmath.hpp"
#include "pressure.hpp"
#include "baseGeometry.hpp"
#include "igen.hpp"
#include "../calibration.hpp"

#include <cstdlib>

Base::Base()
{
	pressure = new Pressure();
	igen = new Igen();
	geometry = new BaseGeometry();
	calibration = new Calibration();
	calibration->Load();
	StartString = -1; // 初期状態
	EndString   = -1; // 初期状態
}

Base::~Base()
{
	delete pressure;
	delete igen;
	delete geometry;
	delete calibration;
}

void Base::SetStringPair(int _StartString, int _EndString)
{
	if(StartString == -1 && EndString == -1){
		//初期状態
		//終了状態チェックはスキップ
	}else{
		//前回の終了状態が今回の初期状態である
		//if(EndString != _StartString){
		//	Error::Critical(0, QObject::tr("前回の終了位置と，カレントの初期位置は同一でなければなりません"));	
		//}
	}

	StartString = _StartString;
	EndString = _EndString;

	//弓圧クラスにストリングペアを設定
	pressure->SetStringPair(StartString, EndString);

	//移弦クラスにストリングペアを設定
	igen->SetStringPair(StartString, EndString);
}

void Base::SetPlayerString(int _playerString)
{
	PlayerString = _playerString;

	//弓圧クラスに演奏弦を設定
	pressure->SetPlayerString(PlayerString);
	//移弦クラスに演奏弦を設定
	igen->SetPlayerString(PlayerString);
}

void Base::SetPressureVector(QVector<double> p)
{
	//弓圧クラスに圧力ベクタを設定
	pressure->SetPressureVector(p);
}

QVector<double> Base::GetPressureVector()
{
	return pressure->pressureVector;
}


/*!
 * \brief
 *
 * 圧力ベクタのプリペア
 *
 * 用語の定義
 * 初期位置： 初期位置
 * 終了位置： 次に演奏する弦を含む両弦位置
 * 演奏弦: 演奏弦
 *
 * ■ΔL
 * 指定された圧力ベクター（10msecごとの圧力％値[0,100]）を満たすように，共通接線条件下で，ΔLを制御する．
 * 制御対象のΔLは，演奏弦の接触円に対して半径が大きくなる方向へ．
 * これは，単に最大のΔLに対して圧力値をデコードするだけでOK．初期弓圧・終了弓圧については考える必要がない
 * という点が肝（これを考え出すと複雑になりすぎて死亡する）
 *
 * ■演奏中移弦とΔL
 * 演奏弦と移弦先弦のΔLを両方見る必要がある．ただし，これらは独立に考えてよい．つまり，演奏弦のΔLは，
 * 上で記載した圧力ベクターによる直接制御でOK．
 * 移弦先弦のΔLは，初期状態から移弦後状態（移弦後弓圧は必ずゼロなのでキャリブレーション状態）への残半径を，
 * 圧力ベクターの長さで割った値を使って線形変化させる．
 * 演奏弦と移弦先弦のΔLを独立に扱えることが肝（これが従属していると複雑になりすぎて死亡するぜ！）
 *
 * ■演奏終了後移弦とΔL
 * 演奏中移弦と同じことではある．演奏弦のΔLを変化させずに，移弦先弦のΔLを上と同様に変化させる．
 * 安全のために演奏弦のΔLをマイナス方向へ若干マージンを与えても良い．
 *
 * ■初期状態について
 * 初期状態の計算が必要なのは，移弦先弦に対する残半径．キャリブレーション位置同士であれば実測は必要ないが，
 * 初期弓圧（残余弓圧）存在下においては，残半径は変化してしまう．従って，P0,P1直線と移弦先弦中心との距離を
 * 計算して，Lから引き算して残余半径を求める（→この残余半径を圧力ベクタ長で除算する）
 *
 */
int Base::Prepare()
{	
	//毎回クリアする
	axis0.clear();
	axis1.clear();

	//弦選択に矛盾が無いかどうかを確認
	if(StartString - PlayerString == 0 || StartString - PlayerString == 1 || StartString - PlayerString == -1){
		//初期状態と同一か前後弦番号しか許容できない
		std::cout << "[ 運弓プリペア ] 初期状態: " << StartString << ", 演奏弦: " << PlayerString  << ", 終了状態: " << EndString << " [ OK ]" << std::endl;
	}else{
		std::cout << "[ 運弓プリペア ] 初期状態: " << StartString << ", 演奏弦: " << PlayerString  << ", 終了状態: " << EndString << " [ NG ]" << std::endl;
		Error::Critical(0, QObject::tr("弦選択状態に矛盾があります．強制終了します．"));
	}

	//弓圧クラスでプリペアする
	pressure->Prepare(axis0,axis1);
	//移弦クラスでプリペアする
	igen->Prepare(axis0,axis1);

	//軸0をプリペア
	Controller *control = Controller::GetInstance();
	control->axis[0]->SetMode(MLinear::CONTINUOUS);  //制御モード（連続）
	control->axis[0]->SetPositionVector(axis0);		 //制御ベクタをセット
	control->axis[0]->Prepare();					 //プリペア

	//軸1をプリペア
	control->axis[1]->SetMode(MLinear::CONTINUOUS);  //制御モード（連続）
	control->axis[1]->SetPositionVector(axis1);		 //制御ベクタをセット
	control->axis[1]->Prepare();					 //プリペア

	//ボーイング時間を返す
	if(axis0.size() == axis1.size()){
		return axis0.size();
	}else{
		//サイズが合わないのは計算ミス
		std::cout << "Error: axis0.size != axis1.size, axis0.size()==" << axis0.size() << ", axis1.size()==" << axis1.size() << std::endl;
		Error::Critical(0, QObject::tr("axis0.size != axis1.size"));
		return 0;
	}
}

/*!
 * \brief
 * コミット（軸0，軸1に対して）
 * 
 */
void Base::Commit()
{
	Controller *control = Controller::GetInstance();
	for(int i=0;i<2;i++){
		control->axis[i]->Commit(); //コミット
	}
}

/*!
 * \brief
 * 初期状態へ初期移動する（これは最初の移動のみ）
 *
 * \remarks
 * この関数の実行で，事前にプリペアされていた内容は消えるので Not Prepared エラーの原因になり易いので最大注意
 * 
 */
void Base::InitialMove(int nb_string)
{
	//if(nb_string == 1 || nb_string == 3 || nb_string == 5){
		//初期移動実行
		QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
		Controller *control = Controller::GetInstance();
		//軸0,1についてプリペア・コミット（ここまでで事前にプリペアされていた内容は消えるので Not Prepared エラーの原因になりやすいいので注意）
		for(int i=0;i<3;i++){
			control->axis[i]->SetMode(MLinear::JOG);
			control->axis[i]->SetPosition(pos[nb_string].start[i]);
			control->axis[i]->SetDuration(2000,0.5,0.5);
			control->axis[i]->Prepare();
		}
		for(int i=0;i<3;i++){
			control->axis[i]->Commit();
		}
		
	//}else{
	//	Error::Critical(0, QObject::tr("[ BASE ] 初期移動位置指定が不正です: %1").arg(nb_string));
	//}
}


//// OBSOLETE ////

//void Base::Prepare1()
//{
	/* Version 2 obsolete
	
	//初期状態が自弦位置か両弦位置かの判定
	int initial = pressure->GetString();
	if(initial%2 == 0){
		//初期状態が自弦位置だった；自弦位置から自弦位置への移動（移弦動作を行う必要はない）
		std::cout << "自弦位置->自弦位置のため移弦動作は行いません" << std::endl;

		//ジャムらない拘束条件の下で，圧力ベクタのプリペア
		pressure->Prepare(axis0, axis1);
		//終了
	}else{
		//初期状態が両弦位置だった；両弦位置から自弦位置への移動

		//// 残余弓圧の有無を判定 ////

		//キャリブレーション位置を取得
		QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
		double p0_end = 0; // 終了状態のp0[0,100]
		double p1_end = 0; // 終了状態のp1[0,100]
		//圧力ベクタ
		QVector<double> p = pressure->pressureVector;
		if(p[p.size()-1] > EPS){ 
			//残余弓圧が残っている→終了状態の再計算が必要
			geometry->deltaL(pressure->ConvertToMM(p[p.size()-1]), p0_end, p1_end);
		}els
			//残余弓圧がない→終了状態は自弦位置
			p0_end = pos[string1].start[0];
			p1_end = pos[string1].start[1];
		}//終了状態の確定完了

		//圧力ベクタ拘束条件，かつ，ジャムらない拘束条件，の下で，両弦位置から自弦位置へ移弦

		//TODO	
	}
	*/

	/* Version 1 obsolete

	//初期状態を取得（ここは１回分なのでカレントの位置を取得；複数回をまとめることを設計上想定していない→計算時間がクリティカル）
	Controller *controller = Controller::GetInstance();
	ActuatorStatus status0 = controller->axis[0]->GetStatus();
	ActuatorStatus status1 = controller->axis[1]->GetStatus();

	//初期状態が自弦位置か両弦位置かの判定，ただし，平行移動拘束条件による弓圧付加状態である可能性があるので，ベクトルとして比較しなければならない．
	int positionType = geometry->positionType(status0.Position, status1.Position, string1);
	if(positionType == 1){
		//初期状態が自弦位置の場合；移弦動作を行う必要はない
		std::cout << "自弦位置のため移弦動作は行いません．" << std::endl;
		
		//初期状態が自弦位置なので，弓圧は自弦に掛ければよい
		pressure->SetString(string1);

		//圧力ベクタのプリペア
		pressure->Prepare(axis0, axis1);

		//終了
	}else if(positionType == 2){
		//初期状態が両弦位置の場合

		//初期状態の残余弓圧があるかどうかを判定し，判定結果に応じて終了状態を計算

		//キャリブレーション位置を取得
		QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
		double p0_end = 0; // 終了状態のp0[0,100]
		double p1_end = 0; // 終了状態のp1[0,100]

		QVector<double> p = pressure->pressureVector;
		if(p[p.size()-1] > EPS){ 
			//残余弓圧が残っている→終了状態の再計算が必要
			geometry->deltaL(pressure->ConvertToMM(p[p.size()-1]), p0_end, p1_end);
		}else{
			//残余弓圧がない→終了状態は自弦位置
			p0_end = pos[string1].start[0];
			p1_end = pos[string1].start[1];
		}

		//今，string1 == string2 条件だった．演奏したいのは，単弦もしくは両弦か？		
	}
	*/
//}


