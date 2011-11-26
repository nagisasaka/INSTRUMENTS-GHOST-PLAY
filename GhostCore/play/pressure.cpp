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

#include "pressure.hpp"
#include "../actuator/controller.hpp"
#include "../calibration.hpp"
#include "../vecmath.hpp"
#include "baseGeometry.hpp"
#include "../error.hpp"

const double Pressure::MaxMM = 5.0;//�ő剟�����ݕ�[mm]

Pressure::Pressure()
{
	geometry = new BaseGeometry();
	calibration = new Calibration();
	calibration->Load();
}

Pressure::~Pressure()
{
	delete calibration;
	delete geometry;
}


/*!
 * \brief
 * [0,100]���K���\�������ۂ̉������ݕ��ɕϊ����ĕԂ��D��ʑw�ł͐��K���\���𗘗p���Ă���̂ŁC���l�́C�x�[�X�W�I���g���ł̌v�Z�݂̂Ɏg�p�D	
 * 
 * \param percent
 * [0,100]���K���\���D��ʑw�ŗ��p���Ă���D
 * 
 * \returns
 * ���ۂ̉������ݕ�[mm]
 * 
 */
double Pressure::ConvertToMM(double percent)
{
	return (percent/100.)*MaxMM;
}


/*!
 * \brief
 * ���͕t���Ώی����w�肷��
 * 
 * \param string
 * ���ԍ�[1,7]
 * 
 * 
 */
void Pressure::SetPlayerString(int _playerString)
{	
	//���͕t���Ώی�
	playerString = _playerString;
}


/*!
 * \brief
 * ���̓x�N�^���w�肷��
 * 
 * \param v
 * ���̓x�N�^�n��
 * 
 */
void Pressure::SetPressureVector(QVector<double> p)
{
	pressureVector = p;
}


/*!
 * \brief
 * �����ʒu�E�I���ʒu���w�肷��
 * 
 * \param _startString
 * �����ʒu[1,3,5]
 * 
 * \param _endString
 * �I���ʒu[1,3,5]
 * 
 */
void Pressure::SetStringPair(int _startString, int _endString)
{
	startString = _startString;
	endString = _endString;
}


/*!
 * \brief
 * �����������t���Ȃ��獶���Ɉ�i�K�ڌ�����i�V�ėp�����G�قڈ���ȉ��t���ڌ��j
 * OK 
 *
 * \param nb_string
 * �S�ʒu[0,1,2,3,4,5,6]
 *
 * \remarks
 * �c�]���a�̃X�e�b�v���́C�����Ώی��ւ̎c�]���a�����X�s�[�h���x�����C�|���ω����x�������ꍇ�ɁC�x�[�X���t�ɓ�������p������D����͖��炩�ɔ�����D
 * �]���āC�Ō�̕ϋȓ_�܂ł͕��s�����i�������͓�~�ڐG�����j�ɂ�蕽�s�t�����āC�Ō�̕ϋȓ_�ȍ~�Ŏc�]���a���ꎟ�Ō���������D
 * 
 */
