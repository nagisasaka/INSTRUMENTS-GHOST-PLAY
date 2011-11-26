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

#include "baseGeometry.hpp"
#include "../vecmath.hpp"
#include "../error.hpp"
#include "../actuator/controller.hpp"

#include <QVector>

const double BaseGeometry::c = 121.25;//[mm] 正中線と軸0（軸1）の距離 //242.5/2 = 121.25
const double BaseGeometry::L = 292.0;//[mm] 軸2と弓の距離

BaseGeometry::BaseGeometry()
{
	calibration = new Calibration();
	calibration->Load();
	//自己呼び出し
	Init();
}

BaseGeometry::~BaseGeometry()
{
	delete calibration;
}

void BaseGeometry::Init()
{
	//ここでジオメトリを計算する；具体的には各弦の平面座標を決定し、制限運動の基礎（接線上の運動）を構築する。

	//第４弦の位置
	origin[0] = CalcStringPosition(0);

	//第３弦の位置
	origin[1] = CalcStringPosition(1);

	//第２弦の位置
	origin[2] = CalcStringPosition(2);

	//第１弦の位置
	origin[3] = CalcStringPosition(3);

	//出力
	/*
	for(int i=0;i<4;i++)
	{
		std::cout << origin[i] << std::endl;
	}
	*/

	//ストリングマージンを計算する
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
	//各弦位置について
	for(int i=0;i<7;i++){
		//軸0
		double pos0 = pos[i].start[0];
		//軸1
		double pos1 = pos[i].start[1];
		//残余半径を計算する
		if(i == 0){
			//1弦シングル位置
			margin[i].leftMargin = 0;
			margin[i].rightMargin= residualRadius(pos0,pos1,1); // 2弦との距離
		}else if(i == 1){
			//1-2弦位置
			margin[i].leftMargin = 0;
			margin[i].rightMargin= residualRadius(pos0,pos1,2); // 3弦との距離
		}else if(i == 2){
			//2弦シングル位置
			margin[i].leftMargin = residualRadius(pos0,pos1,0); // 1弦との距離
			margin[i].rightMargin= residualRadius(pos0,pos1,2); // 3弦との距離
		}else if(i == 3){
			//2-3弦位置
			margin[i].leftMargin = residualRadius(pos0,pos1,0); // 1弦との距離
			margin[i].rightMargin= residualRadius(pos0,pos1,3); // 4弦との距離
		}else if(i == 4){
			//3弦シングル位置
			margin[i].leftMargin = residualRadius(pos0,pos1,1); // 2弦との距離
			margin[i].rightMargin= residualRadius(pos0,pos1,3); // 4弦との距離
		}else if(i == 5){
			//3-4弦位置
			margin[i].leftMargin = residualRadius(pos0,pos1,1); // 2弦との距離
			margin[i].rightMargin= 0;
		}else if(i == 6){
			//4弦位置
			margin[6].leftMargin = residualRadius(pos0,pos1,2); // 3弦との距離
			margin[6].rightMargin = 0;
		}
	}
	//表示
	//for(int i=0;i<7;i++){
	//	std::cout << "nb_string = " << i << ", 左: " << margin[i].leftMargin << ", 右: " << margin[i].rightMargin << std::endl;
	//}
}


/*!
 * \brief
 * パーセント単位をmm単位へ変換する
 * 
 * \param percent
 * パーセント単位
 * 
 * \param nb_axis
 * 軸番号
 * 
 * \returns
 * mm単位
 * 
 */
double BaseGeometry::ConvertPercentToMM(double percent, int nb_axis)
{
	Controller *controller = Controller::GetInstance();
	int max_pulse = ((MLinear*)(controller->axis[nb_axis]))->GetMaxPulse();		  //27000
	int pulse_by_meter = ((MLinear*)(controller->axis[nb_axis]))->GetPulseByMeter();//10000
	int pulse_by_mm = pulse_by_meter/1000;//100
	
	double mm = (percent/100.)*(max_pulse/pulse_by_mm);
	return mm;
}


