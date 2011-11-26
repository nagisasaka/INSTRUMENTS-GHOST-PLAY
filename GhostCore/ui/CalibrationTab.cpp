#include "CalibrationTab.hpp"
#include "GhostCore.hpp"
#include "constants.hpp"
#include "../calibration.hpp"
#include "../actuator/controller.hpp"
#include "../audio/stringRecorder.hpp"

#include <QScrollArea>

CalibrationTab::CalibrationTab(GhostCore *_parent) : parent(_parent)
{
	calibration = new Calibration();

	//// ファイル管理
	//ロード
	connect(parent->ui.calibrationFileLoad, SIGNAL(pressed()), this, SLOT(Load()));
	//セーブ
	connect(parent->ui.calibrationFileSave, SIGNAL(pressed()), this, SLOT(Save()));

	//// 現在位置の取り込み
	connect(parent->ui.calibrationInitialAxis0GetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));
	connect(parent->ui.calibrationInitialAxis1GetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));
	connect(parent->ui.calibrationInitialAxis2StartGetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));
	connect(parent->ui.calibrationInitialAxis2EndGetPosition, SIGNAL(pressed()), this, SLOT(GetCurrentPosition()));

	//// 初期位置

	//// 選択軸変化
	connect(parent->ui.calibrationInitialStringSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectInitialPosition()));
	//// 値変化
	connect(parent->ui.calibrationInitialAxis0, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));
	connect(parent->ui.calibrationInitialAxis1, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));
	connect(parent->ui.calibrationInitialAxis2Start, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));
	connect(parent->ui.calibrationInitialAxis2End, SIGNAL(valueChanged(double)), this, SLOT(UpdateInitialPosition()));

	//// キャリブレーション済位置

	//// 選択軸変化
	connect(parent->ui.calibrationStringSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectCalibratedPosition()));
	//// 値変化
	connect(parent->ui.calibrationAxis0, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));
	connect(parent->ui.calibrationAxis1, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));
	connect(parent->ui.calibrationAxis2Start, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));
	connect(parent->ui.calibrationAxis2End, SIGNAL(valueChanged(double)), this, SLOT(UpdateCalibratedPosition()));

	//// キャリブレーション後の位置を反映
	connect(calibration, SIGNAL(UpdateCalibratedPosition()), this, SLOT(SelectCalibratedPosition()));

	//// キャリブレーション実行
	connect(parent->ui.calibrationStart,SIGNAL(pressed()), this, SLOT(Start()));
	connect(parent->ui.calibrationAllStart,SIGNAL(pressed()), this, SLOT(StartAll()));
	connect(parent->ui.calibrationAllStop,SIGNAL(pressed()), this, SLOT(StopAll()));

	//チェイン実行を接続
	connect(calibration, SIGNAL(CalibrationEnd()), this, SLOT(StartAll()));

	//StartAll内で接続（不具合）
	//connect(calibration, SIGNAL(CalibrationEnd()), this, SLOT(StartAll()));

	//// キャリブレーション停止（人間の耳等，センサーによる自動停止前に手動停止する場合）
	connect(parent->ui.calibrationStop, SIGNAL(pressed()), this, SLOT(Stop()));

	//ロード
	Load();

	//// レンダリングエリア
	r1 = new RenderArea(parent->ui.calibrationSoundAmplitudeDisplay);
	parent->ui.calibrationSoundAmplitudeDisplay->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundAmplitudeDisplay->setWidget(r1);

	r2 = new RenderArea2(parent->ui.calibrationSoundCPSDisplay);
	parent->ui.calibrationSoundCPSDisplay->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundCPSDisplay->setWidget(r2);

	r3 = new RenderArea3(parent->ui.calibrationSoundFFTDisplay);
	parent->ui.calibrationSoundFFTDisplay->setBackgroundRole(QPalette::Dark);
	parent->ui.calibrationSoundFFTDisplay->setWidget(r3);

	//初期化
	currentNbString = 0;
}

CalibrationTab::~CalibrationTab()
{
}

void CalibrationTab::Update()
{
	//自分が管轄している子ウィジェットへの送達
	r1->Update();
	r2->Update();
	r3->Update();
}

void CalibrationTab::SelectInitialPosition()
{
	//// 指定弦番号
	int currentIndex = parent->ui.calibrationInitialStringSelect->currentIndex();
	//弦
	int nb_string = currentIndex + 1;

	//// ポジションセット
	Calibration::Positions pos = calibration->GetInitialPositions(nb_string);

	parent->ui.calibrationInitialAxis0->setValue(pos.start[0]);
	parent->ui.calibrationInitialAxis1->setValue(pos.start[1]);
	parent->ui.calibrationInitialAxis2Start->setValue(pos.start[2]);
	parent->ui.calibrationInitialAxis2End->setValue(pos.end[2]);
}

void CalibrationTab::SelectCalibratedPosition()
{
	//std::cout << "SelectCalibratedPosition()" << std::endl;
	//// 指定弦番号
	int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	//弦（ただし弦間を含む）
	int nb_string = currentIndex + 1;

	Calibration::Positions pos = calibration->GetCalibratedPositions(nb_string);

	parent->ui.calibrationAxis0->setValue(pos.start[0]);
	parent->ui.calibrationAxis1->setValue(pos.start[1]);
	parent->ui.calibrationAxis2Start->setValue(pos.start[2]);
	parent->ui.calibrationAxis2End->setValue(pos.end[2]);
}

