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

#ifndef __STRING_RECORDER_HPP__
#define __STRING_RECORDER_HPP__

#include "audioBuffer.hpp"
#include "ui/renderArea.hpp"
#include "timeDomainProfile.hpp"
#include "frequencyDomainProfile.hpp"

#include <QThread>
#include <QAudioDeviceInfo>
#include <QAudioInput>

class Controller;

class StringRecorder : public QThread
{
	Q_OBJECT

	friend class RenderArea;
	friend class Controller;

public:

	StringRecorder();
	~StringRecorder();

	//�^���J�n
	void StartRecord();

	//�^���I��
	void EndRecord();

	//FFT����
	int GetWindowSize() const{ return windowSize; }

	//��{��͒�
	int GetAnalyzeFrameSize() const{ return analyzeFrameLength; }

	//�^�������ǂ���
	bool isRecording() const{ return flagRecording; }

	//�������ێ����郊���O�o�b�t�@����Ԃ�
	int bufsize() const { return a_length; }

	//�o�b�t�@�[��Ԃ�
	AudioBuffer* GetBuffer() const{ return buffer; }

	//// ��͌��ʂ�Ԃ�

	//���ԕ���
	QVector<TimeDomainProfile*> TimeDomainAnalyze(int analyzeLength, double starttimeoffset, double endtimeoffset) const;
	QVector<TimeDomainProfile*> TimeDomainAnalyze(int analyzeLength, double starttimeoffset) const;

	//���g������
	QVector<FrequencyDomainProfile*> FrequencyDomainAnalyze(int analyzeLength, double starttimeoffset, double endtimeoffset) const;
	QVector<FrequencyDomainProfile*> FrequencyDomainAnalyze(int analyzeLength, double starttimeoffset) const;

protected:
	//�������̓X���b�h
	void run();

	//�ێ����Ă���o�b�t�@�Ƙ^���f�o�C�X
	QAudioInput *audio;
	AudioBuffer *buffer;

	//�������ێ������͌��ʕۑ��p�����O�o�b�t�@
	TimeDomainProfile **tdp;      //���ԕ�����̓����O�o�b�t�@
	FrequencyDomainProfile **fdp; //���g��������̓����O�o�b�t�@

	//�������ێ������͌��ʕۑ��p�����O�o�b�t�@�̃T�C�Y
	int a_length;

	//��͌��ʕۑ��p�����O�o�b�t�@�t�����g�ʒu
	volatile unsigned int a_front; 

	//���ԗ̈��͂̎���
	void TimeDomain(double *v, TimeDomainProfile *c);

	//���g���̈��͂̎���
	void FrequencyDomain(double *v, FrequencyDomainProfile *c);

	//�^�����t���O
	bool flagRecording;

private:

	//�T���v�����O���[�g
	static const int samplerate;

	//FFT�E�B���h�E�T�C�Y
	static const int windowSize;

	//��{��͒P�ʒ����i�֋X�I��FFT�E�B���h�E�T�C�Y�̐����{�ɂ��Ă���j
	static const int analyzeFrameLength;

	//����������������
	bool initialized;

};

#endif