/*!
 * \brief
 * mm単位をパーセント単位へ変換する
 * 
 * \param mm
 * mm単位
 * 
 * \param nb_axis
 * 軸番号
 * 
 * \returns
 * パーセント単位
 * 
 */
double BaseGeometry::ConvertMMToPercent(double mm, int nb_axis)
{
	Controller *controller = Controller::GetInstance();
	int max_pulse = ((MLinear*)(controller->axis[nb_axis]))->GetMaxPulse();		  //27000
	int pulse_by_meter = ((MLinear*)(controller->axis[nb_axis]))->GetPulseByMeter();//10000
	int pulse_by_mm = pulse_by_meter/1000;//100
	
	double percent = 100. * mm / (max_pulse / pulse_by_mm);
	return percent;
}


/*!
 * \brief
 * 残余半径を計算する
 * 
 * \param pos0
 * P0
 * 
 * \param pos1
 * P1
 * 
 * \param nb_string
 * 中心座標
 * 
 * \returns
 * 残余半径
 * 
 */
double BaseGeometry::residualRadius(double pos0, double pos1, int nb_string)
{
	double residual = L - initialRadius(pos0, pos1, nb_string);
	return residual;
}


double BaseGeometry::residualRadius2(double pos0, double pos1, int nb_string)
{
	double residual = L - initialRadius2(pos0, pos1, nb_string);
	return residual;
}


/*!
 * \brief
 * 初期半径（二点P0,P1を通る直線と，弦平面座標との距離）を求める．
 * 
 * \param pos0
 * P0を与える軸0上の位置％[0,100]
 * 
 * \param pos1
 * P1を与える軸1上の位置％[0,100]
 * 
 * \param nb_string
 * 弦番号[0,3]（弦番号の指定方法に注意）
 * 
 * \remarks
 * 弦番号の指定方法に注意
 * 
 */
double BaseGeometry::initialRadius(double pos0, double pos1, int nb_string)
{
	if(!(nb_string == 0 || nb_string == 1 || nb_string == 2 || nb_string == 3)){
		Error::Critical(0, QObject::tr("residualRadius()にて弦番号指定[0,3]が範囲外です: nb_string=%1").arg(nb_string));
	}

	//mm単位に換算
	Point p0 = Point(+c, ConvertPercentToMM(pos0,0));
	Point p1 = Point(-c, ConvertPercentToMM(pos1,0));

	//距離を求める
	double distance = vecmath::distance_l_p(p0,p1,origin[nb_string]);
	return distance;
}

double BaseGeometry::initialRadius2(double pos0, double pos1, int nb_string)
{
	if(nb_string == 1 || nb_string == 3 || nb_string == 5){
		Error::Critical(0, QObject::tr("residualRadius2()は，両弦位置では定義されていません: nb_string=%1").arg(nb_string));
	}

	int nbString = 0;
	if(nb_string == 0){
		nbString = 0;
	}else if(nb_string == 2){
		nbString = 1;
	}else if(nb_string == 4){
		nbString = 2;
	}else if(nb_string == 6){
		nbString = 3;
	}

	//mm単位に換算
	Point p0 = Point(+c, ConvertPercentToMM(pos0,0));
	Point p1 = Point(-c, ConvertPercentToMM(pos1,0));

	//距離を求める
	double distance = vecmath::distance_l_p(p0,p1,origin[nbString]);
	return distance;
}


/*!
 * \brief
 * 2円の共通接線を求める
 * 
 * \param deltaL0
 * 円0のキャリブレーション半径からの差分
 * 
 * \param deltaL1
 * 円1のキャリブレーション半径からの差分
 * 
 * \param pos0
 * 軸0上の位置[0,100]
 * 
 * \param pos1
 * 軸1上の位置[0,100]
 * 
 * \param nb_string
 * 弦番号[1,3,5]
 *
 * 運子運子
 * 
 * \remarks
 * 弦番号は旧仕様における両弦位置のみをサポートしていることに注意．
 * 
 */
