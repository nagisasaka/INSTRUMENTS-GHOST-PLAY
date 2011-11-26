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
 * 共通エラーハンドリング用クラス．
 * 
 * Critical, Warning を定義
 * 
 * \remarks
 * 各エラーレベルのエラー事象発生時に，例外送出か即時終了か，ここの実装を書き換えてください．
 * 
 */
class Error
{
public:
	/*!
	 * \brief
	 * クリティカルエラー
	 * 
	 * \param parent
	 * 呼び出し元QObject．呼び出し元がQObjectでないばあいは0を指定してください．
	 * 
	 * \param text
	 * エラー文字列．
	 * 
	 * \throws <const char*>
	 * 例外を投げるか，即時exit(1)するかはコード内を見てください．
	 * 
	 * \remarks
	 * 例外を投げるか，即時exit(1)するかは実装を変更してください．
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

		//エラー出力
		std::cerr << errorMessage.toStdString() << std::endl;
		int ret = QMessageBox::critical(parent, 
			Constants::PROGRAM_NAME,
			errorMessage);
		//即時終了
		exit(1);
		//例外出力
		//throw errorMessage.toStdString().c_sQObject::tr();
	}

	/*!
	 * \brief
	 * 警告
	 * 
	 * \param parent
	 * 呼び出し元QObject．呼び出し元がQObjectでないばあいは0を指定してください．
	 * 
	 * \param text
	 * エラー文字列．
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

		//エラー出力
		std::cerr << errorMessage.toStdString() << std::endl;
		int ret = QMessageBox::warning(parent, 
			Constants::PROGRAM_NAME,
			errorMessage);

		//特に終了しない
		//即時終了
		//exit(1);
		//例外出力
		//throw errorMessage.toStdString().c_sQObject::tr();
	}
};

#endif