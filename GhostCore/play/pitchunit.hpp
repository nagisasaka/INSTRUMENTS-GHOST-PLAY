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

#ifndef __PITCH_UNIT_HPP__
#define __PITCH_UNIT_HPP__

#include <QThread>
#include <QString>
#include <QVector>

class PitchCalibration;

/*!
 * \brief
 * 音階決定クラス
 *
 * 音階決定部（ピッチユニット）の一切を管轄
 * 
 */
class PitchUnit : public QThread
{
	friend class PitchCalibration;
	friend class PitchCalibrationTab;

public:

	PitchUnit(int nb_unit);

	PitchUnit(QStringList initData);

	~PitchUnit();

	//保存可能な文字列を返す（このStringの意味は文字列．弦ではない）
	QString toString();

	//デフォルトゼロ（デフォルト指定の場合は時間が無視されることに注意）；ここで指定された時間までパディングされる
	void SetPlayTime(int _playTime){ prepared = false; playTime = _playTime; }

	//デフォルトオフ
	void SetVibrateFlag(bool t){ prepared = false; vibrateFlag = t; }

	//デフォルトオフ
	void SetRigenFlag(bool t){ prepared = false; rigenFlag = t; }

	void SetVibrateHz(double hz){ prepared = false; vibrateHz = hz; }

	void SetVibratePercent(double percent){ prepared = false; vibratePercent = percent; }

	int GetPlayTime(){ return playTime; }

	bool GetVibrateFlag(){ return vibrateFlag; }

	bool GetRigenFlag(){ return rigenFlag; }

	void SetTransitionTime(int step){ prepared = false; transitionTime = step; }

	int GetTransitionTime(){ return transitionTime; }

	double GetVibrateHz(){ return vibrateHz; }

	double GetVibratePercent(){ return vibratePercent; }

	//制御弦番号で指定
	void SetString(int nb_string);

	//弦名で指定
	void SetStringName(QString name){ stringName = name; }

	void SetPitch(QString pitchName);

	void CalibrationReLoad();

	int Prepare();

	int Prepare(double lastPosition);

	void Commit();

	int GetNbUnit(){ return nb_unit; }

	QString GetStringName(){ return stringName; }

	int GetStringCode();

	QString GetPitchName(){ return pitchName; }

	double GetCurrentPosition();

	void SetContactDelay(double delay){ contactDelay = delay; }

	double GetContactDelay(){ return contactDelay; }

	//// 対応軸はオブジェクトに一意なのでbowingクラスのように引数は無し

	//主軸（ピッチ決定）の制御ベクトルを返す
	QVector<double> GetPositionVector(){ return pitchSequence; }

	//第二軸（弦選択）の制御ベクトルを返す
	QVector<int> GetStringSelectSequence(){ return stringSelectSequence; }

	//第三軸（弦押下）の制御ベクトルを返す
	QVector<int> GetPushpullSequence(){ return pushpullSequence; }

	//対応軸を取得
	int GetNbAxis(){ return nb_axis; }

	//対応サーボIDを取得
	int GetNbServo(){ return nb_servo; }

	//IAICを取得
	int GetNbIaic(){ return nb_iaic; }
	
	//ジョグ動作
	void Jog(double z_value, double z_time, double c_value);

	//ジョグ動作
	void JogGoToString(QString stringName);

	//機構退避
	void Evacuate();

	//ジョグ動作（セットホーム）
	//void SetHome();

	//プリペア済か？
	bool isPrepared(){ return prepared; }

protected:

	void run();

private:

	//ビブラートシーケンスを追加し，追加した長さを返す
	int MakeVibrate(int togo);

	//ユニット番号
	int nb_unit;

	//このユニットが利用するリニア軸番号
	int nb_axis;

	//このユニットが利用する押込み軸番号
	int nb_servo;

	//このユニットが利用するIAIアクチュエータ番号
	int nb_iaic;

	//設定された軸名
	QString stringName;

	//設定された音程名
	QString pitchName;

	//キャリブレーションクラス
	PitchCalibration *calibration;

	//ビブラートフラグ
	bool vibrateFlag;

	//離弦遷移フラグ
	bool rigenFlag;

	//遷移時間
	int transitionTime;

	//維持時間
	int playTime;

	//ビブラート周波数
	double vibrateHz;

	//ビブラート半値幅
	double vibratePercent;

	//弦押下（コンタクト）開始までのディレイ（ステップ数）
	double contactDelay;

	//動かすべきターゲット
	double c_value;

	double z_value;

	//主軸の制御ベクトル
	QVector<double> pitchSequence;

	//第二軸の制御ベクトル（弦選択）
	QVector<int> stringSelectSequence;

	//第三軸の制御ベクトル（弦押下）
	QVector<int> pushpullSequence;

	//プリペア済フラグ
	bool prepared;
};

#endif