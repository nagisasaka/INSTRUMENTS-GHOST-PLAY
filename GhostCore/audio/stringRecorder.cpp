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

#include "stringRecorder.hpp"
#include "../error.hpp"
#include "audioBuffer.hpp"
#include "audioAnalyzer.hpp"

//�T���v�����O���[�g
const int StringRecorder::samplerate = 44100;

//FFT�E�B���h�E��(byte) 2�̗ݏ�
const int StringRecorder::windowSize = 1024;

//��{��͒��i�E�B���h�E�T�C�Y�̒萔�{�j�G���̃t���[���̕��ϒl����i�����ɗp����
const int StringRecorder::analyzeFrameLength = StringRecorder::windowSize;

/*!
 * \brief
 * �X�g�����O���R�[�_�[�D���̐U�����L�^�E��͂��邽�߂̃N���X�D
 * 
 * \remarks
 * ���ԑ΃o�C�g������
 *	samplesize	samplerate	bps	    byte per sec	kbyte per sec  
 *	2[byte]		44100[Hz]	705600	88200	        88.2		    
 *�y�����z
 * 1024 ��b��:
 * 1024/88200  = 0.0116�b�i100����1�b������Ɓj
 * 
 * \see
 * audioBuffer.hpp|audioAnalyzer.hpp
 */
StringRecorder::StringRecorder()
{
	audio = 0;
	buffer = 0;
	tdp = 0;
	fdp = 0;
	a_length = 0; //StartRecord()���ŏ�����
	flagRecording = false;
	initialized = false;
}

StringRecorder::~StringRecorder()
{
	delete audio;
	delete buffer;
	audio = 0;
	buffer = 0;
}

void StringRecorder::StartRecord()
{
	//PCM 44.1k/16bit
	QAudioFormat format;
	format.setCodec("audio/pcm");
	format.setSampleRate(samplerate);
	format.setSampleSize(16);
	format.setChannels(1);
	format.setSampleType(QAudioFormat::SignedInt);
	format.setByteOrder(QAudioFormat::LittleEndian);

	std::cout << "[ STRING RECORDER ] �t�H�[�}�b�g�ݒ�" << std::endl;
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	if(!info.isFormatSupported(format)){
		Error::Critical(0, QObject::tr("�J�����g�̃I�[�f�B�I�f�o�C�X�����݂��Ȃ����C�K�v�Ș^���t�H�[�}�b�g�ɑΉ����Ă��܂���D"));
	}

	//�I�[�f�B�I�o�b�t�@�̊m��
	buffer = new AudioBuffer(format,this);
	//�^������
	audio = new QAudioInput(format, this);
	//�I�[�f�B�I�o�b�t�@�̊J�n
	buffer->start();
	//�^���J�n
	audio->start(buffer);
	//�������ێ������͌��ʕۑ��p�����O�o�b�t�@�̊m��
	//���Ƃ��Ƃ̘^�������O�o�b�t�@��analyzeFrameLength�����ɕ��f���ď�������̂�
	a_length = buffer->bufsize() / analyzeFrameLength + 1; // ��͌��ʕۑ��p�����O�o�b�t�@�̒���

	//// ��͌��ʕۑ��p�����O�o�b�t�@�m��

	//���ԗ̈�
	std::cout << "[ STRING RECORDER ] ��͌��ʕۑ��p�����O�o�b�t�@�m��" << std::endl;
	std::cout << "[ STRING RECORDER ] ���ԕ���: " << a_length*sizeof(TimeDomainProfile)/1000000. + a_length*analyzeFrameLength*sizeof(double)/1000000. << "MB�i�o�b�t�@��: "<< a_length << "�j �������J�n..." << std::endl;
	tdp = new TimeDomainProfile*[a_length];
	for(int i=0;i<a_length;i++){
		tdp[i] = new TimeDomainProfile(analyzeFrameLength);
	}

	//���g���̈�
	std::cout << "[ STRING RECORDER ] ���g������: " << a_length*sizeof(FrequencyDomainProfile)/1000000. + a_length*windowSize*sizeof(double)*2/1000000.<< "MB�i�o�b�t�@��:" << a_length << "�j �������J�n..."<< std::endl;
	fdp = new FrequencyDomainProfile*[a_length];
	for(int i=0;i<a_length;i++){
		fdp[i] = new FrequencyDomainProfile(windowSize);
	}

	std::cout << "[ STRING RECORDER ] �������I��" << std::endl;
	//���̓X���b�h�J�n
	start();	
}