void BaseGeometry::commonTangent(double deltaL0, double deltaL1, double &pos0, double &pos1, int nb_string)
{
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions は[0,100]正規化されているためmm単位にはなっていないことに注意

	if(!(nb_string == 1 || nb_string == 3 || nb_string == 5)){
		Error::Critical(0, QObject::tr("nb_string is not 1,3,5:%1").arg(nb_string));
	}

	Point p0; // p0 キャリブレーションされた軸0上の位置
	Point p1; // p1 キャリブレーションされた軸1上の位置
	Point s0; // string center 
	Point s1; // string center
	Point un; // unit normal 
	if(nb_string == 1){
		//1-2 strings
		double dp0 = pos[1].start[0];
		double dp1 = pos[1].start[1];
		p0 = Point(c,  ConvertPercentToMM(dp0,0));//軸0上のキャリブレーション位置
		p1 = Point(-c, ConvertPercentToMM(dp1,1));//軸1上のキャリブレーション位置
		s0 = origin[1];//2弦の原点（右側）
		s1 = origin[0];//1弦の原点（左側）
		un = unitnormal[1];
	}else if(nb_string == 3){
		//2-3 strings
		double dp0 = pos[3].start[0];
		double dp1 = pos[3].start[1];
		p0 = Point(c,  ConvertPercentToMM(dp0,0));//軸0上のキャリブレーション位置
		p1 = Point(-c, ConvertPercentToMM(dp1,1));//軸1上のキャリブレーション位置
		s0 = origin[2];//3弦の原点（右側）
		s1 = origin[1];//2弦の原点（左側）
		un = unitnormal[3];
	}else if(nb_string == 5){
		//3-4 strings
		p0 = Point(c,  ConvertPercentToMM(pos[5].start[0],0));//軸0上のキャリブレーション位置
		p1 = Point(-c, ConvertPercentToMM(pos[5].start[1],1));//軸1上のキャリブレーション位置
		s0 = origin[3];//4弦の原点（右側）
		s1 = origin[2];//3弦の原点（左側）
		un = unitnormal[5];
	}else{
		Error::Critical(0,QObject::tr("twoCircle: Invalid nb_string %1").arg(nb_string));
	}

	//回転角αを求める（反時計回りを正）
	double sin_alpha = (deltaL1-deltaL0)/vecmath::distance(s0,s1);
	//確認
	//double sin_alpha = (deltaL0 - deltaL1)/vecmath::distance(s0,s1);
	double alpha = (-1.0)*asin(sin_alpha);

	//両円の共通接線を求める

	//最初に単位法線（逆方向）をα回転させる
	//逆向き法線
	Point rev_un = (-1.0)*un;
	Point rp = rot(rev_un,alpha);

	//円0の接点
	double len0 = L + deltaL0;
	Point rpc0 = rp*len0 + s0;

	//円1の接点
	double len1 = L + deltaL1;
	Point rpc1 = rp*len1 + s1;

	//円0と円1の共通接線に対してP0'とp1'を求める
	Point p0_dash; // ΔLを与える，求めるP0'
	Point p1_dash; // ΔLを与える，求めるP1'
	//p0_dash
	if(vecmath::is_intersected_l(rpc0, rpc1, Point(c,0), Point(c,1))){
		p0_dash = vecmath::intersection_l(rpc0, rpc1, Point(c,0), Point(c,1));
	}else{
		Error::Critical(0,QObject::tr("P0' を求める際にエラーが発生しました（交点がありません）"));
	}
	//p1_dash
	if(vecmath::is_intersected_l(rpc0, rpc1, Point(-c,0), Point(-c,1))){
		p1_dash = vecmath::intersection_l(rpc0, rpc1, Point(-c,0), Point(-c,1));
	}else{
		Error::Critical(0,QObject::tr("P1' を求める際にエラーが発生しました（交点がありません）"));
	}

	//単位換算(mmから[0,100]正規化表現へ）
	pos0 = ConvertMMToPercent(p0_dash.imag(),0);
	pos1 = ConvertMMToPercent(p1_dash.imag(),0);
	if(!(0 <= pos0 && pos0 <= 100)){
		Error::Critical(0,QObject::tr("P0' を求める際にエラーが発生しました（求めたP0'の値が不正です: %1）").arg(pos0));		
	}
	if(!(0 <= pos1 && pos1 <= 100)){
		Error::Critical(0,QObject::tr("P1' を求める際にエラーが発生しました（求めたP1'の値が不正です: %1）").arg(pos1));		
	}
}

