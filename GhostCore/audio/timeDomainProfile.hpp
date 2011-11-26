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
 * タイムドメインのプロファイル情報を保存するクラス
 * 
 * 
 * \remarks
 * subwindow には、オリジナルリングバッファの断片が保存されているため、メモリリークに注意せよ。
 * 
 * \see
 * frequencyDomainProfile.hpp
 */
class TimeDomainProfile
{
public:

	//録音開始からの経過時間[ミリ秒]
	double timeoffset;

	//最大振幅[0,1]
	double maxAmplitude;

	//平均振幅[0,1]
	double averageAmplitude;

	//最大パワー[0,1]
	double maxPower;

	//平均パワー[0,1]
	double averagePower;

	//サブウィンドウに含まれる測定データ，要素は[0,1]
	double *subwindow;

	//分析長（サブウィンドウ長さ）
	int analyzeFrameLength;

	/*!
	 * \brief
	 * デフォルトコンストラクタ
	 * 
	 * \throws <const char *>
	 * メモリが足りない
	 * 
	 */
	TimeDomainProfile() : timeoffset(0), maxAmplitude(0), averageAmplitude(0), maxPower(0), averagePower(0), analyzeFrameLength(4096)
	{
		subwindow = (double*)calloc(analyzeFrameLength, sizeof(double));
		if(subwindow == 0){
			Error::Critical(0, QObject::tr("TimeDomainProfile 確保中にメモリが足りません(1)"));
		}
	}

	/*!
	 * \brief
	 * 通常利用するコンストラクタ
	 * 
	 * \throws <const char *>
	 * メモリが足りない
	 * 
	 */
	TimeDomainProfile(int _analyzeFrameLength) : timeoffset(0), maxAmplitude(0), averageAmplitude(0), maxPower(0), averagePower(0), analyzeFrameLength(_analyzeFrameLength)
	{
		subwindow = (double*)calloc(analyzeFrameLength, sizeof(double));
		if(subwindow == 0){
			Error::Critical(0, QObject::tr("TimeDomainProfile 確保中にメモリが足りません(2)"));
		}
	}

	/*!
	 * \brief
	 * コピーコンストラクタ
	 * 
	 * \throws <const char *>
	 * メモリが足りない
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
			Error::Critical(0, QObject::tr("TimeDomainProfile 確保中にメモリが足りません(3)"));
		}
		for(int i=0;i<analyzeFrameLength;++i){
			subwindow[i] = obj.subwindow[i];
		}
	}

	/*!
	 * \brief
	 * デストラクタ, メモリリークに注意してください. 
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
		//平均振幅
		tmp->averageAmplitude = averageAmplitude + obj.averageAmplitude;  // 単純足し算
		//平均パワー
		tmp->averagePower = averagePower + obj.averagePower;			  // 単純足し算
		//最大振幅
		tmp->maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);		  // ここは単純な足し算ではないです（この扱いが特殊）
		//最大パワー
		tmp->maxPower = qMax(maxPower,obj.maxPower);					  // ここも単純な足し算ではないよ（この扱いが特殊）
		//時間オフセットは自分の時間オフセットを引き継ぐ
		tmp->timeoffset = timeoffset;
		//サンプルの解像度を維持したまま足し算
		for(int i=0;i<analyzeFrameLength;i++){
			tmp->subwindow[i] = subwindow[i] + obj.subwindow[i];
		}
		return *tmp;
	}
	
	TimeDomainProfile operator-(const TimeDomainProfile &obj)
	{
		TimeDomainProfile* tmp = new TimeDomainProfile(analyzeFrameLength);
		//平均振幅
		tmp->averageAmplitude = averageAmplitude - obj.averageAmplitude;  // 単純引き算
		//平均パワー
		tmp->averagePower = averagePower - obj.averagePower;			  // 単純引き算
		//最大振幅
		tmp->maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);		  // ここは単純な引き算ではない
		//最大パワー
		tmp->maxPower = qMax(maxPower,obj.maxPower);					  // ここも単純な引き算ではない
		//時間オフセットは自分の時間オフセットを引き継ぐ
		tmp->timeoffset = timeoffset;
		//サンプルの解像度を維持したまま引き算
		for(int i=0;i<analyzeFrameLength;i++){
			tmp->subwindow[i] = subwindow[i] - obj.subwindow[i];
		}
		return *tmp;
	}

	TimeDomainProfile operator+=(const TimeDomainProfile &obj)
	{
		//平均振幅
		averageAmplitude = averageAmplitude + obj.averageAmplitude;
		//平均パワー
		averagePower = averagePower + obj.averagePower;
		//最大振幅
		maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);
		//最大パワー
		maxPower = qMax(maxPower,obj.maxPower);
		//時間オフセットは自分の時間オフセットを引き継ぐ
		timeoffset = timeoffset;
		//サンプルの解像度を維持したまま引き算
		for(int i=0;i<analyzeFrameLength;i++){
			subwindow[i] = subwindow[i] + obj.subwindow[i];
		}
		return *this;	
	}

	TimeDomainProfile operator-=(const TimeDomainProfile &obj)
	{
		//平均振幅
		averageAmplitude = averageAmplitude - obj.averageAmplitude;
		//平均パワー
		averagePower = averagePower - obj.averagePower;
		//最大振幅
		maxAmplitude = qMax(maxAmplitude, obj.maxAmplitude);
		//最大パワー
		maxPower = qMax(maxPower,obj.maxPower);
		//時間オフセットは自分の時間オフセットを引き継ぐ
		timeoffset = timeoffset;
		//サンプルの解像度を維持したまま引き算
		for(int i=0;i<analyzeFrameLength;i++){
			subwindow[i] = subwindow[i] - obj.subwindow[i];
		}
		return *this;
	}
};

#endif