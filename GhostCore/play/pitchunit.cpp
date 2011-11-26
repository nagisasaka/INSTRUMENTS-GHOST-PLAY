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
 * �R���X�g���N�^�C���p����A�N�`���G�[�^�ԍ���������
 * 
 * \param nb_unit
 * ���K���胆�j�b�g�ԍ�[0,1]
 * 
 */
PitchUnit::PitchUnit(int _nb_unit) : nb_unit(_nb_unit)
{
	//���p����A�N�`���G�[�^�ԍ���������
	if(nb_unit == 0){

		//���ԍ�
		nb_axis  = 3;
		//�T�[�{�ԍ�
		nb_servo = 0;
		//IAIC�ԍ�
		nb_iaic  = 2; 

	}else if(nb_unit == 1){

		//���ԍ�
		nb_axis  = 4;
		//�T�[�{�ԍ�
		nb_servo = 1;
		//IAIC�ԍ�
		nb_iaic  = 1; 

	}

	//���̑�
	stringName = "G"; // ������
	pitchName = "G";  // ������
	vibrateFlag = false;
	rigenFlag = false;
	transitionTime = 0;
	playTime = 0; //�{�[�C���O�}�l�[�W������v���y�A���ɗ^������
	vibrateHz = 0;
	vibratePercent = 0;
	contactDelay = 0;

	//�ݒ胍�[�h
	calibration = new PitchCalibration(nb_unit);
	calibration->Load();
	z_value = 0;
	c_value = 0;

	prepared = false;
}


/*!
 * \brief
 * �f�[�^����̃R���X�g���N�^
 * 
 * \param initData
 * �Z�[�u�f�[�^
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

	playTime = 0; //�O�����ϐ��̂��ߕۑ����Ȃ�

	//���p����A�N�`���G�[�^�ԍ���������
	if(nb_unit == 0){

		//���ԍ�
		nb_axis  = 3;
		//�T�[�{�ԍ�
		nb_servo = 0;
		//IAIC�ԍ�
		nb_iaic  = 2; 

	}else if(nb_unit == 1){

		//���ԍ�
		nb_axis  = 4;
		//�T�[�{�ԍ�
		nb_servo = 1;
		//IAIC�ԍ�
		nb_iaic  = 1; 

	}

	//�ݒ胍�[�h
	calibration = new PitchCalibration(nb_unit);
	calibration->Load();
	z_value = 0;
	c_value = 0;

	prepared = false;
}


/*!
 * \brief
 * �ۑ��\�Ȍ`���ɕϊ�
 * 
 * \returns
 * �ۑ��\�ȕ�����
 * 
 */
