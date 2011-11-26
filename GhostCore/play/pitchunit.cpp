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

#include "pitchunit.hpp"
#include "../actuator/controller.hpp"
#include "../actuator/mlinear/mlinear.hpp"
#include "../pitchCalibration.hpp"

#include <cmath>

/*!
 * \brief
 * コンストラクタ，利用するアクチュエータ番号を初期化
 * 
 * \param nb_unit
 * 音階決定ユニット番号[0,1]
 * 
 */
PitchUnit::PitchUnit(int _nb_unit) : nb_unit(_nb_unit)
{
	//利用するアクチュエータ番号を初期化
	if(nb_unit == 0){

		//軸番号
		nb_axis  = 3;
		//サーボ番号
		nb_servo = 0;
		//IAIC番号
		nb_iaic  = 2; 

	}else if(nb_unit == 1){

		//軸番号
		nb_axis  = 4;
		//サーボ番号
		nb_servo = 1;
		//IAIC番号
		nb_iaic  = 1; 

	}

	//その他
	stringName = "G"; // 初期化
	pitchName = "G";  // 初期化
	vibrateFlag = false;
	rigenFlag = false;
	transitionTime = 0;
	playTime = 0; //ボーイングマネージャからプリペア時に与えられる
	vibrateHz = 0;
	vibratePercent = 0;
	contactDelay = 0;

	//設定ロード
	calibration = new PitchCalibration(nb_unit);
	calibration->Load();
	z_value = 0;
	c_value = 0;

	prepared = false;
}


/*!
 * \brief
 * データからのコンストラクタ
 * 
 * \param initData
 * セーブデータ
 * 
 */
PitchUnit::PitchUnit(QStringList init)
{

	QStringList line1_p = init[1].split(",");
	if(line1_p.size() != 8){
		Error::Critical(0, QObject::tr("[ PITCHUNIT ] Invalid InitData size: %1").arg(line1_p.size()));
	}

	nb_unit        = line1_p[0].toInt();
	stringName     = line1_p[1];
	pitchName      = line1_p[2];
	int vibrate    = line1_p[3].toInt();
	int rigen      = line1_p[4].toInt();
	transitionTime = line1_p[5].toInt();
	vibrateHz      = line1_p[6].toDouble();
	vibratePercent = line1_p[7].toDouble();

	if(vibrate == 1) vibrateFlag = true;
	else vibrateFlag = false;

	if(rigen == 1) rigenFlag = true;
	else rigenFlag = false;

	playTime = 0; //外部性変数のため保存しない

	//利用するアクチュエータ番号を初期化
	if(nb_unit == 0){

		//軸番号
		nb_axis  = 3;
		//サーボ番号
		nb_servo = 0;
		//IAIC番号
		nb_iaic  = 2; 

	}else if(nb_unit == 1){

		//軸番号
		nb_axis  = 4;
		//サーボ番号
		nb_servo = 1;
		//IAIC番号
		nb_iaic  = 1; 

	}

	//設定ロード
	calibration = new PitchCalibration(nb_unit);
	calibration->Load();
	z_value = 0;
	c_value = 0;

	prepared = false;
}


/*!
 * \brief
 * 保存可能な形式に変換
 * 
 * \returns
 * 保存可能な文字列
 * 
 */
QString PitchUnit::toString()
{
	//保存可能な形式へ変換
	int vibrate = 0;
	if(vibrateFlag) vibrate = 1;
	int rigen = 0;
	if(rigenFlag) rigen = 1;

	QString writer = QString("PITCHUNIT\n");
	writer.append(QString("%1,%2,%3,%4,%5,%6,%7,%8\n").arg(nb_unit).arg(stringName).arg(pitchName).arg(vibrate).arg(rigen).arg(transitionTime).arg(vibrateHz).arg(vibratePercent));
	return writer;
}


PitchUnit::~PitchUnit()
{
	delete calibration;
}


void PitchUnit::CalibrationReLoad()
{
	calibration->Load();
}


/*!
 * \brief
 * ピッチユニット主軸の現在位置を返す
 * 
 * \returns
 * 現在位置[0,100]
 * 
 */
double PitchUnit::GetCurrentPosition()
{
	Controller *control = Controller::GetInstance();
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	ActuatorStatus status = axis->GetStatus();
	double pos = status.Position;
	return pos;
}


