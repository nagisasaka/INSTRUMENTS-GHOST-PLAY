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

#include "base.hpp"
#include "../actuator/controller.hpp"
#include "../actuator/actuator.hpp"
#include "../vecmath.hpp"
#include "pressure.hpp"
#include "baseGeometry.hpp"
#include "igen.hpp"
#include "../calibration.hpp"

#include <cstdlib>

Base::Base()
{
	pressure = new Pressure();
	igen = new Igen();
	geometry = new BaseGeometry();
	calibration = new Calibration();
	calibration->Load();
	StartString = -1; // �������
	EndString   = -1; // �������
}

Base::~Base()
{
	delete pressure;
	delete igen;
	delete geometry;
	delete calibration;
}

void Base::SetStringPair(int _StartString, int _EndString)
{
	if(StartString == -1 && EndString == -1){
		//�������
		//�I����ԃ`�F�b�N�̓X�L�b�v
	}else{
		//�O��̏I����Ԃ�����̏�����Ԃł���
		//if(EndString != _StartString){
		//	Error::Critical(0, QObject::tr("�O��̏I���ʒu�ƁC�J�����g�̏����ʒu�͓���łȂ���΂Ȃ�܂���"));	
		//}
	}

	StartString = _StartString;
	EndString = _EndString;

	//�|���N���X�ɃX�g�����O�y�A��ݒ�
	pressure->SetStringPair(StartString, EndString);

	//�ڌ��N���X�ɃX�g�����O�y�A��ݒ�
	igen->SetStringPair(StartString, EndString);
}

void Base::SetPlayerString(int _playerString)
{
	PlayerString = _playerString;

	//�|���N���X�ɉ��t����ݒ�
	pressure->SetPlayerString(PlayerString);
	//�ڌ��N���X�ɉ��t����ݒ�
	igen->SetPlayerString(PlayerString);
}

void Base::SetPressureVector(QVector<double> p)
{
	//�|���N���X�Ɉ��̓x�N�^��ݒ�
	pressure->SetPressureVector(p);
}

QVector<double> Base::GetPressureVector()
{
	return pressure->pressureVector;
}


/*!
 * \brief
 *
 * ���̓x�N�^�̃v���y�A
 *
 * �p��̒�`
 * �����ʒu�F �����ʒu
 * �I���ʒu�F ���ɉ��t���錷���܂ޗ����ʒu
 * ���t��: ���t��
 *
 * ����L
 * �w�肳�ꂽ���̓x�N�^�[�i10msec���Ƃ̈��́��l[0,100]�j�𖞂����悤�ɁC���ʐڐ��������ŁC��L�𐧌䂷��D
 * ����Ώۂ̃�L�́C���t���̐ڐG�~�ɑ΂��Ĕ��a���傫���Ȃ�����ցD
 * ����́C�P�ɍő�̃�L�ɑ΂��Ĉ��͒l���f�R�[�h���邾����OK�D�����|���E�I���|���ɂ��Ă͍l����K�v���Ȃ�
 * �Ƃ����_���́i������l���o���ƕ��G�ɂȂ肷���Ď��S����j
 *
 * �����t���ڌ��ƃ�L
 * ���t���ƈڌ��挷�̃�L�𗼕�����K�v������D�������C�����͓Ɨ��ɍl���Ă悢�D�܂�C���t���̃�L�́C
 * ��ŋL�ڂ������̓x�N�^�[�ɂ�钼�ڐ����OK�D
 * �ڌ��挷�̃�L�́C������Ԃ���ڌ����ԁi�ڌ���|���͕K���[���Ȃ̂ŃL�����u���[�V������ԁj�ւ̎c���a���C
 * ���̓x�N�^�[�̒����Ŋ������l���g���Đ��`�ω�������D
 * ���t���ƈڌ��挷�̃�L��Ɨ��Ɉ����邱�Ƃ��́i���ꂪ�]�����Ă���ƕ��G�ɂȂ肷���Ď��S���邺�I�j
 *
 * �����t�I����ڌ��ƃ�L
 * ���t���ڌ��Ɠ������Ƃł͂���D���t���̃�L��ω��������ɁC�ڌ��挷�̃�L����Ɠ��l�ɕω�������D
 * ���S�̂��߂ɉ��t���̃�L���}�C�i�X�����֎኱�}�[�W����^���Ă��ǂ��D
 *
 * ��������Ԃɂ���
 * ������Ԃ̌v�Z���K�v�Ȃ̂́C�ڌ��挷�ɑ΂���c���a�D�L�����u���[�V�����ʒu���m�ł���Ύ����͕K�v�Ȃ����C
 * �����|���i�c�]�|���j���݉��ɂ����ẮC�c���a�͕ω����Ă��܂��D�]���āCP0,P1�����ƈڌ��挷���S�Ƃ̋�����
 * �v�Z���āCL��������Z���Ďc�]���a�����߂�i�����̎c�]���a�����̓x�N�^���ŏ��Z����j
 *
 */
