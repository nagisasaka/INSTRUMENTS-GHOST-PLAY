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

#ifndef __CALIBRATION_TAB_HPP__
#define __CALIBRATION_TAB_HPP__

#include "../audio/stringRecorder.hpp"
#include "../calibration.hpp"

#include "../audio/ui/renderArea.hpp"
#include "../audio/ui/renderArea2.hpp"
#include "../audio/ui/renderArea3.hpp"

#include <iostream>


class GhostCore;

/*!
 * \brief
 * 位置キャリブレーションを行うUI
 * 
 * 
 * \see
 * Calibration.hpp
 */
class CalibrationTab : public QObject
{
	Q_OBJECT

public:
	CalibrationTab(GhostCore *parent);
	~CalibrationTab();

	/*!
	 * \brief
	 * GUIアップデート処理
	 * 
	 */	
	void Update();

private slots:

	//現在位置をこちらに取り込む
	void GetCurrentPosition();

	//UI関連
	void SelectInitialPosition();
	void SelectCalibratedPosition();

	void UpdateInitialPosition();
	void UpdateCalibratedPosition();


	/*!
	 * \brief
	 * セーブする
	 *
	 */
	void Save();


	/*!
	 * \brief
	 * ロードする．
	 *
	 */
	void Load();


	/*!
	 * \brief
	 * 位置キャリブレーション（軸0,1）を実行する．対象軸は ObjectName() による場合わけ．
	 * 
	 */
	void Start();

	//全軸実行
	void StartAll();
	void StopAll();

	
	/*!
	 * \brief
	 * 位置キャリブレーション（軸0,1）を低すする．
	 *
	 */
	void Stop();

private:

	GhostCore* parent;

	//キャリブレーションクラス
	Calibration* calibration;

	//レンダリングエリア
	RenderArea* r1;
	RenderArea2* r2;
	RenderArea3* r3;

	//全自動キャリブレーションのための現在の進捗保存用変数
	int currentNbString;
};

#endif