void StringRecorder::EndRecord()
{
	flagRecording = false;
}


/*!
 * \brief
 * ��̓X���b�h�̎����i�O���ϐ��̎�舵���ɒ��Ӂj
 * 
 * �^���f�o�C�X�̃����O�o�b�t�@����C�ݒ肳�ꂽ�Œ蒷�̊�{��͒����Ƃɉ�͂��s��������D
 * 
 */
void StringRecorder::run()
{
	std::cout << "[ STRING RECORDER ] ��̓X���b�h�J�n Thread ID: " << QThread::currentThreadId() << std::endl;
	//����������
	initialized = true;
	flagRecording = true;
	//�O��|�[�����O���̃t�����g�l�i����͖{���͑O��|�[�����O���̃t�����g�ʒu�����C�O���͊����ʒu�Ƃ��Ă��p����j
	unsigned long prev_front = 0;
	//�V�K��͗p�ꎞ�o�b�t�@�i�K����{���͒��ɂȂ�j
	double* analyze = (double*)calloc(analyzeFrameLength,sizeof(double));
	//�����O�o�b�t�@�T�C�Y
	unsigned int buflen = buffer->bufsize();
	//��͌��ʕۑ��p�����O�o�b�t�@�t�����g�ʒu��������
	a_front = 0;
	while(flagRecording)
	{
		//�����O�o�b�t�@���č\�����Ď擾
		double *buf = buffer->buffer();
		//���݂̃����O�o�b�t�@���t�����g�ʒu�̎擾
		unsigned int front  = buffer->front();
		//�^���J�n����̒ʎZ�ʒu
		unsigned long long offset = buffer->offset();
		//���~���b���Z
		double timeoffset = offset*(1000./buffer->format().sampleRate());
		//�V�K��͗p�o�b�t�@�̃C���f�b�N�X
		int index = 0;

		//�t�����g�ʒu���ς���Ă��Ȃ���ΓK���ɑ҂�
		if(front == prev_front){
			msleep(100);
			continue;
		}
		//�����O�o�b�t�@���獡��V���Ɏ擾���ꂽ��Ԃ� analyze �z��ɑ}������
		if(prev_front < front){
			//�����O�o�b�t�@���ŒP�������̏ꍇ
			for(int i = prev_front;i<=front;i++){
				if(index == analyzeFrameLength){
					prev_front = i; // ��̓t�����g�ʒu���L�^
					break;
				}
				analyze[index] = buf[i];
				index++;
			}
		}else{
			//�[�����܂������ꍇ
			for(int i = prev_front;i<buflen;i++){
				if(index == analyzeFrameLength){
					prev_front = i; // ��̓t�����g�ʒu���L�^
					break;
				}
				analyze[index] = buf[i];
				index++;
			}
			if(index >= analyzeFrameLength){
				//��Ńu���[�N���Ă����炱��ȏ㌩�Ȃ�
			}else{
				for(int i= 0;i<front;i++){
					if(index == analyzeFrameLength){
						prev_front = i; // ��̓t�����g�ʒu���L�^
						break;
					}
					analyze[index] = buf[i];
					index++;
				}
			}
		}

		//�^�������O�o�b�t�@����V���Ɏ擾���ꂽ��Ԃ���̓o�b�t�@�̍Œ�T�C�Y�ȉ��̏ꍇ�C�K���ɑ҂�
		if(index < analyzeFrameLength){
			msleep(100);
			continue;
		}

		//������ analyze �x�N�^�̃T�C�Y�͏�� analyzeFrameLength �ɑ����Ă��邱�Ƃ��ۏ؂����
		if(index != analyzeFrameLength){
			std::cerr << "ASSERT: index=" << index << ", analyzeFrameLength:" << analyzeFrameLength << std::endl;
			Q_ASSERT(index != analyzeFrameLength);
		}

		//// ��͎��s

		//���̉�̓o�b�t�@�S�̂̕��ρE�ő�l��
		TimeDomain(analyze, tdp[a_front]);
		tdp[a_front]->timeoffset = timeoffset;

		//���̉�̓o�b�t�@�S�̂�FZERO��
		FrequencyDomain(analyze, fdp[a_front]);
		fdp[a_front]->timeoffset = timeoffset;

		a_front++;
		if(a_front >= a_length){			
			a_front = 0; // �����O�o�b�t�@
		}
	}

	//��̓o�b�t�@�s�v
	free(analyze);
}