void Pressure::ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	if(nb_string == 0){
		//�����������݂��Ȃ�
		Error::Critical(0, QObject::tr("[ PRESSURE ] Invalid nb_string: %1").arg(nb_string));
	}

	if(nb_string % 2 == 0){

		//// �P���ʒu ////	

		//�������͗����ʒu�C���t���ƈڌ��悪�قȂ�̂Ŗ�肪����D�Ō�̕ϋȓ_�܂ł͕��s�����ŕ��s�t�����āC�Ō�̕ϋȓ_�ȍ~�Ŏc�]���a����C�ɗ��Ƃ�

		//�ڌ���ʒu
		int left_string = nb_string - 1;

		//�|�̒e���ό`�̕␳�W���i�ő剟���ݕ��̒����ɂ���ĕω�����j
		//double reformCoef = 0.5;

		//�ڌ���ʒu�ł͒����i�|���[���j�C���t���͎c�]�|�������݂���ꍇ�����邪�C���e�ł��Ȃ��d�l�i�a���̕Ќ������������e�����Ƃ͂ł��Ȃ��j
		//�܂�C�P���ʒu���痼���ʒu�։��t���ڌ�����ꍇ�́C�c�]�|�����[���ł��邱�Ƃ��K�{�D
		if(pressureVector[pressureVector.size()-1] != 0){
			Error::Critical(0, QObject::tr("[ PRESSURE ] �P���ʒu���痼���ʒu�ւ̉��t���ڌ��ł́C�c�]�|���̓[���łȂ���΂Ȃ�܂���D"));
		}
		
		//�Ō�̕ϋȓ_��T��
		int lastLocalMaxIndex = 0;
		double maxValue = 0;
		for(int i=0;i<pressureVector.size()-1;i++){
			if(pressureVector[i] < pressureVector[i+1]){
				lastLocalMaxIndex = i+1;
				maxValue = pressureVector[i+1];
			}
			if(maxValue == pressureVector[i]){
				lastLocalMaxIndex = i;
			}
		}

		//���s�ړ������ŒP���P���^��
		for(int i=0;i<lastLocalMaxIndex;i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

		//���s�ړ������ł̒P���P���^���I�����_�ł̎c�]���a�����߂�
		double lastResidue = geometry->residualRadius2(pos0,pos1,left_string-1);
		double residueByStep = lastResidue / (double)(pressureVector.size()-(lastLocalMaxIndex+1));

		//��~�ڐG�����ŗ^��
		int lineCounter = 0;
		for(int i=lastLocalMaxIndex; i<pressureVector.size();i++){

			//���ʐڐ��������C���ΓI�ɍ��������t����
			//�E���G�I���W�i��������
			double deltaL0 = ConvertToMM(pressureVector[i]);
			//�����G
			double deltaL1 = residueByStep*lineCounter - lastResidue;	
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, left_string);
			axis0.append(pos0);
			axis1.append(pos1);
			lineCounter++;

		}

	}else{

		//// �����ʒu ////

		//�������͒P���ʒu�C���t���ƈڌ��悪�����Ȃ̂Ŗ��͔�r�I�ȒP�D

		//�����ʒu����deltaL1�^�����C���s������deltaL1��^����pos0��pos1�ɑ΂��Ďc�]���a�����߁CdeltaL0��0.5mm�����s�����ɋ߂Â��悤�ɒǂ����ރX�e�b�v��
		//���s�����Ɉ�v����܂ŌJ��Ԃ��C���s�����Ɉ�v������́C�c��̋|���x�N�g���𕽍s�����^���ŏ�������D
		
		//�����ʒu���\�����鍶����
		int left_string = nb_string - 1;
		//�����ʒu���\������E����
		int right_string = nb_string + 1;
		
		//1�X�e�b�v�Œǂ�����ł悢�ő啝�i�����܂��Ȓl�j
		double step = 5.0 / (double)pressureVector.size();

		int parallelTransIndex = 0;
		//pressureVector���A�����Ă���ΑO�̎c�]�|���ƍ���̏����|���͓��R�Ɉ�v����D�����œ��i�̔z��������K�v�͂Ȃ��D
		for(int i=0;i<pressureVector.size();i++){
			//���ʐڐ��������C���ΓI�ɍ��������t����

			//�����G�I���W�i�������� deltaL1
			double deltaL1 = ConvertToMM(pressureVector[i]);

			//���s�����^����deltaL1��^����pos0_ref��pos1_ref�́H
			double pos0_ref = 0;
			double pos1_ref = 0;
			geometry->deltaL(deltaL1, pos0_ref, pos1_ref, left_string);


			//���s�����ł̉E���ւ̋�����
			double residue = geometry->residualRadius2(pos0_ref,pos1_ref,right_string)*(-1.);

			//�E���ւ̉����݋��������肷��
			double deltaL0 = 0;

			if(i != 0){

				//���݂̒���pos0,pos1�ƉE���Ƃ̋����́H
				double curr_residue = geometry->residualRadius2(pos0,pos1,right_string)*(-1.); //���ŕԂ遨����

				//���s�����Ƃ̋����̍��́H
				double curr_diff = residue - curr_residue;

				//��������i�����͏��������j
				if(qAbs(curr_diff) < 0.2){ // 0.5mm / 10msec
					//�\���������Ă���̂ň�C�ɕ��s�����֗��Ƃ�

					deltaL0 = residue;		//�ǂ����݊���
					parallelTransIndex = i; //���s�����֗^������

				}else{

					//�܂��ǂ����߂�				
					if(curr_diff < 0){
						//�����ւ̋���������
						deltaL0 = (-1.)*step*i;
					}else{
						//�����̋������O���i�����֒ǂ�����ł����j
						deltaL0 = (+1.)*step*i;
					}
				}

			}

			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
			if(parallelTransIndex != 0){
				break;
			}

		}

		if(parallelTransIndex == 0){
			Error::Critical(0,QObject::tr("[ PRESSURE ] �����ʒu����̉E�������t���ڌ����䂪�������܂���ł���."));
		}
		for(int i=parallelTransIndex+1;i<pressureVector.size();i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, left_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}			
	}
}


