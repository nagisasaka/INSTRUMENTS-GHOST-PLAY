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

#include "PlayTab.hpp"
#include "../ghostcore.hpp"
#include "../bowing.hpp"
#include "../bowingManager.hpp"
#include "../play/pitchunit.hpp"
#include "../pitchManager.hpp"
#include "../playManager.hpp"
#include "../actuator/controller.hpp"

#include <QFileDialog>
#include <QTextCursor>
#include <QTextBlock>
#include <QPalette>
#include <QMessageBox>

PlayTab::PlayTab(GhostCore *_parent) : parent(_parent)
{
	playManager = new PlayManager();

	index = 0;
	nbPlayUnit = 0;

	//���̉�ʂ�
	connect(parent->ui.playPlanNext, SIGNAL(pressed()), this, SLOT(Next()));

	//�O�̉�ʂ�
	connect(parent->ui.playPlanPrev, SIGNAL(pressed()), this, SLOT(Back()));

	//�K�p
	connect(parent->ui.playSave_1, SIGNAL(pressed()), this, SLOT(Apply()));
	connect(parent->ui.playSave_2, SIGNAL(pressed()), this, SLOT(Apply()));
	connect(parent->ui.playSave_3, SIGNAL(pressed()), this, SLOT(Apply()));

	//�Z�[�u�i�S�f�[�^���t�@�C���ɏ������Ƃ��j
	connect(parent->ui.savePlayData, SIGNAL(pressed()), this, SLOT(Save()));

	//���ׂĉ��t
	connect(parent->ui.playAll, SIGNAL(pressed()), this, SLOT(PlayAll()));

	//���̉�ʂ܂ŉ��t
	connect(parent->ui.playFromThisScreen, SIGNAL(pressed()), this, SLOT(PlayFromThisScreen()));

	//���̉�ʂ܂ŉ��t
	connect(parent->ui.playTillThisScreen, SIGNAL(pressed()), this, SLOT(PlayTillThisScreen()));

	//�ʉ��t
	connect(parent->ui.playStart_1, SIGNAL(pressed()), this, SLOT(Play()));
	connect(parent->ui.playStart_2, SIGNAL(pressed()), this, SLOT(Play()));
	connect(parent->ui.playStart_3, SIGNAL(pressed()), this, SLOT(Play()));

	//�v���y�A
	connect(parent->ui.prepareAll, SIGNAL(pressed()), this, SLOT(PrepareAll()));

	//���̉�ʂ��v���y�A
	connect(parent->ui.prepareFromThisScreen, SIGNAL(pressed()), this, SLOT(PrepareFromThisScreen()));

	//���̉�ʂ܂Ńv���y�A
	connect(parent->ui.prepareTillThisScreen, SIGNAL(pressed()), this, SLOT(PrepareTillThisScreen()));

	//�t�@�C�����J��
	connect(parent->ui.openPlayData, SIGNAL(pressed()), this, SLOT(OpenFile()));

	//�����ړ�
	connect(parent->ui.initialMoveStart, SIGNAL(pressed()), this, SLOT(InitialMove1()));
	connect(parent->ui.initialMove2Start, SIGNAL(pressed()), this, SLOT(InitialMove2()));

	//// UI�Ԃ̐ڑ�

	//�|���f�[�^���ύX���ꂽ
	connect(parent->ui.pressureData_1, SIGNAL(cursorPositionChanged()), this, SLOT(PressureDataChanged()));
	connect(parent->ui.pressureData_2, SIGNAL(cursorPositionChanged()), this, SLOT(PressureDataChanged()));
	connect(parent->ui.pressureData_3, SIGNAL(cursorPositionChanged()), this, SLOT(PressureDataChanged()));

	//�|���f�[�^���ύX���ꂽ
	connect(parent->ui.positionData_1, SIGNAL(cursorPositionChanged()), this, SLOT(PositionDataChanged()));
	connect(parent->ui.positionData_2, SIGNAL(cursorPositionChanged()), this, SLOT(PositionDataChanged()));
	connect(parent->ui.positionData_3, SIGNAL(cursorPositionChanged()), this, SLOT(PositionDataChanged()));

	//// �F�ύX

	QPalette pal = parent->ui.prepared_1->palette();
	pal.setColor(QPalette::Background, QColor("#ff9933"));

	parent->ui.prepared_1->setAutoFillBackground(true);
	parent->ui.prepared_1->setPalette(pal);
	parent->ui.prepared_2->setAutoFillBackground(true);
	parent->ui.prepared_2->setPalette(pal);
	parent->ui.prepared_3->setAutoFillBackground(true);
	parent->ui.prepared_3->setPalette(pal);

	//// �f�t�H���g�ŉ��t�{�^���𖳌�

	parent->ui.playAll->setEnabled(false);
	parent->ui.playFromThisScreen->setEnabled(false);
	parent->ui.playTillThisScreen->setEnabled(false);
}