/*!
 * \brief
 * ���ԕ����̉�͏����̎����D
 * 
 * \param v
 * ��͑Ώ۔͈͂̃T���v���C�l��[0,1]�ɐ��K�����Ă����K�v������D
 *
 * \param c
 * ������������TimeDomainProfile�ւ̃|�C���^�i����������Ă���K�v������j
 * 
 * \remarks
 * ���ԗ̈�ł͍��͒P���Ȃ��Ƃ������Ă��Ȃ����߁C�x�^�������Ă��邪�C���������������Ȃ�CaudioAnalyzer.hpp �Ɉڊǂ���ׂ��D 
 *
 */
void StringRecorder::TimeDomain(double* v, TimeDomainProfile* c)
{
	double sum = 0;
	double m_max = 0;
	double p = 0;
	for(int i=0;i<analyzeFrameLength;i++){
		//�ő�U��
		if(m_max < qAbs(v[i])){
			m_max = qAbs(v[i]);
		}
		//�U�����v
		//�p���[���v
		p += v[i]*v[i];
		sum += qAbs(v[i]);
		//�f�[�^
		c->subwindow[i] = v[i];
	}

	//�ő�U���i�ő�U����ێ�����j
	c->maxAmplitude = m_max;
	//���ϐU��
	c->averageAmplitude = sum/(double)analyzeFrameLength;
	//�ő�p���[
	c->maxPower = m_max*m_max;
	//���σp���[
	c->averagePower = p/(double)analyzeFrameLength;
	//�������̂݁i�Ăяo�����Œl�������܂��j
	c->timeoffset= 0;
}


/*!
 * \brief
 * ���g�������̉�͏����̎����i�P�̉�͂ɂ��āj
 * 
 * \param v
 * ��͑Ώ۔͈͂̃T���v���C�l��[0,1]�ɐ��K�����Ă����K�v������D
 * 
 * \returns
 * ��͌��ʂ̎��ԕ����v���t�@�C���\���́i�P�̉�͂�Ԃ��j
 * 
 * \see
 * audioAnalyzer.hpp
 *
 */
void StringRecorder::FrequencyDomain(double *v, FrequencyDomainProfile *c)
{
	//�P�v�X�g�������
	QVector<double> cps  = AudioAnalyzer::CPS(v, analyzeFrameLength, windowSize);
	QVector<double> alog = AudioAnalyzer::ALOG(v, analyzeFrameLength, windowSize);
	for(int i=0;i<windowSize;i++){
		c->cps[i] = cps[i];
		c->dft[i] = alog[i];
	}
	//F0
	c->F0 = AudioAnalyzer::F0(cps, samplerate, windowSize);
	//�������i�Ăяo�����Œl�������܂��j
	c->timeoffset = 0;
}

/*!
 * \brief
 * ���ԗ̈�̉�͌��ʂ�C�ӂ̉�͒��ōĉ�͂��ĕԂ��i���[�e�B���e�B�֐��j
 * 
 * \param analyzeLength
 * �ĉ�͒��i��������{��͒��ł̉�͌��ʂ�p����̂ōĉ�͍͂����Ɏ��s�ł���j
 * ��{��͒��ȏ�C���C��{��͒��̐����{�̒l���w�肵�Ȃ���΂Ȃ�Ȃ��D0���w�肷��ƁC��{��͒��̌��ʂ����̂܂ܕԂ��D
 * 
 * \param offset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 * 
 * \returns
 * �ĉ�͌���
 * 
 */
