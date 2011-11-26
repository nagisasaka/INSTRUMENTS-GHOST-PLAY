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

#ifndef __TIMEDOMAINPROFILE_HPP__
#define __TIMEDOMAINPROFILE_HPP__

#include "../error.hpp"

/*!
 * \brief
 * �^�C���h���C���̃v���t�@�C������ۑ�����N���X
 * 
 * 
 * \remarks
 * subwindow �ɂ́A�I���W�i�������O�o�b�t�@�̒f�Ђ��ۑ�����Ă��邽�߁A���������[�N�ɒ��ӂ���B
 * 
 * \see
 * frequencyDomainProfile.hpp
 */
class TimeDomainProfile
{
public:

	//�^���J�n����̌o�ߎ���[�~���b]
	double timeoffset;

	//�ő�U��[0,1]
	double maxAmplitude;

	//���ϐU��[0,1]
	double averageAmplitude;

	//�ő�p���[[0,1]
	double maxPower;

	//���σp���[[0,1]
	double averagePower;

	//�T�u�E�B���h�E�Ɋ܂܂�鑪��f�[�^�C�v�f��[0,1]
	double *subwindow;

	//���͒��i�T�u�E�B���h�E�����j
	int analyzeFrameLength;

	/*!
	 * \brief
	 * �f�t�H���g�R���X�g���N�^
	 * 
	 * \throws <const char *>
	 * ������������Ȃ�
	 * 
	 */
	TimeDomainProfile() : timeoffset(0), maxAmplitude(0), averageAmplitude(0), maxPower(0), averagePower(0), analyzeFrameLength(4096)
	{
		subwindow = (double*)calloc(analyzeFrameLength, sizeof(double));
		if(subwindow == 0){
			Error::Critical(0, QObject::tr("TimeDomainProfile �m�ے��Ƀ�����������܂���(1)"));
		}
	}

	/*!
	 * \brief
	 * �ʏ험�p����R���X�g���N�^
	 * 
	 * \throws <const char *>
	 * ������������Ȃ�
	 * 
	 */
	TimeDomainProfile(int _analyzeFrameLength) : timeoffset(0), maxAmplitude(0), averageAmplitude(0), maxPower(0), averagePower(0), analyzeFrameLength(_analyzeFrameLength)
	{
		subwindow = (double*)calloc(analyzeFrameLength, sizeof(double));
		if(subwindow == 0){
			Error::Critical(0, QObject::tr("TimeDomainProfile �m�ے��Ƀ�����������܂���(2)"));
		}
	}

	/*!
	 * \brief
	 * �R�s�[�R���X�g���N�^
	 * 
	 * \throws <const char *>
	 * ������������Ȃ�
	 * 
	 */
	TimeDomainProfile(const TimeDomainProfile& obj)
	{
		timeoffset = obj.timeoffset;
		maxAmplitude = obj.maxAmplitude;
		averageAmplitude = obj.averageAmplitude;
		maxPower = obj.maxPower;
		averagePower = obj.averagePower;

		subwindow = (double*)calloc(analyzeFrameLength, sizeof(double));
		if(subwindow == 0){
			Error::Critical(0, QObject::tr("TimeDomainProfile �m�ے��Ƀ�����������܂���(3)"));
		}
		for(int i=0;i<analyzeFrameLength;++i){
			subwindow[i] = obj.subwindow[i];
		}
	}

	/*!
	 * \brief
	 * �f�X�g���N�^, ���������[�N�ɒ��ӂ��Ă�������. 
	 * 
	 */
	~TimeDomainProfile()
	{
		delete [] subwindow;
		subwindow = 0;
	}

	TimeDomainProfile operator+(const TimeDomainProfile& obj)
	{
		TimeDomainProfile* tmp = new TimeDomainProfile(analyzeFrameLength);
		//���ϐU��
		tmp->averageAmplitude = averageAmplitude + obj.averageAmplitude;  // �P�������Z
		//���σp���[
		tmp->averagePower = averagePower + obj.averagePower;			  // �P�������Z
		//�ő�U��
		tmp->maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);		  // �����͒P���ȑ����Z�ł͂Ȃ��ł��i���̈���������j
		//�ő�p���[
		tmp->maxPower = qMax(maxPower,obj.maxPower);					  // �������P���ȑ����Z�ł͂Ȃ���i���̈���������j
		//���ԃI�t�Z�b�g�͎����̎��ԃI�t�Z�b�g�������p��
		tmp->timeoffset = timeoffset;
		//�T���v���̉𑜓x���ێ������܂ܑ����Z
		for(int i=0;i<analyzeFrameLength;i++){
			tmp->subwindow[i] = subwindow[i] + obj.subwindow[i];
		}
		return *tmp;
	}
	
	TimeDomainProfile operator-(const TimeDomainProfile &obj)
	{
		TimeDomainProfile* tmp = new TimeDomainProfile(analyzeFrameLength);
		//���ϐU��
		tmp->averageAmplitude = averageAmplitude - obj.averageAmplitude;  // �P�������Z
		//���σp���[
		tmp->averagePower = averagePower - obj.averagePower;			  // �P�������Z
		//�ő�U��
		tmp->maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);		  // �����͒P���Ȉ����Z�ł͂Ȃ�
		//�ő�p���[
		tmp->maxPower = qMax(maxPower,obj.maxPower);					  // �������P���Ȉ����Z�ł͂Ȃ�
		//���ԃI�t�Z�b�g�͎����̎��ԃI�t�Z�b�g�������p��
		tmp->timeoffset = timeoffset;
		//�T���v���̉𑜓x���ێ������܂܈����Z
		for(int i=0;i<analyzeFrameLength;i++){
			tmp->subwindow[i] = subwindow[i] - obj.subwindow[i];
		}
		return *tmp;
	}

	TimeDomainProfile operator+=(const TimeDomainProfile &obj)
	{
		//���ϐU��
		averageAmplitude = averageAmplitude + obj.averageAmplitude;
		//���σp���[
		averagePower = averagePower + obj.averagePower;
		//�ő�U��
		maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);
		//�ő�p���[
		maxPower = qMax(maxPower,obj.maxPower);
		//���ԃI�t�Z�b�g�͎����̎��ԃI�t�Z�b�g�������p��
		timeoffset = timeoffset;
		//�T���v���̉𑜓x���ێ������܂܈����Z
		for(int i=0;i<analyzeFrameLength;i++){
			subwindow[i] = subwindow[i] + obj.subwindow[i];
		}
		return *this;	
	}

	TimeDomainProfile operator-=(const TimeDomainProfile &obj)
	{
		//���ϐU��
		averageAmplitude = averageAmplitude - obj.averageAmplitude;
		//���σp���[
		averagePower = averagePower - obj.averagePower;
		//�ő�U��
		maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);
		//�ő�p���[
		maxPower = qMax(maxPower,obj.maxPower);
		//���ԃI�t�Z�b�g�͎����̎��ԃI�t�Z�b�g�������p��
		timeoffset = timeoffset;
		//�T���v���̉𑜓x���ێ������܂܈����Z
		for(int i=0;i<analyzeFrameLength;i++){
			subwindow[i] = subwindow[i] - obj.subwindow[i];
		}
		return *this;
	}
};

#endif