/*!
 * \brief
 * 点の原点を中心とした回転処理
 * 
 * \param p
 * 回転前の点
 * 
 * \param theta
 * 回転角度（反時計回りを正）
 * 
 * \returns
 * 回転後の点
 * 
 */
Point BaseGeometry::rot(Point pc, double theta)
{
	double x = pc.real()*cos(theta)-pc.imag()*sin(theta);
	double y = pc.real()*sin(theta)+pc.imag()*cos(theta);
	return Point(x,y);
}


/*!
 * \brief
 * 対象弦の平面座標を求める
 * 
 * \param nb_string
 * 対象弦[0,3]
 * 
 * \returns
 * 対象弦の平面座標（デバッグの容易さのために平面座標はmmに統一）
 * 
 */
Point BaseGeometry::CalcStringPosition(int nb_string)
{
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions は[0,100]正規化されているためmm単位にはなっていないことに注意
	
	Point G_p0;
	Point G_p1;
	Point H_p0;
	Point H_p1;

	if(nb_string == 0){//1弦
		//1弦
		G_p0 = Point(c,  ConvertPercentToMM(pos[0].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[0].start[1],0));
		//1弦-2弦
		H_p0 = Point(c,  ConvertPercentToMM(pos[1].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[1].start[1],0));
	}else if(nb_string == 1){//2弦
		//2弦
		G_p0 = Point(c,  ConvertPercentToMM(pos[2].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[2].start[1],0));
		//1弦-2弦
		H_p0 = Point(c,  ConvertPercentToMM(pos[1].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[1].start[1],0));
	}else if(nb_string == 2){//3弦
		//3弦
		G_p0 = Point(c,  ConvertPercentToMM(pos[4].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[4].start[1],0));
		//2弦-3弦
		H_p0 = Point(c,  ConvertPercentToMM(pos[3].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[3].start[1],0));
	}else if(nb_string == 3){//4弦
		//4弦
		G_p0 = Point(c,  ConvertPercentToMM(pos[6].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[6].start[1],0));
		//3弦-4弦
		H_p0 = Point(c,  ConvertPercentToMM(pos[5].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[5].start[1],0));
	}else{
		Error::Critical(0, QObject::tr("CalcStringPosition() での指定弦[0,3]が範囲外です. nb_string=%1").arg(nb_string));
	}

	//// 単弦に接する直線 ////

	//軸2を示すベクトル
	Point G_vec = G_p1 - G_p0;

	//単位法線ベクトル x L（軸2と弓の距離）
	Point Normal_vec1;
	Point Normal_vec2;
	vecmath::unitnormal(G_vec, &Normal_vec1, &Normal_vec2);

	Point Trans_vec;
		
	//採用する単位法線ベクトルはy軸の値が小さいほう
	if(Normal_vec1.imag() < Normal_vec2.imag()){
		Trans_vec = Normal_vec1;
	}else{
		Trans_vec = Normal_vec2;
	}
	//単位法線ベクトルを保存
	if(nb_string == 0){
		unitnormal[0] = Trans_vec;		
	}else if(nb_string == 1){
		unitnormal[2] = Trans_vec;
	}else if(nb_string == 2){
		unitnormal[4] = Trans_vec;
	}else if(nb_string == 3){
		unitnormal[6] = Trans_vec;
	}
	//L倍して接触円上へ
	Trans_vec = Trans_vec * L;
	
	//弓上の二点を計算
	Point G_p0_dash = Trans_vec + G_p0;
	Point G_p1_dash = Trans_vec + G_p1;

	//// 二弦に接する直線 ////

	Point H_vec= H_p1 - H_p0;
	vecmath::unitnormal(H_vec, &Normal_vec1, &Normal_vec2);
	if(Normal_vec1.imag() < Normal_vec2.imag()){
		Trans_vec = Normal_vec1;
	}else{
		Trans_vec = Normal_vec2;
	}
	if(nb_string == 0){
		unitnormal[1] = Trans_vec;
	}else if(nb_string == 1){
		//nb_string == 0の場合と同じ
	}else if(nb_string == 2){
		unitnormal[3] = Trans_vec;
	}else if(nb_string == 3){
		unitnormal[5] = Trans_vec;
	}
	Trans_vec = Trans_vec * L;
	Point H_p0_dash = Trans_vec + H_p0;
	Point H_p1_dash = Trans_vec + H_p1;

	//// 交点を計算 ////
	Point intersects;
	if(vecmath::is_intersected_l(G_p0_dash, G_p1_dash, H_p0_dash, H_p1_dash)){
		intersects = vecmath::intersection_l(G_p0_dash, G_p1_dash, H_p0_dash, H_p1_dash);
	}else{
		Error::Critical(0,QObject::tr("弦座標を決定中にエラーが発生しました（交点がありません: nb_string= %1）").arg(nb_string));
	}
	return intersects;
}


