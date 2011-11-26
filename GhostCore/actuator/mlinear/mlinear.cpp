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
	//�W���O����p�����[�^�[������
	Dist   = 0;
	StrVel = 0; // �[���i���������j
	MaxVel = 0;
	Tacc   = 0;
	Tdec   = 0;
	SVacc  = 0; // �[���i���������̗̈�Ȃ��Gp41�j
	SVdec  = 0; // �[���i���������̗̈�Ȃ��Gp41�j

	//���Ԏw��
	Duration = 0;

	//�X�e�[�^�X�N���A
	status.Parameter = Actuator::WAITING;

	//�ړ������w��t���O�N���A
	flagPosition = false;
	flagDuration = false;
	flagVelocity = false;

	//�f�o�b�O
	_debug = false;
}


/*!
 * \brief
 * ����x�N�^���w�肷��i�A���ʒu��Ύw��G����x�N�^�̍ŏ��̎w�ߒl�͌��݈ʒu�Ɠ��������Ɓj
 * 
 * \param pos
 * ����x�N�^�i�A���ʒu��Ύw��j
 * 
 * \remarks
 * ��Ύw��ʒu�́C������l�ɕϊ�����ۂɑ��Έʒu�w��ɕϊ�����邪�C�ŏ��̑��Έʒu�� pos[1]-pos[0] �Ŏw�肳���̂ŁC
 * ���݈ʒu�� pos[0] �Ƃ̊Ԃ̑��Έʒu�͖��������D�܂�C���݈ʒu�ɑ΂��鍷���Ƃ��Ă������삵�Ȃ��D
 * �������C����͎d�l�ŁC���˂Ɍ��݈ʒu�Ɨ��ꂽ�ʒu���w�肳��Ă��C���݈ʒu���炻�̈ʒu�܂ł̓���������s���ł���C
 * ����ł����œ��삷�邱�Ƃ͂ł��Ȃ��D�]���āC�w�肳����Έʒu�́C�K�����݈ʒu����̑����ɂȂ��Ă��Ȃ���΂Ȃ炸
 * ����̓G���[�`�F�b�N�����D
 *
 * �������j�Ƃ��ẮC�ŏ��̐���T�C�N���Ō��݈ʒu����pos[0]�܂ň�C�Ɉړ�������Ƃ������j�����邪�C����͎��������ɁC
 * �o�O���������ꍇ�ɁC�ɒZ���ԂŒ��������ړ������邱�ƂɂȂ�C���x�����Ń\�t�g���~�b�g�I�[�o�[�ƂȂ邩�C
 * �\�t�g���~�b�g�ȉ��������ꍇ���C���Ȃ�̍����ړ����������C�\�t�g�E�F�A�̃o�O�������I�Ȋ댯�ɂȂ���\���̂���
 * �������j�ł��邽�߁C���̕��j���̂�Ȃ��D
 * 
 */
