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

	//バグ
	//1秒間のバイト数
	//double byte_per_sec = m_format.sampleSize() * 8 * m_format.channelCount() * m_format.sampleRate();
	//  1秒間のバイト数 =       16[bit]         x 8 x         1[channel]      x       44.1[kHz]

	//リングバッファ自体は例えば10秒の長さを持っている
	//バッファーの長さ[秒]
	double buf_len_sec  = 10; // [sec]

	//バッファサイズ（サンプル数；1秒間のサンプルレートｘリングバッファに記録したい長さ）
	m_bufsize = buf_len_sec * m_format.sampleRate();
	//バッファの確保
	m_buffer = (double*)calloc(m_bufsize, sizeof(double));  // １つのサンプルをdoubleで取り扱うので

	std::cout << "[ AUDIO BUFFER ] 録音リングバッファサイズ: " << m_bufsize*sizeof(double)/1000000. << "MB" << std::endl;

	//通算位置のクリア
	m_offset = 0;

	//最大値チェック
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
	 std::cout << "[ AUDIO BUFFER ] オーディオバッファスタート" << std::endl;
     open(QIODevice::WriteOnly);
 }

 void AudioBuffer::stop()
 {
	 std::cout << "[ AUDIO BUFFER ] オーディオバッファ停止" << std::endl;
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
 * データ書き込み時のフィルタ
 * 
 * \param data
 * データ
 * 
 * \param len
 * データ長
 * 
 * \returns
 * 書き込みデータ
 * 
 * \remarks
 * QIODeviceの仕様
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

         //quint16 maxValue = 0;//区間最大値
         const unsigned char *ptr = reinterpret_cast<const unsigned char *>(data);

		 double frame_sum = 0;
		 int counter = 0 ;
         for (int i = 0; i < numSamples; ++i) {
			for(int j = 0; j < m_format.channels(); ++j) {
				qint16 value = qFromLittleEndian<qint16>(ptr);

				m_buffer[m_front] = (double)value/(double)(m_maxAmplitude);

				//区間合計値
				//frame_sum += value;
				//区間最大値
				//maxValue = qMax(value, maxValue);
				ptr += channelBytes;
				m_front++; //フロント位置	 
				if(m_front >= m_bufsize) m_front = 0;//リングバッファ

				//counter++; //区間平均用
				m_offset++;//通算位置

				//if((int)(m_offset*1000./m_format.sampleRate()) % 1000 == 0){
				//	std::cout << m_offset*1000./m_format.sampleRate() << std::endl;
				//}
			}
         }
		//m_average = frame_sum / (double)counter;

		//区間最大値が仕様上最大値を超えないことを確認
        //maxValue = qMin(maxValue, m_maxAmplitude);
		//区間最大値の仕様上最大値への割合
        //m_level = qreal(maxValue) / m_maxAmplitude;
	 }

     emit update();
     return len;
}