/*!
 * \brief
 * OBSOLETE 自弦位置か両弦位置かを判定する．
 * 
 * \param pos0
 * 軸0の位置（単位は何でもよい）
 * 
 * \param pos1
 * 軸1の位置（単位は何でもよい）
 * 
 * \param nb_string
 * 対象弦
 * 
 * \returns
 * 自弦位置の場合 0 を返す
 * 両弦位置の場合 1 を返す
 * 
 */
int BaseGeometry::positionType(double pos0, double pos1, int nb_string)
{
	std::cerr << "OBSOLETE扱いの関数positionType()が使用されています．" << std::endl;

	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions は[0,100]正規化されているためmm単位にはなっていないことに注意

	//全キャリブレーション位置について
	for(int i=0;i<pos.size();i++){
		//調査対象の二点
		Point p0_dash = Point(c, pos0);
		Point p1_dash = Point(-c,pos1);

		//キャリブレーション位置
		Point c0 = Point(c, pos[i].start[0]); // 軸0のキャリブレーション位置
		Point c1 = Point(-c,pos[i].start[1]); // 軸1のキャリブレーション位置

		//二直線の平行判定
		if(vecmath::is_parallel(p0_dash, p1_dash, c0, c1)){
			if(i%2 == 0){
				//自弦位置; 0[1弦],2[2弦],4[3弦],6[4弦]
				return 0;
			}else{
				//両弦位置; 1[1-2弦],3[2-3弦],5[3-4弦],
				return 1;
			}
		}
	}

	//ここにきた場合は，あらゆるキャリブレーション位置に対して，平行な状態ではなかったということで，これは異常．
	std::cerr << "pos0=" << pos0 << ",pos1=" << pos1 << ",nb_string=" << nb_string << std::endl;
	Error::Critical(0,QObject::tr("positionType()関数でエラーが発生しました．現在軸位置は，あらゆるキャリブレーション位置に対して平行ではありません．"));
	return -1;
}


/*!
 * \brief
 * 自弦位置，もしくは両弦位置からの平行移動拘束条件の下で，弓圧ΔLを与えるP0',P1'を求める．
 * （拘束条件は，つまり，キャリブレーションされた各位置からの平行移動条件という事）
 *
 * \param deltaL
 * ΔL（弓圧の直接比例パラメータ；単位はミリメートル；設計ノート参照）
 * 
 * \param pos0
 * ΔLを与えるP0のY座標[0,100]
 * 
 * \param pos1
 * ΔLを与えるP1のY座標[0,100]
 * 
 * \param nb_string
 * 対象弦[0,6] 
 * 0 : 1弦自弦位置
 * 1 : 1-2弦両弦位置
 * 2 : 2弦自弦位置
 * 3 : 2-3弦両弦位置
 * 4 : 3弦自弦位置
 * 5 : 3-4弦両弦位置
 * 6 : 4弦自弦位置
 * 
 * \remarks
 * 以前OBSOLETE扱いだったが復活させた．
 *
 */