void MLinear::SetPositionVector(QVector<double>& pos)
{
	int plus = 0;
	int minus = 0;

	positionVector = pos;

	continuousControlData.clear();

	//// ���݈ʒu�Ƃ̍����`�F�b�N�i���̃`�F�b�N�̐���ɂ��Ă͌����̗]�n����C�����V�������ɂ��Ă��m�F����j
	long cpc = 0;
	retCode = _7443_get_command(nb_axis, &cpc);
	if(retCode){
		std::cout << "�� #" << uid << " setPositionVector() ���ŃR�}���h�|�W�V�������擾�ł��܂���" << std::endl;
		throw "�R�}���h�|�W�V�������擾�ł��܂���";
	}
	int initialPos = (pos[0]/100.)*max_pulse;
	if(cpc != initialPos){
		std::cout << "[ �x�� ] setPositionVector()�Ɏw�肳�ꂽ����x�N�g���̐擪�l�ƁC���݈ʒu���قȂ�܂�: ����x�N�g���擪�l: "<< initialPos <<"  (" << pos[0] << " ��), ���݈ʒu: " << cpc << "(" << ((double)cpc*100./(double)max_pulse) << "��)" << std::endl;
	}

	//// �������i���ǂ�����̋��������̗p���Cremarks�ɏ����Ă�����j���̗p�����j

	//// ���Βl�ւ̕ϊ�
	for(int i=1;i<pos.size();++i)
	{
		//�l��`�F�b�N
		if(pos[i] < 0){
			//�ʒu�f�[�^�Ȃ̂ŁC���̒l�͎��Ȃ�
			std::cout << "���ӏ��O��10���̃f�[�^\n" << std::endl;
			for(int dd =qMax(i-10,0);dd<qMin(i+10,pos.size());dd++){
				std::cout << "pos[" << dd << "] = " << pos[dd] << std::endl; 
			}
			Error::Critical(0,QObject::tr("�� # %1 �A���ʒu�w��f�[�^�ɕ��̒l���܂܂�܂��D�W���G���[���������������D�ʒu:%2, ���ƂȂ������̒l:%3.").arg(uid).arg(i).arg(pos[i]));
		}

		//�ړ��O�ʒu
		double fromPos = 0;
		//�ړ���ʒu
		double targetPos = 0;

		fromPos   = pos[i-1];
		targetPos = pos[i];


	//// �V�����i������́C��Ɍ��݈ʒu����ɂ���̂ŃR�[�f�B���O�̎��R�x���Ⴂ�G���ߒl�Ǝ��ےl�̂��ꂪ�C�ɂȂ�ꍇ�́CUpdate()���Œ���I�ɃR�}���h�|�W�V�������v�����ăY����␳����悤�ȃp���X�␳�𖄂ߍ��ނׂ��j
	/*
	//// ���Βl�ւ̕ϊ�
	for(int i=0;i<pos.size();++i)
	{
		//�l��`�F�b�N
		if(pos[i] < 0){
			//�ʒu�f�[�^�Ȃ̂ŁC���̒l�͎��Ȃ�
			Error::Critical(0,QObject::tr("�A���ʒu�w��f�[�^�ɕ��̒l���܂܂�܂��D�ʒu:%1, �l:%2").arg(i-1,pos[i-1]));
		}

		//�ړ��O�ʒu
		double fromPos = 0;
		//�ړ���ʒu
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
		//�ړ���ʒu�̃p���X���ł̕\��
		double targetPulse = (targetPos/100.0)*max_pulse;
		//�ړ��O�ʒu�̃p���X���ł̕\��
		double fromPulse   = (fromPos/100.0)*max_pulse;
		//���Έړ������̃p���X�\���i�������ړ������������j
		int relativePulse = (int)(targetPulse - fromPulse);
		//������Ԏ���
		double delta_time = Controller::GetCycle();//ms
		//������ԑ����i���j
		int velocity = (int)(((double)(1.0*abs(relativePulse)))/(delta_time/1000.));
		*/

		//���Έړ�����
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

		//�ۑ�
		ContinuousControlData c = { velocity, relativePulse };
		continuousControlData.append(c);

		//std::cout << "[" << fromPos << "->" << targetPos << "]" << c.velocity << "," << c.distance << std::endl;

		//���΃p���X�`�F�b�N
		if(relativePulse >= 0) plus += relativePulse;
		else if(relativePulse < 0) minus += relativePulse;

	}

	if(_debug){
		Error::Warning(0, QObject::tr("�␳�O�@�������p���X��:%1, �������p���X��:%2, ��:%3").arg(plus).arg(minus).arg(plus+minus));
	}

	//// ���̕␳���|���Ă��܂��ƁC�Е����ɍs��������̏����͎��s�s�\�ɂȂ�D�����ł��̏���������Ӗ��͂Ȃ�
	//�������S�Ƀ[���ɂ��邽�߂̕␳
	/*
	int finalPulse = (-1)*(plus+minus);
	int velocity = abs(finalPulse)/(Controller::GetCycle()/1000.);
	ContinuousControlData oc = { velocity, finalPulse };
	continuousControlData.append(oc);

	//�o���f�[�V����
	if(finalPulse >= 0) plus += finalPulse;
	else if(finalPulse < 0) minus += finalPulse;
	Error::Warning(0, QObject::tr("�␳��@�������p���X��:%1, �������p���X��:%2, ��:%3").arg(plus).arg(minus).arg(plus+minus));
	*/

	//�f�[�^�T�C�Y������
	size = continuousControlData.size();
	curr = 0;
	std::cout << "��#"<< uid << " �̃f�[�^�T�C�Y�����������C�A���ʒu�w��f�[�^��o�^���܂���:" << size << std::endl;
}