PlayTab::~PlayTab()
{
	delete playManager;
}

//// UI�ڑ��n

void PlayTab::PressureDataChanged()
{
	//�|���f�[�^���̓G���A���ł̃J�[�\���ړ��ɂ���ĕω�����UI�p�[�c��S�ĕω�������
	QString obj = QObject::sender()->objectName();

	int unitTime = Controller::GetCycle();

	//�J�����g�s���̕ύX
	QTextCursor cr;
	int line = 0;
	int allLines = 0;
	if(obj == "pressureData_1"){

		cr = parent->ui.pressureData_1->textCursor();
		line = cr.blockNumber();
		parent->ui.pressureCurrentLine_1->setText(QString("%1").arg(line));

		QString text = parent->ui.pressureData_1->toPlainText();
		QStringList p = text.split("\n", QString::SkipEmptyParts);
		allLines = p.size();

		parent->ui.playTime_1->setText(QString("%1 ms").arg(allLines*unitTime));
		parent->ui.pressureLine_1->setText(QString("%1").arg(allLines));

	}else if(obj == "pressureData_2"){

		cr = parent->ui.pressureData_2->textCursor();;
		line = cr.blockNumber();
		parent->ui.pressureCurrentLine_2->setText(QString("%1").arg(line));

		QString text = parent->ui.pressureData_2->toPlainText();
		QStringList p = text.split("\n", QString::SkipEmptyParts);
		allLines = p.size();

		parent->ui.playTime_2->setText(QString("%1 ms").arg(allLines*unitTime));
		parent->ui.pressureLine_2->setText(QString("%1").arg(allLines));

	}else if(obj == "pressureData_3"){

		cr = parent->ui.pressureData_3->textCursor();;
		line = cr.blockNumber();
		parent->ui.pressureCurrentLine_3->setText(QString("%1").arg(line));

		QString text = parent->ui.pressureData_3->toPlainText();
		QStringList p = text.split("\n", QString::SkipEmptyParts);
		allLines = p.size();

		parent->ui.playTime_3->setText(QString("%1 ms").arg(allLines*unitTime));
		parent->ui.pressureLine_3->setText(QString("%1").arg(allLines));
	}

}

void PlayTab::PositionDataChanged()
{
	//�|���f�[�^���̓G���A���ł̃J�[�\���ړ��ɂ���ĕω�����UI�p�[�c��S�ĕω�������

	QString obj = QObject::sender()->objectName();

	//�J�����g�s���̕ύX
	QTextCursor cr;
	int line = 0;
	int allLines = 0;
	if(obj == "positionData_1"){

		cr = parent->ui.positionData_1->textCursor();
		line = cr.blockNumber();
		parent->ui.positionCurrentLine_1->setText(QString("%1").arg(line));

		QString text = parent->ui.positionData_1->toPlainText();
		QStringList p = text.split("\n", QString::SkipEmptyParts);
		allLines = p.size();

		parent->ui.positionLine_1->setText(QString("%1").arg(allLines));

	}else if(obj == "positionData_2"){

		cr = parent->ui.positionData_2->textCursor();;
		line = cr.blockNumber();
		parent->ui.positionCurrentLine_2->setText(QString("%1").arg(line));

		QString text = parent->ui.positionData_2->toPlainText();
		QStringList p = text.split("\n", QString::SkipEmptyParts);
		allLines = p.size();

		parent->ui.positionLine_2->setText(QString("%1").arg(allLines));

	}else if(obj == "positionData_3"){

		cr = parent->ui.positionData_3->textCursor();;
		line = cr.blockNumber();
		parent->ui.positionCurrentLine_3->setText(QString("%1").arg(line));

		QString text = parent->ui.positionData_3->toPlainText();
		QStringList p = text.split("\n", QString::SkipEmptyParts);
		allLines = p.size();

		parent->ui.positionLine_3->setText(QString("%1").arg(allLines));
	}

}