/*!
 * \brief
 * 制御系の弦番号体系から，保存ファイルの可読性のある弦番号形式に変換する
 * 
 * \param nb_string
 * 制御系弦番号体系
 * 
 */
void PitchUnit::SetString(int nb_string)
{
	if(nb_string == 0){
		stringName = "G";
	}else if(nb_string == 1){
		//相対左弦
		stringName = "G";
	}else if(nb_string == 2){
		stringName = "D";
	}else if(nb_string == 3){
		//相対左弦
		stringName = "D";
	}else if(nb_string == 4){
		stringName = "A";
	}else if(nb_string == 5){
		//相対左弦
		stringName = "A";
	}else if(nb_string == 6){
		stringName = "E";
	}else{
		Error::Critical(0, QObject::tr("[ PITCHUNIT ] Invalid nb_string : %1 ").arg(nb_string));
	}
	calibration->SetStringName(stringName);

	//弦が変更されたら再プリペアが必要
	prepared = false;
}


/*!
 * \brief
 * ピッチ名を指定する
 * 
 * \param pitchName
 * ピッチ名文字列
 * 
 */
void PitchUnit::SetPitch(QString _pitchName)
{
	pitchName = _pitchName;
	calibration->SetPitchName(pitchName);

	//ピッチが変更されたら再プリペアが必要
	prepared = false;
}


/*!
 * \brief
 * 前回終了状態未指定のプリペア
 * 
 * \returns
 * プリペア後の確定した動作ステップ数
 * 
 * \remarks
 * 前回終了状態未指定の場合は，現在主軸位置を前回終了位置として用いる．
 * 
 */
int PitchUnit::Prepare()
{
	Controller *control = Controller::GetInstance();
	ActuatorStatus status = control->axis[nb_axis]->GetStatus();

	//未指定の場合は現在位置を前回終了位置として取り扱う．
	return Prepare(status.Position);
}

int PitchUnit::GetStringCode()
{
	int val = 0;
	if(stringName == "G"){
		val = 0;
	}else if(stringName == "D"){
		val = 1;
	}else if(stringName == "A"){
		val = 2;
	}else if(stringName == "E"){
		val = 3;
	}
	return val;
}


/*!
 * \brief
 * 前回終了状態を指定したプリペア
 * 
 * \param lastPosition
 * 前回終了位置[0,100]
 * 
 * \returns
 * プリペア後に確定した動作ステップ数
 * 
 */
