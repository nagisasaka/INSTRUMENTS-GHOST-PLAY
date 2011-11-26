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

#include "pitchManager.hpp"
#include "error.hpp"
#include "actuator/controller.hpp"


PitchManager::PitchManager()
{
	NbPitchUnit = 2;
}


PitchManager::~PitchManager()
{
	//�ێ����Ă���s�b�`���j�b�g���폜
	for(int i=0;i<puVector.size();i++){
		delete puVector[i];
	}
}


void PitchManager::AddPitch(PitchUnit *pu)
{

	//// �����ɃG���[�`�F�b�N�R�[�h���������Ă��ǂ� ////

	puVector.append(pu);
}


void PitchManager::DelPitch(int index)
{
	if(index >= puVector.size()){
		std::cout << "[ �s�b�`�}�l�[�W�� ] �폜�C���f�b�N�X���o�^�x�N�^�T�C�Y�𒴂��Ă��܂�." << std::endl;
		Error::Critical(0, QObject::tr("[ PitchManager ] Invalid delete index"));
	}

	//�폜
	puVector.remove(index);	
}


void PitchManager::UpdatePitch(PitchUnit *pu, int index)
{
	if(index >= puVector.size()){
		std::cout << "[ �s�b�`�}�l�[�W�� ] �X�V�C���f�b�N�X���o�^�x�N�^�T�C�Y�𒴂��Ă��܂�." << std::endl;
		Error::Critical(0, QObject::tr("[ PitchManager ] Invalid update index"));
	}

	//�ǉ�
	puVector.insert(index,pu);

	//�폜
	puVector.remove(index+1);	
}


PitchUnit* PitchManager::GetPitch(int index)
{
	if(index >= puVector.size()){
		std::cout << "[ �s�b�`�}�l�[�W�� ] �擾�C���f�b�N�X���o�^�x�N�^�T�C�Y�𒴂��Ă��܂�." << std::endl;
		Error::Critical(0, QObject::tr("[ PitchManager ] Invalid get index"));
	}

	return puVector.at(index);
}


void PitchManager::InitialMove()
{
	//�s�b�`�}�l�[�W���̃C�j�V�������[�u�͂ǂ�Ȏ����[���_�i�ŏ��Ɏ��Ԃ����K����J�ڎ��Ԃ�����Ȃ��Ƃ��́C�{�[�C���O���œ������Ȃ����ƂŃJ�o�[�j

	int duration = 2000;
	Controller *control = Controller::GetInstance();	
	for(int i=0;i<NbPitchUnit;i++){

		//��{����N���X�Ƃ̐ڑ��֌W�����邽�߂Ɉꎞ�\�z
		PitchUnit *pu = new PitchUnit(i);

		//�厲�̊�{����N���X�������ړ�
		control->axis[pu->GetNbAxis()]->SetMode(MLinear::JOG);
		control->axis[pu->GetNbAxis()]->SetPosition(0);
		control->axis[pu->GetNbAxis()]->SetDuration(duration);
		control->axis[pu->GetNbAxis()]->Prepare();
		control->axis[pu->GetNbAxis()]->Commit();

		//��񎲂̊�{����N���X�������ړ�
		if(stringSelectSequence[i][0] == 0){
			control->iaic[pu->GetNbIaic()-1]->GoToString("G");
		}else if(stringSelectSequence[i][0] == 1){
			control->iaic[pu->GetNbIaic()-1]->GoToString("D");
		}else if(stringSelectSequence[i][0] == 2){
			control->iaic[pu->GetNbIaic()-1]->GoToString("A");
		}else if(stringSelectSequence[i][0] == 3){
			control->iaic[pu->GetNbIaic()-1]->GoToString("E");
		}

		//��O���̊�{����N���X���R�~�b�g
		control->servo[pu->GetNbServo()]->SetReady();

		delete pu;
	}

	//�V�[�P���V�����ɑ҂�
	Sleep(duration+500);
}


