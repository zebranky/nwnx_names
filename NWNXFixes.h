/***************************************************************************
    NWNXFixes.h - Interface for the CNWNXFixes class.
    (c) 2007 virusman (virusman@virusman.ru)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#if !defined(NWNXFIXES_H_)
#define NWNXFIXES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#include <windows.h>

#include "madCHook.h"
#include "CCustomNames.h"
#include "HookFunc.h"
#include "../NWNXdll/IniFile.h"
#include "../NWNXdll/NWNXBase.h"


class CNWNXNames : public CNWNXBase
{
public:
	CNWNXNames();
	virtual ~CNWNXNames();
	BOOL OnCreate(const char* LogDir);
	char* OnRequest(char* gameObject, char* Request, char* Parameters);
	unsigned long OnRequestObject (char *gameObject, char* Request);
	BOOL OnRelease();
	bool bHooked;
	int supressMsg;
    char eventScript[17];
	int logDebug;
	CCustomNames Names;
	char *pGameObject;
	dword nGameObjectID;
	FILE *PacketData;

protected:
	void InitPlayerList(char *value);
	void EnableDisableNames(char *value);
	char *GetDynamicName(char *value);
	void SetDynamicName(char *value);
	void UpdateDynamicName(char *value);
	void UpdatePlayerList(char *value);
	void DeleteDynamicName(char *value);
	void ClearPlayerList(char *value);

};

#endif
