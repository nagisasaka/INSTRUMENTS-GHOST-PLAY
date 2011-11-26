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
 * �N���b�N�Ǘ��N���X
 * 
 * �N���b�N�́C�R���g���[���[�̗��U����P�ʎ��Ԃƒ�`�����D�R���g���[���[�̗��U����P�ʎ��Ԃ�
 * �W���ł́C10msec �ł���̂ŁC���̏ꍇ�́C�N���b�N�� 10msec ���� 1 ���J�E���g�A�b�v����D
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
	 * �N���b�N���C���N�������g����
	 * 
	 * �������N���b�N��enable�ł͂Ȃ����͒P�ɖ��������DStart()�֐��ŃN���b�N��enable
	 * �ɂȂ�CStop()�֐��ŃN���b�N��disable�ɂȂ�D
	 * ����𗘗p���ĉ^�]���̂݃N���b�N���J�E���g�A�b�v���邱�ƂȂǂ������ł���D
	 *
	 * �K����FPS��GUI���A�b�v�f�[�g���邽�߂�UpdateGUI()�V�O�i����emit����D���̎��Ԋ�
	 * �u�̐��x�͈����D
	 * 
	 * \remarks
	 * GUI�A�b�v�f�[�g�Ɋ|���鎞�Ԃ��v�����Ă��Ȃ��̂ŁCFPS�́C�����܂��Ȓl�ɂȂ�܂��D
	 * 
	 * �f�t�H���g�ŃN���b�N��enable�ł͂Ȃ��̂ŁCStart()���K�v�ł��D
	 *
	 * \see
	 * Start() | Stop()
	 */
	void IncrementCurrentClock()
	{
		if(enable){
			currentClock++; 
		}
		
		//�w�肵��FPS��
		if((int)(internalClock*cycle) % (int)guiupdate_cycle == 0){
			//�V�O�i���𑗏o
			emit UpdateGUI();
		}
		internalClock++;
	}

	/*!
	 * \brief
	 * �J�����g�N���b�N��h:m:s:ms�\���ɏ���������
	 * 
	 * \param h
	 * ����
	 * 
	 * \param m
	 * ��
	 * 
	 * \param s
	 * �b
	 * 
	 * \param ms
	 * �~���b
	 * 
	 * \see
	 * TimeToClock()
	 */
	void ClockToTime(int *h, int *m, int *s, int *ms)
	{
		//���Ԃ����߂�
		*h  = currentClock*cycle     / (60*60*1000);
		*m  = (currentClock*cycle - (*h*60*60*1000)) /   (60*1000);
		*s  = (currentClock*cycle - (*h*60*60*1000) - (*m*60*1000)) /    1000;
		*ms = currentClock*cycle -  (*h*60*60*1000) - (*m*60*1000) - (*s*1000);
	}

	void ClockTime(int *h, int *m, int*s, int *ms, int clock)
	{
		//���Ԃ����߂�
		*h  = clock*cycle /     (60*60*1000);
		*m  = (clock*cycle - (*h*60*60*1000)) /   (60*1000);
		*s  = (clock*cycle - (*h*60*60*1000) - (*m*60*1000)) /    1000;
		*ms =  clock*cycle - (*h*60*60*1000) - (*m*60*1000) - (*s*1000);
	}

	/*!
	 * \brief
	 * h:m:s:ms�\����ms�Ɋ��Z����D�N���X�ϐ���currentClock�ɂ͉e����^���Ȃ��D
	 * 
	 * \param h
	 * ����
	 * 
	 * \param m
	 * ��
	 * 
	 * \param s
	 * �b
	 * 
	 * \param ms
	 * �~���b
	 * 
	 * \returns
	 * �~���b
	 * 
	 * \remarks
	 * ���Z�����l��Ԃ��݂̂ł���C�N���X�ϐ���currentClock�ɂ͉e����^���Ȃ��D
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

	//�N���b�N���L�����ǂ���
	bool enable;

	//���݂̗ݐσN���b�N��
	unsigned int currentClock;

	//GUI�X�V�p�̓����N���b�N
	unsigned int internalClock;

	//�T�C�N��
	double cycle;

	//GUI�A�b�v�f�[�g�T�C�N��
	double guiupdate_cycle;

};

#endif