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

#ifndef __FREQUENCYDOMAINPROFILE_HPP__
#define __FREQUENCYDOMAINPROFILE_HPP__

#include "../error.hpp"

/*!
 * \brief
 * ���g���̈�̃V�O�i���v���t�@�C�����Ǘ�����N���X.
 * 
 * 
 * \see
 * timeDomainProfile.hpp
 */
class FrequencyDomainProfile
{
public:
	//�^���J�n����̌o�ߎ���[�~���b]
	double timeoffset;
	
	//����F0[0,�i�C�L�X�g���g��]
	double F0;
	
	//�t�[���G�ϊ�����
	double* dft;
	
	//�P�v�X�g�����ϊ�����
	double* cps;

	//FFT�E�B���h�E�T�C�Y
	int windowSize;

	/*!
	 * \brief
	 * �f�t�H���g�R���X�g���N�^
	 * 
	 * \throws <const char *>
	 * ������������Ȃ�
	 * 
	 */
	FrequencyDomainProfile() : timeoffset(0), F0(0), windowSize(1024)
	{
		dft = (double*)calloc(windowSize, sizeof(double));
		if(dft == 0){
			Error::Critical(0, QObject::tr("FrequencyDomainProfile �m�ے��Ƀ�����������܂���(1)"));
		}
		cps = (double*)calloc(windowSize, sizeof(double));
		if(cps == 0){
			Error::Critical(0, QObject::tr("FrequencyDomainProfile �m�ے��Ƀ�����������܂���(2)"));
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
	FrequencyDomainProfile(int _windowSize) : timeoffset(0), F0(0), windowSize(_windowSize)
	{
		dft = (double*)calloc(windowSize, sizeof(double));
		if(dft == 0){
			Error::Critical(0, QObject::tr("FrequencyDomainProfile �m�ے��Ƀ�����������܂���(3)"));
		}
		cps = (double*)calloc(windowSize, sizeof(double));
		if(cps == 0){
			Error::Critical(0, QObject::tr("FrequencyDomainProfile �m�ے��Ƀ�����������܂���(4)"));
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
	FrequencyDomainProfile(const FrequencyDomainProfile& obj)
	{
		windowSize = 1024;
		timeoffset = obj.timeoffset;
		F0 = obj.F0;

		dft = (double*)calloc(windowSize, sizeof(double));
		if(dft == 0){
			Error::Critical(0, QObject::tr("FrequencyDomainProfile �m�ے��Ƀ�����������܂���(5)"));
		}
		cps = (double*)calloc(windowSize, sizeof(double));
		if(cps == 0){
			Error::Critical(0, QObject::tr("FrequencyDomainProfile �m�ے��Ƀ�����������܂���(6)"));
		}
		//�R�s�[
		for(int i=0;i<windowSize;i++){
			dft[i] = obj.dft[i];
			cps[i] = obj.cps[i];
		}
	}

	~FrequencyDomainProfile()
	{
		delete [] dft;
		dft = 0;
		delete [] cps;
		cps = 0;
	}

	FrequencyDomainProfile operator+(const FrequencyDomainProfile& obj)
	{
		FrequencyDomainProfile* tmp = new FrequencyDomainProfile(windowSize);
		tmp->timeoffset = timeoffset;
		tmp->F0 = F0 + obj.F0;
		for(int i=0;i<windowSize;++i){
			tmp->dft[i] = dft[i] + obj.dft[i];
			tmp->cps[i] = cps[i] + obj.cps[i];
		}			
		return *tmp;
	}
	
	FrequencyDomainProfile operator-(const FrequencyDomainProfile &obj)
	{
		FrequencyDomainProfile* tmp = new FrequencyDomainProfile(windowSize);
		tmp->timeoffset = timeoffset;
		tmp->F0 = F0 - obj.F0;
		for(int i=0;i<windowSize;++i){
			tmp->dft[i] = dft[i] - obj.dft[i];
			tmp->cps[i] = cps[i] - obj.cps[i];
		}			
		return *tmp;
	}

	void dump()
	{
		double dftsum = 0;
		double cpssum = 0;
		for(int i=0;i<windowSize;++i){
			dftsum += dft[i];
			cpssum += cps[i];
		}			
		std::cout << "dftsum:" << dftsum << ", cpssum:" << cpssum << std::endl;
	}

	void add(const FrequencyDomainProfile *obj)
	{
		F0 = F0 + obj->F0;
		for(int i=0;i<windowSize;++i){
			dft[i] = dft[i] + obj->dft[i];
			cps[i] = cps[i] + obj->cps[i];
		}
	}

	FrequencyDomainProfile& operator+=(const FrequencyDomainProfile &obj)
	{
		F0 = F0 + obj.F0;
		for(int i=0;i<windowSize;++i){
			dft[i] = dft[i] + obj.dft[i];
			cps[i] = cps[i] + obj.cps[i];
		}			
		return *this;
	}

	FrequencyDomainProfile& operator-=(const FrequencyDomainProfile &obj)
	{
		F0 = F0 - obj.F0;
		for(int i=0;i<windowSize;++i){
			dft[i] = dft[i] - obj.dft[i];
			cps[i] = cps[i] - obj.cps[i];
		}			
		return *this;
	}
};

#endif