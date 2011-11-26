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

#ifndef __ERROR_HPP__
#define __ERROR_HPP__

#include "constants.hpp"

#include <QMessageBox>
#include <QString>
#include <QWidget>

#include <iostream>

/*!
 * \brief
 * ���ʃG���[�n���h�����O�p�N���X�D
 * 
 * Critical, Warning ���`
 * 
 * \remarks
 * �e�G���[���x���̃G���[���۔������ɁC��O���o�������I�����C�����̎��������������Ă��������D
 * 
 */
class Error
{
public:
	/*!
	 * \brief
	 * �N���e�B�J���G���[
	 * 
	 * \param parent
	 * �Ăяo����QObject�D�Ăяo������QObject�łȂ��΂�����0���w�肵�Ă��������D
	 * 
	 * \param text
	 * �G���[������D
	 * 
	 * \throws <const char*>
	 * ��O�𓊂��邩�C����exit(1)���邩�̓R�[�h�������Ă��������D
	 * 
	 * \remarks
	 * ��O�𓊂��邩�C����exit(1)���邩�͎�����ύX���Ă��������D
	 * 
	 * \see
	 * Warning()
	 */
	static void Critical(QWidget *parent, QString text)
	{
		QString errorMessage;
		if(parent){
			QString objectName = parent->objectName();
			if(objectName.isEmpty()){
				errorMessage = text;
			}else{
				errorMessage = QObject::tr("[") + objectName + "]" + text;
			}
		}else{
			errorMessage = text;
		}

		//�G���[�o��
		std::cerr << errorMessage.toStdString() << std::endl;
		int ret = QMessageBox::critical(parent, 
			Constants::PROGRAM_NAME,
			errorMessage);
		//�����I��
		exit(1);
		//��O�o��
		//throw errorMessage.toStdString().c_sQObject::tr();
	}

	/*!
	 * \brief
	 * �x��
	 * 
	 * \param parent
	 * �Ăяo����QObject�D�Ăяo������QObject�łȂ��΂�����0���w�肵�Ă��������D
	 * 
	 * \param text
	 * �G���[������D
	 * 
	 * \see
	 * Critical()
	 */
	static void Warning(QWidget *parent, QString text)
	{
		QString errorMessage;
		if(parent){
			QString objectName = parent->objectName();
			if(objectName.isEmpty()){
				errorMessage = text;
			}else{
				errorMessage = QObject::tr("[") + objectName + "]" + text;
			}
		}else{
			errorMessage = text;
		}

		//�G���[�o��
		std::cerr << errorMessage.toStdString() << std::endl;
		int ret = QMessageBox::warning(parent, 
			Constants::PROGRAM_NAME,
			errorMessage);

		//���ɏI�����Ȃ�
		//�����I��
		//exit(1);
		//��O�o��
		//throw errorMessage.toStdString().c_sQObject::tr();
	}
};

#endif