void MLinear::SetVelocityVector(QVector<double>& vel)
{
	/// ������ ///
}

void MLinear::SetPosition(double targetPos)
{
	//�n�[�h�E�F�A�̓C���N�������^�����[�h�œ��삵�Ă��邪�C�v���O������ł�
	//0-100���K�����ꂽ��Έʒu�w����ł���悤�ɂ���D

	//�ړ���ʒu���p���X���ł̕\��
	int targetPulse = (int)((targetPos/100.0)*max_pulse); // �덷�����v����������cpc������̂ŗݐς���Ȃ�
		
	//���݂̃R�}���h�|�W�V�����J�E���^�擾
	long cpc = 0;
	retCode = _7443_get_command(nb_axis, &cpc);
	if(retCode){
		std::cout << "�� #" << uid << " setPosition(double targetPos) ���ŃR�}���h�|�W�V�������擾�ł��܂���" << std::endl;
		throw "�R�}���h�|�W�V�������擾�ł��܂���";
	}

	//���Έړ������̃p���X�\���i�������ړ������������j
	Dist = targetPulse - cpc;

	//�t���OOK
	flagPosition = true;
}


void MLinear::SetPosition(double targetPos, double fromPos)
{
	//�ړ���ʒu���p���X���ł̕\��
	int targetPulse = (int)((targetPos/100.0)*max_pulse);

	//�ړ��O�ʒu���p���X���ł̕\��
	int fromPulse = (int)((fromPos/100.0)*max_pulse);

	//���Έړ������̃p���X�\���i�������ړ������������j
	Dist = targetPulse - fromPulse;
	
	//�t���OOK
	flagPosition = true;
}

void MLinear::SetDuration(double time_ms)
{		
	SetDuration(time_ms, 0, 0);
}

void MLinear::SetDuration(double time_ms, double accl_ratio, double decl_ratio)
{
	Duration = time_ms; //�~���b
	if(accl_ratio + decl_ratio > 1){
		Error::Critical(0,QObject::tr("�����E�������Ԃ̍��v��1�𒴂��Ă��܂�"));
	}

	//���Ԑݒ�
	double Tacc_msec = Duration*accl_ratio;
	double Tdec_msec = Duration*decl_ratio;
	double Trun_msec = Duration - ( Tacc_msec + Tdec_msec );

	//�P�ʍ��킹
	Tacc = Tacc_msec / 1000.0;
	Tdec = Tdec_msec / 1000.0;
	Trun = Trun_msec / 1000.0;

	//�t���OOK
	flagDuration = true;
}

void MLinear::SetMode(int _mode)
{
	if(_mode == CONTINUOUS){
		//�A�����샂�[�h�̏ꍇ
		SetDiscreteMove(true);
		std::cout << "���䃂�[�h��A�����샂�[�h�ɐݒ肵�܂���" << std::endl;
	}else if(_mode == JOG){
		//�W���O���[�h�̏ꍇ
		SetDiscreteMove(false);
		std::cout << "���䃂�[�h���W���O���샂�[�h�ɐݒ肵�܂���" << std::endl;
	}else{	
		Error::Critical(0,QObject::tr("���䃂�[�h�̎w��l���s���ł�:%1").arg(mode));
	}
	//�K�p
	mode = _mode;
}