QString PitchUnit::toString()
{
	//�ۑ��\�Ȍ`���֕ϊ�
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
 * �s�b�`���j�b�g�厲�̌��݈ʒu��Ԃ�
 * 
 * \returns
 * ���݈ʒu[0,100]
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
 * ����n�̌��ԍ��̌n����C�ۑ��t�@�C���̉ǐ��̂��錷�ԍ��`���ɕϊ�����
 * 
 * \param nb_string
 * ����n���ԍ��̌n
 * 
 */
void PitchUnit::SetString(int nb_string)
{
	if(nb_string == 0){
		stringName = "G";
	}else if(nb_string == 1){
		//���΍���
		stringName = "G";
	}else if(nb_string == 2){
		stringName = "D";
	}else if(nb_string == 3){
		//���΍���
		stringName = "D";
	}else if(nb_string == 4){
		stringName = "A";
	}else if(nb_string == 5){
		//���΍���
		stringName = "A";
	}else if(nb_string == 6){
		stringName = "E";
	}else{
		Error::Critical(0, QObject::tr("[ PITCHUNIT ] Invalid nb_string : %1 ").arg(nb_string));
	}
	calibration->SetStringName(stringName);

	//�����ύX���ꂽ��ăv���y�A���K�v
	prepared = false;
}


/*!
 * \brief
 * �s�b�`�����w�肷��
 * 
 * \param pitchName
 * �s�b�`��������
 * 
 */
void PitchUnit::SetPitch(QString _pitchName)
{
	pitchName = _pitchName;
	calibration->SetPitchName(pitchName);

	//�s�b�`���ύX���ꂽ��ăv���y�A���K�v
	prepared = false;
}


/*!
 * \brief
 * �O��I����Ԗ��w��̃v���y�A
 * 
 * \returns
 * �v���y�A��̊m�肵������X�e�b�v��
 * 
 * \remarks
 * �O��I����Ԗ��w��̏ꍇ�́C���ݎ厲�ʒu��O��I���ʒu�Ƃ��ėp����D
 * 
 */
int PitchUnit::Prepare()
{
	Controller *control = Controller::GetInstance();
	ActuatorStatus status = control->axis[nb_axis]->GetStatus();

	//���w��̏ꍇ�͌��݈ʒu��O��I���ʒu�Ƃ��Ď�舵���D
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
 * �O��I����Ԃ��w�肵���v���y�A
 * 
 * \param lastPosition
 * �O��I���ʒu[0,100]
 * 
 * \returns
 * �v���y�A��Ɋm�肵������X�e�b�v��
 * 
 */
int PitchUnit::Prepare(double lastPosition)
{
	//// �e����V�[�P���X�N���A

	//�厲�i�s�b�`����j
	pitchSequence.clear();
	//��񎲁i���I���j
	stringSelectSequence.clear();
	//��O���i�������j
	pushpullSequence.clear();

	//// �Z�b�g���ꂽ���e���L�����u���[�V�����t�@�C�����烊�]���u����

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
		std::cout << "[ �s�b�`���j�b�g ] �L�����u���[�V�����t�@�C�����ɐݒ肪������܂���: stringName=" << stringName.toStdString() << ",pitchName=" << pitchName.toStdString() << ",nbUnit=" << nb_unit << std::endl;
		Error::Critical(0, QObject::tr("Could not find calibration data in the pitch calibration file."));
	}

	//�������i�A���l�ɑΉ��ł��Ȃ��̂őO��I���ʒu�͈����Ƃ��Ď擾�����j
	//�s�b�`�V�[�P���X�̓J�����g�̍��W����̍����Ƃ��č��D
	//Controller *control = Controller::GetInstance();
	//ActuatorStatus status = control->axis[nb_axis]->GetStatus();
	//double start_position = status.Position;

	double start_position = lastPosition; // �O��I���ʒu
	double end_position = z_value;        // ����ڕW�ʒu
	double curr = start_position;
	double run_length = end_position - start_position; // ���񑖔j����

	double step = 0;
	if(transitionTime == 0){

		//�J�ڎ��Ԃ���������̎��́C�ŒZ���ԂőJ�ڂ���D
		std::cout << "[ �s�b�`���j�b�g ] �J�ڎ�������" << std::endl;
		//�X�e�b�v�͍ő呬�x 4[%/step]
		step = 4;

	}else{

		//�J�ڎ��Ԃ��w�肳��Ă���D���̎w�莞�ԂőJ�ڂ���悤�ɓ�����
		std::cout << "[ �s�b�`���j�b�g ] �J�ڎ��Ԏw�肳��Ă��܂�: " << transitionTime << " [ms]" << std::endl;

		//run_length[%]��transitionTime[�X�e�b�v]�ňړ�����D
		step = run_length / transitionTime; // ����1step���Ƃ����run_length��1�X�e�b�v�ňړ����邱�ƂɂȂ�D2step���Ƃ����run_length��2step�ňړ��D
		if(step > 4){ // �ő呬�x�����͖{���͎��ɒ�`����ׂ������b�����
			std::cout << "[ �s�b�`���j�b�g ] �ړ����x���߃G���[." << std::endl;
		}

	}

	//���K����ړ��i�厲�̈ړ��j�Ɋ|�������X�e�b�v�������̂ق��ŗ����J�ڂɗp����
	int nbPitchDescition = 0;

	if(abs(run_length) < 1e-5){ // �����̔����͌덷�ɒ��ӂ��邱��
		pitchSequence.append(end_position);
		nbPitchDescition++;
		if(transitionTime){
			//�J�ڎ��Ԃ��w�肳��Ă��鎞�Ɉړ��������Z�����邱�Ƃ̌x��
			std::cout << "[ �s�b�`���j�b�g ] �ړ��������Z�����C�J�ڎ��Ԏw��𖞂����܂���D�w��l�����Ⴂ���Ă��܂��񂩁H" << std::endl;
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
	std::cout << "[ �s�b�`���j�b�g ] �ړ��V�[�P���X����C�X�e�b�v�� : " << 	nbPitchDescition << " �X�e�b�v�D" << std::endl;

	//�Ō�Ƀp�f�B���O
	if(playTime){
		std::cout << "[ �s�b�`���j�b�g ] ���t���Ԃ��w�肳��Ă��邽�߃p�f�B���O���s���܂��D" << std::endl;

		//�p�f�B���O�������߂遁�i���t���ԁj�|�i���݂܂łɎg�����X�e�b�v���j
		int togo = playTime - pitchSequence.size();
		if(togo < 0){
			std::cout << "[ �s�b�`���j�b�g ] �p�f�B���O�l�����ł��D�ݒ�゠�肦�܂���D" << std::endl;
			Error::Critical(0, QObject::tr("[ �s�b�`���j�b�g ] �p�f�B���O�l�����ł��D�ݒ�゠�肦�܂���D"));
		}

		if(vibrateFlag){

			//�r�u���[�g�t���O�������Ă���ꍇ�́C�ړ���Ƀr�u���[�g����D�r�u���[�g���Ȃ���ړ����邱�Ƃ͎b��I�ɖ���
			//�r�u���[�g���Ԃ��K�v�ɂȂ邽�߁C���Ԃ��w�肳����K�v�����邩�H���Ƃ���ƁC�����Ŏ��ԕ��ŏI����l���p�f�B���O���Ă��܂��Ηǂ����H
			//�g��Ȃ��ق��̎��͎��̈ʒu�֐���i�񓯎������j�Ȃ̂łn�j

			int vibrateNbStep = MakeVibrate(togo);

			//�Ō�]���������p�f�B���O����
			int togo2 = playTime - pitchSequence.size();
			std::cout << "[ �s�b�`���j�b�g ] �r�u���[�g��c���p�f�B���O��: " << togo2 << std::endl; 
			double last_value = pitchSequence[pitchSequence.size()-1];
			for(int i=0;i<togo2;i++){
				pitchSequence.append(last_value);
			}

		}else{
			//�r�u���[�g�t���O�������Ă��Ȃ��ꍇ

			double last_value = pitchSequence[pitchSequence.size()-1];
			std::cout << "[ �s�b�`���j�b�g ] �ŏI�l: " << last_value << " �Ńp�f�B���O���s���܂��D�p�f�B���O��: " << togo << std::endl;

			//�p�f�B���O�����s
			for(int i=0;i<togo;i++){
				pitchSequence.append(last_value);
			}
		
		}

	}else{

		std::cout << "[ �s�b�`���j�b�g ] ���t���Ԃ��w�肳��Ă��Ȃ����߁C�p�f�B���O���s���܂���C���̎w��͉��t�v���͗L�蓾�܂��񂪁C���Ⴂ���Ă��܂��񂩁H";

	}

	//// ��񎲂Ɋւ��Đ���V�[�P���X�𖄂߂�
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

	//// ��O���Ɋւ��Đ���V�[�P���X�𖄂߂�

	//�R���^�N�g�f�B���C�Ɨ����J�ڂ̌��ˍ���
	int nbNotContactStep = 0;
	if(rigenFlag){
		//�����J�ڃt���O�������Ă���ꍇ
		if(contactDelay < nbPitchDescition){
			//�R���^�N�g�f�B���C���C�s�b�`����ړ��X�e�b�v�̕��������ꍇ�́C�R���^�N�g�f�B���C�͋z�������
			nbNotContactStep = nbPitchDescition;	
		}else{
			//�R���^�N�g�f�B���C�̕����C�s�b�`����ړ��X�e�b�v��蒷���ꍇ�́C�R���^�N�g�f�B���C����҂�
			nbNotContactStep = contactDelay;
		}
	}else{
		//�����J�ڃt���O�������Ă��Ȃ��ꍇ�i�ʏ�j
		nbNotContactStep = contactDelay; // ������f�t�H���g�̓[��
	}

	for(int i=0;i<pitchSequence.size();i++){
		if(i < nbNotContactStep){
			//����
			pushpullSequence.append(0);
		}else{
			//������
			pushpullSequence.append(1);
		}
	}

	////�v���y�A���s

	//�厲
	Controller *control = Controller::GetInstance();
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->SetMode(MLinear::CONTINUOUS);
	axis->SetPositionVector(pitchSequence);
	axis->Prepare();

	//���
	control->iaic[nb_iaic-1]->SetMode(IAICtrler::PREDEFINED);
	control->iaic[nb_iaic-1]->SetControlVector(stringSelectSequence);
	control->iaic[nb_iaic-1]->Prepare();

	//��O��
	control->servo[nb_servo]->SetMode(Servo::PREDEFINED);
	control->servo[nb_servo]->SetControlVector(pushpullSequence);
	control->servo[nb_servo]->Prepare();

	//�v���y�A�I��
	prepared = true;

	//�T�C�Y��Ԃ��i�}�l�[�W���N���X�ł͂��̃T�C�Y�ɂ��āC�w�莞�ԕ����ŏI����l�Ńp�f�B���O���邱�ƂɂȂ�j
	return pitchSequence.size();
}


/*!
 * \brief
 * �r�u���[�g�V�[�P���X�����C�s�b�`�V�[�P���X�ɒǉ�����D
 * 
 * \param togo
 * �c�藘�p�\�ȃX�e�b�v��
 * 
 * \returns
 * �쐬�����r�u���[�g�V�[�P���X�ŗ��p�����X�e�b�v��
 * 
 * \remarks
 * �֐����Ńs�b�`�V�[�P���X�ɒǉ����Ă���̂Œ���
 * 
 */
int PitchUnit::MakeVibrate(int togo)
{
	std::cout << "[ �s�b�`���j�b�g ] �r�u���[�g�w��." << std::endl;

	//// �r�u���[�g�����Ɣ��l�����w�肳��Ă���D

	//�r�u���[�g1�����̎��Ԃ́H
	double vibrate_time = 1.0 / vibrateHz; // �b ( 5 Hz �̎� 0.2 �b)
	
	//�X�e�b�v�����Z�ŁH
	int vibrate_step = vibrate_time * 1000. / (double) Controller::GetCycle(); // �X�e�b�v ( 5 Hz �̎� 0.2 �b�Ȃ̂� 20 �X�e�b�v)

	//�c��X�e�b�v���ɑ΂��āC����r�u���[�g�����s�ł���H
	int vibrateNb = togo / vibrate_step;

	//���l���𔼎����ŉ������Ȃ���΂Ȃ�Ȃ���1/4�����Ŕ��l���ɒB���邽�߂̈ړ����U�l�́H
	double vibrate_unit = vibratePercent / (vibrate_step / 4.); // 5 Hz, 2% �̎� 2 / 5 �Ȃ̂� 0.4

	//�r�u���[�g�̒��S�ʒu�́H
	double vibrateCenter = pitchSequence[pitchSequence.size()-1];

	std::cout << "[ �s�b�`���j�b�g ] �r�u���[�g�Z���^�[: " << vibrateCenter << "�C�r�u���[�g��: " << vibrateNb << std::endl;

	//�v�Z�����C�s�b�`�V�[�P���X�Ƀr�u���[�g�V�[�P���X��ǉ�
	int ret = 0;
	for(int i=0;i<vibrateNb;i++){

		// �߂�
		// 1/4���܂�
		for(int j=0;j<vibrate_step/4.;j++){
			pitchSequence.append(vibrateCenter - vibrate_unit*(j+1));
			ret++;
		}
		// 1/2���i���l���j�܂�
		for(int j=vibrate_step/4.;j>0.;j--){
			pitchSequence.append(vibrateCenter - vibrate_unit*(j-1));
			ret++;
		}

		// �s��
		// 1/4���܂�
		for(int j=0;j<vibrate_step/4.;j++){
			pitchSequence.append(vibrateCenter + vibrate_unit*(j+1));
			ret++;
		}
		// 1/2���i���l���j�܂�
		for(int j=vibrate_step/4.;j>0.;j--){
			pitchSequence.append(vibrateCenter + vibrate_unit*(j-1));
			ret++;
		}

	}

	return ret;

	/*
	//// �X�^�e�B�b�N�ȃr�u���[�g�i�������j�G�Q�l�܂�
	//�O��0.4%����5�X�e�b�v=2%�C�߂�0.4%����5�X�e�b�v=2%�i���_�j�G���ꂪ�Г���10�X�e�b�v�C����20�X�e�b�v�D0.2�b�D�]����5Hz
	double vibrateStep = 20;
	int vibrateNb = togo / vibrateStep;
	double vibrateCenter = pitchSequence[pitchSequence.size()-1];
	std::cout << "[ �s�b�`���j�b�g ] �r�u���[�g�Z���^�[: " << vibrateCenter << "�C�r�u���[�g��: " << vibrateNb << std::endl;
	for(int i=0;i<vibrateNb;i++){
		//�߂�
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
		//�s��
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
	std::cout << "[ �s�b�`���j�b�g ] ����R�~�b�g" << std::endl;
}



void PitchUnit::run()
{
	Controller *control = Controller::GetInstance();

	//�厲
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->Commit(); // ���b�N���܂���	

	//���
	control->iaic[nb_iaic-1]->Commit(); //���b�N���܂���

	//��O��
	Servo *servo = (Servo*)control->servo[nb_servo];
	//����x�N�^�����ɕύX���܂���
	//std::cout << "c_value: " << c_value << std::endl;
	//servo->SetPosition(c_value);
	servo->Commit(); // ���b�N���܂���

}


/*!
 * \brief
 * �W���O��������s����i���쎎���y�уL�����u���[�V�����ɗ��p����j�C���b�N���܂���D
 * 
 * \param z_value
 * �厲�ڕW�l[0,100]
 * 
 * \param c_value
 * �����ݎ��ڕW�l[0,100]
 * 
 */
void PitchUnit::Jog(double z_value, double z_time, double c_value)
{
	std::cout << "[ �s�b�`���j�b�g ] �W���O��������s���܂��i�ڕW�ʒu: " << z_value << ", ���쎞��: " << z_time << ", �������݊p�x: " << c_value << "�j" << std::endl;
	Controller *control = Controller::GetInstance();

	//�T�[�{
	Servo *servo = (Servo*)control->servo[nb_servo];
	servo->SetPosition(c_value);
	servo->Commit(); // ���b�N���܂���

	//�厲
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->SetMode(MLinear::JOG);
	axis->SetPosition(z_value);
	axis->SetDuration(z_time, 0, 0);
	axis->Prepare();
	axis->Commit(); // ���b�N���܂���
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
 * ���j�b�g���z�[���|�W�V�����ֈړ������܂��C���b�N���܂���D
 * 
 * \remarks
 * ���j�b�g���Ƃ̃z�[���|�W�V�����ړ��֐��ł��D���b�N���܂���̂ŁC�ړ���Ԃ̃`�F�b�N�������̓f�t�E�F�C�g��u���Ă��������D
 * 
 */
/*
void PitchUnit::SetHome()
{
	std::cout << "[ �s�b�`���j�b�g ] ���j�b�g���z�[���|�W�V�����ֈړ����܂��D";
	Controller *control = Controller::GetInstance();

	//�T�[�{
	Servo *servo = (Servo*)control->servo[nb_servo];
	servo->SetPosition(0.0);
	servo->Commit(); // ���b�N���܂���

	//�厲
	MLinear *axis = (MLinear*)control->axis[nb_axis];
	axis->SetHome(); // ���b�N���܂���

	//IAIC�ɂ��Ă̓A�u�\�����[�g�Ȃ̂Ńz�[���|�W�V�����ֈړ�����K�v�͂���܂���D
}
*/