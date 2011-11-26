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

const double BaseGeometry::c = 121.25;//[mm] �������Ǝ�0�i��1�j�̋��� //242.5/2 = 121.25
const double BaseGeometry::L = 292.0;//[mm] ��2�Ƌ|�̋���

BaseGeometry::BaseGeometry()
{
	calibration = new Calibration();
	calibration->Load();
	//���ȌĂяo��
	Init();
}

BaseGeometry::~BaseGeometry()
{
	delete calibration;
}

void BaseGeometry::Init()
{
	//�����ŃW�I���g�����v�Z����G��̓I�ɂ͊e���̕��ʍ��W�����肵�A�����^���̊�b�i�ڐ���̉^���j���\�z����B

	//��S���̈ʒu
	origin[0] = CalcStringPosition(0);

	//��R���̈ʒu
	origin[1] = CalcStringPosition(1);

	//��Q���̈ʒu
	origin[2] = CalcStringPosition(2);

	//��P���̈ʒu
	origin[3] = CalcStringPosition(3);

	//�o��
	/*
	for(int i=0;i<4;i++)
	{
		std::cout << origin[i] << std::endl;
	}
	*/

	//�X�g�����O�}�[�W�����v�Z����
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
	//�e���ʒu�ɂ���
	for(int i=0;i<7;i++){
		//��0
		double pos0 = pos[i].start[0];
		//��1
		double pos1 = pos[i].start[1];
		//�c�]���a���v�Z����
		if(i == 0){
			//1���V���O���ʒu
			margin[i].leftMargin = 0;
			margin[i].rightMargin= residualRadius(pos0,pos1,1); // 2���Ƃ̋���
		}else if(i == 1){
			//1-2���ʒu
			margin[i].leftMargin = 0;
			margin[i].rightMargin= residualRadius(pos0,pos1,2); // 3���Ƃ̋���
		}else if(i == 2){
			//2���V���O���ʒu
			margin[i].leftMargin = residualRadius(pos0,pos1,0); // 1���Ƃ̋���
			margin[i].rightMargin= residualRadius(pos0,pos1,2); // 3���Ƃ̋���
		}else if(i == 3){
			//2-3���ʒu
			margin[i].leftMargin = residualRadius(pos0,pos1,0); // 1���Ƃ̋���
			margin[i].rightMargin= residualRadius(pos0,pos1,3); // 4���Ƃ̋���
		}else if(i == 4){
			//3���V���O���ʒu
			margin[i].leftMargin = residualRadius(pos0,pos1,1); // 2���Ƃ̋���
			margin[i].rightMargin= residualRadius(pos0,pos1,3); // 4���Ƃ̋���
		}else if(i == 5){
			//3-4���ʒu
			margin[i].leftMargin = residualRadius(pos0,pos1,1); // 2���Ƃ̋���
			margin[i].rightMargin= 0;
		}else if(i == 6){
			//4���ʒu
			margin[6].leftMargin = residualRadius(pos0,pos1,2); // 3���Ƃ̋���
			margin[6].rightMargin = 0;
		}
	}
	//�\��
	//for(int i=0;i<7;i++){
	//	std::cout << "nb_string = " << i << ", ��: " << margin[i].leftMargin << ", �E: " << margin[i].rightMargin << std::endl;
	//}
}


/*!
 * \brief
 * �p�[�Z���g�P�ʂ�mm�P�ʂ֕ϊ�����
 * 
 * \param percent
 * �p�[�Z���g�P��
 * 
 * \param nb_axis
 * ���ԍ�
 * 
 * \returns
 * mm�P��
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
 * mm�P�ʂ��p�[�Z���g�P�ʂ֕ϊ�����
 * 
 * \param mm
 * mm�P��
 * 
 * \param nb_axis
 * ���ԍ�
 * 
 * \returns
 * �p�[�Z���g�P��
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
 * �c�]���a���v�Z����
 * 
 * \param pos0
 * P0
 * 
 * \param pos1
 * P1
 * 
 * \param nb_string
 * ���S���W
 * 
 * \returns
 * �c�]���a
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
 * �������a�i��_P0,P1��ʂ钼���ƁC�����ʍ��W�Ƃ̋����j�����߂�D
 * 
 * \param pos0
 * P0��^���鎲0��̈ʒu��[0,100]
 * 
 * \param pos1
 * P1��^���鎲1��̈ʒu��[0,100]
 * 
 * \param nb_string
 * ���ԍ�[0,3]�i���ԍ��̎w����@�ɒ��Ӂj
 * 
 * \remarks
 * ���ԍ��̎w����@�ɒ���
 * 
 */
