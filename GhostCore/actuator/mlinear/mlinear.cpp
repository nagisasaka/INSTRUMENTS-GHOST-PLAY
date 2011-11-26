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

#include "../controller.hpp"
#include "mlinear.hpp"
#include "../../error.hpp"
#include "../../clock.hpp"

#include <QMessageBox>

void MLinear::ParameterInit()
{
	//ジョグ動作パラメーター初期化
	Dist   = 0;
	StrVel = 0; // ゼロ（初速無し）
	MaxVel = 0;
	Tacc   = 0;
	Tdec   = 0;
	SVacc  = 0; // ゼロ（直線部分の領域なし；p41）
	SVdec  = 0; // ゼロ（直線部分の領域なし；p41）

	//時間指定
	Duration = 0;

	//ステータスクリア
	status.Parameter = Actuator::WAITING;

	//移動条件指定フラグクリア
	flagPosition = false;
	flagDuration = false;
	flagVelocity = false;

	//デバッグ
	_debug = false;
}


/*!
 * \brief
 * 制御ベクタを指定する（連続位置絶対指定；制御ベクタの最初の指令値は現在位置と等しいこと）
 * 
 * \param pos
 * 制御ベクタ（連続位置絶対指定）
 * 
 * \remarks
 * 絶対指定位置は，生制御値に変換する際に相対位置指定に変換されるが，最初の相対位置が pos[1]-pos[0] で指定されるので，
 * 現在位置と pos[0] との間の相対位置は無視される．つまり，現在位置に対する差分としてしか動作しない．
 * ただし，これは仕様で，唐突に現在位置と離れた位置を指定されても，現在位置からその位置までの動作条件が不明であり，
 * それでだけで動作することはできない．従って，指定される絶対位置は，必ず現在位置からの続きになっていなければならず
 * これはエラーチェックされる．
 *
 * 実装方針としては，最初の制御サイクルで現在位置からpos[0]まで一気に移動させるという方針もあるが，これは試験中等に，
 * バグが入った場合に，極短時間で長距離を移動させることになり，速度制限でソフトリミットオーバーとなるか，
 * ソフトリミット以下だった場合も，かなりの高速移動が発生し，ソフトウェアのバグが物理的な危険につながる可能性のある
 * 実装方針であるため，この方針を採らない．
 * 
 */
