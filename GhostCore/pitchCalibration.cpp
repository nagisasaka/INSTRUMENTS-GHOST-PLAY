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

#include "pitchCalibration.hpp"
#include "calibration.hpp"
#include "play/pitchunit.hpp"
#include "constants.hpp"
#include "error.hpp"
#include "bowing.hpp"

#include <QSettings>
#include <QStringList>
#include <QString>

#include <cmath>

PitchCalibration::PitchCalibration(int _nb_pitchUnit) : nb_pitchUnit(_nb_pitchUnit)
{
	//// �|�W�V������`

	//G����̃|�W�V����
	pg["G"]  = 195.97; // �J���� // 1
	pg["G#"] = 207.65; // 2
	pg["A"]  = 220.08; // 3
	pg["A#"] = 233.08; // 4
	pg["B"]  = 246.94; // 5
	pg["C"]  = 261.62; // 6
	pg["C#"] = 277.18; // 7
	pg["D"]  = 293.66; // 8
	//pg["D#"] = 311.12; // 9

	//D����̃|�W�V����
	pd["D"]  = 293.66; // �J���� // 1
	pd["D#"] = 311.12; // 2
	pd["E"]  = 329.62; // 3
	pd["F"]  = 349.23; // 4
	pd["F#"] = 369.99; // 5
	pd["G"]  = 391.99; // 6
	pd["G#"] = 415.30; // 7
	pd["A"]  = 440.00; // 8
	//pd["A#"] = 466.16; // 9

	//A����̃|�W�V����
	pa["A"]  = 440.00; // �J���� // 1
	pa["A#"] = 466.16; // 2
	pa["B"]  = 493.88; // 3
	pa["C"]  = 523.25; // 4
	pa["C#"] = 554.36; // 5
	pa["D"]  = 587.32; // 6
	pa["D#"] = 622.25; // 7
	pa["E"]  = 659.25; // 8
	//pa["F"]  = 698.45; // 9

	//E����̃|�W�V����
	pe["E"]  = 659.25; // �J���� // 1
	pe["F"]  = 698.45; // 2
	pe["F#"] = 739.98; // 3
	pe["G"]  = 783.99; // 4
	pe["G#"] = 830.61; // 5
	pe["A"]  = 880.00; // 6
	pe["A#"] = 932.33; // 7
	pe["B"]  = 987.76; // 8
	//pe["C"]  = 1046.50;// 9

	//// ���̑�

	currentString = "G"; // ������
	currentPitch  = "G"; // ������
	operationMode = 0;

	//// �L�����u���[�V�����p�{�[�C���O�ݒ�
	MakeStandardBowing();
}

PitchCalibration::~PitchCalibration()
{
	pg.clear();
	pd.clear();
	pa.clear();
	pe.clear();
}

