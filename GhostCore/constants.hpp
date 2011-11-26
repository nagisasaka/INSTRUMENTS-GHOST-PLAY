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
 * グローバル定数クラス．定数を利用するクラスが限られる場合は，そのクラス内で
 * 定数定義を行っている．
 * 
 */
class Constants
{
public:		
	//会社名（QSettings用）
	static const QString COMPANY_NAME;
	//プログラム名（QSettings用）
	static const QString PROGRAM_NAME;
	//PPCI7443設定ファイル
	static const QString INI_FILE_1;
	//このプログラムの設定ファイル
	static const QString INI_FILE_2;
	//ピッチキャリブレーション設定ファイル
	static const QString INI_FILE_3;
};

#endif