/*!
 * \brief
 * �o�^����Ă���S�s�b�`���j�b�g�I�u�W�F�N�g�ɂ��ăv���y�A�����s����
 * 
 * \returns
 * �v���y�A�Ŋm�肵���s�b�`���j�b�g�̓���X�e�b�v
 * 
 */
int PitchManager::Prepare()
{
	int start = 0;
	int end = puVector.size();

	return Prepare(start,end);
}


/*!
 * \brief
 * �o�^����Ă���s�b�`���j�b�g�I�u�W�F�N�g�̈ꕔ���X���C�X���ăv���y�A����
 * 
 * \param start
 * �X���C�X�J�n�C���f�b�N�X
 * 
 * \param end
 * �X���C�X�I���C���f�b�N�X�i���̒l�����Gfor���[�v���Łu<�v �ŕ]�������j
 * 
 * \returns
 * �v���y�A�Ŋm�肵���s�b�`���j�b�g�̓���X�e�b�v
 * 
 */
int PitchManager::Prepare(int start, int end)
{
	if(puVector.size() < end){
		Error::Warning(0, QObject::tr("[ �s�b�`�}�l�[�W�� ] �v���y�A�͈͎w�肪�o�^�f�[�^���𒴂��Ă��܂��D"));
		return 0;
	}

	//// �e�����ƂɌ��̖����悤�ɖ��߂Ă����D

	//�J�����g�̃s�b�`���j�b�g�ɑ΂��āC���̃s�b�`���j�b�g�̔ԍ������āG
	//(1)����̏ꍇ�́C�Y���s�b�`���j�b�g�͒ʏ�ʂ�v���y�A�D����ȊO�̃s�b�`���j�b�g�͑O��I���l���g���ăp�f�B���O�D
	//(2)�قȂ�ꍇ�́C�Y���s�b�`���j�b�g�͒ʏ�ʂ�v���y�A�D���̃s�b�`���j�b�g�ɊY������s�b�`���j�b�g�̓J�����g�̃v���C�^�C���ȓ��ɏ����ړ��V�[�P���X��ǉ����ăp�f�B���O�D
	//   ����ȊO�̃s�b�`���j�b�g�́C�O��I���l���g���ăp�f�B���O�i��3,4�������䉺�ɖ����ꍇ�́u����ȊO�v�͑��݂��Ȃ��j�D




	//// �e���䎲�̒l���N���A

	//�厲�Q�G�s�b�`����
	axis[0].clear();  //��3�N���A #0�s�b�`���j�b�g
	axis[1].clear();  //��4�N���A #1�s�b�`���j�b�g

	//��񎲌Q�G���I��
	stringSelectSequence[0].clear(); //#0�s�b�`���j�b�g
	stringSelectSequence[1].clear(); //#1�s�b�`���j�b�g

	//��O���Q�G������
	pushpullSequence[0].clear(); //#0�s�b�`���j�b�g
	pushpullSequence[1].clear(); //#1�s�b�`���j�b�g

	//// �G���h�C���f�b�N�X���v�Z�ipuVector�̃��[�v�łЂƂ�����Ă���̂ŁCpuVector�̍Ō�̗v�f�������Ӂj
	int endIndex = end;
	if(end == puVector.size()){
		endIndex = puVector.size() - 1;
	}

	//�s�b�`���j�b�g�Ō��ݗ��p����Ă��Ȃ����j�b�g�ɂ��Ă��C����x�N�^�̃M���b�v�������Ă͂Ȃ�Ȃ��̂ŁC
	//����𖄂߂�K�v������̂ƁC���̉��t�ɔ����āC���O�ړ������Ă������Ƃ������ōs��Ȃ���΂Ȃ�Ȃ��D
	//
	//���̍ۂɁC����x�N�^�T�C�Y�́C�ǂ�ȂƂ��ł���Ɉ��ɂȂ��Ă��邱�Ƃ��`�F�b�N�����ɂȂ�D

	for(int i=start;i<endIndex;i++){

		//�J�����g�̃s�b�`���j�b�g
		PitchUnit *puCurr = puVector.at(i);

		//���̃s�b�`���j�b�g
		PitchUnit *puNext = puVector.at(i+1);
		
		//�Y���s�b�`���j�b�g�ɂ��Ă͒ʏ�ʂ�v���y�A
		if(axis[puCurr->GetNbUnit()].size() == 0){
			//�ŏ��̍ŏ�
			puCurr->Prepare(0);
		}else{
			puCurr->Prepare(axis[puCurr->GetNbUnit()][axis[puCurr->GetNbUnit()].size()-1]);
		}

		//�厲
		QVector<double> temp = puCurr->GetPositionVector();
		for(int k=0;k<temp.size();k++){
			axis[puCurr->GetNbUnit()].append(temp[k]);
		}

		//��񎲁i���I���j
		QVector<int> temp2 = puCurr->GetStringSelectSequence();
		for(int k=0;k<temp2.size();k++){
			stringSelectSequence[puCurr->GetNbUnit()].append(temp2[k]);
		}

		//��O���i�������j
		QVector<int> temp3 = puCurr->GetPushpullSequence();
		for(int k=0;k<temp3.size();k++){
			pushpullSequence[puCurr->GetNbUnit()].append(temp3[k]);
		}
				
		//// ��Y���s�b�`���j�b�g�ɂ��ẮC���̃s�b�`���j�b�g�̔ԍ��ƁC���̑I�����ɂ��C���삪�قȂ�

		if(puCurr->GetNbUnit() == puNext->GetNbUnit()){

			//// �J�����g�̃s�b�`���j�b�g�Ǝ��̃s�b�`���j�b�g������̏ꍇ�G

			for(int j=0;j<NbPitchUnit;j++){
				
				if(puCurr->GetNbUnit() == j){

					//// �J�����g�̃s�b�`���j�b�g�Ɋւ��Ă͊��ɏ����ςȂ̂ŉ������Ȃ�

					//// ���̃s�b�`���j�b�g�Ɋւ��āC���t�����قȂ�ꍇ�ɂ́C���̃��j�b�g���ڌ����邱�ƂɂȂ�̂ŃR���^�N�g�f�B���C���d����ł���

					if(puCurr->GetStringName() != puNext->GetStringName()){
						////���t�����قȂ�΁G
						puNext->SetContactDelay(20); // 100�X�e�b�v�̃R���^�N�g�f�B���C���d����ł��� 40step = 200msec = 0.4�b
					}
					
				}else{

					////�J�����g�̃s�b�`���j�b�g�ƁC���̃s�b�`���j�b�g�ȊO�́C���삳���Ȃ��D
					
					//�厲�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ńp�f�B���O����
					int playTime = puCurr->GetPlayTime();		  //�J�����g�̉��t����
					double lastValue = 0;
					if(axis[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = axis[j][axis[j].size()-1]; //�O��I���l
					}
					for(int h=0;h<playTime;h++){			   //�J�����g�̉��t���ԕ��C�O��I���l�Ńp�f�B���O�D
						axis[j].append(lastValue);
					}

					//��񎲁i���I���j�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ŗ��߂�
					if(stringSelectSequence[j].size() == 0){
						//�������͒��ӂ��恦
						if(j == 0) lastValue = 2;     // 0�ԃs�b�`���j�b�g�̏����ʒu��A��
						else if(j == 1) lastValue = 1;// 1�ԃs�b�`���j�b�g�̏����ʒu��D��
					}else{
						lastValue = stringSelectSequence[j][stringSelectSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						stringSelectSequence[j].append(lastValue);
					}

					//��O���i�������j�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ŗ��߂�
					if(pushpullSequence[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = pushpullSequence[j][pushpullSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						pushpullSequence[j].append(lastValue);
					}

				}

			}
		
		}else{

			//// �J�����g�̃s�b�`���j�b�g�Ǝ��̃s�b�`���j�b�g���قȂ�ꍇ

			for(int j=0; j<NbPitchUnit; j++){

				if(puCurr->GetNbUnit() == j){

					//// �J�����g�̊Y���s�b�`���j�b�g�́C�����ςȂ̂ŉ������Ȃ��ėǂ�

				}else if(puNext->GetNbUnit() == j){

					//// ���̊Y���s�b�`���j�b�g�ɂ��ẮC�����ňڌ������Ă���

					//�J�����g�̉��t����
					int playTime = puCurr->GetPlayTime();

					//���z�s�b�`���j�b�g���\�z
					PitchUnit *virtual_pu = new PitchUnit(j); 

					virtual_pu->SetPlayTime(playTime);
					virtual_pu->SetStringName(puNext->GetStringName()); //���̌�
					virtual_pu->SetPitch(puNext->GetPitchName());       //���̉��K
					//�R���^�N�g�f�B���C�Ɋւ��đI��
					//virtual_pu->SetContactDelay(playTime*0.5);	 // playTime�̔����̃R���^�N�g�f�B���C

					////�������̃R���^�N�g�f�B���C�͗]�T������D��Ɏ厲�𓮂����̂ŃR���^�N�g�f�B���C��300msec�𒴂��āC����ɑ傫��

					virtual_pu->SetContactDelay(80);//0.8sec				 // 100�X�e�b�v�̃R���^�N�g�f�B���C- 100�X�e�b�v = 1�b 50�X�e�b�v=0.5�b 10�X�e�b�v=100mec = 0.1�b

					//�g�����U�N�V�����^�C���Ɋւ��đI��
					//virtual_pu->SetTransitionTime(playTime * 0.5); // �J�����g�̉��t���Ԃ�50%�ňړ��I����ڎw��
					if(axis[j].size()==0){
						virtual_pu->Prepare(0);
					}else{
						virtual_pu->Prepare(axis[j][axis[j].size()-1]);
					}

					//�厲
					QVector<double> temp = virtual_pu->GetPositionVector();
					for(int k=0;k<temp.size();k++){
						axis[j].append(temp[k]);
					}

					//��񎲁i���I���j

					QVector<int> temp2 = virtual_pu->GetStringSelectSequence();
					int margin = 30;

					if(temp2.size() < margin){
						for(int k=0;k<temp2.size();k++){
							stringSelectSequence[j].append(temp2[k]);
						}
						std::cout << "���t���Ԃ�" << margin << "�X�e�b�v���Z���󋵂ł̈ڌ�����͓����������܂���D" << std::endl;
					}else{

						//�J�����g�̃s�b�`���j�b�g�Ɠ����ɓ����Ă����ꍇ
						//����Ɏ厲��D�悷��i�s�b�`���قȂ�̂ŃW������Ȃ��j
						//if(puNext->GetStringCode() <= puCurr->GetStringCode()){

							//�ŏ���margin msec�܂ł͑O��̎����̌��ԍ��𓊓��ipuCurr�͑��l�Ȃ̂Œ��ӁI�j
							int lastString = stringSelectSequence[j][stringSelectSequence[j].size()-1];
							for(int k=0;k<margin;k++){
								stringSelectSequence[j].append(lastString);
							}
							//n msec����Ō�܂ŐV�������ԍ��𓊓�
							for(int k=margin;k<temp2.size();k++){
								stringSelectSequence[j].append(temp2[k]);
							}

						//}else{

							//�J�����g�̃s�b�`���j�b�g�Ɠ�������o�čs���ꍇ
							//����ɑ�񎲂�D�悷��i�����قȂ�̂ŃW������Ȃ��j

							//for(int k=0;k<temp2.size();k++){
							//	stringSelectSequence[j].append(temp2[k]);
							//}

							//��ɑ�񎲂𓮂����������C���Ȃ�g���b�L�[�Ȏ����ɂȂ邽�߁C������߂�I

						//}
					}

					//��O���i�������j
					QVector<int> temp3 = virtual_pu->GetPushpullSequence();
					for(int k=0;k<temp3.size();k++){
						pushpullSequence[j].append(temp3[k]);
					}

					//���z�s�b�`���j�b�g���폜
					delete virtual_pu;

				}else{

					//// �J�����g�̃s�b�`���j�b�g�Ǝ��̃s�b�`���j�b�g���قȂ�ꍇ�ŁC���̂ǂ���ɂ��Y�����Ȃ��s�b�`���j�b�g�Ɋւ��ẮC�������Ȃ�
					
					//�厲�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ńp�f�B���O����
					int playTime = puCurr->GetPlayTime();		  //�J�����g�̉��t����
					double lastValue = 0;
					if(axis[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = axis[j][axis[j].size()-1]; //�O��I���l
					}
					for(int h=0;h<playTime;h++){				  //�J�����g�̉��t���ԕ��C�O��I���l�Ńp�f�B���O�D
						axis[j].append(lastValue);
					}

					//��񎲁i���I���j�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ŗ��߂�
					if(stringSelectSequence[j].size() == 0){
						//�������͒��ӂ��恦
						if(j == 0) lastValue = 2;     // 0�ԃs�b�`���j�b�g�̏����ʒu��A��
						else if(j == 1) lastValue = 1;// 1�ԃs�b�`���j�b�g�̏����ʒu��D��
					}else{
						lastValue = stringSelectSequence[j][stringSelectSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						stringSelectSequence[j].append(lastValue);
					}

					//��O���i�������j�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ŗ��߂�
					if(pushpullSequence[j].size() == 0){
						lastValue = 0;
					}else{
						lastValue = pushpullSequence[j][pushpullSequence[j].size()-1];
					}
					for(int h=0;h<playTime;h++){
						pushpullSequence[j].append(lastValue);
					}									
				}
			}
		}

		//���̎��_�ŁC�o�^�S�Ă̎��̐���l�̃T�C�Y������łȂ���΂Ȃ�Ȃ��D
		for(int k=0;k<NbPitchUnit-1;k++){
			if(axis[k].size() != axis[k+1].size()){
				//�G���[
				std::cout << "[ �s�b�`�}�l�[�W�� ] i �Ԗ� : " << i << "�Ԗ�" << std::endl;
				std::cout << "[ �s�b�`�}�l�[�W�� ] �s�b�`���j�b�g�Ԑ���l�̃T�C�Y���قȂ�܂�: k, k+1: " << k << ", " << k+1 << " | " << axis[k].size() << "," << axis[k+1].size() << std::endl;
				Error::Critical(0, QObject::tr("[ PitchManager ] control vector size error"));
			}
		}
	}


	if(end == puVector.size()){
		//puVector��t��t�g���Ƃ��́C�Ō�̂ЂƂɂ��ď������Ȃ��Ƃ����Ȃ�

		//�Ō�̂ЂƂɂ��ď���

		//�J�����g�̃s�b�`���j�b�g
		PitchUnit *puCurr = puVector[puVector.size()-1];

		//�Y���s�b�`���j�b�g�ɂ��Ă͒ʏ�ʂ�v���y�A
		puCurr->Prepare(axis[puCurr->GetNbUnit()][axis[puCurr->GetNbUnit()].size()-1]);

		//�厲
		QVector<double> temp = puCurr->GetPositionVector();
		for(int k=0;k<temp.size();k++){
			axis[puCurr->GetNbUnit()].append(temp[k]);
		}

		//���
		QVector<int> temp2 = puCurr->GetStringSelectSequence();
		for(int k=0;k<temp2.size();k++){
			stringSelectSequence[puCurr->GetNbUnit()].append(temp2[k]);
		}

		//��O��
		QVector<int> temp3 = puCurr->GetPushpullSequence();
		for(int k=0;k<temp3.size();k++){
			pushpullSequence[puCurr->GetNbUnit()].append(temp3[k]);
		}

		//����ȊO�̃s�b�`���j�b�g�ɂ��Ă͒P�Ƀp�f�B���O�i����͕ʂɖ����Ă��\��Ȃ��j
		for(int j=0;j<NbPitchUnit;j++){
			if(puCurr->GetNbUnit() != j){
				//�J�����g�̃s�b�`���j�b�g�ȊO�́G

				//�厲�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ńp�f�B���O����
				int playTime = puCurr->GetPlayTime();		  //�J�����g�̉��t����
				double lastValue = axis[j][axis[j].size()-1]; //�O��I���l
				for(int h=0;h<playTime;h++){				  //�J�����g�̉��t���ԕ��C�O��I���l�Ńp�f�B���O�D
					axis[j].append(lastValue);
				}

				//��񎲁i���I���j�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ŗ��߂�
				lastValue = stringSelectSequence[j][stringSelectSequence[j].size()-1];
				for(int h=0;h<playTime;h++){
					stringSelectSequence[j].append(lastValue);
				}

				//��O���i�������j�ɑ΂��ẮC�J�����g�̉��t���ԕ����C�O��I���l�Ŗ��߂�
				lastValue = pushpullSequence[j][pushpullSequence[j].size()-1];
				for(int h=0;h<playTime;h++){
					pushpullSequence[j].append(lastValue);
				}									
			}
		}

	}

	//���������e���̐���x�N�g�����Z�b�g�E�v���y�A
	Controller *control = Controller::GetInstance();
	
	for(int i=0;i<NbPitchUnit;i++){

		//��{����N���X�Ƃ̐ڑ��֌W�����邽�߂Ɉꎞ�\�z
		PitchUnit *pu = new PitchUnit(i);

		//�厲�̊�{����N���X���v���y�A
		control->axis[pu->GetNbAxis()]->SetMode(MLinear::CONTINUOUS);
		control->axis[pu->GetNbAxis()]->SetPositionVector(axis[i]);
		control->axis[pu->GetNbAxis()]->Prepare();

		//��񎲂̊�{����N���X���v���y�A
		control->iaic[pu->GetNbIaic()-1]->SetMode(IAICtrler::PREDEFINED);
		control->iaic[pu->GetNbIaic()-1]->SetControlVector(stringSelectSequence[i]);
		control->iaic[pu->GetNbIaic()-1]->Prepare();

		//��O���̊�{����N���X���v���y�A
		control->servo[pu->GetNbServo()]->SetMode(Servo::PREDEFINED);
		control->servo[pu->GetNbServo()]->SetControlVector(pushpullSequence[i]);
		control->servo[pu->GetNbServo()]->Prepare();

		delete pu;
	}

	return axis[0].size();
}


void PitchManager::Commit()
{
	std::cout << "[ �s�b�`�}�l�[�W�� ] �R�~�b�g�D" << std::endl;

	Controller *control = Controller::GetInstance();	
	for(int i=0;i<NbPitchUnit;i++){

		//��{����N���X�Ƃ̐ڑ��֌W�����邽�߂Ɉꎞ�\�z
		PitchUnit *pu = new PitchUnit(i);

		//�厲�̊�{����N���X���R�~�b�g
		control->axis[pu->GetNbAxis()]->Commit();

		//��񎲂̊�{����N���X���R�~�b�g
		control->iaic[pu->GetNbIaic()-1]->Commit();

		//��O���̊�{����N���X���R�~�b�g
		control->servo[pu->GetNbServo()]->Commit();

		delete pu;
	}

}