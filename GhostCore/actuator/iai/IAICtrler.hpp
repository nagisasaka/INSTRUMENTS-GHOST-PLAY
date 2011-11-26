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

#include "serialcom.hpp"

#include <QString>
#include <QThread>
#include <QVector>

class IAICtrler : public QThread
{
	Q_OBJECT

public:

	//制御モード
	enum
	{
		JOG ,		// ジョグモード
		PREDEFINED  // 定義済み移動モード
	};


	IAICtrler(StellarSerialCom *scc, int nb_axis);

	~IAICtrler();

	bool OnServo();

	bool ReturnOrigin();

	void Evacuate();

	void Close();

	void GoToString(QString stringName); 

	bool Move(int Destination, int Accuracy, int Vel, int Acceleration);

	void Prepare();

	void Commit();

	void Update();

	QVector<int> GetControlVector(){ return controlVector; }

	void SetControlVector(QVector<int> _controlVector){ controlVector = _controlVector; controlVectorSize = controlVector.size(); }

	void SetMode(int _mode){ mode = _mode; }

	int GetMode(){ return mode; }

	//ジョグ用
	void SetCurrentStringName(QString stringName){ currentStringName = stringName; }

	//ジョグ用
	QString GetCurrentStringName(){ return currentStringName; }

protected:

	void run();

private:

	bool Init();

	// プリディファインド位置への移動
	void GoToCurrentString(); 

	// 軸番号
	int nb_axis;

	// 制御ベクタ
	QVector<int> controlVector;

	// モード
	int mode;

	// シリアル通信
	StellarSerialCom *scc;

	// カレント位置
	int curr;

	// カレントの移動先弦
	QString currentStringName;

	// 動作開始フラグ
	bool flagStart;

	// 制御ベクタサイズ
	int controlVectorSize;

};
