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

#ifndef __BASE_GEOMETRY_HPP__
#define __BASE_GEOMETRY_HPP__

#include "vecmath.hpp"
#include "calibration.hpp"
#include "../bowingSimulation.hpp"

#include <QPoint>

class BaseGeometry
{
	friend class BowingSimulation;
	friend class PitchunitSimulation;

public:

	//�e���ʒu�ł̍��E���ɑ΂���}�[�W��
	typedef struct StringMarginStruct{
		double leftMargin;
		double rightMargin;
	}StringMargin;

	BaseGeometry();

	~BaseGeometry();

	void Init();

	//���ʐڐ������߂�
	void commonTangent(double deltaL0, double deltaL1, double &pos0, double &pos1, int nb_string);

	//�c�]���a�����߂�
	double residualRadius(double pos0, double pos1, int nb_string);

	//�c�]���a�����߂�
	double residualRadius2(double pos0, double pos1, int nb_string);

	//�X�g�����O�}�[�W���\����
	StringMargin margin[7];

	//QPoint�^����Point�^�ւ̕ϊ�
	static Point QPointToPoint(QPoint point){ return Point(point.x(),point.y()); }

	//Point�^����QPoint�^�ւ̕ϊ�
	static QPoint PointToQPoint(Point point){ return QPoint(point.real(), point.imag()); }

	//�yOBSOLETE�z�w�茷�ɑ΂��ă�L��^����2�_P0,P1��Y���W��Ԃ�
	void deltaL(double deltaL, double &pos0, double &pos1, int nb_string);

	//�yOBSOLETE�z�����ʒu�������ʒu���𔻒肷��i�����ʒu0, �����ʒu1�j
	int positionType(double pos0, double pos1, int nb_string);

private:

	//�L�����u���[�V�����N���X
	Calibration *calibration;

	//�����ʍ��W�����߂�
	Point CalcStringPosition(int nb_string);

	//�������a�����߂�
	double initialRadius(double pos0, double pos1, int nb_string);

	//�������a�����߂�
	double initialRadius2(double pos0, double pos1, int nb_string);

	//��]
	Point rot(Point p, double theta);

	//%�P�ʂ�mm�P�ʂɕϊ�����
	double ConvertPercentToMM(double percent, int nb_axis);

	//mm�P�ʂ�%�P�ʂɕϊ�����
	double ConvertMMToPercent(double mm, int nb_axis);

	//�e���̕��ʍ��W�imm�P�ʁj
	Point origin[4];

	//�e���ʒu�̃x�[�X���C�������̒P�ʖ@���x�N�g���i�S�����ʒu�{�S�����ʒu��7�{�j�G�����́C�x�[�X���C�����猷�����ł��邱�Ƃɒ���
	Point unitnormal[7];

	//// �@�\�p�����[�^�i�����l�j

	static const double c;

	static const double L;
};

#endif