void MLinear::SetPositionVector(QVector<double>& pos)
{
	int plus = 0;
	int minus = 0;

	positionVector = pos;

	continuousControlData.clear();

	//// 現在位置との差分チェック（このチェックの是非については検討の余地あり，続く新旧実装についても確認せよ）
	long cpc = 0;
	retCode = _7443_get_command(nb_axis, &cpc);
	if(retCode){
		std::cout << "軸 #" << uid << " setPositionVector() 内でコマンドポジションを取得できません" << std::endl;
		throw "コマンドポジションを取得できません";
	}
	int initialPos = (pos[0]/100.)*max_pulse;
	if(cpc != initialPos){
		std::cout << "[ 警告 ] setPositionVector()に指定された制御ベクトルの先頭値と，現在位置が異なります: 制御ベクトル先頭値: "<< initialPos <<"  (" << pos[0] << " ％), 現在位置: " << cpc << "(" << ((double)cpc*100./(double)max_pulse) << "％)" << std::endl;
	}

	//// 旧実装（結局こちらの旧実装を採用し，remarksに書いてある方針を採用した）

	//// 相対値への変換
	for(int i=1;i<pos.size();++i)
	{
		//値域チェック
		if(pos[i] < 0){
			//位置データなので，負の値は取らない
			std::cout << "問題箇所前後10件のデータ\n" << std::endl;
			for(int dd =qMax(i-10,0);dd<qMin(i+10,pos.size());dd++){
				std::cout << "pos[" << dd << "] = " << pos[dd] << std::endl; 
			}
			Error::Critical(0,QObject::tr("軸 # %1 連続位置指定データに負の値が含まれます．標準エラーもご覧ください．位置:%2, 問題となった負の値:%3.").arg(uid).arg(i).arg(pos[i]));
		}

		//移動前位置
		double fromPos = 0;
		//移動先位置
		double targetPos = 0;

		fromPos   = pos[i-1];
		targetPos = pos[i];


	//// 新実装（こちらは，常に現在位置を基準にするのでコーディングの自由度が低い；命令値と実際値のずれが気になる場合は，Update()内で定期的にコマンドポジションを計測してズレを補正するようなパルス補正を埋め込むべき）
	/*
	//// 相対値への変換
	for(int i=0;i<pos.size();++i)
	{
		//値域チェック
		if(pos[i] < 0){
			//位置データなので，負の値は取らない
			Error::Critical(0,QObject::tr("連続位置指定データに負の値が含まれます．位置:%1, 値:%2").arg(i-1,pos[i-1]));
		}

		//移動前位置
		double fromPos = 0;
		//移動先位置
		double targetPos = 0;

		if(i == 0){
			fromPos = ((double)cpc*100./(double)max_pulse);
			targetPos = pos[i];	
		}else{
			fromPos   = pos[i-1];
			targetPos = pos[i];
		}
		*/

		/*
		//移動先位置のパルス数での表現
		double targetPulse = (targetPos/100.0)*max_pulse;
		//移動前位置のパルス数での表現
		double fromPulse   = (fromPos/100.0)*max_pulse;
		//相対移動距離のパルス表現（正負が移動方向を示す）
		int relativePulse = (int)(targetPulse - fromPulse);
		//微小区間時間
		double delta_time = Controller::GetCycle();//ms
		//微小区間速さ（一定）
		int velocity = (int)(((double)(1.0*abs(relativePulse)))/(delta_time/1000.));
		*/

		//相対移動距離
		int relativePulse = 0;
		double r1 = ((targetPos - fromPos)/100.0)*max_pulse;
		int r2 = ((targetPos - fromPos)/100.0)*max_pulse;
		double r3 = r1 - r2;
		if(-0.5 < r3 && r3 < 0.5){
			relativePulse = r2;				
		}else if( r3 <= -0.5){
			relativePulse = r2 - 1;
		}else if( 0.5 <= r3){
			relativePulse = r2 + 1;
		}


		int velocity = abs(relativePulse)/(Controller::GetCycle()/1000.);

		//保存
		ContinuousControlData c = { velocity, relativePulse };
		continuousControlData.append(c);

		//std::cout << "[" << fromPos << "->" << targetPos << "]" << c.velocity << "," << c.distance << std::endl;

		//相対パルスチェック
		if(relativePulse >= 0) plus += relativePulse;
		else if(relativePulse < 0) minus += relativePulse;

	}

	if(_debug){
		Error::Warning(0, QObject::tr("補正前　正方向パルス数:%1, 負方向パルス数:%2, 差:%3").arg(plus).arg(minus).arg(plus+minus));
	}

	//// この補正を掛けてしまうと，片方向に行ったきりの処理は実行不能になる．ここでこの処理を入れる意味はない
	//差を完全にゼロにするための補正
	/*
	int finalPulse = (-1)*(plus+minus);
	int velocity = abs(finalPulse)/(Controller::GetCycle()/1000.);
	ContinuousControlData oc = { velocity, finalPulse };
	continuousControlData.append(oc);

	//バリデーション
	if(finalPulse >= 0) plus += finalPulse;
	else if(finalPulse < 0) minus += finalPulse;
	Error::Warning(0, QObject::tr("補正後　正方向パルス数:%1, 負方向パルス数:%2, 差:%3").arg(plus).arg(minus).arg(plus+minus));
	*/

	//データサイズ初期化
	size = continuousControlData.size();
	curr = 0;
	std::cout << "軸#"<< uid << " のデータサイズを初期化し，連続位置指定データを登録しました:" << size << std::endl;
}

void MLinear::SetVelocityVector(QVector<double>& vel)
{
	/// 未実装 ///
}

void MLinear::SetPosition(double targetPos)
{
	//ハードウェアはインクリメンタルモードで動作しているが，プログラム上では
	//0-100正規化された絶対位置指定をできるようにする．

	//移動先位置をパルス数での表現
	int targetPulse = (int)((targetPos/100.0)*max_pulse); // 誤差発生要因だが毎回cpcを見るので累積されない
		
	//現在のコマンドポジションカウンタ取得
	long cpc = 0;
	retCode = _7443_get_command(nb_axis, &cpc);
	if(retCode){
		std::cout << "軸 #" << uid << " setPosition(double targetPos) 内でコマンドポジションを取得できません" << std::endl;
		throw "コマンドポジションを取得できません";
	}

	//相対移動距離のパルス表現（正負が移動方向を示す）
	Dist = targetPulse - cpc;

	//フラグOK
	flagPosition = true;
}


