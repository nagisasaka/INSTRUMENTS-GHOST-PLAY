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

#include "igen.hpp"
#include "baseGeometry.hpp"
#include "../error.hpp"
#include "../actuator/controller.hpp"
#include "../calibration.hpp"

//��i�K�ڌ��̏��v���ԁi�@�B���E���l�����Đݒ肷�邱�Ɓj
//const int Igen::DurationTime = 100;//[ms]
const int Igen::DurationTime = 80;//[ms]

Igen::Igen()
{
	geometry = new BaseGeometry();
	calibration = new Calibration();
	calibration->Load();
	startString = 0;
	endString = 0;
}

Igen::~Igen()
{
	delete geometry;
	delete calibration;
}

/*!
 * \brief
 * �����ʒu�ƏI���ʒu��ݒ�
 * 
 * \param _string1
 * �����ʒu[0,6]
 * 
 * \param _string2
 * �I���ʒu[0,6]
 * 
 */
void Igen::SetStringPair(int _startString, int _endString)
{
	startString = _startString;
	endString = _endString;
}

void Igen::SetPlayerString(int _playerString)
{
	playerString = _playerString;
}


/*!
 * \brief
 * �E�ֈڌ��i�V�����j
 * 
 * \param nb_string
 * �ڌ��O�|�W�V����
 * 
 * 
 */