double BaseGeometry::initialRadius(double pos0, double pos1, int nb_string)
{
	if(!(nb_string == 0 || nb_string == 1 || nb_string == 2 || nb_string == 3)){
		Error::Critical(0, QObject::tr("residualRadius()�ɂČ��ԍ��w��[0,3]���͈͊O�ł�: nb_string=%1").arg(nb_string));
	}

	//mm�P�ʂɊ��Z
	Point p0 = Point(+c, ConvertPercentToMM(pos0,0));
	Point p1 = Point(-c, ConvertPercentToMM(pos1,0));

	//���������߂�
	double distance = vecmath::distance_l_p(p0,p1,origin[nb_string]);
	return distance;
}

double BaseGeometry::initialRadius2(double pos0, double pos1, int nb_string)
{
	if(nb_string == 1 || nb_string == 3 || nb_string == 5){
		Error::Critical(0, QObject::tr("residualRadius2()�́C�����ʒu�ł͒�`����Ă��܂���: nb_string=%1").arg(nb_string));
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

	//mm�P�ʂɊ��Z
	Point p0 = Point(+c, ConvertPercentToMM(pos0,0));
	Point p1 = Point(-c, ConvertPercentToMM(pos1,0));

	//���������߂�
	double distance = vecmath::distance_l_p(p0,p1,origin[nbString]);
	return distance;
}


/*!
 * \brief
 * 2�~�̋��ʐڐ������߂�
 * 
 * \param deltaL0
 * �~0�̃L�����u���[�V�������a����̍���
 * 
 * \param deltaL1
 * �~1�̃L�����u���[�V�������a����̍���
 * 
 * \param pos0
 * ��0��̈ʒu[0,100]
 * 
 * \param pos1
 * ��1��̈ʒu[0,100]
 * 
 * \param nb_string
 * ���ԍ�[1,3,5]
 *
 * �^�q�^�q
 * 
 * \remarks
 * ���ԍ��͋��d�l�ɂ����闼���ʒu�݂̂��T�|�[�g���Ă��邱�Ƃɒ��ӁD
 * 
 */
void BaseGeometry::commonTangent(double deltaL0, double deltaL1, double &pos0, double &pos1, int nb_string)
{
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions ��[0,100]���K������Ă��邽��mm�P�ʂɂ͂Ȃ��Ă��Ȃ����Ƃɒ���

	if(!(nb_string == 1 || nb_string == 3 || nb_string == 5)){
		Error::Critical(0, QObject::tr("nb_string is not 1,3,5:%1").arg(nb_string));
	}

	Point p0; // p0 �L�����u���[�V�������ꂽ��0��̈ʒu
	Point p1; // p1 �L�����u���[�V�������ꂽ��1��̈ʒu
	Point s0; // string center 
	Point s1; // string center
	Point un; // unit normal 
	if(nb_string == 1){
		//1-2 strings
		double dp0 = pos[1].start[0];
		double dp1 = pos[1].start[1];
		p0 = Point(c,  ConvertPercentToMM(dp0,0));//��0��̃L�����u���[�V�����ʒu
		p1 = Point(-c, ConvertPercentToMM(dp1,1));//��1��̃L�����u���[�V�����ʒu
		s0 = origin[1];//2���̌��_�i�E���j
		s1 = origin[0];//1���̌��_�i�����j
		un = unitnormal[1];
	}else if(nb_string == 3){
		//2-3 strings
		double dp0 = pos[3].start[0];
		double dp1 = pos[3].start[1];
		p0 = Point(c,  ConvertPercentToMM(dp0,0));//��0��̃L�����u���[�V�����ʒu
		p1 = Point(-c, ConvertPercentToMM(dp1,1));//��1��̃L�����u���[�V�����ʒu
		s0 = origin[2];//3���̌��_�i�E���j
		s1 = origin[1];//2���̌��_�i�����j
		un = unitnormal[3];
	}else if(nb_string == 5){
		//3-4 strings
		p0 = Point(c,  ConvertPercentToMM(pos[5].start[0],0));//��0��̃L�����u���[�V�����ʒu
		p1 = Point(-c, ConvertPercentToMM(pos[5].start[1],1));//��1��̃L�����u���[�V�����ʒu
		s0 = origin[3];//4���̌��_�i�E���j
		s1 = origin[2];//3���̌��_�i�����j
		un = unitnormal[5];
	}else{
		Error::Critical(0,QObject::tr("twoCircle: Invalid nb_string %1").arg(nb_string));
	}

	//��]�p�������߂�i�����v���𐳁j
	double sin_alpha = (deltaL1-deltaL0)/vecmath::distance(s0,s1);
	//�m�F
	//double sin_alpha = (deltaL0 - deltaL1)/vecmath::distance(s0,s1);
	double alpha = (-1.0)*asin(sin_alpha);

	//���~�̋��ʐڐ������߂�

	//�ŏ��ɒP�ʖ@���i�t�����j������]������
	//�t�����@��
	Point rev_un = (-1.0)*un;
	Point rp = rot(rev_un,alpha);

	//�~0�̐ړ_
	double len0 = L + deltaL0;
	Point rpc0 = rp*len0 + s0;

	//�~1�̐ړ_
	double len1 = L + deltaL1;
	Point rpc1 = rp*len1 + s1;

	//�~0�Ɖ~1�̋��ʐڐ��ɑ΂���P0'��p1'�����߂�
	Point p0_dash; // ��L��^����C���߂�P0'
	Point p1_dash; // ��L��^����C���߂�P1'
	//p0_dash
	if(vecmath::is_intersected_l(rpc0, rpc1, Point(c,0), Point(c,1))){
		p0_dash = vecmath::intersection_l(rpc0, rpc1, Point(c,0), Point(c,1));
	}else{
		Error::Critical(0,QObject::tr("P0' �����߂�ۂɃG���[���������܂����i��_������܂���j"));
	}
	//p1_dash
	if(vecmath::is_intersected_l(rpc0, rpc1, Point(-c,0), Point(-c,1))){
		p1_dash = vecmath::intersection_l(rpc0, rpc1, Point(-c,0), Point(-c,1));
	}else{
		Error::Critical(0,QObject::tr("P1' �����߂�ۂɃG���[���������܂����i��_������܂���j"));
	}

	//�P�ʊ��Z(mm����[0,100]���K���\���ցj
	pos0 = ConvertMMToPercent(p0_dash.imag(),0);
	pos1 = ConvertMMToPercent(p1_dash.imag(),0);
	if(!(0 <= pos0 && pos0 <= 100)){
		Error::Critical(0,QObject::tr("P0' �����߂�ۂɃG���[���������܂����i���߂�P0'�̒l���s���ł�: %1�j").arg(pos0));		
	}
	if(!(0 <= pos1 && pos1 <= 100)){
		Error::Critical(0,QObject::tr("P1' �����߂�ۂɃG���[���������܂����i���߂�P1'�̒l���s���ł�: %1�j").arg(pos1));		
	}
}

/*!
 * \brief
 * �_�̌��_�𒆐S�Ƃ�����]����
 * 
 * \param p
 * ��]�O�̓_
 * 
 * \param theta
 * ��]�p�x�i�����v���𐳁j
 * 
 * \returns
 * ��]��̓_
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
 * �Ώی��̕��ʍ��W�����߂�
 * 
 * \param nb_string
 * �Ώی�[0,3]
 * 
 * \returns
 * �Ώی��̕��ʍ��W�i�f�o�b�O�̗e�Ղ��̂��߂ɕ��ʍ��W��mm�ɓ���j
 * 
 */
Point BaseGeometry::CalcStringPosition(int nb_string)
{
	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions ��[0,100]���K������Ă��邽��mm�P�ʂɂ͂Ȃ��Ă��Ȃ����Ƃɒ���
	
	Point G_p0;
	Point G_p1;
	Point H_p0;
	Point H_p1;

	if(nb_string == 0){//1��
		//1��
		G_p0 = Point(c,  ConvertPercentToMM(pos[0].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[0].start[1],0));
		//1��-2��
		H_p0 = Point(c,  ConvertPercentToMM(pos[1].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[1].start[1],0));
	}else if(nb_string == 1){//2��
		//2��
		G_p0 = Point(c,  ConvertPercentToMM(pos[2].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[2].start[1],0));
		//1��-2��
		H_p0 = Point(c,  ConvertPercentToMM(pos[1].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[1].start[1],0));
	}else if(nb_string == 2){//3��
		//3��
		G_p0 = Point(c,  ConvertPercentToMM(pos[4].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[4].start[1],0));
		//2��-3��
		H_p0 = Point(c,  ConvertPercentToMM(pos[3].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[3].start[1],0));
	}else if(nb_string == 3){//4��
		//4��
		G_p0 = Point(c,  ConvertPercentToMM(pos[6].start[0],0));
		G_p1 = Point(-c, ConvertPercentToMM(pos[6].start[1],0));
		//3��-4��
		H_p0 = Point(c,  ConvertPercentToMM(pos[5].start[0],0));
		H_p1 = Point(-c, ConvertPercentToMM(pos[5].start[1],0));
	}else{
		Error::Critical(0, QObject::tr("CalcStringPosition() �ł̎w�茷[0,3]���͈͊O�ł�. nb_string=%1").arg(nb_string));
	}

	//// �P���ɐڂ��钼�� ////

	//��2�������x�N�g��
	Point G_vec = G_p1 - G_p0;

	//�P�ʖ@���x�N�g�� x L�i��2�Ƌ|�̋����j
	Point Normal_vec1;
	Point Normal_vec2;
	vecmath::unitnormal(G_vec, &Normal_vec1, &Normal_vec2);

	Point Trans_vec;
		
	//�̗p����P�ʖ@���x�N�g����y���̒l���������ق�
	if(Normal_vec1.imag() < Normal_vec2.imag()){
		Trans_vec = Normal_vec1;
	}else{
		Trans_vec = Normal_vec2;
	}
	//�P�ʖ@���x�N�g����ۑ�
	if(nb_string == 0){
		unitnormal[0] = Trans_vec;		
	}else if(nb_string == 1){
		unitnormal[2] = Trans_vec;
	}else if(nb_string == 2){
		unitnormal[4] = Trans_vec;
	}else if(nb_string == 3){
		unitnormal[6] = Trans_vec;
	}
	//L�{���ĐڐG�~���
	Trans_vec = Trans_vec * L;
	
	//�|��̓�_���v�Z
	Point G_p0_dash = Trans_vec + G_p0;
	Point G_p1_dash = Trans_vec + G_p1;

	//// �񌷂ɐڂ��钼�� ////

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
		//nb_string == 0�̏ꍇ�Ɠ���
	}else if(nb_string == 2){
		unitnormal[3] = Trans_vec;
	}else if(nb_string == 3){
		unitnormal[5] = Trans_vec;
	}
	Trans_vec = Trans_vec * L;
	Point H_p0_dash = Trans_vec + H_p0;
	Point H_p1_dash = Trans_vec + H_p1;

	//// ��_���v�Z ////
	Point intersects;
	if(vecmath::is_intersected_l(G_p0_dash, G_p1_dash, H_p0_dash, H_p1_dash)){
		intersects = vecmath::intersection_l(G_p0_dash, G_p1_dash, H_p0_dash, H_p1_dash);
	}else{
		Error::Critical(0,QObject::tr("�����W�����蒆�ɃG���[���������܂����i��_������܂���: nb_string= %1�j").arg(nb_string));
	}
	return intersects;
}