int Base::Prepare()
{	
	//����N���A����
	axis0.clear();
	axis1.clear();

	//���I���ɖ������������ǂ������m�F
	if(StartString - PlayerString == 0 || StartString - PlayerString == 1 || StartString - PlayerString == -1){
		//������ԂƓ��ꂩ�O�㌷�ԍ��������e�ł��Ȃ�
		std::cout << "[ �^�|�v���y�A ] �������: " << StartString << ", ���t��: " << PlayerString  << ", �I�����: " << EndString << " [ OK ]" << std::endl;
	}else{
		std::cout << "[ �^�|�v���y�A ] �������: " << StartString << ", ���t��: " << PlayerString  << ", �I�����: " << EndString << " [ NG ]" << std::endl;
		Error::Critical(0, QObject::tr("���I����Ԃɖ���������܂��D�����I�����܂��D"));
	}

	//�|���N���X�Ńv���y�A����
	pressure->Prepare(axis0,axis1);
	//�ڌ��N���X�Ńv���y�A����
	igen->Prepare(axis0,axis1);

	//��0���v���y�A
	Controller *control = Controller::GetInstance();
	control->axis[0]->SetMode(MLinear::CONTINUOUS);  //���䃂�[�h�i�A���j
	control->axis[0]->SetPositionVector(axis0);		 //����x�N�^���Z�b�g
	control->axis[0]->Prepare();					 //�v���y�A

	//��1���v���y�A
	control->axis[1]->SetMode(MLinear::CONTINUOUS);  //���䃂�[�h�i�A���j
	control->axis[1]->SetPositionVector(axis1);		 //����x�N�^���Z�b�g
	control->axis[1]->Prepare();					 //�v���y�A

	//�{�[�C���O���Ԃ�Ԃ�
	if(axis0.size() == axis1.size()){
		return axis0.size();
	}else{
		//�T�C�Y������Ȃ��̂͌v�Z�~�X
		std::cout << "Error: axis0.size != axis1.size, axis0.size()==" << axis0.size() << ", axis1.size()==" << axis1.size() << std::endl;
		Error::Critical(0, QObject::tr("axis0.size != axis1.size"));
		return 0;
	}
}

/*!
 * \brief
 * �R�~�b�g�i��0�C��1�ɑ΂��āj
 * 
 */
void Base::Commit()
{
	Controller *control = Controller::GetInstance();
	for(int i=0;i<2;i++){
		control->axis[i]->Commit(); //�R�~�b�g
	}
}

/*!
 * \brief
 * ������Ԃ֏����ړ�����i����͍ŏ��̈ړ��̂݁j
 *
 * \remarks
 * ���̊֐��̎��s�ŁC���O�Ƀv���y�A����Ă������e�͏�����̂� Not Prepared �G���[�̌����ɂȂ�Ղ��̂ōő咍��
 * 
 */
void Base::InitialMove(int nb_string)
{
	//if(nb_string == 1 || nb_string == 3 || nb_string == 5){
		//�����ړ����s
		QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
		Controller *control = Controller::GetInstance();
		//��0,1�ɂ��ăv���y�A�E�R�~�b�g�i�����܂łŎ��O�Ƀv���y�A����Ă������e�͏�����̂� Not Prepared �G���[�̌����ɂȂ�₷�����̂Œ��Ӂj
		for(int i=0;i<3;i++){
			control->axis[i]->SetMode(MLinear::JOG);
			control->axis[i]->SetPosition(pos[nb_string].start[i]);
			control->axis[i]->SetDuration(2000,0.5,0.5);
			control->axis[i]->Prepare();
		}
		for(int i=0;i<3;i++){
			control->axis[i]->Commit();
		}
		
	//}else{
	//	Error::Critical(0, QObject::tr("[ BASE ] �����ړ��ʒu�w�肪�s���ł�: %1").arg(nb_string));
	//}
}


