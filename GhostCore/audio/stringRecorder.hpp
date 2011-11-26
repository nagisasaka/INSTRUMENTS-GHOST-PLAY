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

	//録音開始
	void StartRecord();

	//録音終了
	void EndRecord();

	//FFT窓長
	int GetWindowSize() const{ return windowSize; }

	//基本解析長
	int GetAnalyzeFrameSize() const{ return analyzeFrameLength; }

	//録音中かどうか
	bool isRecording() const{ return flagRecording; }

	//自分が保持するリングバッファ長を返す
	int bufsize() const { return a_length; }

	//バッファーを返す
	AudioBuffer* GetBuffer() const{ return buffer; }

	//// 解析結果を返す

	//時間方向
	QVector<TimeDomainProfile*> TimeDomainAnalyze(int analyzeLength, double starttimeoffset, double endtimeoffset) const;
	QVector<TimeDomainProfile*> TimeDomainAnalyze(int analyzeLength, double starttimeoffset) const;

	//周波数方向
	QVector<FrequencyDomainProfile*> FrequencyDomainAnalyze(int analyzeLength, double starttimeoffset, double endtimeoffset) const;
	QVector<FrequencyDomainProfile*> FrequencyDomainAnalyze(int analyzeLength, double starttimeoffset) const;

protected:
	//音声分析スレッド
	void run();

	//保持しているバッファと録音デバイス
	QAudioInput *audio;
	AudioBuffer *buffer;

	//自分が保持する解析結果保存用リングバッファ
	TimeDomainProfile **tdp;      //時間方向解析リングバッファ
	FrequencyDomainProfile **fdp; //周波数方向解析リングバッファ

	//自分が保持する解析結果保存用リングバッファのサイズ
	int a_length;

	//解析結果保存用リングバッファフロント位置
	volatile unsigned int a_front; 

	//時間領域解析の実装
	void TimeDomain(double *v, TimeDomainProfile *c);

	//周波数領域解析の実装
	void FrequencyDomain(double *v, FrequencyDomainProfile *c);

	//録音中フラグ
	bool flagRecording;

private:

	//サンプリングレート
	static const int samplerate;

	//FFTウィンドウサイズ
	static const int windowSize;

	//基本解析単位長さ（便宜的にFFTウィンドウサイズの整数倍にしている）
	static const int analyzeFrameLength;

	//初期化完了したか
	bool initialized;

};

#endif