void MLinear::SetDiscreteMove(bool t)
{
	if(t){
		//�A�����샂�[�h�̏�����
		//���荞�݂̗L����...���荞�ݕK�v���H
		retCode = _7443_int_control(nb_axis, 1);
		//���荞�݃C�x���g��L���ɐݒ�
		//retCode = _7443_int_enable(nb_axis, HANDLE *phEvent); //�|�[�����O��p����̂ŗ��p���Ȃ�
		//INT�v�f�̐ݒ�
		unsigned long int_factor = 4;
		retCode = _7443_set_int_factor(nb_axis,4);
		//�A�����샂�[�h�̗L����
		retCode = _7443_set_continuous_move(nb_axis,1);
	}else{
		//���荞�݂̖�����
		retCode = _7443_int_control(nb_axis,0);			
		//�A�����샂�[�h�̖�����
		retCode = _7443_set_continuous_move(nb_axis,0);
	}
}

void MLinear::Prepare()
{
	if(mode == JOG){
		//// �G���[�`�F�b�N
		if(!flagDuration){
			Error::Critical(0, QObject::tr("�� #%1 �W���O���쎞�Ԃ��w�肳��Ă��܂���").arg(uid));
		}

		if(flagPosition && flagVelocity){
			Error::Critical(0, QObject::tr("�� #%1 �W���O����̋����w������ƈʒu�w������������Ɏw�肳��Ă���C������������ł��܂���").arg(uid));			
		}

		if(!flagPosition && !flagVelocity){
			Error::Critical(0, QObject::tr("�� #%1 �W���O����̋����w����ʒu�w����ׂ���Ă��܂���").arg(uid));			
		}

		if(Dist == 0){
			std::cout << "�� #" << uid << " �ړ��ʂ��[���D���̈ړ����߂̓X�L�b�v����܂��D" << std::endl;
		}	

		//// �K�v����f�[�^�̒ǉ��v�Z
		if(flagPosition){
			//d-t�n����v-d�n�֊��Z
			MaxVel = (2.0*abs(Dist)) / (Tacc+Tdec+2.0*Trun);
		}else if(flagVelocity){
			//v-t�n����v-d�n�֊��Z

			//// ... ������ 
		}

	}else if(mode == CONTINUOUS){

		//// �G���[�`�F�b�N
		if(size == 0){
			Error::Critical(0,QObject::tr("�� #%1 �A�����샂�[�h�̐���f�[�^�����݂��܂���D").arg(uid));
		}

		//OK

	}else{
		Error::Critical(0,QObject::tr("�� #%1 ���䃂�[�h�̎w��l���s���ł�:%1").arg(uid,mode));
	}
			
	//�^�]��������
	status.Parameter = Actuator::PREPARED;
}

void MLinear::Commit()
{
	if(status.Parameter == Actuator::WAITING){
		Error::Critical(0,QObject::tr("Axis #%1 Not Prepared()").arg(uid));		
	}

	//// �^�]���s
	if(mode == JOG){

		if(Dist == 0){
			//�ړ������������̂ŉ������Ȃ��ŏI��
			std::cout << "�� #" << uid << " �ړ��ʃ[���D�ړ����߃X�L�b�v�D" << std::endl;
			ParameterInit();
			return;
		}

		//�f�t�H���g�D�����x�^���iS������j�ɂ�銊�炩�ȉ�����
		retCode = _7443_start_sr_move(nb_axis, Dist, StrVel, MaxVel, Tacc, Tdec, SVacc, SVdec);

		if(!retCode){
			std::cout << "�� #" << uid << " �ړ����s, Dist="<< Dist <<", MaxVel=" << MaxVel << std::endl;
		}else{
			ParameterInit();
			Error::Warning(0, QObject::tr("�� #%1 �ړ����s:%2").arg(uid).arg(retCode));
		}

		//�ړ��Ɏg�����p�����[�^�����ׂč폜
		ParameterInit();

		//�W���O�^�]���s�t���O�𗧂Ă�i�^�]�I���Q�l�V�O�i�����o�̂��߂̂݁j
		flagJogOperationStart = true;

	}else if(mode == CONTINUOUS){

		//�A���^�]���s�t���O�𗧂Ă�D�A���^�]���߂� Update() ���ōs����D
		flagContinuousOperationStart = true;
		std::cout << "�� #" << uid << " �A������R�~�b�g�D"<< std::endl;

	}		
}