void MLinear::SetPosition(double targetPos, double fromPos)
{
	//移動先位置をパルス数での表現
	int targetPulse = (int)((targetPos/100.0)*max_pulse);

	//移動前位置をパルス数での表現
	int fromPulse = (int)((fromPos/100.0)*max_pulse);

	//相対移動距離のパルス表現（正負が移動方向を示す）
	Dist = targetPulse - fromPulse;
	
	//フラグOK
	flagPosition = true;
}

void MLinear::SetDuration(double time_ms)
{		
	SetDuration(time_ms, 0, 0);
}

void MLinear::SetDuration(double time_ms, double accl_ratio, double decl_ratio)
{
	Duration = time_ms; //ミリ秒
	if(accl_ratio + decl_ratio > 1){
		Error::Critical(0,QObject::tr("加速・減速時間の合計が1を超えています"));
	}

	//時間設定
	double Tacc_msec = Duration*accl_ratio;
	double Tdec_msec = Duration*decl_ratio;
	double Trun_msec = Duration - ( Tacc_msec + Tdec_msec );

	//単位合わせ
	Tacc = Tacc_msec / 1000.0;
	Tdec = Tdec_msec / 1000.0;
	Trun = Trun_msec / 1000.0;

	//フラグOK
	flagDuration = true;
}

void MLinear::SetMode(int _mode)
{
	if(_mode == CONTINUOUS){
		//連続動作モードの場合
		SetDiscreteMove(true);
		std::cout << "制御モードを連続動作モードに設定しました" << std::endl;
	}else if(_mode == JOG){
		//ジョグモードの場合
		SetDiscreteMove(false);
		std::cout << "制御モードをジョグ動作モードに設定しました" << std::endl;
	}else{	
		Error::Critical(0,QObject::tr("制御モードの指定値が不正です:%1").arg(mode));
	}
	//適用
	mode = _mode;
}

void MLinear::SetDiscreteMove(bool t)
{
	if(t){
		//連続動作モードの初期化
		//割り込みの有効化...割り込み必要か？
		retCode = _7443_int_control(nb_axis, 1);
		//割り込みイベントを有効に設定
		//retCode = _7443_int_enable(nb_axis, HANDLE *phEvent); //ポーリングを用いるので利用しない
		//INT要素の設定
		unsigned long int_factor = 4;
		retCode = _7443_set_int_factor(nb_axis,4);
		//連続動作モードの有効化
		retCode = _7443_set_continuous_move(nb_axis,1);
	}else{
		//割り込みの無効化
		retCode = _7443_int_control(nb_axis,0);			
		//連続動作モードの無効化
		retCode = _7443_set_continuous_move(nb_axis,0);
	}
}

void MLinear::Prepare()
{
	if(mode == JOG){
		//// エラーチェック
		if(!flagDuration){
			Error::Critical(0, QObject::tr("軸 #%1 ジョグ動作時間が指定されていません").arg(uid));
		}

		if(flagPosition && flagVelocity){
			Error::Critical(0, QObject::tr("軸 #%1 ジョグ動作の距離指定方式と位置指定方式が同時に指定されており，制御方式を特定できません").arg(uid));			
		}

		if(!flagPosition && !flagVelocity){
			Error::Critical(0, QObject::tr("軸 #%1 ジョグ動作の距離指定も位置指定も為されていません").arg(uid));			
		}

		if(Dist == 0){
			std::cout << "軸 #" << uid << " 移動量がゼロ．次の移動命令はスキップされます．" << std::endl;
		}	

		//// 必要制御データの追加計算
		if(flagPosition){
			//d-t系からv-d系へ換算
			MaxVel = (2.0*abs(Dist)) / (Tacc+Tdec+2.0*Trun);
		}else if(flagVelocity){
			//v-t系からv-d系へ換算

			//// ... 未実装 
		}

	}else if(mode == CONTINUOUS){

		//// エラーチェック
		if(size == 0){
			Error::Critical(0,QObject::tr("軸 #%1 連続動作モードの制御データが存在しません．").arg(uid));
		}

		//OK

	}else{
		Error::Critical(0,QObject::tr("軸 #%1 制御モードの指定値が不正です:%1").arg(uid,mode));
	}
			
	//運転準備完了
	status.Parameter = Actuator::PREPARED;
}