/*!
 * \brief
 * �E���������t���Ȃ���E���Ɉ�i�K�ڌ�����i�V�ėp�����G�قڈ���ȉ��t���ڌ��j
 * OK 
 *
 * \param nb_string
 * �S�ʒu[0,1,2,3,4,5,6]
 *
 * \remarks
 * �c�]���a�̃X�e�b�v���́C�����Ώی��ւ̎c�]���a�����X�s�[�h���x�����C�|���ω����x�������ꍇ�ɁC�x�[�X���t�ɓ�������p������D����͖��炩�ɔ�����D
 * �]���āC�Ō�̕ϋȓ_�܂ł͕��s�����i�������͓�~�ڐG�����j�ɂ�蕽�s�t�����āC�Ō�̕ϋȓ_�ȍ~�Ŏc�]���a���ꎟ�Ō���������D
 * 
 */
void Pressure::ToRightSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{

	double pos0 = 0;
	double pos1 = 0;

	if(nb_string == 6){
		//�E���������݂��Ȃ�
		Error::Critical(0, QObject::tr("[ PRESSURE ] Invalid nb_string: %1 ( Right side string does not exists! )").arg(nb_string));
	}

	if(nb_string % 2 == 0){

		//// �P���ʒu ////	

		//�E�����͗����ʒu�C�Ō�̕ϋȓ_�܂ł͕��s�^�����C�Ō�̕ϋȓ_�ȍ~�Ŏc�]���a����C�Ɍ���������

		//�ڌ���ʒu
		int right_string = nb_string + 1;

		//�|�̒e���ό`�̕␳�W���i�ő剟���ݕ��̒����ɂ���ĕω�����j
		//double reformCoef = 0.5;

		//�ڌ���ʒu�ł͒����i�|���[���j�C���t���͎c�]�|�������݂���ꍇ�����邪�C���e�ł��Ȃ��d�l�i�a���̕Ќ������������e�����Ƃ͂ł��Ȃ��j
		//�܂�C�P���ʒu���痼���ʒu�։��t���ڌ�����ꍇ�́C�c�]�|�����[���ł��邱�Ƃ��K�{�D
		if(pressureVector[pressureVector.size()-1] != 0){
			Error::Critical(0, QObject::tr("[ PRESSURE ] �P���ʒu���痼���ʒu�ւ̉��t���ڌ��ł́C�c�]�|���̓[���łȂ���΂Ȃ�܂���i�E���ڌ��j"));
		}
		
		//�Ō�̕ϋȓ_��T��
		int lastLocalMaxIndex = 0;
		double maxValue = 0;
		for(int i=0;i<pressureVector.size()-1;i++){
			if(pressureVector[i] < pressureVector[i+1]){
				lastLocalMaxIndex = i+1;
				maxValue = pressureVector[i+1];
			}
			if(maxValue == pressureVector[i]){
				lastLocalMaxIndex = i;
			}
		}

		//���s�ړ������ŒP���P���^��
		for(int i=0;i<lastLocalMaxIndex;i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}

		//���s�ړ������ł̒P���P���^���I�����_�ł̎c�]���a�����߂�
		double lastResidue = geometry->residualRadius2(pos0,pos1,right_string+1); // ���̒l�ŕԂ� ... 
		double residueByStep = lastResidue / (double)(pressureVector.size()-(lastLocalMaxIndex+1));

		//��~�ڐG�����ŗ^��
		int lineCounter = 0;
		for(int i=lastLocalMaxIndex; i<pressureVector.size();i++){

			//���ʐڐ��������C���ΓI�ɍ��������t����

			//�E���G
			double deltaL0 = residueByStep*lineCounter - lastResidue; // �}�C�i�X�l���[���� OK	
			//�����G�I���W�i��������
			double deltaL1 = ConvertToMM(pressureVector[i]);

			//�E���ʒu�ŗ��~�ڐG�����œW�J
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, right_string);
			axis0.append(pos0);
			axis1.append(pos1);
			lineCounter++;

			if(i == pressureVector.size()-1){
				std::cout << std::endl;
			}

		}

	}else{

		//// �����ʒu ////

		//�E�����͒P���ʒu�C���t���ƈڌ��悪�����Ȃ̂Ŗ��͔�r�I�ȒP�D

		//�����ʒu����deltaL1�^�����C���s������deltaL1��^����pos0��pos1�ɑ΂��Ďc�]���a�����߁CdeltaL0��0.5mm�����s�����ɋ߂Â��悤�ɒǂ����ރX�e�b�v��
		//���s�����Ɉ�v����܂ŌJ��Ԃ��C���s�����Ɉ�v������́C�c��̋|���x�N�g���𕽍s�����^���ŏ�������D
		
		//�����ʒu���\�����鍶����
		int left_string = nb_string - 1;
		//�����ʒu���\������E����
		int right_string = nb_string + 1;
		
		//1�X�e�b�v�Œǂ�����ł悢�ő啝�i�����܂��Ȓl�j
		double step = 5.0 / (double)pressureVector.size();

		int parallelTransIndex = 0;
		//pressureVector���A�����Ă���ΑO�̎c�]�|���ƍ���̏����|���͓��R�Ɉ�v����D�����œ��i�̔z��������K�v�͂Ȃ��D
		for(int i=0;i<pressureVector.size();i++){
			//���ʐڐ��������C���ΓI�ɉE�������t����

			//�E���G�I���W�i�������� deltaL0
			double deltaL0 = ConvertToMM(pressureVector[i]);

			//���s������deltaL0��^����pos0��pos1�́H
			double pos0_ref = 0;
			double pos1_ref = 0;
			geometry->deltaL(deltaL0, pos0_ref, pos1_ref, right_string);

			//����pos0_ref-pos1_ref�ƍ����ւ̋����́i���ꂪ���݂�deltaL0�ŕ��s�����Ɏ��邽�߂̋����j
			double residue = geometry->residualRadius2(pos0_ref,pos1_ref,left_string)*(-1.); // ���ŕԂ遨����

			//�����ւ̉����݋��������肷��
			double deltaL1 = 0;

			if(i != 0){
				//���݂̒���pos0,pos1�ƍ����Ƃ̋����́H
				double curr_residue = geometry->residualRadius2(pos0,pos1,left_string)*(-1.); //���ŕԂ遨����

				//���s�����Ƃ̋����̍��́H
				double curr_diff = residue - curr_residue;

				//��������i�����͏��������j
				if(qAbs(curr_diff) < 0.2){ // 0.5mm / 10msec
					//�\���������Ă���̂ň�C�ɕ��s�����֗��Ƃ�

					deltaL1 = residue;		//�ǂ����݊���
					parallelTransIndex = i; //���s�����֗^������

				}else{

					//�܂��ǂ����߂�				
					if(curr_diff < 0){
						//�����ւ̋���������
						deltaL1 = (-1.)*step*i;
					}else{
						//�����̋������O���i�����֒ǂ�����ł����j
						deltaL1 = (+1.)*step*i;
					}

				}
			}

			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
			axis0.append(pos0);
			axis1.append(pos1);
			if(parallelTransIndex != 0){
				break;
			}
		}

		//���s�^��
		if(parallelTransIndex == 0){
			Error::Critical(0,QObject::tr("[ PRESSURE ] �����ʒu����̉E�������t���ڌ����䂪�������܂���ł���."));
		}
		for(int i=parallelTransIndex+1;i<pressureVector.size();i++){
			double deltaL = ConvertToMM(pressureVector[i]);
			geometry->deltaL(deltaL, pos0, pos1, right_string);
			axis0.append(pos0);
			axis1.append(pos1);
		}
				
	}

}