QVector<TimeDomainProfile*> StringRecorder::TimeDomainAnalyze(int analyzeLength, double timeoffset) const
{
	int index = 0;
	if(a_front == 0){
		index = 0;
	}else{
		index = a_front-1;
	}
	double endoffset = tdp[index]->timeoffset;
	return TimeDomainAnalyze(analyzeLength, timeoffset, endoffset);
}

/*!
 * \brief
 * ���ԗ̈�̉�͌��ʂ�C�ӂ̉�͒��ōĉ�͂��ĕԂ��i�{�̎����j
 * 
 * \param analyzeLength
 * �ĉ�͒��i��������{��͒��ł̉�͌��ʂ�p����̂ōĉ�͍͂����Ɏ��s�ł���j
 * ��{��͒��ȏ�C���C��{��͒��̐����{�̒l���w�肵�Ȃ���΂Ȃ�Ȃ��D0���w�肷��ƁC��{��͒��̌��ʂ����̂܂ܕԂ��D
 * 
 * \param startoffset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 *
 * \param endoffset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 *
 * \returns
 * �ĉ�͌���
 * 
 */
QVector<TimeDomainProfile*> StringRecorder::TimeDomainAnalyze(int analyzeLength, double timeoffset, double endtimeoffset) const
{

	//�s���Ȓl���`�F�b�N
	if(analyzeLength != 0 && analyzeLength < analyzeFrameLength){
		//���݂̎d�l�D�Z�����邱�Ƃ��{���͏o����̂ŁC�������Ă��ǂ��D
		std::cerr << "[ STRING RECORDER ] TimeDomainAnalyze() �������Ŏw�肳�ꂽ�ĉ�͒�����{��͒���������Ă��܂��i��{��͒�:" << analyzeFrameLength << " > �w�蒷:" << analyzeLength << "�j"<< std::endl;		
		Q_ASSERT(false);
	}
	if(analyzeLength != 0 && analyzeLength % analyzeFrameLength != 0){
		std::cerr << "[ STRING RECORDER ] TimeDomainAnalyze() �������Ŏw�肳�ꂽ�ĉ�͍s����{��͒��̐����{�ɂȂ��Ă��Ȃ����ߌ��ʂɌ����������܂��D" << std::endl;
	}
	//�߂�l
	QVector<TimeDomainProfile*> result_tdp;

	//����������Ă��Ȃ��ꍇ
	if(!initialized){
		return result_tdp;//�ʂ�
	}

	//�ꎞ�ϐ�
	QVector<TimeDomainProfile*> temp; 

	//��͌��ʕۑ��p�����O�o�b�t�@�̒����V�[�P���V�����X�L����
	//�Â����̐�[�� a_front �ł���
	for(int i=a_front;i<a_length;i++){
		if(timeoffset <= tdp[i]->timeoffset && tdp[i]->timeoffset <= endtimeoffset){
			temp.append(tdp[i]);
		}
	}

	//�[���N���X���ĐV��������
	for(int i=0;i<a_front;i++){
		if(timeoffset <= tdp[i]->timeoffset && tdp[i]->timeoffset <= endtimeoffset){
			temp.append(tdp[i]);
		}
	}

	//���݂̃����O�o�b�t�@�̒��ɂ͎w�肳�ꂽ�^�C���I�t�Z�b�g�ȍ~�̌��ʂ͓����Ă��Ȃ������i�������w�肵���j
	if(temp.size() == 0){
		std::cerr << "[ STRING RECORDER ] TimeDomainAnalyze() ���ԃI�t�Z�b�g�̎w�肪�����ł�( timeoffset:" << timeoffset << " endtimeoffset:" << endtimeoffset << " )" << std::endl;
		std::cerr << "a_front: " << a_front << " ���ӂ̃^�C���I�t�Z�b�g�l���_���v���܂�." << std::endl;
		int j = 0;
		for(int i=a_front-10;i<a_length;i++){
			std::cout << "i:" << i << " t: " << tdp[i]->timeoffset << std::endl;
			j++;
			if(j > 20) break;
		}
		Error::Critical(0, QObject::tr("TimeDomainAnalyze() ���ԃI�t�Z�b�g�w��G���["));
	}

	//�[���w��̏ꍇ�͊�{��͒��̌��ʂ����̂܂ܕԂ�
	if(analyzeLength == 0){
		return temp;
	}

	//// ���͋�ԍč\���i������������̂݁D�Z������ꍇ�͕ʓr�C�ꍇ�����Ŏ������K�v�j

	//�T�u�E�B���h�E�����́C�����O�o�b�t�@�[�̂P�v�f�����H
	double subcount = analyzeLength / analyzeFrameLength;

	//���όv�Z�p
	int j = 0;
	TimeDomainProfile *avg = temp[0];
	for(int i=1;i<temp.size();i++){
		(*avg) += *(temp[i]);
		j++;
		if(j >= subcount){
			avg->averageAmplitude /= subcount;
			avg->averagePower /= subcount;
			//����
			for(int i=0;i<analyzeFrameLength;i++){
				avg->subwindow[i] /= subcount;
			}
			//�č\�����ʂ�߂�l�ɕۑ�
			result_tdp.append(avg);
			j = 0;
		}
	}
	//���͋�Ԃ��č\���������ʂ�Ԃ�
	return result_tdp;
}