void PitchCalibration::MakeStandardBowing()
{
	//������O�ł����G
	//1%->170=1.7mm
	//10msec(1/100�b)��1%->10msec(1/100�b)��1.7mm->1000msec(1�b)��170mm=�b��170mm=�b��17cm=�b��0.17m=1�b�ԂŃX�g���[�N���g���؂鑬�x

	//10msec��2%->�b��34cm(0.34m/s)
	//10msec��3%->�b��51cm(0.51m/s)
	//10msec��4%->�b��68cm(0.68m/s)
	//10msec��5%->�b��85cm(0.85m/s)

	double p = 0;
	int c = 0;
	
	//// �Г�
	pressure.clear();
	bowposition.clear();

	if(currentString != "G"){

		for(int k=0;k<2;k++){

			//�˓����x�͍�����ۂ�
			for(int i=0;i<=50;i+=25){
				pressure.append(i);
			}//20msec

			//50%����� 1960msec
			for(int i=0;i<196;i++){		
				pressure.append(50);
			}

			//�����V�[�P���X
			for(int i=50;i>=0;i-=25){
				pressure.append(i);
			}//20msec

			//2220msec
		}

	}else{

		for(int k=0;k<2;k++){

			//�˓����x�͍�����ۂ�
			for(int i=0;i<=80;i+=10){ // 16sequence
				pressure.append(i);
			}//80msec

			//80%����� 1840msec
			for(int i=0;i<184;i++){		
				pressure.append(80);
			}

			//�����V�[�P���X
			for(int i=80;i>=0;i-=10){
				pressure.append(i);
			}//80msec
		}

	}

	//// �|����

	//�ŏ���150msec�͓����Ȃ�
	for(int i=0;i<15;i++){
		bowposition.append(0);
	}

	//2�b�����A�b�v�{�E

	for(int i=0;i<50;i++){
		bowposition.append(i);
		bowposition.append(i+0.25);
		bowposition.append(i+0.50);
		bowposition.append(i+0.75);
	}

	//50msec�҂�
	for(int i=0;i<5;i++){
		bowposition.append(50);
	}

	//2200msec

	//���̓V�[�P���X�I��

	//�ŏ���150msec�͓����Ȃ�
	for(int i=0;i<15;i++){
		bowposition.append(50);
	}

	//2�b�����_�E���{�E
	for(int i=50;i>0;i--){
		bowposition.append(i);
		bowposition.append(i-0.25);
		bowposition.append(i-0.5);
		bowposition.append(i-0.75);
	}

	//50msec wait
	for(int i=0;i<5;i++){
		bowposition.append(0);
	}

	//2200msec
	//4400msec
}

/*!
 * \brief
 * �s�b�`�L�����u���[�V�����������[�h����
 * 
 * �y�`���z
 * ID�ԍ�, ����, �s�b�`���C�s�b�`���j�b�g�ԍ��C���ʒu�C�T�[�{�����ʒu
 *
 *
 */
void PitchCalibration::Load()
{
	//�Â��|�W�V�����̓N���A����
	positions.clear();

	QSettings settings(Constants::INI_FILE_3, QSettings::IniFormat);

	QStringList keys = settings.allKeys();
	int size = keys.size();
	if(size == 0){
		std::cout << "[ �s�b�`�L�����u���[�V���� ] �L�����u���[�V�����ݒ�t�@�C�������݂��܂���" << std::endl;
	}else{
		std::cout << "[ �s�b�`�L�����u���[�V���� ] " << size << " �̃L�����u���[�V�����ʒu��ǂݍ��݂܂���" << std::endl;
	}
	for(int i=0;i<size;i++){
		QString line = settings.value(keys[i]).toString();
		QStringList temp = line.split(",");
		if(temp.size() != 6){
			std::cout << "[ �s�b�`�L�����u���[�V���� ] �s�`�����s���ł�: " << line.toStdString() << std::endl;
			Error::Critical(0, QObject::tr("[ �s�b�`�L�����u���[�V���� ] �s�`�����s���ł��D"));
		}
		CalibratedPositions pos;
		pos.id           = temp[0].toInt();    // ID�ԍ�
		pos.stringName   = temp[1];            // ����
		pos.pitchName    = temp[2];            // �s�b�`��
		pos.nb_pitchUnit = temp[3].toInt();    // ���ԍ�
		pos.z_value      = temp[4].toDouble(); // Z���ʒu
		pos.c_value      = temp[5].toDouble(); // C���ʒu
		positions.append(pos);		
	}
}

/*!
 * \brief
 * �s�b�`�L�����u���[�V���������Z�[�u����
 * 
 * �y�`���z
 * ID�ԍ�, ����, �s�b�`���C���ԍ��C���ʒu�C�T�[�{�����ʒu
 *
 *
 */