//// ���t�n

void PlayTab::InitialMove1()
{
	//�����ړ�����i�v���y�A���番���Ď��o�����j
	playManager->InitialMove1();

	//�����ړ���C�{�^����L��������
	parent->ui.playAll->setEnabled(true);
	parent->ui.playFromThisScreen->setEnabled(true);
	parent->ui.playTillThisScreen->setEnabled(true);
}

void PlayTab::InitialMove2()
{
	//�����ړ�����i�v���y�A���番���Ď��o�����j
	playManager->InitialMove2();

	//�����ړ���C�{�^����L��������
	parent->ui.playAll->setEnabled(true);
	parent->ui.playFromThisScreen->setEnabled(true);
	parent->ui.playTillThisScreen->setEnabled(true);
}


void PlayTab::Play()
{
	//����̎w��l���x�N�g���ɓo�^����D
	QString obj = QObject::sender()->objectName();

	Bowing *bow = 0;
	PitchUnit *pu = 0;

	if(obj == "playStart_1"){

		bow = GenerateBowing(0);
		pu  = GeneratePitchUnit(0);

	}else if(obj == "playStart_2"){

		bow = GenerateBowing(1);
		pu  = GeneratePitchUnit(1);

	}else if(obj == "playStart_3"){

		bow = GenerateBowing(2);
		pu  = GeneratePitchUnit(2);

	}

	//�����ړ������Ȃ̂ŁC��ŏ����ړ����邱�Ɓi�����ړ����Ȃ��ƒP�ɋ�U�肷�邺�j
	int playTime = bow->Prepare();

	//�O��I���ʒu���w��Ńv���y�A���C���̌��݈ʒu��O��I���ʒu�Ƃ���̂ŁC�����ړ������C���S�D
	pu->Prepare();

	bow->Start();
	pu->Commit();	
}

void PlayTab::PrepareCommon()
{
	//�����t���ԍX�V
	int time_ms  = playTime * Controller::GetCycle();
	double time_sec = playTime * Controller::GetCycle() / 1000.;
	parent->ui.fullPlayTime->setText(QString("%1 sec").arg(time_sec));

	//�V�~�����[�^�X�V
	//�I������
	parent->ui.simulationEndTime->setText(QString("%1 ms").arg(time_ms));
	//�X���C�_
	parent->ui.simulationCurrentTimeSlider->setMinimum(0);
	parent->ui.simulationCurrentTimeSlider->setMaximum(time_ms);
	parent->ui.simulationCurrentTimeSlider->setSliderPosition(0);

	//�C���f�b�N�X�͕ς���Ă��Ȃ����C��ʂ��X�V���邽�߂ɌĂԁD
	IndexChange();
}


void PlayTab::PrepareAll()
{
	playTime = playManager->Prepare();
	PrepareCommon();
}


void PlayTab::PrepareTillThisScreen()
{
	playTime = playManager->Prepare(0, index+3);
	PrepareCommon();
}


void PlayTab::PrepareFromThisScreen()
{
	playTime = playManager->Prepare(index, nbPlayUnit);
	PrepareCommon();
}


void PlayTab::PlayAll()
{
	std::cout << "[ �v���C�^�u ] �o�^�ϑS�f�[�^���t���s." << std::endl;

	QMessageBox msgBox;
	msgBox.setText(QObject::tr("���t���s�̊m�F"));
	msgBox.setInformativeText(QObject::tr("�o�^����Ă���S�͈͂̉��t���s���܂��D��낵���ł����H"));
	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	msgBox.setDefaultButton(QMessageBox::No);
	int ret = msgBox.exec();
	if(ret == QMessageBox::Yes){
		playManager->Prepare();
		playManager->Commit();
	}
}

void PlayTab::PlayFromThisScreen()
{
	std::cout << "[ �v���C�^�u ] ��ʔ͈̓f�[�^���牉�t���s." << std::endl;

	QMessageBox msgBox;
	msgBox.setText(QObject::tr("���t���s�̊m�F"));
	msgBox.setInformativeText(QObject::tr("���̉�ʂ���̉��t���s���܂��D��낵���ł����H"));
	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	msgBox.setDefaultButton(QMessageBox::No);
	int ret = msgBox.exec();
	if(ret == QMessageBox::Yes){
		playManager->Prepare(index, nbPlayUnit);
		playManager->Commit();
	}
}


