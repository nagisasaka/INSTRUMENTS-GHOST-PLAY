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

#ifndef __CONSTANTS_HPP__
#define __CONSTANTS_HPP__

#include <QString>

/*!
 * \brief
 * �O���[�o���萔�N���X�D�萔�𗘗p����N���X��������ꍇ�́C���̃N���X����
 * �萔��`���s���Ă���D
 * 
 */
class Constants
{
public:		
	//��Ж��iQSettings�p�j
	static const QString COMPANY_NAME;
	//�v���O�������iQSettings�p�j
	static const QString PROGRAM_NAME;
	//PPCI7443�ݒ�t�@�C��
	static const QString INI_FILE_1;
	//���̃v���O�����̐ݒ�t�@�C��
	static const QString INI_FILE_2;
	//�s�b�`�L�����u���[�V�����ݒ�t�@�C��
	static const QString INI_FILE_3;
};

#endif