int PitchUnit::Prepare(double lastPosition)
{
	//// 各制御シーケンスクリア

	//主軸（ピッチ決定）
	pitchSequence.clear();
	//第二軸（弦選択）
	stringSelectSequence.clear();
	//第三軸（弦押下）
	pushpullSequence.clear();

	//// セットされた内容をキャリブレーションファイルからリゾルブする

	QVector<PitchCalibration::CalibratedPositions> positions = calibration->GetCalibratedPositions();
	bool found = false;
	for(int i=0;i<positions.size();i++){
		PitchCalibration::CalibratedPositions cpos = positions[i];
		if(cpos.nb_pitchUnit == nb_unit && cpos.stringName == stringName && cpos.pitchName == pitchName){
			z_value = cpos.z_value;
			c_value = cpos.c_value;
			found = true;
			break;
		}
	}

	if(!found){
		std::cout << "[ ピッチユニット ] キャリブレーションファイル中に設定が見つかりません: stringName=" << stringName.toStdString() << ",pitchName=" << pitchName.toStdString() << ",nbUnit=" << nb_unit << std::endl;
		Error::Critical(0, QObject::tr("Could not find calibration data in the pitch calibration file."));
	}

	//旧実装（連続値に対応できないので前回終了位置は引数として取得した）
	//ピッチシーケンスはカレントの座標からの差分として作る．
	//Controller *control = Controller::GetInstance();
	//ActuatorStatus status = control->axis[nb_axis]->GetStatus();
	//double start_position = status.Position;

	double start_position = lastPosition; // 前回終了位置
	double end_position = z_value;        // 今回目標位置
	double curr = start_position;
	double run_length = end_position - start_position; // 今回走破距離

	double step = 0;
	if(transitionTime == 0){

		//遷移時間が自動決定の時は，最短時間で遷移する．
		std::cout << "[ ピッチユニット ] 遷移自動決定" << std::endl;
		//ステップは最大速度 4[%/step]
		step = 4;

	}else{

		//遷移時間が指定されている．この指定時間で遷移するように動かす
		std::cout << "[ ピッチユニット ] 遷移時間指定されています: " << transitionTime << " [ms]" << std::endl;

		//run_length[%]をtransitionTime[ステップ]で移動する．
		step = run_length / transitionTime; // 仮に1stepだとするとrun_lengthを1ステップで移動することになる．2stepだとするとrun_lengthを2stepで移動．
		if(step > 4){ // 最大速度制限は本来は軸に定義するべきだが暫定実装
			std::cout << "[ ピッチユニット ] 移動速度超過エラー." << std::endl;
		}

	}

	//音階決定移動（主軸の移動）に掛かったステップ数→下のほうで離弦遷移に用いる
	int nbPitchDescition = 0;

	if(abs(run_length) < 1e-5){ // ここの判定基準は誤差に注意すること
		pitchSequence.append(end_position);
		nbPitchDescition++;
		if(transitionTime){
			//遷移時間が指定されている時に移動距離が短すぎることの警告
			std::cout << "[ ピッチユニット ] 移動距離が短すぎ，遷移時間指定を満たせません．指定値を勘違いしていませんか？" << std::endl;
		}
	}else{
		if(abs(run_length) < step){
			pitchSequence.append(start_position);
			pitchSequence.append(end_position);
			nbPitchDescition+=2;
		}else{
			while(true){
				pitchSequence.append(curr);
				nbPitchDescition++;
				if(run_length > 0){
					curr = curr+step;
					if(curr > end_position){
						break;
					}
				}else{
					curr = curr-step;
					if(curr < end_position){
						break;
					}
				}
			}
			pitchSequence.append(end_position);
			nbPitchDescition++;
		}
	}
	std::cout << "[ ピッチユニット ] 移動シーケンス決定，ステップ数 : " << 	nbPitchDescition << " ステップ．" << std::endl;

	//最後にパディング
	if(playTime){
		std::cout << "[ ピッチユニット ] 演奏時間が指定されているためパディングを行います．" << std::endl;

		//パディング数を求める＝（演奏時間）－（現在までに使ったステップ数）
		int togo = playTime - pitchSequence.size();
		if(togo < 0){
			std::cout << "[ ピッチユニット ] パディング値が負です．設定上ありえません．" << std::endl;
			Error::Critical(0, QObject::tr("[ ピッチユニット ] パディング値が負です．設定上ありえません．"));
		}

		if(vibrateFlag){

			//ビブラートフラグが立っている場合は，移動後にビブラートする．ビブラートしながら移動することは暫定的に無視
			//ビブラート時間が必要になるため，時間を指定させる必要があるか？だとすると，ここで時間分最終制御値をパディングしてしまえば良いか？
			//使わないほうの軸は次の位置へ先回り（非同軸条件）なのでＯＫ

			int vibrateNbStep = MakeVibrate(togo);

			//最後余った分をパディングする
			int togo2 = playTime - pitchSequence.size();
			std::cout << "[ ピッチユニット ] ビブラート後残存パディング数: " << togo2 << std::endl; 
			double last_value = pitchSequence[pitchSequence.size()-1];
			for(int i=0;i<togo2;i++){
				pitchSequence.append(last_value);
			}

		}else{
			//ビブラートフラグがたっていない場合

			double last_value = pitchSequence[pitchSequence.size()-1];
			std::cout << "[ ピッチユニット ] 最終値: " << last_value << " でパディング実行します．パディング数: " << togo << std::endl;

			//パディングを実行
			for(int i=0;i<togo;i++){
				pitchSequence.append(last_value);
			}
		
		}

	}else{

		std::cout << "[ ピッチユニット ] 演奏時間が指定されていないため，パディングを行いません，この指定は演奏計画上は有り得ませんが，勘違いしていませんか？";

	}

	//// 第二軸に関して制御シーケンスを埋める
	int val = 0;
	if(stringName == "G"){
		val = 0;
	}else if(stringName == "D"){
		val = 1;
	}else if(stringName == "A"){
		val = 2;
	}else if(stringName == "E"){
		val = 3;
	}
	for(int i=0;i<pitchSequence.size();i++){
		stringSelectSequence.append(val);
	}

	//// 第三軸に関して制御シーケンスを埋める

	//コンタクトディレイと離弦遷移の兼ね合い
	int nbNotContactStep = 0;
	if(rigenFlag){
		//離弦遷移フラグが立っている場合
		if(contactDelay < nbPitchDescition){
			//コンタクトディレイより，ピッチ決定移動ステップの方が長い場合は，コンタクトディレイは吸収される
			nbNotContactStep = nbPitchDescition;	
		}else{
			//コンタクトディレイの方が，ピッチ決定移動ステップより長い場合は，コンタクトディレイ分を待つ
			nbNotContactStep = contactDelay;
		}
	}else{
		//離弦遷移フラグが立っていない場合（通常）
		nbNotContactStep = contactDelay; // これもデフォルトはゼロ
	}

	for(int i=0;i<pitchSequence.size();i++){
		if(i < nbNotContactStep){
			//離弦
			pushpullSequence.append(0);
		}else{
			//弦押下
			pushpullSequence.append(1);
		}
	}

	////プリペア実行

	//主軸
	Controller *control = Controller::GetInstance();
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->SetMode(MLinear::CONTINUOUS);
	axis->SetPositionVector(pitchSequence);
	axis->Prepare();

	//第二軸
	control->iaic[nb_iaic-1]->SetMode(IAICtrler::PREDEFINED);
	control->iaic[nb_iaic-1]->SetControlVector(stringSelectSequence);
	control->iaic[nb_iaic-1]->Prepare();

	//第三軸
	control->servo[nb_servo]->SetMode(Servo::PREDEFINED);
	control->servo[nb_servo]->SetControlVector(pushpullSequence);
	control->servo[nb_servo]->Prepare();

	//プリペア終了
	prepared = true;

	//サイズを返す（マネージャクラスではこのサイズについて，指定時間分を最終制御値でパディングすることになる）
	return pitchSequence.size();
}