void PlayTab::PlayTillThisScreen()
{
	std::cout << "[ �v���C�^�u ] ��ʔ͈̓f�[�^�܂ŉ��t���s." << std::endl;

	QMessageBox msgBox;
	msgBox.setText(QObject::tr("���t���s�̊m�F"));
	msgBox.setInformativeText(QObject::tr("���̉�ʂ܂ł̉��t���s���܂��D��낵���ł����H"));
	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
	msgBox.setDefaultButton(QMessageBox::No);
	int ret = msgBox.exec();
	if(ret == QMessageBox::Yes){
		playManager->Prepare(0, index+3);
		playManager->Commit();
	}
}




//// �f�[�^�n


void PlayTab::Next()
{
	index++;
	IndexChange();
}

void PlayTab::Back()
{
	index--;
	IndexChange();
}

void PlayTab::IndexChange()
{
	if(index == 0){
		parent->ui.playPlanPrev->setEnabled(false);
	}else{
		parent->ui.playPlanPrev->setEnabled(true);
	}

	parent->ui.playPlanIndex_1->setTitle(QString("#%1").arg(index));
	parent->ui.playPlanIndex_2->setTitle(QString("#%1").arg(index+1));
	parent->ui.playPlanIndex_3->setTitle(QString("#%1").arg(index+2));

	//UI�ɔ��f����

	//�S�y�C�����N���A
	for(int i=0;i<3;i++){
		ClearPane(i);
	}

	int j=0;
	for(int i=index;i<qMin(index+3,nbPlayUnit);i++){
		Bowing *bow = playManager->bowingManager->GetBowing(i);
		PitchUnit *pu = playManager->pitchManager->GetPitch(i);

		//�P��ʂR�y�C���\���C���݂̃C���f�b�N�X���K�����[�y�C���inbPane=0�j�Ȃ̂�
		SetupPane(bow,pu,j);
		j++;
	}
}

void PlayTab::Save()
{
	//�o�^���ꂽ�s�b�`�}�l�[�W���ƃ{�[�C���O�}�l�[�W���N���X���t�@�C���ɕۑ�����D
	playManager->Save(fileName);
}

void PlayTab::Apply()
{
	//����̎w��l���x�N�g���ɓo�^����D
	QString obj = QObject::sender()->objectName();

	Bowing *bow = 0;
	PitchUnit *pu = 0;

	int curr = 0;
	if(obj == "playSave_1"){
		curr = index;

		bow = GenerateBowing(0);
		pu = GeneratePitchUnit(0);

	}else if(obj == "playSave_2"){
		curr = index+1;

		bow = GenerateBowing(1);
		pu = GeneratePitchUnit(1);

	}else if(obj == "playSave_3"){
		curr = index+2;

		bow = GenerateBowing(2);
		pu = GeneratePitchUnit(2);

	}

	//�o�^���̃C���f�b�N�X�A�����`�F�b�N�̓}�l�[�W�����ōs���Ă���
	playManager->RegisterPlay(bow,pu,curr);
}

void PlayTab::OpenFile()
{
	fileName = QFileDialog::getOpenFileName(parent, tr("Open Play Data"),"",tr("���t�f�[�^ (*.txt)"));
	parent->ui.playDataFileName->setText(fileName);

	nbPlayUnit = playManager->Load(fileName);

	//index�`�F���W���Ȃ�
	//index = 0;
	IndexChange();
}