/*!
 * \brief
 * ���g���̈�̉�͌��ʂ�Ԃ��i���[�e�B���e�B�֐��j
 *
 * \param analyzeLength
 * �ĉ�͒��i��������{��͒��ł̉�͌��ʂ�p����̂ōĉ�͍͂����Ɏ��s�ł���j
 * ��{��͒��ȏ�C���C��{��͒��̐����{�̒l���w�肵�Ȃ���΂Ȃ�Ȃ��D0���w�肷��ƁC��{��͒��̌��ʂ����̂܂ܕԂ��D
 * 
 * \param timeoffset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 *
 * \param endtimeoffset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 *
 * \returns
 * ���g���̈�̉�͌���
 * 
 * \remarks
 * ���g���̈�̉�͌��ʂ͉�͒�����C�ӎw�肷�邱�Ƃ͂ł��Ȃ��d�l�D
 * 
 */
QVector<FrequencyDomainProfile*> StringRecorder::FrequencyDomainAnalyze(int analyzeLength, double timeoffset) const
{
	int index = 0;
	if(a_front == 0){
		index = 0;
	}else{
		index = a_front - 1;
	}
	double endtimeoffset = fdp[index]->timeoffset;
	return FrequencyDomainAnalyze(analyzeLength, timeoffset, endtimeoffset);
}

/*!
 * \brief
 * ���g���̈�̉�͌��ʂ�Ԃ��i�{�̎����j
 *
 * \param analyzeLength
 * �ĉ�͒��i��������{��͒��ł̉�͌��ʂ�p����̂ōĉ�͍͂����Ɏ��s�ł���j
 * ��{��͒��ȏ�C���C��{��͒��̐����{�̒l���w�肵�Ȃ���΂Ȃ�Ȃ��D0���w�肷��ƁC��{��͒��̌��ʂ����̂܂ܕԂ��D
 * 
 * \param timeoffset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 *
 * \param endtimeoffset
 * �^���J�n����̌o�ߎ���[�~���b]�i���̎��Ԉȍ~�̃f�[�^���Ԃ���鏈���ΏۂƂȂ�j
 *
 * \returns
 * ���g���̈�̉�͌���
 * 
 * \remarks
 * ���g���̈�̉�͌��ʂ͉�͒�����C�ӎw�肷�邱�Ƃ͂ł��Ȃ��d�l�D
 * 
 */