void BaseGeometry::deltaL(double deltaL, double &pos0, double &pos1, int nb_string)
{
	//std::cerr << "OBSOLETE扱いの関数deltaL()が使用されています．" << std::endl;

	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions は[0,100]正規化されているためmm単位にはなっていないことに注意

	Point p0 = Point(c,  ConvertPercentToMM(pos[nb_string].start[0],0));//軸0上のキャリブレーション位置
	Point p1 = Point(-c, ConvertPercentToMM(pos[nb_string].start[1],0));//軸1上のキャリブレーション位置

	Point p0_dash; // ΔLを与える，求めるP0'
	Point p1_dash; // ΔLを与える，求めるP1'
	Point s_origin;   // 利用する弦中心
	Point s_two_dash; // L+ΔL離れた接触円上の点
	Point p_two_dash; // L+ΔL離れた平行移動後のベースライン直線上の点

	if(nb_string == 0){//1弦
		//1弦の弦座標
		s_origin = origin[0];
	}else if(nb_string == 1){//1-2弦
		//1弦の弦座標
		s_origin = origin[0];
	}else if(nb_string == 2){//2弦
		//2弦の弦座標
		s_origin = origin[1];
	}else if(nb_string == 3){//2-3弦
		//2弦の弦座標
		s_origin = origin[1];
	}else if(nb_string == 4){//3弦
		//3弦の弦座標
		s_origin = origin[2];
	}else if(nb_string == 5){//3-4弦
		//3弦の弦座標
		s_origin = origin[2];
	}else if(nb_string == 6){//4弦
		//4弦の弦座標
		s_origin = origin[3];
	}
	
	//弦座標から L + ΔL 離れた接触円上の点の座標を求める s''
	s_two_dash = s_origin - ( unitnormal[nb_string]*(L+deltaL) ); // 法線ベクトルが逆方向なので (-1) です

	//s''を通り，ベースラインと平行なベクトルを求める p''
	p_two_dash = s_two_dash + (p1-p0);

	//s'' と p'' を通る直線とx=cの交点 = p0;	
	if(vecmath::is_intersected_l(p_two_dash, s_two_dash, Point(c,0), Point(c,1))){
		p0_dash = vecmath::intersection_l(p_two_dash, s_two_dash, Point(c,0), Point(c,1));
	}else{
		Error::Critical(0,QObject::tr("P0' を求める際にエラーが発生しました（交点がありません）"));
	}

	//s''とp''を通る直線とx=-cの交点 = p1;
	if(vecmath::is_intersected_l(p_two_dash, s_two_dash, Point(-c,0), Point(-c,1))){
		p1_dash = vecmath::intersection_l(p_two_dash, s_two_dash, Point(-c,0), Point(-c,1));
	}else{
		Error::Critical(0,QObject::tr("P1' を求める際にエラーが発生しました（交点がありません）"));
	}

	//単位換算(mmから[0,100]正規化表現へ）
	//pos0 = p0_dash.imag()*100. / (max_pulse / pulse_by_mm);
	//pos1 = p1_dash.imag()*100. / (max_pulse / pulse_by_mm);
	pos0 = ConvertMMToPercent(p0_dash.imag(),0);
	pos1 = ConvertMMToPercent(p1_dash.imag(),0);
	if(!(0 <= pos0 && pos0 <= 100)){
		Error::Critical(0,QObject::tr("P0' を求める際にエラーが発生しました（求めたP0'の値が不正です: %1）").arg(pos0));		
	}
	if(!(0 <= pos1 && pos1 <= 100)){
		Error::Critical(0,QObject::tr("P1' を求める際にエラーが発生しました（求めたP1'の値が不正です: %1）").arg(pos1));		
	}
}

