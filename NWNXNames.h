/***************************************************************************
    NWNXNames.h - Interface for the CNWNXNames class.
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

#if !defined(NWNXNAMES_H_)
#define NWNXNAMES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#include <windows.h>

#include "CCustomNames.h"
#include "HookFunc.h"
#include "NWNStructures.h"
#include "../NWNXdll/IniFile.h"
#include "../NWNXdll/NWNXBase.h"

#define PLAYERID_ALL_CLIENTS 0xFFFFFFFF
#define PLAYERID_INVALIDID 0xFFFFFFFE
#define PLAYERID_SERVER 0xFFFFFFFD
#define PLAYERID_ALL_PLAYERS 0xFFFFFFF7
#define PLAYERID_ALL_GAMEMASTERS 0xFFFFFFF6
#define PLAYERID_ALL_SERVERADMINS 0xFFFFFFF5

// HookFunc.cpp
CNWSPlayer *GetPlayerByClientID(dword nClientID);
void *GetObjectByID(dword ObjID);
extern CNWSObject *(__fastcall *GetPlayerObject)(void *pPlayer);

// Hook Handlers
int Handler_CNWSMessage__SendServerToPlayerChat_Tell();

class CNWNXNames : public CNWNXBase
{
public:
	CNWNXNames();
	virtual ~CNWNXNames();
	BOOL OnCreate(const char* LogDir);
	char* OnRequest(char* gameObject, char* Request, char* Parameters);
	BOOL OnRelease();
	bool bHooked;
	int supressMsg;
    char eventScript[17];
	int logDebug;
	CCustomNames Names;
	CNWSObject *pGameObject;
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

extern CNWNXNames names;

#endif