void PitchCalibration::Save()
{
	QSettings settings(Constants::INI_FILE_3, QSettings::IniFormat);

	int size = positions.size();
	if(size == 0){
		std::cout << "[ �s�b�`�L�����u���[�V���� ] �L�����u���[�V�����f�[�^�����݂��Ȃ���Ԃŕۑ������s���܂����D" << std::endl;
	}else{
		std::cout << "[ �s�b�`�L�����u���[�V���� ] " << size << " �̃L�����u���[�V�����f�[�^�̕ۑ����s���܂��D" << std::endl;
	}

	//�L�����u���[�V�����S�̂�ۑ�����̂Ŗⓚ���p�ŒǋL����Ă��܂����Ƃɒ��ӁD
	for(int i=0;i<size;i++){
		CalibratedPositions pos = positions[i];

		QString writer = QString("%1,").arg(pos.id);
		writer.append(pos.stringName).append(",");
		writer.append(pos.pitchName).append(",");
		writer.append(QString("%1,").arg(pos.nb_pitchUnit));
		writer.append(QString("%1,").arg(pos.z_value));
		writer.append(QString("%1").arg(pos.c_value));

		QString key = QString();
		key.append(pos.stringName);
		key.append(pos.pitchName);
		key.append(QString("%1").arg(pos.nb_pitchUnit));

		settings.setValue(key, writer);
	}
	std::cout << "�ۑ����܂����D" << std::endl;
}

void PitchCalibration::Save(double z_value, double c_value)
{

	int update_target = -1; // �����f�[�^�̃C���f�b�N�X
	for(int k=0;k<positions.size();k++){
		CalibratedPositions pos = positions[k];
		if(pos.stringName == currentString && 
		   pos.pitchName == currentPitch && 
		   pos.nb_pitchUnit == nb_pitchUnit){
			   std::cout << "[ �s�b�`�L�����u���[�V���� ] ��: " << currentString.toStdString() << ", �s�b�`:" << currentPitch.toStdString() << ", ���j�b�g�ԍ�:" << nb_pitchUnit << std::endl;
				update_target = k;
				break;
		}
	}

	if(update_target == -1){
		//�����f�[�^�ɂȂ��̂Œǉ�
		std::cout << "[ �s�b�`�L�����u���[�V���� ] �V�K�ǉ����܂��D" << std::endl;
		CalibratedPositions pos;
		pos.id           = positions.size()+1; // ID�ԍ�
		pos.stringName   = currentString;      // ����
		pos.pitchName    = currentPitch;	   // �s�b�`��
		pos.nb_pitchUnit = nb_pitchUnit;       // ���ԍ�
		pos.z_value      = z_value;				// Z���ʒu
		pos.c_value      = c_value;			   // C���ʒu
		//�ǉ�
		positions.append(pos);					
	}else{
		//�����f�[�^���X�V
		std::cout << "[ �s�b�`�L�����u���[�V���� ] �X�V���܂��D" << std::endl;
		positions[update_target].z_value = z_value;
		positions[update_target].c_value = c_value;
	}
	Save();
}


/*!
 * \brief
 * ���Ɋ܂܂�邷�ׂẴ|�W�V�����΂���L�����u���[�V�������s
 * 
 * \param stringName
 * �Ώی�
 * 
 */
void PitchCalibration::Start(QString stringName)
{
	currentString = stringName;	
	operationMode = 1;
	start();
}

/*!
 * \brief
 * �P���ɑ΂���L�����u���[�V�������s
 * 
 * \param stringName
 * �Ώی�
 *
 * \param pitchName
 * �Ώۃs�b�`��
 *
 */
void PitchCalibration::Start(QString stringName, QString pitchName)
{
	currentString = stringName;
	currentPitch  = pitchName;
	operationMode = 2;
	start();
}