void MLinear::Update()
{
	if(flagUpdating){
		std::cout << "[ �x�� ] �����������o" << std::endl;
	}
	flagUpdating = true;

	//���̓���X�e�[�^�X���A�b�v�f�[�g
	UpdateStatus();

	//���̈ʒu�X�e�[�^�X
	UpdatePosition();

	if(mode == JOG){
		//JOG����̏ꍇ�C������֐���Commit()��1�񔭍s�����̂݁D�����ł͊Ď��ƍX�V�̂݁D
		if(flagJogOperationStart){
			if(status.Movement == Actuator::STOP){
				//�^�]�I���i�������C����̓p���T�[�w�ł̉^�]�I���ł����āC���ۂ̉��q�͂܂������Ă��邱�Ƃɒ��Ӂj
				flagJogOperationStart = false;
				std::cout << "�W���O���삪�I�����܂���" << std::endl;
				//�Q�l�V�O�i���𑗏o
				emit moveComplete(uid);				
			}
		}
	}else if(mode == CONTINUOUS){
		//�A������̏ꍇ�́C������֐��͂����Ŕ��s����D

		//�A������J�n�t���O�̃`�F�b�N
		if(!flagContinuousOperationStart){
			flagUpdating = false;
			return; //�ȍ~�̏����͎��s����Ȃ�
		}

		if(curr >= size){
			//�A�����쎩���I��
			flagContinuousOperationStart = false;
			flagUpdating = false;

			//�N���b�N���~
			Controller* control = Controller::GetInstance();
			control->GetClock()->Stop();
			std::cout << "�� #" << uid << "�A�����삪�I�����܂���" << std::endl;
			//�Q�l�V�O�i���𑗏o
			emit moveComplete(uid);

			return; //�ȍ~�̏����͎��s����Ȃ�
		}
		//�A������J�n�t���O�������Ă��Ȃ����́C�ȍ~�̏����͎��s����Ȃ�

		//�G���[�`�F�b�N
		if(size == 0){
			Error::Warning(0,QObject::tr("�� #%1 �A�����쐧��f�[�^�T�C�Y���[���ł��D").arg(uid));		
		}

		//�A������f�[�^
		ContinuousControlData *c = 0;

		//�ő�A���������ߐ�
		int cont_max = 0;

		//�A������o�b�t�@���`�F�b�N����
		retCode = _7443_check_continuous_buffer(nb_axis);	

		if(retCode == 0){
			//�S�ċ󂫂Ȃ̂ŘA���O���߂𑗏o����
			cont_max = 3;
		}else if(retCode == 1){
			//1,2���󂫂Ȃ̂ŁC�A���񖽗߂𑗏o����
			cont_max = 2;
		}else if(retCode == 2){
			//2���󂫂Ȃ̂ŁC�ꖽ�߂݂̂𑗏o����
			cont_max = 1;
		}else if(retCode == 3){
			//����p�v�����W�X�^�͖��t�Ȃ̂ő҂�
			cont_max = 0;
		}else{
			Error::Critical(0, QObject::tr("�A������R�}���h�o�b�t�@���o�ŗ\�����Ȃ��߂�l���Ԃ���܂���:%1").arg(retCode));
		}

		int session_start = curr;
		while(curr < session_start + cont_max){
			c = &(continuousControlData[curr]);
			if(c->distance == 0){
				//�ړ��������[���̏ꍇ�͎��ցC�t���O���Ƃ���Update()�I���D
				curr++; 
				flagUpdating = false;
				return;
			}
			//�ړ��������[���ł͖����ꍇ�́C�ړ����s
			retCode = _7443_start_tr_move(nb_axis, c->distance, c->velocity, c->velocity, 0, 0);//tr�n
			if(retCode){
				std::cout << "[ WARNING ] _7443_start_tr_move �G���[: retCode=" << retCode << std::endl;
			}
			curr++;
			if(curr >= size){
				//�I�[�o�[�����h�~�C�t���O���Ƃ���Update()�I���D�N���b�N����̎��̌Ăяo���ŏI�����Ă����D
				std::cout << "[ NOTICE ] �҂�����I��" << std::endl;
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
	//// �ړ���Ԃ̊m��
	int s = _7443_motion_done(nb_axis);
	if(s == 0 || s == 1 || s == 2 || s == 3 || s == 4 || s == 5 || s == 6 || s == 14 || s == 15){
		//�p���T�[�͒�~���i�p���T�[�͉������Ă��Ȃ��j
		unsigned short io_sts = 0;
		_7443_get_io_status(nb_axis, &io_sts);
		if(io_sts & 0x2000){
			//�A���v����~���i�������Ă��Ȃ��j
			status.Movement = Actuator::STOP;
		}else{
			//�A���v�Ŏ����쒆�i�����C�����͕s���j
			status.Movement = Actuator::RUNNING;				
		}
	}else if(s == 7){
		//�o�b�N���b�V���␳��
		status.Movement = Actuator::ADJUSTING;
	}else if(s == 9){
		//���_�ֈړ���
		status.Movement = Actuator::RUNNING;
	}else if(s == 10){
		//�ړ��J�n���x�ňړ���
		status.Movement = Actuator::RUNNING;
	}else if(s == 11){
		//������
		status.Movement = Actuator::ACCL;
	}else if(s == 12){
		//�ō����x�ŉ^�]��
		status.Movement = Actuator::RUNNING;
	}else if(s == 13){
		//������
		status.Movement = Actuator::DECL;
	}else{
		//�}�j���A���ɖ���`
		Error::Critical(0, QObject::tr("�� #%1 �}�j���A������`�X�e�[�^�X�R�[�h���o").arg(uid));
	}
}

void MLinear::UpdatePosition()
{
	//�ʒu�̊m��
	long cpc = 0;
	//�R�}���h�|�W�V�����J�E���^p164�́C�\�t�g���~�b�g�ɂ��Cmax_pulse�𒴂����Ȃ�
	retCode = _7443_get_command(nb_axis, &cpc);
	//[0,100]�ŕԂ�
	status.Position = (double)(cpc*100)/(double)max_pulse;
	//�ʒu�i���f�[�^�G�J���E�f�o�b�O�p�j
	status.PositionRaw = cpc;
}

void MLinear::SetHome()
{
	std::cout << "�� #" << uid << " �z�[���|�W�V�����ړ��D" << std::endl;

	//���_���A�́C�p���T�[�̋@�\�͗��p�����C�T�[�{�I���M���̗�����ɐݒ肵��
	retCode = _7443_set_servo(nb_axis,1);
	if(retCode){
		Error::Critical(0,QObject::tr("�� #%1 SVON�M�����㎸�s�i%2�j�D").arg(uid,retCode));
	}

	//�p���T�[�@�\�𗘗p���Ă��Ȃ����߂̃E�F�C�g
	HardTimer::Wait(10);

	retCode = _7443_set_servo(nb_axis,0);
	if(retCode){
		Error::Critical(0,QObject::tr("�� #%1 SVON�M���������s�i%2�j�D").arg(uid,retCode));
	}

	//�p���T�[�@�\�𗘗p���Ă��Ȃ����߂̃E�F�C�g
	HardTimer::Wait(10);

	//�R�}���h�|�W�V�����J�E���^���[���N���A
	retCode = _7443_set_command(nb_axis,0);
	if(!retCode){
		std::cout << "�� #" << uid << " �R�}���h�|�W�V�����N���A�D" << std::endl;
	}else{
		Error::Critical(0,QObject::tr("�� #%1 �R�}���h�|�W�V�����N���A���s�i%2�j�D").arg(uid,retCode));
	}
}