Bowing* PlayTab::GenerateBowing(int nbPane)
{
	//�V�K�o�^�p�I�u�W�F�N�g
	Bowing *bowing = new Bowing();

	QStringList positionList;
	QStringList pressureList;

	if(nbPane == 0){
		
		bowing->startString  = parent->ui.startString_1->value();
		bowing->endString    = parent->ui.endString_1->value();
		bowing->playerString = parent->ui.playerString_1->value();
		QString positionText = parent->ui.positionData_1->toPlainText();
		QString pressureText = parent->ui.pressureData_1->toPlainText();	
		positionList = positionText.split("\n",QString::SkipEmptyParts);
		pressureList = pressureText.split("\n", QString::SkipEmptyParts);

	}else if(nbPane == 1){

		bowing->startString  = parent->ui.startString_2->value();
		bowing->endString    = parent->ui.endString_2->value();
		bowing->playerString = parent->ui.playerString_2->value();
		QString positionText = parent->ui.positionData_2->toPlainText();
		QString pressureText = parent->ui.pressureData_2->toPlainText();	
		positionList = positionText.split("\n",QString::SkipEmptyParts);
		pressureList = pressureText.split("\n", QString::SkipEmptyParts);

	}else if(nbPane == 2){

		bowing->startString  = parent->ui.startString_3->value();
		bowing->endString    = parent->ui.endString_3->value();
		bowing->playerString = parent->ui.playerString_3->value();
		QString positionText = parent->ui.positionData_3->toPlainText();
		QString pressureText = parent->ui.pressureData_3->toPlainText();	
		positionList = positionText.split("\n",QString::SkipEmptyParts);
		pressureList = pressureText.split("\n", QString::SkipEmptyParts);

	}

	//�|�����Z�b�g
	QVector<double> position;
	for(int i=0;i<positionList.size();i++){
		position.append(positionList.at(i).toDouble());
	}
	bowing->SetPositionVector(position);

	//�|�����Z�b�g
	QVector<double> pressure;
	for(int i=0;i<pressureList.size();i++){
		pressure.append(pressureList.at(i).toDouble());
	}
	bowing->SetPressureVector(pressure);		

	return bowing;
}


PitchUnit* PlayTab::GeneratePitchUnit(int nbPane)
{

	//�V�K�o�^�p�I�u�W�F�N�g
	PitchUnit *pu = 0;

	if(nbPane == 0){

		pu = new PitchUnit(parent->ui.pitchUnit_1->currentIndex());
		pu->SetPitch(parent->ui.pitch_1->currentText());
		pu->SetTransitionTime(parent->ui.pitchTime_1->text().toInt());
		pu->SetVibrateHz(parent->ui.vibrateHz_1->text().toDouble());
		pu->SetVibratePercent(parent->ui.vibratePercent_1->text().toDouble());

		//pu->SetString(parent->ui.playerString_1->value());
		int nb_string = parent->ui.pitchUnitString_1->currentIndex();
		if(nb_string == 0){
			pu->SetStringName("G");
		}else if(nb_string == 1){
			pu->SetStringName("D");
		}else if(nb_string == 2){
			pu->SetStringName("A");
		}else if(nb_string == 3){
			pu->SetStringName("E");
		}

		if(parent->ui.vibrate_1->isChecked()){
			pu->SetVibrateFlag(true);
		}else{
			pu->SetVibrateFlag(false);
		}
		if(parent->ui.rigen_1->isChecked()){
			pu->SetRigenFlag(true);
		}else{
			pu->SetRigenFlag(false);
		}
		
	}else if(nbPane == 1){

		pu = new PitchUnit(parent->ui.pitchUnit_2->currentIndex());
		pu->SetPitch(parent->ui.pitch_2->currentText());
		pu->SetTransitionTime(parent->ui.pitchTime_2->text().toInt());
		pu->SetVibrateHz(parent->ui.vibrateHz_2->text().toDouble());
		pu->SetVibratePercent(parent->ui.vibratePercent_2->text().toDouble());

		//pu->SetString(parent->ui.playerString_2->value());
		int nb_string = parent->ui.pitchUnitString_2->currentIndex();
		if(nb_string == 0){
			pu->SetStringName("G");
		}else if(nb_string == 1){
			pu->SetStringName("D");
		}else if(nb_string == 2){
			pu->SetStringName("A");
		}else if(nb_string == 3){
			pu->SetStringName("E");
		}

		if(parent->ui.vibrate_2->isChecked()){
			pu->SetVibrateFlag(true);
		}else{
			pu->SetVibrateFlag(false);
		}
		if(parent->ui.rigen_2->isChecked()){
			pu->SetRigenFlag(true);
		}else{
			pu->SetRigenFlag(false);
		}

	}else if(nbPane == 2){

		pu = new PitchUnit(parent->ui.pitchUnit_3->currentIndex());
		pu->SetPitch(parent->ui.pitch_3->currentText());
		pu->SetTransitionTime(parent->ui.pitchTime_3->text().toInt());
		pu->SetVibrateHz(parent->ui.vibrateHz_3->text().toDouble());
		pu->SetVibratePercent(parent->ui.vibratePercent_3->text().toDouble());

		//pu->SetString(parent->ui.playerString_3->value());
		int nb_string = parent->ui.pitchUnitString_3->currentIndex();
		if(nb_string == 0){
			pu->SetStringName("G");
		}else if(nb_string == 1){
			pu->SetStringName("D");
		}else if(nb_string == 2){
			pu->SetStringName("A");
		}else if(nb_string == 3){
			pu->SetStringName("E");
		}

		if(parent->ui.vibrate_3->isChecked()){
			pu->SetVibrateFlag(true);
		}else{
			pu->SetVibrateFlag(false);
		}
		if(parent->ui.rigen_3->isChecked()){
			pu->SetRigenFlag(true);
		}else{
			pu->SetRigenFlag(false);
		}

	}

	return pu;
}