/*!
 * \brief
 * �P�����s�ړ������ɂ��|���t���֐��i�������͕t���Ǝ����㓯�������ɂȂ�j
 * 
 * \param axis0
 * ��0�̐���x�N�g��
 * 
 * \param axis1
 * ��1�̐���x�N�g��
 * 
 * \param nb_string
 * ���ʒu�ԍ�
 * 
 */
void Pressure::SinglePressure(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	for(int i=0;i<pressureVector.size();i++){
		double dL = ConvertToMM(pressureVector[i]);
		geometry->deltaL(dL,pos0,pos1,nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * ���������t����
 * 
 * \param nb_string
 * �����ʒu���w��[1,3,5]
 * 
 */
void Pressure::ToBothSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;
	for(int i=0;i<pressureVector.size();i++){
		//���ʐڐ��������C���������t����ideltaL0:�ω��CdeltaL1:�ω��i�����j�j
		double deltaL0 = ConvertToMM(pressureVector[i]); //�E��
		double deltaL1 = deltaL0;						 //����
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * �����ʒu���瑊�ΓI�Ɍ������č����������t����i�V�����G�|�̒e���ό`���l�����āC�������������ގ����j
 * OK
 *
 * \param nb_string
 * �����ʒu���w��[1,3,5]
 *
 * \remarks
 * �|�̒e���ό`�������Ɍv�Z���邽�߂ɂ́C��2�̈ʒu���K�v�ɂȂ�C��0,1�Ǝ�2�̓Ɨ����������邽�ߋߎ�����D
 * 
 */
void Pressure::ToLeftSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;
	//pressureVector���A�����Ă���ΑO�̎c�]�|���ƍ���̏����|���͓��R�Ɉ�v����D�����œ��i�̔z��������K�v�͂Ȃ��D
	for(int i=0;i<pressureVector.size();i++){
		//���ʐڐ��������C���ΓI�ɍ��������t����ideltaL0:���CdeltaL1:�ω��j
		//double deltaL0 = 0;									    //�E�� ; �I���W�i��
		double deltaL0 = ConvertToMM(pressureVector[i])*0.5;	//�E�� ; �e���ό`�l���ɂ��n�|���œ�������։������ށi�W���́u�ő剟�����v�̒����ɂ���ĕς��Ǝv����j
		double deltaL1 = ConvertToMM(pressureVector[i]);		//����
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}


/*!
 * \brief
 * �E���������t����i�V�����G�|�̒e���ό`���l�����āC�������������ގ����j
 * OK
 *
 * \param nb_string
 * �����ʒu���w��[1,3,5]
 *
 * \remarks
 * TODO
 *
 */
void Pressure::ToRightSide(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;
	for(int i=0;i<pressureVector.size();i++){
		//���ʐڐ��������C���ΓI�ɉE�������t����ideltaL0:�ω��CdeltaL1:���j
		double deltaL0 = ConvertToMM(pressureVector[i]);	 //�E��
		double deltaL1 = ConvertToMM(pressureVector[i])*0.5; //���� ... �|�̒e���ό`�l��0.5�|���ł��������������
		//double deltaL1 = 0;									 //���� ... �I���W�i��
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string);
		axis0.append(pos0);
		axis1.append(pos1);
	}
}



/*!
 * \brief
 * ���̓x�N�^���v���y�A����i�J�n���ʒu�ƏI�����ʒu�ɐ����̂Ȃ��V�����j
 * 0	1��
 * 1	1-2��
 * 2	2��
 * 3	2-3��
 * 4	3��
 * 5	3-4��
 * 6	4��
 * 
 * \param axis0
 * ��0�̐���x�N�^�iappend����܂��j
 * 
 * \param axis1
 * ��1�̐���x�N�^�iappend����܂��j
 * 
 */
void Pressure::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString % 2 == 0){
		//�J�n�����P���ʒu
		if(startString != playerString){
			//�J�n���Ɖ��t���͓��ꌷ�łȂ���΂Ȃ�Ȃ�
			Error::Critical(0, QObject::tr("�J�n���Ɖ��t���͓��ꌷ�łȂ���΂Ȃ�܂���: %1 != %2").arg(startString).arg(playerString));
		}

		if(startString - endString == 1){
			//�������։��t���ڌ�
			ToLeftSideWithIgen(axis0, axis1, startString);
		}else if(startString - endString == -1){
			//�E�����։��t���ڌ�
			ToRightSideWithIgen(axis0, axis1, startString);
		}else{
			//���t���ڌ��������s�^���i���u���̏ꍇ�C���t���ڌ�������C���̕����ֈ�i�K���t���ڌ����Ă��ǂ����C�ڌ��N���X�ŉ��t��Ɉڌ�����݌v�Ƃ���j
			SinglePressure(axis0, axis1, startString);
		}


	}else{
		//�J�n���������ʒu
		if(!(startString == playerString || startString - playerString == 1 || startString - playerString == -1)){
			//�J�n���Ɖ��t���̊֌W�͂��̒ʂ�
			Error::Critical(0, QObject::tr("�����ʒu�ł́C���t���͓���ʒu�������ʒu�Ɋ܂܂�鍶�E�̒P���ł���K�v������܂�: %1, %2").arg(startString).arg(playerString));
		}

		if(startString == playerString){

			//�I�����Ɋւ�炸���~�ڐG�����^��
			ToBothSide(axis0,axis1,startString);

		}else if(startString - playerString == 1){

			//���������t��

			if(endString == startString){
				//���̈ʒu�ɖ߂�ꍇ�̂�
				ToLeftSide(axis0,axis1,startString);
			}else{
				//����ȊO�͑S�ĉ��t����Ɉڌ�������i�E���u���̏ꍇ�C���̕����֖߂��Ă��悢���C�ڌ��N���X�ŉ��t��Ɉڌ�����݌v�Ƃ���j
				ToLeftSideWithIgen(axis0,axis1,startString);
			}

		}else if(startString - playerString == -1){

			//�E�������t��

			if(endString == startString){
				//���̈ʒu�ɖ߂�ꍇ�̂�
				ToRightSide(axis0,axis1,startString);
			}else{
				//����ȊO�͑S�ĉ��t����Ɉڌ�������i�E���u���̏ꍇ�C���̕����֖߂��Ă��悢���C�ڌ��N���X�ŉ��t��Ɉڌ�����݌v�Ƃ���j
				ToRightSideWithIgen(axis0,axis1,startString);
			}

		}

	}

}



/*!
 * \brief
 * �����������t���Ȃ��獶���Ɉ�i�K�ڌ�����i�V�����G�c�]���a�̃X�e�b�v���j
 * 
 * \param nb_string
 * �S�ʒu[0,1,2,3,4,5,6]
 * 
 */
/*
void Pressure::ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	//������Ԃ��v�Z����
	//Controller *controller = Controller::GetInstance();
	//ActuatorStatus status0 = controller->axis[0]->GetStatus();
	//ActuatorStatus status1 = controller->axis[1]->GetStatus();

	//�y�݌v�w�j�z
	//������Ԃ́C�L�����u���[�V�������ꂽ�������ʒu�ł��邱�Ƃɒ��ӁD�|�����|�����Ă����Ԃ���̑J�ڂ�
	//�T�|�[�g���Ă��Ȃ����Ƃɒ��ӁD�܂�C�e�{�[�C���O�P�ʂŕK���|���͂̓[���ɗ����Ă��邱�Ƃ��O��D
	//
	//������Ԃ������p�������ꍇ�́CBowing::Prepare()�̈�����pitchunit�̂悤�ɒǉ����āCBase::Prepare()�ɂ�
	//�ǉ��C�Ō��Pressure::Prepare()�ɒǉ����āC���̊֐��܂ň��������Ă���D
	//�����āC���̏�Ԃ���̎c�]���a���Čv�Z����igeometry->margin[]�̓L�����u���[�V�����ʒu����̃}�[�W���Ȃ̂Ŏg��Ȃ��j

	if(nb_string == 0){
		Error::Critical(0,QObject::tr("ToLeftSideWithIgen(): Invalid nb_string: %1").arg(nb_string));
	}

	//�ڌ����邽�߂̎c�]���a
	double radius = 0;

	//���Ƃ��ƃV���O���ʒu�̏ꍇ
	if(nb_string == 2){
		//2���V���O���ʒu����C2�������t���Ȃ���C1-2���ʒu�ցi1���ɒ��n�j
		radius = geometry->margin[2].leftMargin; // 2���V���O���ʒu�ł̍����}�[�W��
	}else if(nb_string == 4){
		//3���V���O���ʒu����C3�������t���Ȃ���C2-3���ʒu�ցi2���ɒ��n�j
		radius = geometry->margin[4].leftMargin; // 3���V���O���ʒu�ł̍����}�[�W��
	}else if(nb_string == 6){
		//4���V���O���ʒu����C4�������t���Ȃ���C3-4���ʒu�ցi3���ɒ��n�j
		radius = geometry->margin[6].leftMargin; // 4���V���O���ʒu�ł̍����}�[�W��
	}

	//���Ƃ��Ɨ����ʒu�̏ꍇ
	if(nb_string == 1){
		//1-2���ʒu����1�������t���Ȃ���C1���V���O���ʒu��
		radius = geometry->margin[0].rightMargin; // 1���V���O���ʒu�ł̉E���}�[�W��
	}else if(nb_string == 3){
		//2-3���ʒu����2�������t���Ȃ���C2���V���O���ʒu��
		radius = geometry->margin[2].rightMargin; // 1���̌��ʒu�̉E�}�[�W��
	}else if(nb_string == 5){
		//3-4���ʒu����3�������t���Ȃ���C3���V���O���ʒu��
		radius = geometry->margin[4].rightMargin;
	}

	//�ڌ��挷�ւ̎c�]���a������
	//double radius = geometry->residualRadius(status0.Position, status1.Position, to_string);
	//�ڌ��ɕK�v�ȃ�L�����߂�
	double step_deltaL = radius/(double)pressureVector.size();

	for(int i=0;i<pressureVector.size();i++){

		//���ʐڐ��������C2�������t����
		double deltaL0 = 0; //�E��
		double deltaL1 = 0; //����
		if(nb_string % 2 == 0){//2,4,6

			//���Ƃ��ƃV���O���ʒu�̏ꍇ

			deltaL0 = ConvertToMM(pressureVector[i]);	//�E���i�����ֈړ�����̂ŁC���Ƃ��ƍ����ֈ�i�K�ڌ������ʒu�ɑ΂��ẲE���͋|���t���Ώ�
			deltaL1 = (-radius)+step_deltaL*i;			//�����i������͒����Ώہj

			//���ʐڐ�������
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string-1); //�����ֈ�i�K�ڌ����������ʒu�Ōv�Z

		}else{
			//���Ƃ��Ɨ����ʒu�̏ꍇ
			deltaL0 = (-radius)+step_deltaL*i;			//�E���i�����Ώہj
			deltaL1 = ConvertToMM(pressureVector[i]);	//�����i�|���t���Ώہj

			//���ʐڐ�������
			geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string); //�w��ʒu�Ōv�Z
		}
		axis0.append(pos0);
		axis1.append(pos1);
	}
}
*/

/*!
 * \brief
 * �����������t���Ȃ��獶���Ɉ�i�K�ڌ�����i���ėp�����j
 * 
 * \param nb_string
 * �����ʒu���w��[1,3,5]
 * 
 */
/*
void Pressure::ToLeftSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	//������Ԃ��v�Z����
	Controller *controller = Controller::GetInstance();
	ActuatorStatus status0 = controller->axis[0]->GetStatus();
	ActuatorStatus status1 = controller->axis[1]->GetStatus();
	
	int to_string = 0;
	if(nb_string == 1){
		Error::Critical(0,QObject::tr("ToRightSideWithIgen() �̎w�肪�s��"));
	}else if(nb_string == 3){
		//2-3���ʒu����2�������t���Ȃ���1-2���ʒu�ցi��1���ɒ��n�j
		to_string = 0;
	}else if(nb_string == 5){
		//3-4���ʒu����3�������t���Ȃ���2-3���ʒu�ցi��2���ɒ��n�j
		to_string = 1;
	}

	//�ڌ��挷�ւ̎c�]���a������
	double radius = geometry->residualRadius(status0.Position, status1.Position, to_string);
	//�ڌ��ɕK�v�ȃ�L�����߂�
	double step_deltaL = radius/(double)pressureVector.size();

	for(int i=0;i<pressureVector.size();i++){

		//���ʐڐ��������C2�������t����
		double deltaL0 = ConvertToMM(pressureVector[i]);	//�E��
		double deltaL1 = (-radius)+step_deltaL*i;					//����
		//���ʐڐ�������
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string-2); //�����ֈ�i�K�ڌ����������ʒu�Ōv�Z
		axis0.append(pos0);
		axis1.append(pos1);
	}
}
*/


/*!
 * \brief
 * ���̓x�N�^���v���y�A����i����ʉ������j
 * StartString[1,3,5], EndString[1,3,5]
 * 0	1��
 * 1	1-2��
 * 2	2��
 * 3	2-3��
 * 4	3��
 * 5	3-4��
 * 6	4��
 * 
 * \param axis0
 * ��0�̐���x�N�^�iappend����܂��j
 * 
 * \param axis1
 * ��1�̐���x�N�^�iappend����܂��j
 * 
 */
/*
void Pressure::Prepare(QVector<double> &axis0, QVector<double> &axis1)
{
	if(startString == 1){
		//1-2�������ʒu����
		if(endString == 1){ 
			//1-2�������ʒu�ցi�ڌ������j
			if(playerString == 0){
				//1�������t
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 1){
				//1-2�����������t
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 2){
				//2�������t
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 3){ // ... endString == 5 �Ɠ�����
			//2-3�������ʒu��
			if(playerString == 0){
				//1�������t����
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 1){
				//1-2���������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 2){
				//2�������t����i���t���Ȃ���2-3�������ʒu�ֈڌ��j
				ToRightSideWithIgen(axis0,axis1,startString);
			}
		}else if(endString == 5){// ... endString == 3 �Ɠ�����
			//3-4�������ʒu��
			if(playerString == 0){
				//1�������t����
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 1){
				//1-2���������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 2){
				//2�������t����i���t���Ȃ���2-3�������ʒu�ֈڌ��j�����̌�3-4�������ʒu�ֈڌ�
				ToRightSideWithIgen(axis0,axis1,startString);
			}
		}
	}else if(startString == 3){
		//2-3�������ʒu����
		if(endString == 1){
			//1-2�������ʒu��
			if(playerString == 2){
				//2�������t����i���t���Ȃ���1-2�������ʒu�ֈڌ��j
				ToLeftSideWithIgen(axis0,axis1,startString);
			}else if(playerString == 3){
				//2-3�����������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 4){
				//3�������t����
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 3){
			//2-3�������ʒu�ցi�ڌ������j
			if(playerString == 2){
				//2�������t����
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 3){
				//2-3�����������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 4){
				//3�������t����
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 5){
			//3-4�������ʒu��
			if(playerString == 2){
				//2�������t����
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 3){
				//2-3�����������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 4){
				//3�������t����i���t���Ȃ���3-4�������ʒu�ֈڌ��j
				ToRightSideWithIgen(axis0,axis1,startString);
			}
		}
	}else if(startString == 5){
		//3-4�������ʒu����
		if(endString == 1){ // ... endString == 3 �Ɠ�����
			//1-2�������ʒu��
			if(playerString == 4){
				//3�������t����i���t���Ȃ���2-3�������ʒu�ֈڌ��j�����̌�1-2�������ʒu�ֈڌ�
				ToLeftSideWithIgen(axis0,axis1,startString);
			}else if(playerString == 5){
				//3-4�����������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 6){
				//4�������t����
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 3){// ... endString == 1 �Ɠ�����
			//2-3�������ʒu��
			if(playerString == 4){
				//3�������t����i���t���Ȃ���2-3�������ʒu�ֈڌ��j
				ToLeftSideWithIgen(axis0,axis1,startString);
			}else if(playerString == 5){
				//3-4�������t����
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 6){
				//4�������t����
				ToRightSide(axis0,axis1,startString);
			}
		}else if(endString == 5){
			//3-4�������ʒu�ցi�ڌ������j
			if(playerString == 4){
				ToLeftSide(axis0,axis1,startString);
			}else if(playerString == 5){
				ToBothSide(axis0,axis1,startString);
			}else if(playerString == 6){
				ToRightSide(axis0,axis1,startString);
			}
		}
	}		
}
*/


/*!
 * \brief
 * �E���������t���Ȃ���E���Ɉ�i�K�ڌ�����i���ėp�����j
 * 
 * \param nb_string
 * �����ʒu���w��[1,3,5]
 * 
 */
/*
void Pressure::ToRightSideWithIgen(QVector<double> &axis0, QVector<double> &axis1,int nb_string)
{
	double pos0 = 0;
	double pos1 = 0;

	//������Ԃ��v�Z����
	Controller *controller = Controller::GetInstance();
	ActuatorStatus status0 = controller->axis[0]->GetStatus();
	ActuatorStatus status1 = controller->axis[1]->GetStatus();
	
	int to_string = 0;
	if(nb_string == 1){
		//1-2���ʒu����2�������t���Ȃ���2-3���ʒu�ցi��3���ɒ��n�j
		to_string = 2;
	}else if(nb_string == 3){
		//2-3���ʒu����3�������t���Ȃ���3-4���ʒu�ցi��4���ɒ��n�j
		to_string = 3;
	}else if(nb_string == 5){
		Error::Critical(0,QObject::tr("ToRightSideWithIgen() �̎w�肪�s��"));
	}

	//�ڌ��挷�ւ̎c�]���a������
	double radius = geometry->residualRadius(status0.Position, status1.Position, to_string);
	//�ڌ��ɕK�v�ȃ�L�����߂�
	double step_deltaL = radius/(double)pressureVector.size();

	for(int i=0;i<pressureVector.size();i++){

		//���ʐڐ��������C2�������t����: deltaL0���CdeltaL1�ω�
		double deltaL0 = (-radius)+step_deltaL*i;					//�E��
		double deltaL1 = ConvertToMM(pressureVector[i]);	//����
		//���ʐڐ�������
		geometry->commonTangent(deltaL0, deltaL1, pos0, pos1, nb_string+2); //�E���ֈ�i�K�ڌ����������ʒu�Ōv�Z
		axis0.append(pos0);
		axis1.append(pos1);
	}
}
*/