void Igen::ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int _startString)
{

	//// axis�ɒǉ����Ă����̂Ō��݂�axis�̍ŏI�l�������̏����l�i�L�����u���[�V�����͎g��Ȃ��j

	//������Ԃ̎擾
	double initial_pos0 = axis0[axis0.size()-1];
	double initial_pos1 = axis1[axis1.size()-1];

	//1�X�e�b�v�̎��Ԃ��擾
	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//�ڌ��p�ɒǉ�����x�N�^�T�C�Y
	int size = (double)DurationTime / (double)cycle;

	//����ʒu
	double pos0 = 0;
	double pos1 = 0;

	//// �ڌ��X�e�b�v�����Ɉڌ�����

	//���[�v���Ō��肷��
	int current_string = 0;
	int left_string = 0;
	int right_string = 0;

	//�N���A�����X�̓��[�v���ŌŒ肪�ǂ��Ǝv����
	double deltaL  = -5.0;
	double deltaL0 = deltaL;	//�E���ɑ΂���N���A�����X�i�����Ӂ���P�ʉE�̗����ʒu���g���̂ŁC���ڌ������΍����ɂȂ邱�Ƃɒ��Ӂj
	double deltaL1 = deltaL;	//���ڌ��ɑ΂���N���A�����X

	nbIgenStep = qAbs(nbIgenStep);

	for(int i=0;i<nbIgenStep;i++){

		//(1)�ŏ��ɁC�����ƉE���ɑ΂��Ĉ��̃N���A�����X��ۂ܂ŉ^������D
		//(2)���ɁC�E���ƁC����ɉE���ɑ΂��Ĉ��̃N���A�����X��ۂ܂ŉ^������D

		current_string = _startString + i;  //���ڌ�
		left_string = current_string - 1;   //�����icurrent_string==0�̎��̂ݍ����͑��݂��Ȃ������Ȃ��j
		right_string = current_string + 1;  //�E��

		if(current_string % 2 == 0){
			//���ݒ��ڂ��Ă���ʒu���P���ʒu�̏ꍇ�G

			//���[�v�̏����ʒu
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){
				//�E���̎��̒P���ʒu
				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, right_string+1)*(-1.);
				//���ڌ�
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, current_string)*(-1.);
			}else{
				//�E���̎��̒P���ʒu
				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, right_string+1)*(-1.);
				//���ڌ�
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, current_string)*(-1.);
			}
			
			double deltaL0_step = (deltaL0-curr_deltaL0) / (double)size;	//�E���X�e�b�v
			double deltaL1_step = (deltaL1-curr_deltaL1) / (double)size;	//���ڌ��X�e�b�v

			//�E���ʒu�ŗ��~�ڐG�����ɂēW�J
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step; //�E���X�e�b�v
				curr_deltaL1 += deltaL1_step; //���ڌ�
				geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, right_string); //�E���ʒu�œW�J
				axis0.append(pos0);
				axis1.append(pos1);
			}

			curr_deltaL0 = 0;
			curr_deltaL1 = 0;

		}else{
			//���ݒ��ڂ��Ă���ʒu�������ʒu�̏ꍇ�G

			//���[�v�̏����ʒu
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){
				//�E���P���ʒu
				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, right_string)*(-1.);
				//�����P���ʒu
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, left_string)*(-1.);
			}else{
				//�E���P���ʒu
				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, right_string)*(-1.);
				//�����P���ʒu
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, left_string)*(-1.);
			}

			//�E���ɕ��s������deltaL0��^���钼��pos0_ref,pos1_ref�́C�����Ƃ̎c�]���a�́H
			double pos0_ref = 0;
			double pos1_ref = 0;

			//�E���̕��s������deltaL0��^���钼��pos0_ref,pos1_ref�����߂�
			geometry->deltaL(deltaL0, pos0_ref, pos1_ref, right_string);

			//�����Ƃ̎c�]���a�����߂�
			double residue = geometry->residualRadius2(pos0_ref, pos1_ref, left_string)*(-1.);

			//�X�e�b�v�����߂�
			double deltaL0_step = (deltaL0 - curr_deltaL0) / (double)size; //�E���X�e�b�v
			double deltaL1_step = (residue - curr_deltaL1) / (double)size; //�����X�e�b�v

			//���ڌ��ʒu�ŗ��~�ڐG�����ɂēW�J
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step;
				curr_deltaL1 += deltaL1_step;
				geometry->commonTangent(curr_deltaL0,curr_deltaL1,pos0,pos1,current_string);//���ڌ��ʒu�œW�J
				axis0.append(pos0);
				axis1.append(pos1);
			}
			
			curr_deltaL0 = 0;
			curr_deltaL1 = 0;
		}

	}

	//���̃��[�v�𔲂������_�ŁC���s�����������͗��~�ڐG�����ŃN���A�����XdeltaL1=deltaL0��ۂ�����ԂƂȂ��Ă���̂�
	//�L�����u���[�V�����ʒu�܂ŕ��s�ړ�����D

	current_string++;

	//�Ō�̕��s�ړ�
	if(current_string % 2 == 0){

		//�ŏI��Ԃ��P���ʒu�̏ꍇ�́G
		double curr_deltaL = geometry->residualRadius2(pos0,pos1,current_string)*(-1.);
		double deltaL_step = curr_deltaL / (double)size;

		//���ڌ��ʒu�ŕ��s�ړ�������
		for(int i=0;i<size;i++){
			curr_deltaL -= deltaL_step; // ���X�Ƀ[����
			geometry->deltaL(curr_deltaL, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}else{
		right_string = current_string + 1;
		left_string = current_string - 1;

		//�ŏI��Ԃ������ʒu�̏ꍇ�́G
		double curr_deltaL0 = geometry->residualRadius2(pos0,pos1,right_string)*(-1.);
		double curr_deltaL1 = geometry->residualRadius2(pos0,pos1,left_string)*(-1.);

		double deltaL0_step = curr_deltaL0 / (double)size;
		double deltaL1_step = curr_deltaL1 / (double)size;

		//���ڌ��ʒu�ŗ��~�ڐG�����ňړ�������
		for(int i=0;i<size;i++){
			curr_deltaL0 -= deltaL0_step;
			curr_deltaL1 -= deltaL1_step;
			geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}

	//����ōŏI������ԂƂȂ����D
}


/*!
 * \brief
 * ���ֈڌ��i�V�����j
 * 
 * \param nb_string
 * �ڌ��O�|�W�V����
 * 
 * 
 */
void Igen::ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nbIgenStep, int _startString)
{

	//// axis�ɒǉ����Ă����̂Ō��݂�axis�̍ŏI�l�������̏����l�i�L�����u���[�V�����͎g��Ȃ��j

	//������Ԃ̎擾
	double initial_pos0 = axis0[axis0.size()-1];
	double initial_pos1 = axis1[axis1.size()-1];

	//1�X�e�b�v�̎��Ԃ��擾
	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//�ڌ��p�ɒǉ�����x�N�^�T�C�Y
	int size = (double)DurationTime / (double)cycle;

	//����ʒu
	double pos0 = 0;
	double pos1 = 0;

	//// �ڌ��X�e�b�v�����Ɉڌ�����

	//���[�v���Ō��肷��
	int current_string = 0;
	int left_string = 0;
	int right_string = 0;

	//�N���A�����X�̓��[�v���ŌŒ肪�ǂ��Ǝv����
	double deltaL  = -5.0;
	double deltaL0 = deltaL;	//���ڌ��ɑ΂���N���A�����X
	double deltaL1 = deltaL;	//�����ɑ΂���N���A�����X

	for(int i=0;i<nbIgenStep;i++){

		//(1)�ŏ��ɁC�����ƍ����ɑ΂��Ĉ��̃N���A�����X��ۂ܂ŉ^������D
		//(2)���ɁC�����Ƃ���ɍ����ɑ΂��Ĉ��̃N���A�����X��ۂ܂ŉ^������D

		current_string = _startString - i;  //���ڌ�
		left_string = current_string - 1;   //����
		right_string = current_string + 1;  //�E��

		if(current_string % 2 == 0){
			//���ݒ��ڂ��Ă���ʒu���P���ʒu�̏ꍇ�G

			//���[�v�̏����ʒu
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){

				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, current_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, left_string-1)*(-1.);  //���̒l�ŕԂ遨����

			}else{
				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, current_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, left_string-1)*(-1.); //���̒l�ŕԂ遨����

			}

			double deltaL0_step = (deltaL0-curr_deltaL0) / (double)size;	//���ڌ��X�e�b�v
			double deltaL1_step = (deltaL1-curr_deltaL1) / (double)size;	//�����X�e�b�v

			//�����ʒu�ŗ��~�ڐG�����ɂēW�J
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step; //���ڌ�
				curr_deltaL1 += deltaL1_step; //����
				geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, left_string); //�����ʒu�œW�J
				axis0.append(pos0);
				axis1.append(pos1);
			}

			curr_deltaL0 = 0;
			curr_deltaL1 = 0;

		}else{
			//���ݒ��ڂ��Ă���ʒu�������ʒu�̏ꍇ�G

			//���[�v�̏����ʒu
			double curr_deltaL0 = 0;
			double curr_deltaL1 = 0;
			if(i == 0){

				curr_deltaL0 = geometry->residualRadius2(initial_pos0, initial_pos1, right_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(initial_pos0, initial_pos1, left_string)*(-1.);

			}else{

				curr_deltaL0 = geometry->residualRadius2(pos0, pos1, right_string)*(-1.);
				curr_deltaL1 = geometry->residualRadius2(pos0, pos1, left_string)*(-1.);

			}

			//�����ɕ��s������deltaL1��^���钼��pos0_ref,pos1_ref�́C�E���Ƃ̎c�]���a�́H
			double pos0_ref = 0;
			double pos1_ref = 0;

			//�����̕��s������deltaL1��^���钼��pos0,pos1�����߂�
			geometry->deltaL(deltaL1, pos0_ref, pos1_ref, left_string);

			//�E���Ƃ̎c�]���a�����߂�
			double residue = geometry->residualRadius2(pos0_ref, pos1_ref, right_string)*(-1.);

			//�X�e�b�v�����߂�
			double deltaL0_step = (residue - curr_deltaL0) / (double)size; //�E���X�e�b�v
			double deltaL1_step = (deltaL1 - curr_deltaL1) / (double)size; //�����X�e�b�v

			//���ڌ��ʒu�ŗ��~�ڐG�����ɂēW�J
			for(int j=0;j<size;j++){
				curr_deltaL0 += deltaL0_step;
				curr_deltaL1 += deltaL1_step;
				geometry->commonTangent(curr_deltaL0,curr_deltaL1,pos0,pos1,current_string);//���ڌ��ʒu�œW�J
				axis0.append(pos0);
				axis1.append(pos1);
			}
			
			curr_deltaL0 = 0;
			curr_deltaL1 = 0;
		}

	}

	//���̃��[�v�𔲂������_�ŁC���s�����������͗��~�ڐG�����ŃN���A�����XdeltaL1=deltaL0��ۂ�����ԂƂȂ��Ă���̂�
	//�L�����u���[�V�����ʒu�܂ŕ��s�ړ�����D

	current_string--;

	//�Ō�̕��s�ړ�
	if(current_string % 2 == 0){

		//�ŏI��Ԃ��P���ʒu�̏ꍇ�́G
		double curr_deltaL = geometry->residualRadius2(pos0,pos1,current_string)*(-1.);
		double deltaL_step = curr_deltaL / (double)size;

		//���ڌ��ʒu�ŕ��s�ړ�������
		for(int i=0;i<size;i++){
			curr_deltaL -= deltaL_step; // ���X�Ƀ[����
			geometry->deltaL(curr_deltaL, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}else{
		right_string = current_string + 1;
		left_string = current_string - 1;

		//�ŏI��Ԃ������ʒu�̏ꍇ�́G
		double curr_deltaL0 = geometry->residualRadius2(pos0,pos1,right_string)*(-1.);
		double curr_deltaL1 = geometry->residualRadius2(pos0,pos1,left_string)*(-1.);

		double deltaL0_step = curr_deltaL0 / (double)size;
		double deltaL1_step = curr_deltaL1 / (double)size;

		//���ڌ��ʒu�ŗ��~�ڐG�����ňړ�������
		for(int i=0;i<size;i++){
			curr_deltaL0 -= deltaL0_step;
			curr_deltaL1 -= deltaL1_step;
			geometry->commonTangent(curr_deltaL0, curr_deltaL1, pos0, pos1, current_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

	}

	//����ōŏI������ԂƂȂ����D
}



/*!
 * \brief
 * �E�ֈڌ�
 * 
 * \param nb_string
 * �ڌ��O�|�W�V����
 * 
 * 
 */
void Igen::ToRight(QVector<double> &axis0, QVector<double> &axis1,int _startString)
{
	//�L�����u���[�V�����ʒu���g���i������Ԃ��v�Z����K�v�͂Ȃ��j
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions ��[0,100]���K������Ă��邽��mm�P�ʂɂ͂Ȃ��Ă��Ȃ����Ƃɒ���

	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//�ڌ��p�ɒǉ�����x�N�^�T�C�Y
	int size = (double)DurationTime / (double)cycle;

	//����ʒu
	double pos0 = 0;
	double pos1 = 0;
	
	//�c�]���a�v�Z�Ώی�
	int nb_string = 0;

	//�V���O���E��
	if(_startString == 1){//1-2���C�V���O���E��2-3��
		//�c�]���a�v�Z�Ώی��͑�3��
		nb_string = 2;
	}else if(_startString == 3){//2-3���C�V���O���E��3-4��
		//�c�]���a�v�Z�Ώی��͑�4��
		nb_string = 3;
	}else{
		//�E�͂Ȃ�
		Error::Critical(0, QObject::tr("ToRight()�֐��̑��������s���ł�"));
	}

	double radius = geometry->residualRadius(pos[_startString].start[0], pos[_startString].start[1], nb_string);//�ڌ���͑�3��
	//�ڌ��ɕK�v�ȃ�L�����߂�
	double step_deltaL = radius/(double)size;
	//�ڌ����s�����ԕ��̐���x�N�g����ǉ�����
	for(int i=1;i<=size;i++){				
		double deltaL0 = (-radius)+step_deltaL*i; //���ΉE��
		double deltaL1 = 0;//�����͈��S�}�[�W�������Ȃ����Ă��ǂ� // ���΍���
		//if(i == 50){
		//	cout << "debug";
		//}
		if(i == size){
			std::cout << "deltaL0=" << deltaL0 << "deltaL1=" << deltaL1 << " deltaL0 ��������0�ɗ��Ƃ��܂�." << std::endl;
			deltaL0 = 0;
		}
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, _startString + 2);
		if(i == size){
			std::cout << "pos0=" << pos0 << "pos1=" << pos1 << std::endl;
		}
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * ���ֈڌ��i�������j
 * 
 * \param nb_string
 * �ڌ��O�|�W�V����
 * 
 * 
 */
void Igen::ToLeft(QVector<double> &axis0, QVector<double> &axis1,int _startString)
{
	//�L�����u���[�V�����ʒu���g���i������Ԃ��v�Z����K�v�͂Ȃ��j
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions ��[0,100]���K������Ă��邽��mm�P�ʂɂ͂Ȃ��Ă��Ȃ����Ƃɒ���

	Controller* control = Controller::GetInstance();
	int cycle = control->GetCycle();

	//�ڌ��p�ɒǉ�����x�N�^�T�C�Y
	int size = (double)DurationTime / (double)cycle;

	//����ʒu
	double pos0 = 0;
	double pos1 = 0;
	
	//�c�]���a�v�Z�Ώی�
	int nb_string = 0;

	if(_startString == 3){//2-3���C�V���O������1-2��
		//�c�]���a�v�Z�Ώی��͑�1��
		nb_string = 0;
	}else if(_startString == 5){//3-4���C�V���O������2-3��
		//�c�]���a�v�Z�Ώی��͑�2��
		nb_string = 1;
	}else{
		//���͂Ȃ�
		Error::Critical(0, QObject::tr("ToLeft()�֐��̑��������s���ł�"));
	}

	double radius = geometry->residualRadius(pos[_startString].start[0], pos[_startString].start[1], nb_string);
	//�ڌ��ɕK�v�ȃ�L�����߂�
	double step_deltaL = radius/(double)size;
	//�ڌ����s�����ԕ��̐���x�N�g����ǉ�����
	for(int i=1;i<=size;i++){				
		double deltaL0 = 0;	//�����͈��S�}�[�W�������Ȃ����Ă��ǂ� //���ΉE��
		double deltaL1 = (-radius)+step_deltaL*i;					   //���΍���
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, _startString - 2);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * �v���y�A�̎��s�D�^����ꂽ������x�N�^�[�Ɉڌ��p�̃V�[�P���X���A�y���h����D
 * 
 * \param axis0
 * ��0�̐���x�N�^�i��Έʒu���w��[0,100]�j
 * 
 * \param axis1
 * ��1�̐���x�N�^�i��Έʒu���w��[0,100]�j
 * 
 */
void Igen::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString == endString) return;

	//�ڌ��X�e�b�v��
	int nbIgenStep = startString - endString;

	if(startString % 2 == 0){
		//// �P���ʒu

		if(!(startString - endString == 1 || startString - endString == -1)){

			//// ���t���ڌ�����Ă��Ȃ��ꍇ�ɂ��āC������ŕK�v���ڌ�

			if(nbIgenStep < 0){
				//�E���Ɉڌ�
				ToRightSide(axis0,axis1,nbIgenStep,startString);
			}else{
				//�����Ɉڌ�
				ToLeftSide(axis0,axis1,nbIgenStep,startString);
			}
		}

	}else{

		//// �����ʒu

		// ���t����i�K�ڌ����\�����C���͎b�薢�����D

		if(startString == playerString){

			if(nbIgenStep < 0){
				ToRightSide(axis0,axis1,nbIgenStep,startString);
			}else{
				ToLeftSide(axis0,axis1,nbIgenStep,startString);
			}

		}else if(startString - playerString == 1){

			//���΍��������t���C���΍����։��t���ڌ��ςł���

			if(startString - endString == 1){
				//�ڌ��͊������Ă���
			}else{
				//�ڌ�������
				nbIgenStep = playerString - endString; // �㏑��
				if(nbIgenStep < 0){
					//�ڌ���ʒu���J�n���ʒu�ɂȂ�
					ToRightSide(axis0,axis1,nbIgenStep,playerString);
				}else{
					ToLeftSide(axis0,axis1,nbIgenStep,playerString);
				}
			}
			
		}else if(startString - playerString == -1){

			//���ΉE�������t���C���ΉE���ɉ��t���ڌ��ςł���

			if(startString - endString == -1){
				//�ڌ��͊������Ă���
			}else{
				//�ڌ�������
				nbIgenStep = playerString - endString;
				if(nbIgenStep < 0){
					ToRightSide(axis0,axis1,nbIgenStep,playerString);
				}else{
					ToLeftSide(axis0,axis1,nbIgenStep,playerString);
				}
			}
		}
	}
}


/*!
 * \brief
 * �v���y�A�̎��s�D�^����ꂽ������x�N�^�[�Ɉڌ��p�̃V�[�P���X���A�y���h����D
 * 
 * \param axis0
 * ��0�̐���x�N�^�i��Έʒu���w��[0,100]�j
 * 
 * \param axis1
 * ��1�̐���x�N�^�i��Έʒu���w��[0,100]�j
 * 
 */
/*
void Igen::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString == 1){//1-2(1)������
		if(endString == 1){//1-2(1)����
			//�ڌ�����
		}else if(endString == 3){//2-3(3)����
			if(playerString == 0){
				//1(0)��
				ToRight(axis0,axis1,1);
			}else if(playerString == 1){
				//1-2(1)��
				ToRight(axis0,axis1,1);
			}else if(playerString == 2){
				//2(2)��
				//���t���Ȃ���1-2(1)��2-3(3)�ֈڌ���
			}
		}else if(endString == 5){//3-4(5)����
			if(playerString == 0){
				//1��
				ToRight(axis0,axis1,1);
			}else if(playerString == 1){
				//1-2��
				ToRight(axis0,axis1,1);
			}else if(playerString == 2){
				//2��
				//���t���Ȃ���1-2(1)��2-3(3)�ֈڌ���
			}
			//2-3(3)��3-4(5)�ֈڌ�
			ToRight(axis0,axis1,3);
		}
	}else if(startString == 3){//2-3(3)����
		if(endString == 1){//1-2(1)����
			if(playerString == 2){
				//2(2)��
				//���t���Ȃ���2-3(3)��1-2(1)�ֈڌ���
			}else if(playerString == 3){
				//2-3(3)��
				ToLeft(axis0,axis1,3);
			}else if(playerString == 4){
				//3(4)��
				ToLeft(axis0,axis1,3);
			}
		}else if(endString == 3){//2-3(3)����
			//�ڌ�����
		}else if(endString == 5){//3-4(5)����
			if(playerString == 2){
				//2(2)��
				ToRight(axis0,axis1,3);
			}else if(playerString == 3){
				//2-3(3)��
				ToRight(axis0,axis1,3);
			}else if(playerString == 4){
				//3(4)��
				//���t���Ȃ���2-3(3)��3-4(5)���ֈڌ���
			}
		}
	}else if(startString == 5){//3-4(5)������
		if(endString == 1){//1-2����
			if(playerString == 4){
				//3��
				//���t���Ȃ���3-4(5)��2-3(3)�ֈڌ���
			}else if(playerString == 5){
				//3-4��
				ToLeft(axis0,axis1,5);
			}else if(playerString == 6){
				//4��
				ToLeft(axis0,axis1,5);
			}
			ToLeft(axis0,axis1,3);
		}else if(endString == 3){//2-3����
			if(playerString == 4){
				//3(4)��
				//���t���Ȃ���3-4(5)��2-3(3)�ֈڌ���
			}else if(playerString == 5){
				//3-4(5)��
				ToLeft(axis0,axis1,5);
			}else if(playerString == 6){
				//4(6)��
				ToLeft(axis0,axis1,5);
			}
		}else if(endString == 5){//3-4(5)����
			//�ڌ�����
		}
	}
}
*/