void PitchCalibration::run()
{
	//�s�b�`�L�����u���[�V�������ʂ��������[�h
	Load();

	//�{�[�C���O�č\�z
	MakeStandardBowing();

	//�ꎞ�ۑ�
	QMap<double, double> scale;

	Controller *control = Controller::GetInstance();
	StringRecorder *recorder = control->GetStringRecorder();
	
	//// ���p����s�b�`���j�b�g���\�z
	PitchUnit *punit = 0;
	punit = new PitchUnit(nb_pitchUnit);

	//// ���p���Ȃ��s�b�`���j�b�g�͑ޔ��i�s�b�`���j�b�g���R�ȏ㉻�����ꍇ�͕ʓr�Ή�����j
	int nb_otherPitchUnit = 0;
	if(nb_pitchUnit == 0){
		nb_otherPitchUnit = 1;
	}else{
		nb_otherPitchUnit = 0;
	}
	PitchUnit *others = 0;
	others = new PitchUnit(nb_otherPitchUnit);
	others->Evacuate();	

	if(operationMode == 1){
		//�S�|�W�V����

		//�{�[�C���O����
		Bowing *bowing = new Bowing();

		//�s�b�`���j�b�g�����ړ����s
		punit->JogGoToString(currentString);
		//0.5�b�f�t�E�F�C�g
		Sleep(500);

		//�s�b�`���j�b�g���s, 70%�܂�140�b(1%/2�b)/����90�x

		double c_value = 0;
		if(nb_pitchUnit == 0){
			c_value = 55;
		}else{
			c_value = 130;
		}
		punit->Jog(75, 160000, c_value);
		std::cout << "[ �s�b�`�L�����u���[�V���� ] �s�b�`���j�b�g�̋쓮�J�n" << std::endl;

		int main_counter = 0;
		while(true){
			//�����ړ�
			if(main_counter == 0){
				bowing->SetInitialMoveFlag(true);
			}else{
				bowing->SetInitialMoveFlag(false);
			}
			//���I��
			if(currentString == "G"){
				bowing->SetStringPair(0,0);
				bowing->SetPlayerString(0);

			}else if(currentString == "D"){
				bowing->SetStringPair(2,2);
				bowing->SetPlayerString(2);

			}else if(currentString == "A"){

				bowing->SetStringPair(4,4);
				bowing->SetPlayerString(4);

			}else if(currentString == "E"){

				bowing->SetStringPair(6,6);
				bowing->SetPlayerString(6);

			}
			std::cout << "A";
			bowing->SetPressureVector(pressure);
			std::cout << "B";
			bowing->SetPositionVector(bowposition);
			std::cout << "C";
			bowing->Prepare();
			std::cout << "D";
			//�{�[�C���O���s�i���b�N���Ȃ��j
			bowing->Start();
			std::cout << "[ �s�b�`�L�����u���[�V���� ] # " << main_counter << " �Z�b�g �W���{�[�C���O�J�n�DCPS��͂��J�n���܂��D" << std::endl;

			//// ���g���v��
			int sound_counter = 0;
			double min_fzero = 100;
			double max_fzero = 0;
			while(true){
				std::cout << 1;
				//��͌��ʃR�s�[�擾
				DWORD stime = GetTickCount();
				std::cout << 2;
				QVector<FrequencyDomainProfile*> fdp = recorder->FrequencyDomainAnalyze(1024, 0);//�I�[����i�ŐV�j��1024*20�T���v���i��0.2�b���j
				std::cout << 3;
				//FZERO�̎擾
				double fzero = fdp.at(fdp.size()-1)->F0;
				std::cout << 4;
				//���݈ʒu�̎擾
				double pos = punit->GetCurrentPosition();
				std::cout << 5;
				//�L�^
				if(fzero != 0){
					//�[���X�L�b�v
					scale[pos] = fzero;
				}
				//�͈͂��L�^
				if(fzero < min_fzero){
					min_fzero = fzero;
				}
				if(max_fzero < fzero){
					max_fzero = fzero;
				}
				//��͌��ʂ̍폜
				for(int i=0;i<fdp.size();i++) delete fdp[i];
				std::cout << 6;

				DWORD etime = GetTickCount();
				double timecost = etime - stime;				
				//�f�t�E�F�C�g
				Sleep(100 - timecost); // 0.1�b���Ƃɑ���
				sound_counter++;
				if(main_counter == 0){
					if(sound_counter > 65){// 7�b�ԘA���v���i�擪2�b�̃C�j�V�������[�u�j
						break;
					}
				}else{
					if(sound_counter > 45){// 5�b�ԘA���v��
						break;
					}
				}
			}
			std::cout << "[ �s�b�`�L�����u���[�V���� ] # " << main_counter << " �ŏ�F0: " << min_fzero << ", �ő�F0: " << max_fzero << " ���L�^���܂����D�Z�b�g�I�����܂��D" << std::endl;

			main_counter++;
			if(main_counter > 30){
				//���悻140�b��70%�𒴂��ďI��
				break;
			}
		}
		std::cout << "[ �s�b�`�L�����u���[�V���� ] ����I���D" << std::endl;
		
		//�㏈��
		QMap<QString, double> rscale;
		if(currentString == "G"){
			rscale = pg;
		}else if(currentString == "D"){
			rscale = pd;
		}else if(currentString == "E"){
			rscale = pe;
		}else if(currentString == "A"){
			rscale = pa;
		}

		QMapIterator<QString, double> i(rscale);
		while(i.hasNext()){
			i.next();
			//�Q�ƃX�P�[���ɑ΂���
			currentPitch = i.key();  //�s�b�`��
			double frequency = i.value(); //�s�b�`�̎Q�Ǝ��g��
			std::cout << "[ �s�b�`�L�����u���[�V���� ] " << currentPitch.toStdString() << " �i�Q�Ǝ��g��: " << frequency << "Hz�j�̍ŋߖT����_���Z�o��..." << std::endl;

			double mindistance = 100; // �v���X�P�[���ƎQ�ƃX�P�[���̍ŏ������i���g���j
			double nearestpos  = 0;   // �ŋߖT�v���_

			QMapIterator<double, double> j(scale);
			double m_pos = 0;
			double m_frequency = 0;
			double m_distance = 0;
			double minfreq = 0;
			while(j.hasNext()){
				j.next();
				//�v���X�P�[���ɑ΂���
				m_pos = j.key();         //�v���_
				m_frequency = j.value(); //�v�����g��
	
				m_distance = fabs((double)(frequency-m_frequency));
				if(m_distance < mindistance){
					//�ŋߖT�_��������
					mindistance = m_distance; 
					minfreq = m_frequency;
					nearestpos = m_pos;
				}
			}
			std::cout << "[ �s�b�`�L�����u���[�V���� ] �v���_: " << nearestpos << ", �v�����g��: " << minfreq  << " �� "<< mindistance << " Hz ���ŁC�ŋߖT�_�Ƃ��Ċm��D" << std::endl;

			//�ŋߖT�_��S�����L�����u���[�V�������ʂƂ��ĕۑ�

			//�����f�[�^�ɑ��݂���΍X�V�C������Βǉ�
			int update_target = -1; // �����f�[�^�̃C���f�b�N�X
			for(int k=0;k<positions.size();k++){
				CalibratedPositions pos = positions[k];
				if(pos.stringName == currentString && 
					pos.pitchName == currentPitch && 
					pos.nb_pitchUnit == nb_pitchUnit){
						update_target = k;
						break;
				}
			}
			if(update_target == -1){
				//�����f�[�^�ɂȂ��̂Œǉ�
				std::cout << "[ �s�b�`�L�����u���[�V���� ] �V�K�ǉ����܂��D" << std::endl;
				CalibratedPositions pos;
				pos.id           = positions.size()+1; // ID�ԍ�
				pos.stringName   = currentString;      // ����
				pos.pitchName    = currentPitch;	   // �s�b�`��
				pos.nb_pitchUnit = nb_pitchUnit;       // ���ԍ�
				pos.z_value      = nearestpos;		   // Z���ʒu
				pos.c_value      = c_value;			   // C���ʒu
				//�ǉ�
				positions.append(pos);					
			}else{
				//�����f�[�^���X�V
				std::cout << "[ �s�b�`�L�����u���[�V���� ] �X�V���܂��D" << std::endl;
				positions[update_target].z_value = nearestpos;
				positions[update_target].c_value = c_value;
			}
		}
		std::cout << "[ �s�b�`�L�����u���[�V���� ] ���L�����u���[�V�������I�����܂����D�ۑ����܂��D" << std::endl;

		//�s�b�`�L�����u���[�V�������ʂ��Z�[�u
		Save();

	}else if(operationMode == 2){
		//�P��

		//�^�[�Q�b�g���g���̊m�F




	}else{
		Error::Critical(0, QString("[ Pitch Calibration ] Invalid Operation Mode."));
	}

	delete punit;
}