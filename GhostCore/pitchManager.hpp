#ifndef __PITCH_MANAGER_HPP__
#define __PITCH_MANAGER_HPP__

#include "play/pitchunit.hpp"

#include <QObject>
#include <QString>
#include <QVector>

class PitchManager : public QObject
{
	Q_OBJECT

public:

	PitchManager();

	~PitchManager();

	void AddPitch(PitchUnit *pu);

	void UpdatePitch(PitchUnit *pu, int index);

	void DelPitch(int index);

	PitchUnit* GetPitch(int index);

	int Prepare();

	int Prepare(int start, int end);

	void Commit();

	void InitialMove();

private:

	//�s�b�`���j�b�g�x�N�^�i��0�s�b�`���j�b�g�C��1�s�b�`���j�b�g����������j
	QVector<PitchUnit*> puVector;

	//�s�b�`�}�l�[�W�������䉺�ɂ����s�b�`���j�b�g���i���㎲�����g�����Ă����ꍇ�ɔ����āj
	int NbPitchUnit;

	//// �s�b�`�}�l�[�W�������䉺�ɂ����s�b�`���j�b�g�������̐���x�N�^

	//�厲�Q�F��3,��4�̐���x�N�^
	QVector<double> axis[2];

	//��񎲌Q�i���I���j
	QVector<int> stringSelectSequence[2];

	//��O���Q�i�������j
	QVector<int> pushpullSequence[2];

};


#endif