QVector<FrequencyDomainProfile*> StringRecorder::FrequencyDomainAnalyze(int analyzeLength, double timeoffset, double endtimeoffset) const
{
	//�s���Ȓl���`�F�b�N
	if(analyzeLength != 0 && analyzeLength < analyzeFrameLength){
		//���݂̎d�l�D�Z�����邱�Ƃ��{���͏o����̂ŁC�������Ă��ǂ��D
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() �������Ŏw�肳�ꂽ�ĉ�͒�����{��͒���������Ă��܂��i��{��͒�:" << analyzeFrameLength << " > �w�蒷:" << analyzeLength << "�j"<< std::endl;		
		Q_ASSERT(false);
	}
	if(analyzeLength != 0 && analyzeLength % analyzeFrameLength != 0){
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() �������Ŏw�肳�ꂽ�ĉ�͍s����{��͒��̐����{�ɂȂ��Ă��Ȃ����ߌ��ʂɌ����������܂��D" << ( analyzeLength%analyzeFrameLength )<<std::endl;
	}

	//�߂�l
	QVector<FrequencyDomainProfile*> result_fdp;

	if(!initialized){
		std::cout << "[ STRING RECORDER ] FrequencyDomainAnalyze() �������O�ɌĂяo����܂���" << std::endl;
		return result_fdp;
	}
	
	//�ꎞ�ϐ�
	QVector<FrequencyDomainProfile*> temp;

	//��͌��ʕۑ��p�����O�o�b�t�@�̒����V�[�P���V�����X�L����
	//�Â����̐�[�� a_front ����
	for(int i=a_front;i<a_length;i++){
		if(timeoffset <= fdp[i]->timeoffset && fdp[i]->timeoffset <= endtimeoffset){
			temp.append(fdp[i]);
		}
	}
	for(int i=0;i<a_front;i++){
		if(timeoffset <= fdp[i]->timeoffset && fdp[i]->timeoffset <= endtimeoffset){
			temp.append(fdp[i]);
		}
	}

	//���݂̃����O�o�b�t�@�̒��ɂ͎w�肳�ꂽ�^�C���I�t�Z�b�g�ȍ~�̌��ʂ͓����Ă��Ȃ������i�������w�肵���j
	if(temp.size() == 0){
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() ���ԃI�t�Z�b�g�̎w�肪�ُ�( timeoffset:" << timeoffset << " endtimeoffset:" << endtimeoffset << " )" << std::endl;
		Error::Critical(0, QObject::tr("FrequencyDomainAnalyze() ���ԃI�t�Z�b�g�w��G���["));
		return result_fdp;
	}

	//�ĉ�͂��Ȃ��̂Ŋ�{��͒��̌��ʂ����̂܂ܕԂ�
	if(analyzeLength == 0){
		return temp;
	}

	//// �ĉ��
	
	double subcount = analyzeLength / analyzeFrameLength;
	//���όv�Z�p
	int j = 0;
	FrequencyDomainProfile *avg = new FrequencyDomainProfile(windowSize);
	for(int i=0;i<temp.size();i++){
		avg->add(temp[i]);
		j++;
		if(j >= subcount){
			//����
			avg->F0 = avg->F0/subcount;
			for(int k=0;k<windowSize;k++){
				avg->cps[k] = avg->cps[k]/subcount;
				avg->dft[k] = avg->dft[k]/subcount;
			}
			//�č\�����ʂ�߂�l�ɕۑ�
			result_fdp.append(avg);
			avg = new FrequencyDomainProfile(windowSize);
			j = 0;
		}
	}
	//�Ō�̃������A���P�[�V�����͗]�v
	delete avg;

	if(result_fdp.size() == 0){
		std::cerr << "[ STRING RECORDER ] FrequencyDomainAnalyze() ���ʂ��ُ�( timeoffset:" << timeoffset << " endtimeoffset:" << endtimeoffset << " )" << std::endl;
		Error::Critical(0, QObject::tr("FrequencyDomainAnalyze() ���g���ĉ�͎��s�G���["));
	}

	//���͋�Ԃ��č\���������ʂ�Ԃ�
	return result_fdp;
}