/*!
 * \brief
 * OBSOLETE �����ʒu�������ʒu���𔻒肷��D
 * 
 * \param pos0
 * ��0�̈ʒu�i�P�ʂ͉��ł��悢�j
 * 
 * \param pos1
 * ��1�̈ʒu�i�P�ʂ͉��ł��悢�j
 * 
 * \param nb_string
 * �Ώی�
 * 
 * \returns
 * �����ʒu�̏ꍇ 0 ��Ԃ�
 * �����ʒu�̏ꍇ 1 ��Ԃ�
 * 
 */
int BaseGeometry::positionType(double pos0, double pos1, int nb_string)
{
	std::cerr << "OBSOLETE�����̊֐�positionType()���g�p����Ă��܂��D" << std::endl;

	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions ��[0,100]���K������Ă��邽��mm�P�ʂɂ͂Ȃ��Ă��Ȃ����Ƃɒ���

	//�S�L�����u���[�V�����ʒu�ɂ���
	for(int i=0;i<pos.size();i++){
		//�����Ώۂ̓�_
		Point p0_dash = Point(c, pos0);
		Point p1_dash = Point(-c,pos1);

		//�L�����u���[�V�����ʒu
		Point c0 = Point(c, pos[i].start[0]); // ��0�̃L�����u���[�V�����ʒu
		Point c1 = Point(-c,pos[i].start[1]); // ��1�̃L�����u���[�V�����ʒu

		//�񒼐��̕��s����
		if(vecmath::is_parallel(p0_dash, p1_dash, c0, c1)){
			if(i%2 == 0){
				//�����ʒu; 0[1��],2[2��],4[3��],6[4��]
				return 0;
			}else{
				//�����ʒu; 1[1-2��],3[2-3��],5[3-4��],
				return 1;
			}
		}
	}

	//�����ɂ����ꍇ�́C������L�����u���[�V�����ʒu�ɑ΂��āC���s�ȏ�Ԃł͂Ȃ������Ƃ������ƂŁC����ُ͈�D
	std::cerr << "pos0=" << pos0 << ",pos1=" << pos1 << ",nb_string=" << nb_string << std::endl;
	Error::Critical(0,QObject::tr("positionType()�֐��ŃG���[���������܂����D���ݎ��ʒu�́C������L�����u���[�V�����ʒu�ɑ΂��ĕ��s�ł͂���܂���D"));
	return -1;
}