void PlayTab::SetupPane(Bowing *bow, PitchUnit *pu, int nbPane)
{
	if(nbPane== 0){

		//�Ώی�
		parent->ui.pitchUnitString_1->setCurrentIndex(pu->GetStringCode());
		//���K
		parent->ui.pitch_1->setCurrentIndex(parent->ui.pitch_1->findText(pu->GetPitchName()));
		//�s�b�`���j�b�g
		parent->ui.pitchUnit_1->setCurrentIndex(pu->GetNbUnit());
		//�\��
		if(pu->GetVibrateFlag()) parent->ui.vibrate_1->setChecked(true);
		else parent->ui.vibrate_1->setChecked(false);
		if(pu->GetRigenFlag()) parent->ui.rigen_1->setChecked(true);
		else parent->ui.rigen_1->setChecked(false);
		//�r�u���[�g�֘A�ϐ�
		parent->ui.vibrateHz_1->setText(QString("%1").arg(pu->GetVibrateHz()));
		parent->ui.vibratePercent_1->setText(QString("%1").arg(pu->GetVibratePercent()));
		//�s�b�`�J�ڎ���
		parent->ui.pitchTime_1->setText(QString("%1").arg(pu->GetTransitionTime()));
		//�J�n��
		parent->ui.startString_1->setValue(bow->GetStartString());
		//�I����
		parent->ui.endString_1->setValue(bow->GetEndString());
		//���t��
		parent->ui.playerString_1->setValue(bow->GetPlayerString());
		//�|���l
		QString pressureData = QString();
		QVector<double> pressureVector = bow->GetPressureVector();
		for(int i=0;i<pressureVector.size();i++){
			pressureData.append(QString("%1\n").arg(pressureVector[i]));
		}
		parent->ui.pressureData_1->setPlainText(pressureData);
		//�|���l
		QString positionData = QString();
		QVector<double> positionVector = bow->GetPositionVector();
		for(int i=0;i<positionVector.size();i++){
			positionData.append(QString("%1\n").arg(positionVector[i]));
		}
		parent->ui.positionData_1->setPlainText(positionData);

		//�v���y�A���
		QPalette pal = parent->ui.prepared_1->palette();
		if(bow->isPrepared()){
			pal.setColor(QPalette::Background, QColor("#0066ff"));
			parent->ui.prepared_1->setPalette(pal);
			parent->ui.prepared_1->setText(" PREPARED ");
			//�����m��
			int cycle = Controller::GetCycle();
			parent->ui.playTime_1->setText(QObject::tr("%1 ms").arg(bow->GetPlayTime()*cycle));
		}else{
			pal.setColor(QPalette::Background, QColor("#ff9933"));
			parent->ui.prepared_1->setPalette(pal);
			parent->ui.prepared_1->setText(" NOT PREPARED ");
		}

	}else if(nbPane == 1){

		//�Ώی�
		parent->ui.pitchUnitString_2->setCurrentIndex(pu->GetStringCode());
		//���K
		parent->ui.pitch_2->setCurrentIndex(parent->ui.pitch_2->findText(pu->GetPitchName()));
		//�s�b�`���j�b�g
		parent->ui.pitchUnit_2->setCurrentIndex(pu->GetNbUnit());
		//�\��
		if(pu->GetVibrateFlag()) parent->ui.vibrate_2->setChecked(true);
		else parent->ui.vibrate_2->setChecked(false);
		if(pu->GetRigenFlag()) parent->ui.rigen_2->setChecked(true);
		else parent->ui.rigen_2->setChecked(false);
		//�r�u���[�g�֘A�ϐ�
		parent->ui.vibrateHz_2->setText(QString("%1").arg(pu->GetVibrateHz()));
		parent->ui.vibratePercent_2->setText(QString("%1").arg(pu->GetVibratePercent()));
		//�s�b�`�J�ڎ���
		parent->ui.pitchTime_2->setText(QString("%1").arg(pu->GetTransitionTime()));
		//�J�n��
		parent->ui.startString_2->setValue(bow->GetStartString());
		//�I����
		parent->ui.endString_2->setValue(bow->GetEndString());
		//���t��
		parent->ui.playerString_2->setValue(bow->GetPlayerString());
		//�|���l
		QString pressureData = QString();
		QVector<double> pressureVector = bow->GetPressureVector();
		for(int i=0;i<pressureVector.size();i++){
			pressureData.append(QString("%1\n").arg(pressureVector[i]));
		}
		parent->ui.pressureData_2->setPlainText(pressureData);
		//�|���l
		QString positionData = QString();
		QVector<double> positionVector = bow->GetPositionVector();
		for(int i=0;i<positionVector.size();i++){
			positionData.append(QString("%1\n").arg(positionVector[i]));
		}
		parent->ui.positionData_2->setPlainText(positionData);

		//�v���y�A���
		QPalette pal = parent->ui.prepared_2->palette();
		if(bow->isPrepared()){
			pal.setColor(QPalette::Background, QColor("#0066ff"));
			parent->ui.prepared_2->setPalette(pal);
			parent->ui.prepared_2->setText(" PREPARED ");
			//�����m��
			int cycle = Controller::GetCycle();
			parent->ui.playTime_2->setText(QObject::tr("%1 ms").arg(bow->GetPlayTime()*cycle));
		}else{
			pal.setColor(QPalette::Background, QColor("#ff9933"));
			parent->ui.prepared_2->setPalette(pal);
			parent->ui.prepared_2->setText(" NOT PREPARED ");
		}

	}else if(nbPane == 2){

		//�Ώی�
		parent->ui.pitchUnitString_3->setCurrentIndex(pu->GetStringCode());
		//���K
		parent->ui.pitch_3->setCurrentIndex(parent->ui.pitch_3->findText(pu->GetPitchName()));
		//�s�b�`���j�b�g
		parent->ui.pitchUnit_3->setCurrentIndex(pu->GetNbUnit());
		//�\��
		if(pu->GetVibrateFlag()) parent->ui.vibrate_3->setChecked(true);
		else parent->ui.vibrate_3->setChecked(false);
		if(pu->GetRigenFlag()) parent->ui.rigen_3->setChecked(true);
		else parent->ui.rigen_3->setChecked(false);
		//�r�u���[�g�֘A�ϐ�
		parent->ui.vibrateHz_3->setText(QString("%1").arg(pu->GetVibrateHz()));
		parent->ui.vibratePercent_3->setText(QString("%1").arg(pu->GetVibratePercent()));
		//�s�b�`�J�ڎ���
		parent->ui.pitchTime_3->setText(QString("%1").arg(pu->GetTransitionTime()));
		//�J�n��
		parent->ui.startString_3->setValue(bow->GetStartString());
		//�I����
		parent->ui.endString_3->setValue(bow->GetEndString());
		//���t��
		parent->ui.playerString_3->setValue(bow->GetPlayerString());
		//�|���l
		QString pressureData = QString();
		QVector<double> pressureVector = bow->GetPressureVector();
		for(int i=0;i<pressureVector.size();i++){
			pressureData.append(QString("%1\n").arg(pressureVector[i]));
		}
		parent->ui.pressureData_3->setPlainText(pressureData);
		//�|���l
		QString positionData = QString();
		QVector<double> positionVector = bow->GetPositionVector();
		for(int i=0;i<positionVector.size();i++){
			positionData.append(QString("%1\n").arg(positionVector[i]));
		}
		parent->ui.positionData_3->setPlainText(positionData);


		//�v���y�A���
		QPalette pal = parent->ui.prepared_1->palette();
		if(bow->isPrepared()){
			pal.setColor(QPalette::Background, QColor("#0066ff"));
			parent->ui.prepared_3->setPalette(pal);
			parent->ui.prepared_3->setText(" PREPARED ");
			//�����m��
			int cycle = Controller::GetCycle();
			parent->ui.playTime_3->setText(QObject::tr("%1 ms").arg(bow->GetPlayTime()*cycle));
		}else{
			pal.setColor(QPalette::Background, QColor("#ff9933"));
			parent->ui.prepared_3->setText(" NOT PREPARED ");
			parent->ui.prepared_3->setPalette(pal);
		}

	}else{
		Error::Critical(0, QObject::tr("[ PLAYTAB ] SetupPane �̃y�[���ԍ����s���ł�: %1").arg(nbPane));
	}

}

