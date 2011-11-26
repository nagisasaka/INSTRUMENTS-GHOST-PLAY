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

#include "audioBuffer.hpp"
#include <QAudioFormat>
#include <QIODevice>
#include <QtCore/qendian.h>

#include <iostream>

AudioBuffer::AudioBuffer(const QAudioFormat &format, QObject *parent)
     :   QIODevice(parent)
     ,   m_format(format)
     ,   m_maxAmplitude(0)
     ,   m_level(0.0)
	 ,   m_bufsize(0)
	 ,   m_front(0)
	 ,	 init(false)
{

	//�o�O
	//1�b�Ԃ̃o�C�g��
	//double byte_per_sec = m_format.sampleSize() * 8 * m_format.channelCount() * m_format.sampleRate();
	//  1�b�Ԃ̃o�C�g�� =       16[bit]         x 8 x         1[channel]      x       44.1[kHz]

	//�����O�o�b�t�@���̂͗Ⴆ��10�b�̒����������Ă���
	//�o�b�t�@�[�̒���[�b]
	double buf_len_sec  = 10; // [sec]

	//�o�b�t�@�T�C�Y�i�T���v�����G1�b�Ԃ̃T���v�����[�g�������O�o�b�t�@�ɋL�^�����������j
	m_bufsize = buf_len_sec * m_format.sampleRate();
	//�o�b�t�@�̊m��
	m_buffer = (double*)calloc(m_bufsize, sizeof(double));  // �P�̃T���v����double�Ŏ�舵���̂�

	std::cout << "[ AUDIO BUFFER ] �^�������O�o�b�t�@�T�C�Y: " << m_bufsize*sizeof(double)/1000000. << "MB" << std::endl;

	//�ʎZ�ʒu�̃N���A
	m_offset = 0;

	//�ő�l�`�F�b�N
	switch (m_format.sampleSize()) {
    case 8:
         switch (m_format.sampleType()) {
         case QAudioFormat::UnSignedInt:
             m_maxAmplitude = 255;
			 break;
         default:
			 Q_ASSERT(false);
             break;
         }
         break;
    case 16:
         switch (m_format.sampleType()) {
         case QAudioFormat::UnSignedInt:
             m_maxAmplitude = 65535;
             break;
         default:
             m_maxAmplitude = 32767;
             break;
         }
         break;
    default:
		Q_ASSERT(false);
        break;
    }

	init = true;
}

 AudioBuffer::~AudioBuffer()
 {
	 free(m_buffer);
	 m_buffer = 0;
 }
 
 double* AudioBuffer::buffer()
 {
	 if(init){
		 return m_buffer;
	 }else{
		 return 0;
	 }
 }

 void AudioBuffer::start()
 {
	 std::cout << "[ AUDIO BUFFER ] �I�[�f�B�I�o�b�t�@�X�^�[�g" << std::endl;
     open(QIODevice::WriteOnly);
 }

 void AudioBuffer::stop()
 {
	 std::cout << "[ AUDIO BUFFER ] �I�[�f�B�I�o�b�t�@��~" << std::endl;
     close();
 }

 qint64 AudioBuffer::readData(char *data, qint64 maxlen)
 {
     Q_UNUSED(data)
     Q_UNUSED(maxlen)

     return 0;
 }

/*!
 * \brief
 * �f�[�^�������ݎ��̃t�B���^
 * 
 * \param data
 * �f�[�^
 * 
 * \param len
 * �f�[�^��
 * 
 * \returns
 * �������݃f�[�^
 * 
 * \remarks
 * QIODevice�̎d�l
 * 
 */
qint64 AudioBuffer::writeData(const char *data, qint64 len)
{
     if (m_maxAmplitude) {
         Q_ASSERT(m_format.sampleSize() % 8 == 0);
         const int channelBytes = m_format.sampleSize() / 8;
         const int sampleBytes = m_format.channels() * channelBytes;
         Q_ASSERT(len % sampleBytes == 0);
         const int numSamples = len / sampleBytes;

         //quint16 maxValue = 0;//��ԍő�l
         const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

		 double frame_sum = 0;
		 int counter = 0 ;
         for (int i = 0; i < numSamples; ++i) {
			for(int j = 0; j < m_format.channels(); ++j) {
				qint16 value = qFromLittleEndian<qint16>(ptr);

				m_buffer[m_front] = (double)value/(double)(m_maxAmplitude);

				//��ԍ��v�l
				//frame_sum += value;
				//��ԍő�l
				//maxValue = qMax(value, maxValue);
				ptr += channelBytes;
				m_front++; //�t�����g�ʒu	 
				if(m_front >= m_bufsize) m_front = 0;//�����O�o�b�t�@

				//counter++; //��ԕ��ϗp
				m_offset++;//�ʎZ�ʒu

				//if((int)(m_offset*1000./m_format.sampleRate()) % 1000 == 0){
				//	std::cout << m_offset*1000./m_format.sampleRate() << std::endl;
				//}
			}
         }
		//m_average = frame_sum / (double)counter;

		//��ԍő�l���d�l��ő�l�𒴂��Ȃ����Ƃ��m�F
        //maxValue = qMin(maxValue, m_maxAmplitude);
		//��ԍő�l�̎d�l��ő�l�ւ̊���
        //m_level = qreal(maxValue) / m_maxAmplitude;
	 }

     emit update();
     return len;
}