void MLinear::Commit()
{
	if(status.Parameter == Actuator::WAITING){
		Error::Critical(0,QObject::tr("Axis #%1 Not Prepared()").arg(uid));		
	}

	//// 運転実行
	if(mode == JOG){

		if(Dist == 0){
			//移動距離が無いので何もしないで終了
			std::cout << "軸 #" << uid << " 移動量ゼロ．移動命令スキップ．" << std::endl;
			ParameterInit();
			return;
		}

		//デフォルト．等躍度運動（S字動作）による滑らかな加減速
		retCode = _7443_start_sr_move(nb_axis, Dist, StrVel, MaxVel, Tacc, Tdec, SVacc, SVdec);

		if(!retCode){
			std::cout << "軸 #" << uid << " 移動実行, Dist="<< Dist <<", MaxVel=" << MaxVel << std::endl;
		}else{
			ParameterInit();
			Error::Warning(0, QObject::tr("軸 #%1 移動失敗:%2").arg(uid).arg(retCode));
		}

		//移動に使ったパラメータをすべて削除
		ParameterInit();

		//ジョグ運転実行フラグを立てる（運転終了参考シグナル送出のためのみ）
		flagJogOperationStart = true;

	}else if(mode == CONTINUOUS){

		//連続運転実行フラグを立てる．連続運転命令は Update() 内で行われる．
		flagContinuousOperationStart = true;
		std::cout << "軸 #" << uid << " 連続動作コミット．"<< std::endl;

	}		
}

void MLinear::Update()
{
	if(flagUpdating){
		std::cout << "[ 警告 ] 処理落ち検出" << std::endl;
	}
	flagUpdating = true;

	//軸の動作ステータスをアップデート
	UpdateStatus();

	//軸の位置ステータス
	UpdatePosition();

	if(mode == JOG){
		//JOG動作の場合，軸制御関数はCommit()で1回発行したのみ．ここでは監視と更新のみ．
		if(flagJogOperationStart){
			if(status.Movement == Actuator::STOP){
				//運転終了（ただし，これはパルサー層での運転終了であって，実際の可動子はまだ動いていることに注意）
				flagJogOperationStart = false;
				std::cout << "ジョグ動作が終了しました" << std::endl;
				//参考シグナルを送出
				emit moveComplete(uid);				
			}
		}
	}else if(mode == CONTINUOUS){
		//連続動作の場合は，軸制御関数はここで発行する．

		//連続動作開始フラグのチェック
		if(!flagContinuousOperationStart){
			flagUpdating = false;
			return; //以降の処理は実行されない
		}

		if(curr >= size){
			//連続動作自動終了
			flagContinuousOperationStart = false;
			flagUpdating = false;

			//クロックを停止
			Controller* control = Controller::GetInstance();
			control->GetClock()->Stop();
			std::cout << "軸 #" << uid << "連続動作が終了しました" << std::endl;
			//参考シグナルを送出
			emit moveComplete(uid);

			return; //以降の処理は実行されない
		}
		//連続動作開始フラグが立っていない時は，以降の処理は実行されない

		//エラーチェック
		if(size == 0){
			Error::Warning(0,QObject::tr("軸 #%1 連続動作制御データサイズがゼロです．").arg(uid));		
		}

		//連続動作データ
		ContinuousControlData *c = 0;

		//最大連続投入命令数
		int cont_max = 0;

		//連続動作バッファをチェックする
		retCode = _7443_check_continuous_buffer(nb_axis);	

		if(retCode == 0){
			//全て空きなので連続三命令を送出する
			cont_max = 3;
		}else if(retCode == 1){
			//1,2が空きなので，連続二命令を送出する
			cont_max = 2;
		}else if(retCode == 2){
			//2が空きなので，一命令のみを送出する
			cont_max = 1;
		}else if(retCode == 3){
			//動作用プリレジスタは満杯なので待つ
			cont_max = 0;
		}else{
			Error::Critical(0, QObject::tr("連続動作コマンドバッファ検出で予期しない戻り値が返されました:%1").arg(retCode));
		}

		int session_start = curr;
		while(curr < session_start + cont_max){
			c = &(continuousControlData[curr]);
			if(c->distance == 0){
				//移動距離がゼロの場合は次へ，フラグ落としてUpdate()終了．
				curr++; 
				flagUpdating = false;
				return;
			}
			//移動距離がゼロでは無い場合は，移動実行
			retCode = _7443_start_tr_move(nb_axis, c->distance, c->velocity, c->velocity, 0, 0);//tr系
			if(retCode){
				std::cout << "[ WARNING ] _7443_start_tr_move エラー: retCode=" << retCode << std::endl;
			}
			curr++;
			if(curr >= size){
				//オーバーラン防止，フラグ落としてUpdate()終了．クロックからの次の呼び出しで終了してくれる．
				std::cout << "[ NOTICE ] ぴったり終了" << std::endl;
				flagUpdating = false;
				return;
			}
		}
	}
	flagUpdating = false;
	return;
}