void PlayTab::ClearPane(int nbPane)
{
	if(nbPane== 0){

		//���K
		parent->ui.pitch_1->setCurrentIndex(0);
		//�s�b�`���j�b�g
		parent->ui.pitchUnit_1->setCurrentIndex(0);
		//�\��
		parent->ui.vibrate_1->setChecked(false);
		parent->ui.rigen_1->setChecked(false);
		//�s�b�`�J�ڎ���
		parent->ui.pitchTime_1->setText(0);
		//�J�n��
		parent->ui.startString_1->setValue(0);
		//�I����
		parent->ui.endString_1->setValue(0);
		//���t��
		parent->ui.playerString_1->setValue(0);
		//�|���l
		parent->ui.pressureData_1->setPlainText(QString());
		//�|���l
		parent->ui.positionData_1->setPlainText(QString());

		QPalette pal = parent->ui.prepared_1->palette();
		pal.setColor(QPalette::Background, QColor("#ff9933"));
		parent->ui.prepared_1->setPalette(pal);
		parent->ui.prepared_1->setText(" NOT PREPARED ");

	}else if(nbPane == 1){

		//���K
		parent->ui.pitch_2->setCurrentIndex(0);
		//�s�b�`���j�b�g
		parent->ui.pitchUnit_2->setCurrentIndex(0);
		//�\��
		parent->ui.vibrate_2->setChecked(false);
		parent->ui.rigen_2->setChecked(false);
		//�s�b�`�J�ڎ���
		parent->ui.pitchTime_2->setText(0);
		//�J�n��
		parent->ui.startString_2->setValue(0);
		//�I����
		parent->ui.endString_2->setValue(0);
		//���t��
		parent->ui.playerString_2->setValue(0);
		//�|���l
		parent->ui.pressureData_2->setPlainText(QString());
		//�|���l
		parent->ui.positionData_2->setPlainText(QString());

		QPalette pal = parent->ui.prepared_2->palette();
		pal.setColor(QPalette::Background, QColor("#ff9933"));
		parent->ui.prepared_2->setPalette(pal);
		parent->ui.prepared_2->setText(" NOT PREPARED ");

	}else if(nbPane == 2){

		//���K
		parent->ui.pitch_3->setCurrentIndex(0);
		//�s�b�`���j�b�g
		parent->ui.pitchUnit_3->setCurrentIndex(0);
		//�\��
		parent->ui.vibrate_3->setChecked(false);
		parent->ui.rigen_3->setChecked(false);
		//�s�b�`�J�ڎ���
		parent->ui.pitchTime_3->setText(0);
		//�J�n��
		parent->ui.startString_3->setValue(0);
		//�I����
		parent->ui.endString_3->setValue(0);
		//���t��
		parent->ui.playerString_3->setValue(0);
		//�|���l
		parent->ui.pressureData_3->setPlainText(QString());
		//�|���l
		parent->ui.positionData_3->setPlainText(QString());

		//�F
		QPalette pal = parent->ui.prepared_3->palette();
		pal.setColor(QPalette::Background, QColor("#ff9933"));
		parent->ui.prepared_3->setPalette(pal);
		parent->ui.prepared_3->setText(" NOT PREPARED ");

	}else{
		Error::Critical(0, QObject::tr("[ PLAYTAB ] SetupPane �̃y�[���ԍ����s���ł�: %1").arg(nbPane));
	}

}
void PlayTab::Update()
{
	//do nothing
}
