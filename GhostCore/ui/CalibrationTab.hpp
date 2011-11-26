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
 * �ʒu�L�����u���[�V�������s��UI
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
	 * GUI�A�b�v�f�[�g����
	 * 
	 */	
	void Update();

private slots:

	//���݈ʒu��������Ɏ�荞��
	void GetCurrentPosition();

	//UI�֘A
	void SelectInitialPosition();
	void SelectCalibratedPosition();

	void UpdateInitialPosition();
	void UpdateCalibratedPosition();


	/*!
	 * \brief
	 * �Z�[�u����
	 *
	 */
	void Save();


	/*!
	 * \brief
	 * ���[�h����D
	 *
	 */
	void Load();


	/*!
	 * \brief
	 * �ʒu�L�����u���[�V�����i��0,1�j�����s����D�Ώێ��� ObjectName() �ɂ��ꍇ�킯�D
	 * 
	 */
	void Start();

	//�S�����s
	void StartAll();
	void StopAll();

	
	/*!
	 * \brief
	 * �ʒu�L�����u���[�V�����i��0,1�j��Ⴗ����D
	 *
	 */
	void Stop();

private:

	GhostCore* parent;

	//�L�����u���[�V�����N���X
	Calibration* calibration;

	//�����_�����O�G���A
	RenderArea* r1;
	RenderArea2* r2;
	RenderArea3* r3;

	//�S�����L�����u���[�V�����̂��߂̌��݂̐i���ۑ��p�ϐ�
	int currentNbString;
};

#endif