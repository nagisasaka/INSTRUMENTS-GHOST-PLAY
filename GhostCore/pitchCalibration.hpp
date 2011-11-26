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

#ifndef __PITCHCALIBRATION_HPP__
#define __PITCHCALIBRATION_HPP__

#include "actuator/controller.hpp"
#include "audio/stringRecorder.hpp"
#include "audio/frequencyDomainProfile.hpp"
#include "audio/timeDomainProfile.hpp"

#include <QVector>
#include <QString>
#include <QThread>

class PitchCalibrationTab;

class PitchCalibration : public QThread
{
	friend class PitchCalibrationTab;

public:

	typedef struct CalibratedPositionsStrcut
	{
		int id; // ID
		QString stringName; // 弦名[G,D,A,E]
		QString pitchName;  // ピッチ名
		int nb_pitchUnit;	// ピッチユニット番号
		double z_value;		// Z軸の値（音階決定部主軸）
		double c_value;		// C軸の値（サーボモーターの回転軸）

	}CalibratedPositions;

	PitchCalibration(int nb_pitchUnit);

	~PitchCalibration();

	void Save();

	void Save(double z_value, double c_value);

	void Load();

	void SetStringName(QString name){ currentString = name; }

	void SetPitchName(QString name){ currentPitch = name; }

	void Start(QString stringName);

	void Start(QString stringName, QString pitchName);

	QVector<PitchCalibration::CalibratedPositions> GetCalibratedPositions(){ return positions; }

	QString GetCurrentString(){ return currentString; }

	QString GetCurrentPitch(){ return currentPitch; }

	double GetRefFrequency()
	{
		if(currentString == "G"){
			return pg[currentPitch];
		}else if(currentString == "D"){
			return pd[currentPitch];
		}else if(currentString == "E"){
			return pe[currentPitch];
		}else if(currentString == "F"){
			return pa[currentPitch];
		}else{
			return 0;
		}
	}

protected:

	void run();

	void MakeStandardBowing();

	//カレントのキャリブレーション対象弦
	QString currentString;

	//カレントのキャリブレーション対象音階
	QString currentPitch;

	//キャリブレーション対象ピッチユニット番号
	int nb_pitchUnit;

	//単音か，弦全体の自動実行か（モード）
	int operationMode;

	//キャリブレーション済データ
	QVector<PitchCalibration::CalibratedPositions> positions;

	QMap<QString, double> pg; // G線上のポジション
	QMap<QString, double> pd; // D線上のポジション
	QMap<QString, double> pe; // E線上のポジション
	QMap<QString, double> pa; // A線上のポジション

	//キャリブレーション用ボーイングデータ
	//弓圧
	QVector<double> pressure;
	//弓速
	QVector<double> bowposition;
};


#endif