void MLinear::UpdateStatus()
{
	//// 移動状態の確定
	int s = _7443_motion_done(nb_axis);
	if(s == 0 || s == 1 || s == 2 || s == 3 || s == 4 || s == 5 || s == 6 || s == 14 || s == 15){
		//パルサーは停止中（パルサーは何もしていない）
		unsigned short io_sts = 0;
		_7443_get_io_status(nb_axis, &io_sts);
		if(io_sts & 0x2000){
			//アンプも停止中（何もしていない）
			status.Movement = Actuator::STOP;
		}else{
			//アンプで軸動作中（加速，減速は不明）
			status.Movement = Actuator::RUNNING;				
		}
	}else if(s == 7){
		//バックラッシュ補正中
		status.Movement = Actuator::ADJUSTING;
	}else if(s == 9){
		//原点へ移動中
		status.Movement = Actuator::RUNNING;
	}else if(s == 10){
		//移動開始速度で移動中
		status.Movement = Actuator::RUNNING;
	}else if(s == 11){
		//加速中
		status.Movement = Actuator::ACCL;
	}else if(s == 12){
		//最高速度で運転中
		status.Movement = Actuator::RUNNING;
	}else if(s == 13){
		//減速中
		status.Movement = Actuator::DECL;
	}else{
		//マニュアルに未定義
		Error::Critical(0, QObject::tr("軸 #%1 マニュアル未定義ステータスコード検出").arg(uid));
	}
}

void MLinear::UpdatePosition()
{
	//位置の確定
	long cpc = 0;
	//コマンドポジションカウンタp164は，ソフトリミットにより，max_pulseを超えられない
	retCode = _7443_get_command(nb_axis, &cpc);
	//[0,100]で返す
	status.Position = (double)(cpc*100)/(double)max_pulse;
	//位置（生データ；開発・デバッグ用）
	status.PositionRaw = cpc;
}

void MLinear::SetHome()
{
	std::cout << "軸 #" << uid << " ホームポジション移動．" << std::endl;

	//原点復帰は，パルサーの機能は利用せず，サーボオン信号の立下りに設定した
	retCode = _7443_set_servo(nb_axis,1);
	if(retCode){
		Error::Critical(0,QObject::tr("軸 #%1 SVON信号立上失敗（%2）．").arg(uid,retCode));
	}

	//パルサー機能を利用していないためのウェイト
	HardTimer::Wait(10);

	retCode = _7443_set_servo(nb_axis,0);
	if(retCode){
		Error::Critical(0,QObject::tr("軸 #%1 SVON信号立下失敗（%2）．").arg(uid,retCode));
	}

	//パルサー機能を利用していないためのウェイト
	HardTimer::Wait(10);

	//コマンドポジションカウンタをゼロクリア
	retCode = _7443_set_command(nb_axis,0);
	if(!retCode){
		std::cout << "軸 #" << uid << " コマンドポジションクリア．" << std::endl;
	}else{
		Error::Critical(0,QObject::tr("軸 #%1 コマンドポジションクリア失敗（%2）．").arg(uid,retCode));
	}
}