void CalibrationTab::UpdateInitialPosition()
{
	//// 指定弦番号
	int currentIndex = parent->ui.calibrationInitialStringSelect->currentIndex();
	//弦
	int nb_string = currentIndex + 1;

	//// ポジションセット
	Calibration::Positions pos = calibration->GetInitialPositions(nb_string);

	QString obj = QObject::sender()->objectName();
	if(obj == "calibrationInitialAxis0"){
		pos.start[0] = parent->ui.calibrationInitialAxis0->value();
		pos.end[0] = pos.start[0];
	}else if(obj == "calibrationInitialAxis1"){
		pos.start[1] = parent->ui.calibrationInitialAxis1->value();
		pos.end[1] = pos.start[1];
	}else if(obj == "calibrationInitialAxis2Start"){
		pos.start[2] = parent->ui.calibrationInitialAxis2Start->value();
	}else if(obj == "calibrationInitialAxis2End"){
		pos.end[2] = parent->ui.calibrationInitialAxis2End->value();
	}

	//// 保存
	calibration->SetInitialPosition(nb_string, pos);
}

void CalibrationTab::UpdateCalibratedPosition()
{
	/*
	int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	int nb_string = currentIndex + 1;
	Calibration::Positions pos = calibration->GetCalibratedPositions(nb_string);
	QString obj = QObject::sender()->objectName();
	if(obj == "calibrationAxis0"){
		pos.start[0] = parent->ui.calibrationInitialAxis0->value();
		pos.end[0] = pos.start[0];
	}else if(obj == "calibrationAxis1"){
		pos.start[1] = parent->ui.calibrationInitialAxis1->value();
		pos.end[1] = pos.start[1];
	}else if(obj == "calibrationAxis2Start"){
		pos.start[2] = parent->ui.calibrationAxis2Start->value();
	}else if(obj == "calibrationAxis2End"){
		pos.end[2] = parent->ui.calibrationAxis2End->value();
	}
	calibration->SetCalibratedPosition(nb_string, pos);
	*/
}

void CalibrationTab::GetCurrentPosition()
{
	//// 指定弦番号
	int currentIndex = parent->ui.calibrationInitialStringSelect->currentIndex();
	//弦
	int nb_string = currentIndex + 1;

	//// ポジションセット
	Calibration::Positions pos = calibration->GetInitialPositions(nb_string);

	//// 現在位置
	std::cout << "[ CALIBRATION TAB ] 現在位置の取り込み" << std::endl;
	QString obj = QObject::sender()->objectName();
	Controller *control = Controller::GetInstance();
	if(obj == "calibrationInitialAxis0GetPosition"){
		//軸0
		ActuatorStatus status = control->axis[0]->GetStatus();
		parent->ui.calibrationInitialAxis0->setValue(status.Position);
		//保存
		pos.start[0] = status.Position;
		pos.end[0] = status.Position;
	}else if(obj == "calibrationInitialAxis1GetPosition"){
		//軸1
		ActuatorStatus status = control->axis[1]->GetStatus();
		parent->ui.calibrationInitialAxis1->setValue(status.Position);
		//保存
		pos.start[1] = status.Position;
		pos.end[1] = status.Position;
	}else if(obj == "calibrationInitialAxis2StartGetPosition"){
		//軸2 開始点
		ActuatorStatus status = control->axis[2]->GetStatus();
		parent->ui.calibrationInitialAxis2Start->setValue(status.Position);
		//保存
		pos.start[2] = status.Position;
	}else if(obj == "calibrationInitialAxis2EndGetPosition"){
		//軸2 終了点
		ActuatorStatus status = control->axis[2]->GetStatus();
		parent->ui.calibrationInitialAxis2End->setValue(status.Position);
		//保存
		pos.end[2] = status.Position;
	}

	//// 保存
	calibration->SetInitialPosition(nb_string, pos);
}

void CalibrationTab::Load()
{
	//ロード実行
	calibration->Load();

	//設定ファイル名を表示（ファイル名を選択できないINIファイルです）
	parent->ui.calibrationFileName->setText(Constants::INI_FILE_2);

	//UI初期状態に適用
	Calibration::Positions pos = calibration->GetInitialPositions(parent->ui.calibrationInitialStringSelect->currentIndex()+1);
	parent->ui.calibrationInitialAxis0->setValue(pos.start[0]);
	parent->ui.calibrationInitialAxis1->setValue(pos.start[1]);
	parent->ui.calibrationInitialAxis2Start->setValue(pos.start[2]);
	parent->ui.calibrationInitialAxis2End->setValue(pos.end[2]);
}

void CalibrationTab::Save()
{
	//セーブ実行
	calibration->Save();
}

void CalibrationTab::Start()
{
	//// 指定弦番号
	int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	//弦
	int nb_string = currentIndex + 1;

	calibration->Start(nb_string);

	//チェイン実行を防ぐ
	currentNbString = 7;
}

void CalibrationTab::StartAll()
{
	std::cout << "[ キャリブレーション実行対象 ] currentNbString=" << currentNbString << std::endl;

	//int currentIndex = parent->ui.calibrationStringSelect->currentIndex();
	//弦
	//int nb_string = currentIndex + 1;
	if(currentNbString == 7){
		//終了
		parent->ui.calibrationStart->setEnabled(true);
		currentNbString = 0;
		return;		
	}
	//GUIの変更
	parent->ui.calibrationStringSelect->setCurrentIndex(currentNbString);
	parent->ui.calibrationInitialStringSelect->setCurrentIndex(currentNbString);
	parent->ui.calibrationStart->setEnabled(false);

	calibration->Start(currentNbString+1);
	//次の弦ポジションへ
	currentNbString++;
}

void CalibrationTab::StopAll()
{
	if(currentNbString != 0){
		//次の回で終了へ
		currentNbString = 7;
	}
}

void CalibrationTab::Stop()
{
	//手動停止
	calibration->Stop();
}

