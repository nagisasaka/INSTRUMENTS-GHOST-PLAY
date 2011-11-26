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

	//���䃂�[�h
	enum
	{
		JOG ,		// �W���O���[�h
		PREDEFINED  // ��`�ς݈ړ����[�h
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

	//�W���O�p
	void SetCurrentStringName(QString stringName){ currentStringName = stringName; }

	//�W���O�p
	QString GetCurrentStringName(){ return currentStringName; }

protected:

	void run();

private:

	bool Init();

	// �v���f�B�t�@�C���h�ʒu�ւ̈ړ�
	void GoToCurrentString(); 

	// ���ԍ�
	int nb_axis;

	// ����x�N�^
	QVector<int> controlVector;

	// ���[�h
	int mode;

	// �V���A���ʐM
	StellarSerialCom *scc;

	// �J�����g�ʒu
	int curr;

	// �J�����g�̈ړ��挷
	QString currentStringName;

	// ����J�n�t���O
	bool flagStart;

	// ����x�N�^�T�C�Y
	int controlVectorSize;

};