/*!
 * \brief
 * ビブラートシーケンスを作り，ピッチシーケンスに追加する．
 * 
 * \param togo
 * 残り利用可能なステップ数
 * 
 * \returns
 * 作成したビブラートシーケンスで利用したステップ数
 * 
 * \remarks
 * 関数内でピッチシーケンスに追加しているので注意
 * 
 */
int PitchUnit::MakeVibrate(int togo)
{
	std::cout << "[ ピッチユニット ] ビブラート指定." << std::endl;

	//// ビブラート周期と半値幅が指定されている．

	//ビブラート1周期の時間は？
	double vibrate_time = 1.0 / vibrateHz; // 秒 ( 5 Hz の時 0.2 秒)
	
	//ステップ数換算で？
	int vibrate_step = vibrate_time * 1000. / (double) Controller::GetCycle(); // ステップ ( 5 Hz の時 0.2 秒なので 20 ステップ)

	//残りステップ数に対して，何回ビブラートを実行できる？
	int vibrateNb = togo / vibrate_step;

	//半値幅を半周期で往復しなければならない→1/4周期で半値幅に達するための移動離散値は？
	double vibrate_unit = vibratePercent / (vibrate_step / 4.); // 5 Hz, 2% の時 2 / 5 なので 0.4

	//ビブラートの中心位置は？
	double vibrateCenter = pitchSequence[pitchSequence.size()-1];

	std::cout << "[ ピッチユニット ] ビブラートセンター: " << vibrateCenter << "，ビブラート回数: " << vibrateNb << std::endl;

	//計算完了，ピッチシーケンスにビブラートシーケンスを追加
	int ret = 0;
	for(int i=0;i<vibrateNb;i++){

		// 戻り
		// 1/4幅まで
		for(int j=0;j<vibrate_step/4.;j++){
			pitchSequence.append(vibrateCenter - vibrate_unit*(j+1));
			ret++;
		}
		// 1/2幅（半値幅）まで
		for(int j=vibrate_step/4.;j>0.;j--){
			pitchSequence.append(vibrateCenter - vibrate_unit*(j-1));
			ret++;
		}

		// 行き
		// 1/4幅まで
		for(int j=0;j<vibrate_step/4.;j++){
			pitchSequence.append(vibrateCenter + vibrate_unit*(j+1));
			ret++;
		}
		// 1/2幅（半値幅）まで
		for(int j=vibrate_step/4.;j>0.;j--){
			pitchSequence.append(vibrateCenter + vibrate_unit*(j-1));
			ret++;
		}

	}

	return ret;

	/*
	//// スタティックなビブラート（旧実装）；参考まで
	//前に0.4%ずつ5ステップ=2%，戻り0.4%ずつ5ステップ=2%（原点）；これが片道で10ステップ，往復20ステップ．0.2秒．従って5Hz
	double vibrateStep = 20;
	int vibrateNb = togo / vibrateStep;
	double vibrateCenter = pitchSequence[pitchSequence.size()-1];
	std::cout << "[ ピッチユニット ] ビブラートセンター: " << vibrateCenter << "，ビブラート回数: " << vibrateNb << std::endl;
	for(int i=0;i<vibrateNb;i++){
		//戻り
		pitchSequence.append(vibrateCenter - 0.4); //10msec
		pitchSequence.append(vibrateCenter - 0.8); //20
		pitchSequence.append(vibrateCenter - 1.2); //30
		pitchSequence.append(vibrateCenter - 1.6); //40
		pitchSequence.append(vibrateCenter - 2.0); //50
		pitchSequence.append(vibrateCenter - 1.6); //60
		pitchSequence.append(vibrateCenter - 1.2); //70
		pitchSequence.append(vibrateCenter - 0.8); //80
		pitchSequence.append(vibrateCenter - 0.4); //90
		pitchSequence.append(vibrateCenter - 0.0); //100
		//行き
		pitchSequence.append(vibrateCenter + 0.4); //10msec
		pitchSequence.append(vibrateCenter + 0.8); //20
		pitchSequence.append(vibrateCenter + 1.2); //30
		pitchSequence.append(vibrateCenter + 1.6); //40
		pitchSequence.append(vibrateCenter + 2.0); //50
		pitchSequence.append(vibrateCenter + 1.6); //60
		pitchSequence.append(vibrateCenter + 1.2); //70
		pitchSequence.append(vibrateCenter + 0.8); //80
		pitchSequence.append(vibrateCenter + 0.4); //90
		pitchSequence.append(vibrateCenter + 0.0); //100
	}
	*/

}