/*!
 * \brief
 * �����ʒu�C�������͗����ʒu����̕��s�ړ��S�������̉��ŁC�|����L��^����P0',P1'�����߂�D
 * �i�S�������́C�܂�C�L�����u���[�V�������ꂽ�e�ʒu����̕��s�ړ������Ƃ������j
 *
 * \param deltaL
 * ��L�i�|���̒��ڔ��p�����[�^�G�P�ʂ̓~�����[�g���G�݌v�m�[�g�Q�Ɓj
 * 
 * \param pos0
 * ��L��^����P0��Y���W[0,100]
 * 
 * \param pos1
 * ��L��^����P1��Y���W[0,100]
 * 
 * \param nb_string
 * �Ώی�[0,6] 
 * 0 : 1�������ʒu
 * 1 : 1-2�������ʒu
 * 2 : 2�������ʒu
 * 3 : 2-3�������ʒu
 * 4 : 3�������ʒu
 * 5 : 3-4�������ʒu
 * 6 : 4�������ʒu
 * 
 * \remarks
 * �ȑOOBSOLETE�����������������������D
 *
 */
void BaseGeometry::deltaL(double deltaL, double &pos0, double &pos1, int nb_string)
{
	//std::cerr << "OBSOLETE�����̊֐�deltaL()���g�p����Ă��܂��D" << std::endl;

	QList<Calibration::Positions> pos = calibration->GetCalibratedPositions(); // Calibration::Positions ��[0,100]���K������Ă��邽��mm�P�ʂɂ͂Ȃ��Ă��Ȃ����Ƃɒ���

	Point p0 = Point(c,  ConvertPercentToMM(pos[nb_string].start[0],0));//��0��̃L�����u���[�V�����ʒu
	Point p1 = Point(-c, ConvertPercentToMM(pos[nb_string].start[1],0));//��1��̃L�����u���[�V�����ʒu

	Point p0_dash; // ��L��^����C���߂�P0'
	Point p1_dash; // ��L��^����C���߂�P1'
	Point s_origin;   // ���p���錷���S
	Point s_two_dash; // L+��L���ꂽ�ڐG�~��̓_
	Point p_two_dash; // L+��L���ꂽ���s�ړ���̃x�[�X���C��������̓_

	if(nb_string == 0){//1��
		//1���̌����W
		s_origin = origin[0];
	}else if(nb_string == 1){//1-2��
		//1���̌����W
		s_origin = origin[0];
	}else if(nb_string == 2){//2��
		//2���̌����W
		s_origin = origin[1];
	}else if(nb_string == 3){//2-3��
		//2���̌����W
		s_origin = origin[1];
	}else if(nb_string == 4){//3��
		//3���̌����W
		s_origin = origin[2];
	}else if(nb_string == 5){//3-4��
		//3���̌����W
		s_origin = origin[2];
	}else if(nb_string == 6){//4��
		//4���̌����W
		s_origin = origin[3];
	}
	
	//�����W���� L + ��L ���ꂽ�ڐG�~��̓_�̍��W�����߂� s''
	s_two_dash = s_origin - ( unitnormal[nb_string]*(L+deltaL) ); // �@���x�N�g�����t�����Ȃ̂� (-1) �ł�

	//s''��ʂ�C�x�[�X���C���ƕ��s�ȃx�N�g�������߂� p''
	p_two_dash = s_two_dash + (p1-p0);

	//s'' �� p'' ��ʂ钼����x=c�̌�_ = p0;	
	if(vecmath::is_intersected_l(p_two_dash, s_two_dash, Point(c,0), Point(c,1))){
		p0_dash = vecmath::intersection_l(p_two_dash, s_two_dash, Point(c,0), Point(c,1));
	}else{
		Error::Critical(0,QObject::tr("P0' �����߂�ۂɃG���[���������܂����i��_������܂���j"));
	}

	//s''��p''��ʂ钼����x=-c�̌�_ = p1;
	if(vecmath::is_intersected_l(p_two_dash, s_two_dash, Point(-c,0), Point(-c,1))){
		p1_dash = vecmath::intersection_l(p_two_dash, s_two_dash, Point(-c,0), Point(-c,1));
	}else{
		Error::Critical(0,QObject::tr("P1' �����߂�ۂɃG���[���������܂����i��_������܂���j"));
	}

	//�P�ʊ��Z(mm����[0,100]���K���\���ցj
	//pos0 = p0_dash.imag()*100. / (max_pulse / pulse_by_mm);
	//pos1 = p1_dash.imag()*100. / (max_pulse / pulse_by_mm);
	pos0 = ConvertMMToPercent(p0_dash.imag(),0);
	pos1 = ConvertMMToPercent(p1_dash.imag(),0);
	if(!(0 <= pos0 && pos0 <= 100)){
		Error::Critical(0,QObject::tr("P0' �����߂�ۂɃG���[���������܂����i���߂�P0'�̒l���s���ł�: %1�j").arg(pos0));		
	}
	if(!(0 <= pos1 && pos1 <= 100)){
		Error::Critical(0,QObject::tr("P1' �����߂�ۂɃG���[���������܂����i���߂�P1'�̒l���s���ł�: %1�j").arg(pos1));		
	}
}

