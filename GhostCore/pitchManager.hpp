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

	//ピッチユニットベクタ（第0ピッチユニット，第1ピッチユニットが混合する）
	QVector<PitchUnit*> puVector;

	//ピッチマネージャが制御下におくピッチユニット数（今後軸数が拡張していく場合に備えて）
	int NbPitchUnit;

	//// ピッチマネージャが制御下におくピッチユニットが持つ軸の制御ベクタ

	//主軸群：軸3,軸4の制御ベクタ
	QVector<double> axis[2];

	//第二軸群（弦選択）
	QVector<int> stringSelectSequence[2];

	//第三軸群（弦押下）
	QVector<int> pushpullSequence[2];

};


#endif