//// OBSOLETE ////

//void Base::Prepare1()
//{
	/* Version 2 obsolete
	
	//������Ԃ������ʒu�������ʒu���̔���
	int initial = pressure->GetString();
	if(initial%2 == 0){
		//������Ԃ������ʒu�������G�����ʒu���玩���ʒu�ւ̈ړ��i�ڌ�������s���K�v�͂Ȃ��j
		std::cout << "�����ʒu->�����ʒu�̂��߈ڌ�����͍s���܂���" << std::endl;

		//�W������Ȃ��S�������̉��ŁC���̓x�N�^�̃v���y�A
		pressure->Prepare(axis0, axis1);
		//�I��
	}else{
		//������Ԃ������ʒu�������G�����ʒu���玩���ʒu�ւ̈ړ�

		//// �c�]�|���̗L���𔻒� ////

		//�L�����u���[�V�����ʒu���擾
		QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
		double p0_end = 0; // �I����Ԃ�p0[0,100]
		double p1_end = 0; // �I����Ԃ�p1[0,100]
		//���̓x�N�^
		QVector<double> p = pressure->pressureVector;
		if(p[p.size()-1] > EPS){ 
			//�c�]�|�����c���Ă��遨�I����Ԃ̍Čv�Z���K�v
			geometry->deltaL(pressure->ConvertToMM(p[p.size()-1]), p0_end, p1_end);
		}els
			//�c�]�|�����Ȃ����I����Ԃ͎����ʒu
			p0_end = pos[string1].start[0];
			p1_end = pos[string1].start[1];
		}//�I����Ԃ̊m�芮��

		//���̓x�N�^�S�������C���C�W������Ȃ��S�������C�̉��ŁC�����ʒu���玩���ʒu�ֈڌ�

		//TODO	
	}
	*/

	/* Version 1 obsolete

	//������Ԃ��擾�i�����͂P�񕪂Ȃ̂ŃJ�����g�̈ʒu���擾�G��������܂Ƃ߂邱�Ƃ�݌v��z�肵�Ă��Ȃ����v�Z���Ԃ��N���e�B�J���j
	Controller *controller = Controller::GetInstance();
	ActuatorStatus status0 = controller->axis[0]->GetStatus();
	ActuatorStatus status1 = controller->axis[1]->GetStatus();

	//������Ԃ������ʒu�������ʒu���̔���C�������C���s�ړ��S�������ɂ��|���t����Ԃł���\��������̂ŁC�x�N�g���Ƃ��Ĕ�r���Ȃ���΂Ȃ�Ȃ��D
	int positionType = geometry->positionType(status0.Position, status1.Position, string1);
	if(positionType == 1){
		//������Ԃ������ʒu�̏ꍇ�G�ڌ�������s���K�v�͂Ȃ�
		std::cout << "�����ʒu�̂��߈ڌ�����͍s���܂���D" << std::endl;
		
		//������Ԃ������ʒu�Ȃ̂ŁC�|���͎����Ɋ|����΂悢
		pressure->SetString(string1);

		//���̓x�N�^�̃v���y�A
		pressure->Prepare(axis0, axis1);

		//�I��
	}else if(positionType == 2){
		//������Ԃ������ʒu�̏ꍇ

		//������Ԃ̎c�]�|�������邩�ǂ����𔻒肵�C���茋�ʂɉ����ďI����Ԃ��v�Z

		//�L�����u���[�V�����ʒu���擾
		QList<Calibration::Positions> pos = calibration->GetCalibratedPositions();
		double p0_end = 0; // �I����Ԃ�p0[0,100]
		double p1_end = 0; // �I����Ԃ�p1[0,100]

		QVector<double> p = pressure->pressureVector;
		if(p[p.size()-1] > EPS){ 
			//�c�]�|�����c���Ă��遨�I����Ԃ̍Čv�Z���K�v
			geometry->deltaL(pressure->ConvertToMM(p[p.size()-1]), p0_end, p1_end);
		}else{
			//�c�]�|�����Ȃ����I����Ԃ͎����ʒu
			p0_end = pos[string1].start[0];
			p1_end = pos[string1].start[1];
		}

		//���Cstring1 == string2 �����������D���t�������̂́C�P���������͗������H		
	}
	*/
//}