void PitchUnit::Commit()
{
	start();
	std::cout << "[ ピッチユニット ] 動作コミット" << std::endl;
}



void PitchUnit::run()
{
	Controller *control = Controller::GetInstance();

	//主軸
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->Commit(); // ロックしません	

	//第二軸
	control->iaic[nb_iaic-1]->Commit(); //ロックしません

	//第三軸
	Servo *servo = (Servo*)control->servo[nb_servo];
	//制御ベクタ方式に変更しました
	//std::cout << "c_value: " << c_value << std::endl;
	//servo->SetPosition(c_value);
	servo->Commit(); // ロックしません

}


/*!
 * \brief
 * ジョグ動作を実行する（動作試験及びキャリブレーションに利用する），ロックしません．
 * 
 * \param z_value
 * 主軸目標値[0,100]
 * 
 * \param c_value
 * 押込み軸目標値[0,100]
 * 
 */
void PitchUnit::Jog(double z_value, double z_time, double c_value)
{
	std::cout << "[ ピッチユニット ] ジョグ動作を実行します（目標位置: " << z_value << ", 動作時間: " << z_time << ", 押し込み角度: " << c_value << "）" << std::endl;
	Controller *control = Controller::GetInstance();

	//サーボ
	Servo *servo = (Servo*)control->servo[nb_servo];
	servo->SetPosition(c_value);
	servo->Commit(); // ロックしません

	//主軸
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->SetMode(MLinear::JOG);
	axis->SetPosition(z_value);
	axis->SetDuration(z_time, 0, 0);
	axis->Prepare();
	axis->Commit(); // ロックしません
}


void PitchUnit::JogGoToString(QString stringName)
{
	Controller *control = Controller::GetInstance();
	control->iaic[nb_iaic-1]->GoToString(stringName);
}


void PitchUnit::Evacuate()
{
	Controller *control = Controller::GetInstance();
	control->iaic[nb_iaic-1]->Evacuate();
}

/*!
 * \brief
 * ユニットをホームポジションへ移動させます，ロックしません．
 * 
 * \remarks
 * ユニットごとのホームポジション移動関数です．ロックしませんので，移動状態のチェックもしくはデフウェイトを置いてください．
 * 
 */
/*
void PitchUnit::SetHome()
{
	std::cout << "[ ピッチユニット ] ユニットをホームポジションへ移動します．";
	Controller *control = Controller::GetInstance();

	//サーボ
	Servo *servo = (Servo*)control->servo[nb_servo];
	servo->SetPosition(0.0);
	servo->Commit(); // ロックしません

	//主軸
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->SetHome(); // ロックしません

	//IAICについてはアブソリュートなのでホームポジションへ移動する必要はありません．
}
*/