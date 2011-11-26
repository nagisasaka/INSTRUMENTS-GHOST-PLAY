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

#ifndef _7443ERR_H
#define _7443ERR_H

enum {
    ERR_NoError,			//0
    ERR_BoardNoInit,                    //1
    ERR_InvalidBoardNumber,             //2
    ERR_InitializedBoardNumber,         //3
    ERR_BaseAddressError,               //4
    ERR_BaseAddressConflict,            //5
    ERR_DuplicateBoardSetting,          //6
    ERR_DuplicateIrqSetting,            //7
    ERR_PCIBiosNotExist,                //8
    ERR_PCIIrqNotExist,                 //9
    ERR_PCICardNotExist,                //10
    ERR_SpeedError,   		        //11
    ERR_MoveRatioError,			//12
    ERR_PosOutOfRange,			//13
    ERR_AxisAlreadyStop,		//14
    ERR_AxisArrayError,			//15
    ERR_SlowDownPointError,		//16
    ERR_CompareMethodError,		//17
    ERR_CompareNoError,			//18
	ERR_CompareAxisError,		//19
	ERR_CompareTableSizeError,	//20
	ERR_CompareFunctionError,	//21
	ERR_CompareTableNotReady,	//22
	ERR_CompareLineNotReady,	//23
	ERR_NoCardFound,		//24
	ERR_LatchNoError,		//25
	ERR_AxisRangeError,		//26
	ERR_DioNoError,			//27
	ERR_PChangeSlowDownPointError,  //28
    ERR_SpeedChangeError,               //29
	ERR_CardNoError,		//30
	ERR_LinkIntError,		//31
	ERR_HardwareCompareAxisWrong,	//32
	ERR_AutoCompareSourceWrong,		//33
	ERR_CompareDeviceTypeError,			//34
	ERR_PulserHomeTypeError,		//35
	ERR_EventAlreadyEnable,			//36
	ERR_EventNotEnableYet,			//37
	ERR_LineArcParameterError,		//38
	ERR_ConfigFileOpenError,		//39
	ERR_CompareFIFONotReady,		//40
	ERR_EventInitError,				//41
	ERR_MemAllocError,				//42
	ERR_FIFOSourceERROR,			//43
	ERR_OtherProcessExist,			//44
	ERR_DelayTimeError,				//45
	ERR_DelayDistError,				//46
	ERR_FIFOModeOn,					//47
	ERR_FIFOBusy,					//48
	ERR_OpenDriverFailed,			//49
	ERR_OSVersionError,				//50
	ERR_OwnerSet,				    //51
	ERR_SignalHandle,			    //52
	ERR_SignalNotify,			    //53	
	ERR_AllocateMemory,		    	//54
	ERR_VChangeTimeError,			//55
	ERR_EventInvalid,				//56
	ERR_ErrorIntCome,				//57
	ERR_Unknown,					//58
	ERR_WaitAbandoned,				//59
	ERR_WaitDelayTimeOut,			//60	
	ERR_NoSeqAttached,				//61
	ERR_CardTypeWrong,				//62
	ERR_RotarySourceWrong,			//63
};

#endif
