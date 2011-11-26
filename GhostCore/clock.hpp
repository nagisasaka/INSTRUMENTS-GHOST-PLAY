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

#ifndef __CLOCK_HPP__
#define __CLOCK_HPP__

#include <QObject>
#include <QString>
#include <iostream>

/*!
 * \brief
 * クロック管理クラス
 * 
 * クロックは，コントローラーの離散制御単位時間と定義した．コントローラーの離散制御単位時間は
 * 標準では，10msec であるので，この場合は，クロックは 10msec 毎に 1 ずつカウントアップする．
 * 
 * \see
 * controller.hpp
 */
class Clock : public QObject
{
	Q_OBJECT

signals:
	void UpdateGUI();

public:
	Clock(double _cycle) : cycle(_cycle), currentClock(0), internalClock(0)
	{
		guiupdate_cycle = 40.;//msec (25fps)	
		enable = false;		
	}
	~Clock(){}

	unsigned int GetCurrentClock(){ return currentClock; }
	void SetCurrentClock(unsigned int clock){ currentClock = clock; }
	void ResetCurrentClock(){ currentClock = 0; }
	void Stop(){ enable = false; }
	void Start(){ enable = true; }

	/*!
	 * \brief
	 * クロックをインクリメントする
	 * 
	 * ただしクロックがenableではない時は単に無視される．Start()関数でクロックがenable
	 * になり，Stop()関数でクロックがdisableになる．
	 * これを利用して運転中のみクロックをカウントアップすることなどが実現できる．
	 *
	 * 適当なFPSでGUIをアップデートするためのUpdateGUI()シグナルをemitする．この時間間
	 * 隔の精度は悪い．
	 * 
	 * \remarks
	 * GUIアップデートに掛かる時間を計測していないので，FPSは，おおまかな値になります．
	 * 
	 * デフォルトでクロックはenableではないので，Start()が必要です．
	 *
	 * \see
	 * Start() | Stop()
	 */
	void IncrementCurrentClock()
	{
		if(enable){
			currentClock++; 
		}
		
		//指定したFPSで
		if((int)(internalClock*cycle) % (int)guiupdate_cycle == 0){
			//シグナルを送出
			emit UpdateGUI();
		}
		internalClock++;
	}

	/*!
	 * \brief
	 * カレントクロックをh:m:s:ms表現に書き換える
	 * 
	 * \param h
	 * 時間
	 * 
	 * \param m
	 * 分
	 * 
	 * \param s
	 * 秒
	 * 
	 * \param ms
	 * ミリ秒
	 * 
	 * \see
	 * TimeToClock()
	 */
	void ClockToTime(int *h, int *m, int *s, int *ms)
	{
		//時間を求める
		*h  = currentClock*cycle     / (60*60*1000);
		*m  = (currentClock*cycle - (*h*60*60*1000)) /   (60*1000);
		*s  = (currentClock*cycle - (*h*60*60*1000) - (*m*60*1000)) /    1000;
		*ms = currentClock*cycle -  (*h*60*60*1000) - (*m*60*1000) - (*s*1000);
	}

	void ClockTime(int *h, int *m, int*s, int *ms, int clock)
	{
		//時間を求める
		*h  = clock*cycle /     (60*60*1000);
		*m  = (clock*cycle - (*h*60*60*1000)) /   (60*1000);
		*s  = (clock*cycle - (*h*60*60*1000) - (*m*60*1000)) /    1000;
		*ms =  clock*cycle - (*h*60*60*1000) - (*m*60*1000) - (*s*1000);
	}

	/*!
	 * \brief
	 * h:m:s:ms表現をmsに換算する．クラス変数のcurrentClockには影響を与えない．
	 * 
	 * \param h
	 * 時間
	 * 
	 * \param m
	 * 分
	 * 
	 * \param s
	 * 秒
	 * 
	 * \param ms
	 * ミリ秒
	 * 
	 * \returns
	 * ミリ秒
	 * 
	 * \remarks
	 * 換算した値を返すのみであり，クラス変数のcurrentClockには影響を与えない．
	 * 
	 * \see
	 * ClockToTime()
	 */

	unsigned int TimeToClock(int h, int m, int s, int ms)
	{
		//  h[hour]*60[min]*60[sec]*1000[msec]
		//           m[min]*60[sec]*1000[msec]
		//                   s[sec]*1000[msec]
		//                            ms[msec]
		return (h*60.*60.*1000 + m*60.*1000 + s*1000 + ms)/cycle; // == currentClock
	}

private:

	//クロックが有効かどうか
	bool enable;

	//現在の累積クロック数
	unsigned int currentClock;

	//GUI更新用の内部クロック
	unsigned int internalClock;

	//サイクル
	double cycle;

	//GUIアップデートサイクル
	double guiupdate_cycle;

};

#endif