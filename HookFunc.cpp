/***************************************************************************
    Names plugin for NWNX  - hooks implementation
    (c) 2006 virusman (virusman@virusman.ru)

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

#include <sys/types.h>
#include <stdarg.h>

#include <limits.h>		/* for PAGESIZE */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#include "HookFunc.h"
#include "CCustomNames.h"
#include "NWNXNames.h"
#include "madCHook.h"
#include "HookHandlers.h"

int (__fastcall *GetNameNextHook)(void *, void *, int, CExoString *, unsigned char);

// *(dword*)&pGetObjByOID = 0x00466980; // CGameObjectArray::GetGameObject(unsigned long, CGameObject **)
unsigned char (__fastcall *pGetObjByOID)(CGameObjectArray *pObjectClass, void *, nwn_objid_t ObjID, void **buf);
// *(dword*)&pRetObjByOID = 0x0042C840;  //CServerExoApp::GetCreatureByGameObjectID(unsigned long)
CNWSCreature *(__fastcall *pRetObjByOID)(CServerExoApp *pServerExo, void *, nwn_objid_t ObjID);

//void *(*pGetPlayer)(void *pServerExo4, dword ObjID);
//void *(*pSendNewName)(void *pServerMessage, void *pPlayer, void *pObject);

// *(dword*)&d_newstrcpy = 0x0040B9D0;  // CExoString & __thiscall CExoString::operator=(class CExoString const &)
//char *(*d_newstrcpy)(char *buf, char **source);
CExoString *(__fastcall *d_newstrcpy)(CExoString *buf, void *, CExoString *source);
// *(dword*)&GetPlayerObject = 0x004364E0;  //CNWSPlayer::GetGameObject(void)
CNWSObject *(__fastcall *GetPlayerObject)(void *pPlayer);
// *(dword*)&pGetPlayerByOID = 0x0042CD20;  //CServerExoApp::GetClientObjectByObjectId(unsigned long)
CNWSPlayer *(__fastcall *pGetPlayerByOID)(CServerExoApp *pThis, void *, nwn_objid_t ObjID);
// *(dword*)&pGetPlayerList = 0x0042C920;  //CServerExoApp::GetPlayerList(void)
void *(__fastcall *pGetPlayerList)(CServerExoApp *pServerExo);
// *(dword*)&pGetServerMessage = 0x0042C940;  //CServerExoApp::GetNWSMessage(void)
void *(__fastcall *pGetServerMessage)(CServerExoApp *pServerExo);


// *(dword*)&GetClientObjectByPlayerId = 0x0042CD30; // CServerExoApp::GetClientObjectByPlayerId(unsigned long, unsigned char)
// NOTE: actually returns CNWSClient*
CNWSPlayer *(__fastcall *GetClientObjectByPlayerId)(CServerExoApp *pServerExoApp, void *, dword nClientID, unsigned char flag);
// *(dword*)&CNWMessage__CreateWriteMessage = 0x00507E30; // CNWMessage::CreateWriteMessage(unsigned long, unsigned long, int)
void (__fastcall *CNWMessage__CreateWriteMessage)(CNWSMessage *pMessage, void *, dword length, dword recipient, dword flag);
// *(dword*)&CNWSMessage__WriteGameObjUpdate_UpdateObject = 0x00445160; // CNWSMessage::WriteGameObjUpdate_UpdateObject(CNWSPlayer *, CNWSObject *, CLastUpdateObject *, unsigned long, unsigned long)
void (__fastcall *CNWSMessage__WriteGameObjUpdate_UpdateObject)(CNWSMessage *pMessage, void *, CNWSPlayer *pPlayer, CNWSObject *pObject, CLastUpdateObject *pLUO, dword flags, dword AppearanceFlags);
// *(dword*)&CNWMessage__GetWriteMessage = 0x00508B80; // CNWMessage::GetWriteMessage(unsigned char **, unsigned long *)
int (__fastcall *CNWMessage__GetWriteMessage)(CNWSMessage *pMessage, void *, char **ppData, dword *pLength);
// *(dword*)&CNWSMessage__SendServerToPlayerMessage = 0x00449F40; // int __cdecl CNWSMessage__SendServerToPlayerMessage(int, int, char, char, void *ptr, int)
int (__fastcall *CNWSMessage__SendServerToPlayerMessage)(CNWSMessage *pMessage, void *, dword nPlayerID, byte type, byte subtype, char *dataPtr, dword length);
// *(dword*)&CNWSMessage__SendServerToPlayerPlayerList_All = 0x0044A4C0; // CNWSMessage::SendServerToPlayerPlayerList_All(CNWSPlayer *)
int (__fastcall *CNWSMessage__SendServerToPlayerPlayerList_All)(CNWSMessage *pMessage, void *, CNWSPlayer *pPlayer);
int (__fastcall *ExamineCreatureNextHook)(void *, void *, CNWSPlayer *, unsigned long);
void (__fastcall *MsgSetOIDNextHook)(void *, void *, int, unsigned long);

int (__fastcall *pRunScript)(CVirtualMachine *, void *, CExoString *, unsigned long, int);

dword *ppServer = 0;
void *pServer = 0;
CServerExoApp *pServerExo = 0;
CServerExoAppInternal *pServerExo4 = 0;
CVirtualMachine *pScriptThis = 0;
CGameObjectArray *pObjectClass = 0;
void *pFactionClass = 0;
void *pClientClass = 0;
dword *pEBP;

char scriptRun = 0;

void GetTypeFromVtable(dword* pVtable, char *buf);

// A bit of a hack to make this compatible with the nwnx_events examine creature hook.
nwn_objid_t nExCrePlayerObjID = OBJECT_INVALID;
nwn_objid_t nExCreCreatureObjID = OBJECT_INVALID;
// And this one because a stack var on Linux was optimized into a register on Windows.
nwn_objid_t nPartyInviteeObjID = OBJECT_INVALID;

int __fastcall ExamineCreatureHookProc(void *thisptr, void *, CNWSPlayer *a, unsigned long b)
{
	nExCrePlayerObjID = a->m_oidGameObjID;
	nExCreCreatureObjID = b;
	int ret = ExamineCreatureNextHook(thisptr, NULL, a, b);
	nExCrePlayerObjID = OBJECT_INVALID;
	nExCreCreatureObjID = OBJECT_INVALID;
	return ret;
}

// void __thiscall CNWCCMessageData::SetObjectID(int, unsigned long)
void __fastcall MsgSetOIDHookProc(void *thisptr, void *, int a, unsigned long b)
{
	nPartyInviteeObjID = b;
	MsgSetOIDNextHook(thisptr, NULL, a, b);
}

//################################################################

/*
void *CExoLinkedListInternal__GetFirst(void **pList)
{
	if(!pList) return NULL;
	return *pList;
}

void *CExoLinkedListInternal__GetLast(void **pList)
{
	if(!pList) return NULL;
	return *(pList+1);
}

int CExoLinkedListInternal__GetSize(void *pList)
{
	if(!pList) return -1;
	return *((int *)pList+2);
}

void *CExoLinkedListInternal__GetAtPos(void *pList, void **pListItem)
{
	if(!pListItem) return NULL;
	return *(pListItem+2);
}

void *CExoLinkedListInternal__GetNext(void *pList, void **ppListItem)
{
	if(!ppListItem) return NULL;
	void **pListItem = (void **) *ppListItem;
	if(!pListItem) return NULL;
	if(*(pListItem+1))
	{
		*ppListItem = *(pListItem+1);
		return *(*(void ***)ppListItem+2);
	}
	else
	{
		ppListItem = NULL;
		return NULL;
	}
}
*/

//################################################################

void *GetObjectByID(dword ObjID)
{
	if(!pServerExo) InitConstants();
	void *pObject;
	pGetObjByOID(pObjectClass, NULL, ObjID, &pObject);
	return pObject;
}
/*
long GetOIDByObj(void *pObject)
{
	return *((dword*)pObject+0x4);
}
*/
void *GetPlayer(dword ObjID)
{
	if(!pServerExo) InitConstants();
	return pGetPlayerByOID(pServerExo, NULL, ObjID);
}

CNWSPlayer *GetPlayerByClientID(dword nClientID)
{
	if(!pServerExo) InitConstants();
	return GetClientObjectByPlayerId(pServerExo, NULL, nClientID, 0);
}

int GetIsPC(dword ObjID)
{
	if(!pServerExo) InitConstants();
	void *pPlayer = GetPlayer(ObjID);
	if(pPlayer) return 1;
	else return 0;
}

int GetIsDM(dword ObjID)
{
	if(!pServerExo) InitConstants();
	CNWSCreature *pCreature = (CNWSCreature *) GetObjectByID(ObjID);
	if(!pCreature || !pCreature->m_pCreStats) return 0;
	if(pCreature->m_pCreStats->m_bIsDM) return 1;
	else return 0;
}
/*
void *GetPlayerList()
{
	if(!pServerExo) InitConstants();
	return pGetPlayerList(pServerExo);
}
*/
void SendNewName(dword nPlayerObjID, dword nObjID)
{
	if(!pServerExo) InitConstants();
	CNWSMessage *pServerMessage = (CNWSMessage *) pGetServerMessage(pServerExo);
	CNWSPlayer *pPlayer = (CNWSPlayer *) GetPlayer(nPlayerObjID);
	CNWSObject *pObject = (CNWSObject *) GetObjectByID(nObjID);
	CLastUpdateObject luo;
	char *pData;
	dword length;
	if(!pServerMessage || !pPlayer || !pObject || pObject->m_nObjectType != 5) return;
	//pSendNewName(pServerMessage, pPlayer, pObject);
	CNWMessage__CreateWriteMessage(pServerMessage, NULL, 0x400, pPlayer->PlayerID, 1);
	CNWSMessage__WriteGameObjUpdate_UpdateObject(pServerMessage, NULL, pPlayer, pObject, &luo, 0, 0x400);
	CNWMessage__GetWriteMessage(pServerMessage, NULL, &pData, &length);
	if(length)
	{
		CNWSMessage__SendServerToPlayerMessage(pServerMessage, NULL, pPlayer->PlayerID, 5, 1, pData, length);
	}
}

void SendPlayerList(dword nPlayerObjID)
{
	if(!pServerExo) InitConstants();
	CNWSMessage *pServerMessage = (CNWSMessage *) pGetServerMessage(pServerExo);
	CNWSPlayer *pPlayer = (CNWSPlayer *) GetPlayer(nPlayerObjID);
	if(!pServerMessage || !pPlayer) return;
	CNWSMessage__SendServerToPlayerPlayerList_All(pServerMessage, NULL, pPlayer);
}

char *GetServerFuncName(dword Addr)
{	
	if(Addr==0x0043B3DC || Addr==0x0043B3E9) 						return "CNWSMessage::SendServerToPlayerCharList";
	if(Addr==0x004D56E1) 											return "CNWSModule::PackModuleIntoMessage";
	if(Addr==0x0050F09E) 											return "CNWSArea::PackAreaIntoMessage";
	if(Addr==0x0044A3A8 || Addr==0x0044A3B6) 						return "CNWSMessage::SendServerToPlayerPlayerList_Add";
	if(Addr==0x0044A76F || Addr==0x0044A77D) 						return "CNWSMessage::SendServerToPlayerPlayerList_All";
	if(Addr==0x0043EA88 || Addr==0x0043EA9B) 						return "CNWSMessage::WriteGameObjUpdate_UpdateAppearance";
	if(Addr==0x00446C36 || Addr==0x00446D09 || Addr==0x00446D1D) 	return "CNWSMessage::SendServerToPlayerExamineGui_CreatureData";
	if(Addr==0x00440626 || Addr==0x00440902) 						return "CNWSMessage::CreateNewLastUpdateObject";
	if(Addr==0x0044617C) 											return "CNWSMessage::WriteGameObjUpdate_UpdateObject*";
	if(Addr==0x0044671B || Addr==0x0044672E || 
		Addr==0x00446609 || Addr==0x0044661C) 						return "CNWSMessage::WriteGameObjUpdate_UpdateObject";
	if(Addr==0x00436CF4) 											return "CNWSMessage::AddActiveItemPropertiesToMessage*";
	if(Addr==0x0043E0AB || Addr==0x0043E0C0) 						return "CNWSMessage::SendServerToPlayerChat_Shout*";
	if(Addr==0x0043D8DE || Addr==0x0043D8F3) 						return "CNWSMessage::SendServerToPlayerChat_Party";
	if(Addr==0x0043DDDE || Addr==0x0043DDF3) 						return "CNWSMessage::SendServerToPlayerChat_Tell";
	
	// TODO: particularly test these addresses
	if(Addr==0x0057095F || Addr==0x005706CB)						return "CNWVirtualMachineCommands::ExecuteCommandGetName#";

	// TODO: and these
	if(Addr==0x00597321 || Addr==0x0059744B ||
		Addr==0x0059769D || Addr==0x005977F5) 						return "CNWVirtualMachineCommands::ExecuteCommandGetDescription";

	if(Addr==0x00448833) 											return "CNWSMessage::SendServerToPlayerDungeonMasterAreaList";
	if(Addr==0x0046D505) 											return "CNWSCreatureStats::GetFullName";
	
	return "unknown";
}

void DebugPrintHookProc(long someshit, char *str, ...)
{
	if(names.logDebug)
	{
		va_list argList;
		char acBuffer[2048];

		va_start(argList, str);
		vsnprintf(acBuffer, 2047, str, argList);
		acBuffer[2047] = 0;
		va_end(argList);
		int nlen = strlen(acBuffer);
		if(nlen>0 && nlen<2047 && acBuffer[nlen-1]!='\n') 
		{
			acBuffer[nlen]='\n';
			acBuffer[nlen+1]=0;
		}

		names.Log(1, "NWNDEBUG: %s", acBuffer);
	}
}

// This function is pretty awful. Unfortunately, we can't unwind stack frames
// like we can in Linux (Win uses FPO), so instead we take the stupid/naive
// approach and walk up the stack until we find a known address. Alternatives
// include finding the stack size at all calls of interest, or hooking all
// callers of interest and setting values/flags. This is simpler, but less
// robust.
dword *GetSecondLevelStackFrame(dword *firstframe, dword firstlevel)
{
	names.Log(1, "GetSecondLevelStackFrame: firstframe=%08lX, *firstframe=%08lX, firstlevel=%08lX\n", firstframe, *firstframe, firstlevel);
	for(int i=0; i<150; i++)
	{
		dword *secondleveltest = firstframe + i;
		if(firstlevel == 0x00508D7E)
		{
			names.Log(4, "GetSecondLevelStackFrame: i=%03lX, secondleveltest=%08lX, *secondleveltest=%08lX\n", i, secondleveltest, *secondleveltest);
			if(*secondleveltest == 0x0043EA88 || *secondleveltest == 0x0043EA9B)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::WriteGameObjUpdate_UpdateAppearance (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0044671B || *secondleveltest==0x0044672E || *secondleveltest==0x00446609 || *secondleveltest==0x0044661C)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::WriteGameObjUpdate_UpdateObject (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0044A3A8 || *secondleveltest==0x0044A3B6)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerPlayerList_Add (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0044A76F || *secondleveltest==0x0044A77D)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerPlayerList_All (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x00450FC3 || *secondleveltest==0x00450FD0)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerPlayModuleCharacterListResponse (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x00446D09 || *secondleveltest==0x00446D1D)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerExamineGui_CreatureData (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x00440626 || *secondleveltest==0x00440902)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::CreateNewLastUpdateObject (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0043D8DE || *secondleveltest==0x0043D8F3)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerChat_Party (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0043DDDE || *secondleveltest==0x0043DDF3)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerChat_Tell (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0043E0AB || *secondleveltest==0x0043E0C0)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSMessage::SendServerToPlayerChat_Shout (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
		}
		else if(firstlevel == 0x005091CD || firstlevel == 0x00509116)
		{
			if(*secondleveltest==0x0057095F || *secondleveltest==0x005706CB)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWVirtualMachineCommands::ExecuteCommandGetName (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
			else if(*secondleveltest==0x0046D505)
			{
				names.Log(1, "GetSecondLevelStackFrame: CNWSCreatureStats::GetFullName (%08lX)\n", *secondleveltest);
				return secondleveltest - 1;
			}
		}
	}
	names.Log(1, "GetSecondLevelStackFrame: NOT FOUND (first-level function %08lX)\n", firstlevel);
	return NULL;
}

void GetTypeFromVtable(dword* pVtable, char *buf)
{
	CNWSPlayer *pl = NULL;
	CNWSObject *obj = NULL;
	CNWSCreature *cre = NULL;

	dword vtable_addr = *pVtable;
	switch(vtable_addr)
	{
		case 0x006336BC: // CNWSPlayer
			pl = (CNWSPlayer*)pVtable;
			_snprintf(buf, 200, "CNWSPlayer (id %08lX, bicfile %s)", pl->PlayerID, pl->pl_bicfile);
			break;
		case 0x00633D70: // CNWSObject
			obj = (CNWSObject*)pVtable;
			_snprintf(buf, 200, "CNWSObject (id %08lX, lastname ?)", obj->m_oid);
			break;
		case 0x00633A24: // CNWSCreature
			cre = (CNWSCreature*)pVtable;
			_snprintf(buf, 200, "CNWSCreature (id %08lX, lastname ?)", cre->m_oid);
			break;
		default:
			sprintf(buf, "");
	}
	buf[199] = '\0';
}

// int __thiscall CExoLocString::GetStringLoc(int, class CExoString *, unsigned char)
int __fastcall GetNameHookProc(void *thisptr, void *, int some_flag, CExoString *objName_buf, unsigned char flag)
{
	// grab return address
	__asm { lea ebx, [ebp] }
	__asm { mov pEBP, ebx }
	dword retaddr = *(dword *)(((unsigned char *)*(pEBP+1))+1);

	dword nPlayerObjID = 0;
	dword nCreatureObjID = 0;
	bool bBroadcast = 0;

	char **pNewName;
	char *NewName;
	CPlayerNames *pPlayerEntry;
	int name_type;

	dword *secondlevel_base = GetSecondLevelStackFrame(pEBP, retaddr);
	if(secondlevel_base == NULL)
	{
		// not a caller of interest -- behave normally
		names.Log(2, "GetName: secondlevel_base NULL\n");
		goto ext;
	}

	// hook this call as appropriate for the caller
	dword secondlevel = *(secondlevel_base+1);
	// AddrNameMapping mapping = GetMapping(secondlevel);
	names.Log(2, "GetName: secondlevel_base=%08lX, secondlevel=%08lX (%s)\n", secondlevel_base, secondlevel, GetServerFuncName(secondlevel));
	// names.Log(2, "GetName: secondlevel_base=%08lX, secondlevel=%08lX (%s)\n", secondlevel_base, secondlevel, mapping.name);
	if(retaddr == 0x00508D7E)
	{
		// CNWSMessage::WriteGameObjUpdate_UpdateAppearance
		if(secondlevel==0x0043EA88 || secondlevel==0x0043EA9B)
		{
			if(!CALL_HANDLER(CNWSMessage__WriteGameObjUpdate_UpdateAppearance))
				goto ext;
		}
		// CNWSMessage::WriteGameObjUpdate_UpdateObject
		else if(secondlevel==0x0044671B || secondlevel==0x0044672E || secondlevel==0x00446609 || secondlevel==0x0044661C)
		{
			if(!CALL_HANDLER(CNWSMessage__WriteGameObjUpdate_UpdateObject))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerPlayerList_Add
		else if(secondlevel==0x0044A3A8 || secondlevel==0x0044A3B6)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerPlayerList_Add))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerPlayerList_All
		else if(secondlevel==0x0044A76F || secondlevel==0x0044A77D)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerPlayerList_All))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerPlayModuleCharacterListResponse
		else if(secondlevel==0x00450FC3 || secondlevel==0x00450FD0)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerPlayModuleCharacterListResponse))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerExamineGui_CreatureData
		else if(secondlevel==0x00446D09 || secondlevel==0x00446D1D)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerExamineGui_CreatureData))
				goto ext;
		}
		// CNWSMessage::CreateNewLastUpdateObject
		else if(secondlevel==0x00440626 || secondlevel==0x00440902)
		{
			if(!CALL_HANDLER(CNWSMessage__CreateNewLastUpdateObject))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerChat_Party
		else if(secondlevel==0x0043D8DE || secondlevel==0x0043D8F3)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerChat_Party))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerChat_Tell
		else if(secondlevel==0x0043DDDE || secondlevel==0x0043DDF3)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerChat_Tell))
				goto ext;
		}
		// CNWSMessage::SendServerToPlayerChat_Shout
		else if(secondlevel==0x0043E0AB || secondlevel==0x0043E0C0)
		{
			if(!CALL_HANDLER(CNWSMessage__SendServerToPlayerChat_Shout))
				goto ext;
		}
		
		else goto ext;
	}
	else if(retaddr==0x005091CD || retaddr==0x00509116) // GetLocStringServer
	{		
		// CNWVirtualMachineCommands::ExecuteCommandGetName
		if(secondlevel==0x0057095F || secondlevel==0x005706CB)
		{
			if(!CALL_HANDLER(CNWVirtualMachineCommands__ExecuteCommandGetName))
				goto ext;
		}
		// CNWSCreatureStats::GetFullName
		else if(secondlevel==0x0046D505)
		{
			if(retaddr == 0x00509116) name_type = eFirstName;
			else if(retaddr == 0x005091CD) name_type = eLastName;
			if(!CALL_HANDLER(CNWSCreatureStats__GetFullName))
				goto ext;
		}
		else
		{
			//dbg = 0;
			//names.Log(4+dbg, "--GetLocStringServer_caller = %08lX (%s)\n", GetLocStringServer_caller, GetServerFuncName(GetLocStringServer_caller));
			goto ext;
		}
		//names.Log(4+dbg, "--GetLocStringServer_caller = %08lX (%s)\n", GetLocStringServer_caller, GetServerFuncName(GetLocStringServer_caller));
	}

	//
	// Data's all set up, so replace the name as appropriate
	//

	CNWSCreature *pPlayerObj = (CNWSCreature *)GetObjectByID(nPlayerObjID);
	CNWSCreature *pCreature = (CNWSCreature *)GetObjectByID(nCreatureObjID);

	// set up nPlayerObjID properly (if player is possessing, get master's oid)
	if(pPlayerObj)
	{
		// if possessed (familiar etc.), set nPlayerObjID to master's ID
		// TODO: why don't we have to check != 0 on Linux?
		if(pPlayerObj->m_oidMaster != OBJECT_INVALID && pPlayerObj->m_oidMaster != 0)
		{
			void *pCreatureTmp = GetObjectByID(pPlayerObj->m_oidMaster);
			if(pCreatureTmp)
			{
				nPlayerObjID = pPlayerObj->m_oidMaster;
			}
		}
	}
	
	pNewName = new char *;
	names.Log(3, "Finding custom name for %08lX-%08lX... ", nPlayerObjID, nCreatureObjID);

	// get player entry, bail on failure
	pPlayerEntry = names.Names.FindPlayerEntry(nPlayerObjID);
	if(!pPlayerEntry || !pPlayerEntry->bEnabled)
	{
		names.Log("couldn't find custom name; bailing.\n");
		goto ext;
	}

	// find custom name for creature in question wrt player's POV
	NewName = names.Names.FindCustomName(nPlayerObjID, nCreatureObjID);
	names.Log(3, "done.\n");

	if(!NewName || bBroadcast) 
	{
		names.Log(3, "No new name\n");
		// If (target is not the player OR this is a broadcast) AND the target is a mortal PC...
		if(((nPlayerObjID!=nCreatureObjID) || bBroadcast) && GetIsPC(nCreatureObjID) && !GetIsDM(nCreatureObjID))
		{
			names.Log(3, "IsPC\n");
			int nStyle;
			pPlayerEntry = names.Names.FindPlayerEntry(nPlayerObjID);
			if(!pPlayerEntry) nStyle=1;
			else
			{
				nStyle = pPlayerEntry->UnknownStyle;
			}
			if(nStyle==0)
			{
				NewName = "";
			}
			if(nStyle==1)
			{
				CNWSCreature *nCreature = (CNWSCreature*)pCreature;
				uint32_t nCreatureGender = nCreature->m_pCreStats->m_nGender;
				if(nCreatureGender==1) NewName = "<c–––>A Mysterious Lady</c>";
				else NewName = "<c–––>A Mysterious Dude</c>";
			}
			else if(nStyle==2)
			{
				NewName = "<c–––>Unknown</c>";	
			}
			else
			{
				NewName = "RUSSIAN EXCLAMATION MARKS!!!";
			}
		}
		else
		{
			goto ext2;
		}
	}

	// Copy new name to output buffer
	if(name_type==eFirstName) *pNewName=NewName;
	else *pNewName="";
	CExoString *sNewName = new CExoString();
	sNewName->Text = *pNewName;
	sNewName->Length = strlen(*pNewName);
	d_newstrcpy(objName_buf, NULL, sNewName);
	delete pNewName;
	delete sNewName;
	names.Log(4, "--Return value: %s\n", *(char **)objName_buf);
	names.Log(1, "END GetName (overrode name)\n\n");
	return 1;
	
	ext2:
	delete pNewName;
	goto ext;

	ext:
	names.Log(1, "END GetName (ext)\n\n");
	return GetNameNextHook(thisptr, NULL, some_flag, objName_buf, flag);
}

// int __thiscall CVirtualMachine::RunScript(class CExoString *, unsigned long, int)
void RunScript(char * sname, int ObjID)
{
	// set up script name
	CExoString s;
	s.Text = sname;
	s.Length = strlen(sname);
	// run script
	scriptRun = 1;
	pRunScript(pScriptThis, NULL, &s, ObjID, 1);
	scriptRun = 0;
}

void InitConstants()
{
	*(dword*)&pServer = *ppServer; //CAppManager
	*(dword*)&pServerExo = *(dword*)((char*)pServer+0x4);  //CServerExoApp
	*(dword*)&pServerExo4 = *(dword*)((char*)pServerExo+0x4);  //CServerExoAppInternal
	
	*(dword*)&pObjectClass = *(dword*)(*(dword*)((char*)pServerExo+0x4)+0x10080);
	*(dword*)&pFactionClass = *(dword*)(*(dword*)((char*)pServerExo+0x4)+0x10074);
	*(dword*)&pClientClass = *(dword*)(*(dword*)((char*)pServerExo+0x4)+0x10060);
}

int HookFunctions()
{
	//dword org_SaveChar = FindHookSaveChar();
    //dword org_Run = FindHookRunScript();
	dword org_GetName = 0x00610DB0;  //CExoLocString::GetStringLoc(int, CExoString *, unsigned char)
	dword org_ExamineCreature = 0x00446B00; // int __thiscall CNWSMessage::SendServerToPlayerExamineGui_CreatureData(class CNWSPlayer *, unsigned long)
	dword org_MsgSetOID = 0x005067D0; // int (__fastcall *ExamineCreatureNextHook)(void *, void *, CNWSPlayer *, unsigned long)
	*(dword*)&d_newstrcpy = 0x0040B9D0;  //CExoString::__as(CExoString const &) (aka CExoString::operator=)
	*(dword*)&GetPlayerObject = 0x004364E0;  //CNWSPlayer::GetGameObject(void)
	*(dword*)&pGetPlayerByOID = 0x0042CD20;  //CServerExoApp::GetClientObjectByObjectId(unsigned long)
	*(dword*)&pGetPlayerList = 0x0042C920;  //CServerExoApp::GetPlayerList(void)
	*(dword*)&pRetObjByOID = 0x0042C840;  //CServerExoApp::GetCreatureByGameObjectID(unsigned long)
	*(dword*)&pGetServerMessage = 0x0042C940;  //CServerExoApp::GetNWSMessage(void)
	*(dword*)&pGetObjByOID = 0x00466980; // CGameObjectArray::GetGameObject(unsigned long, CGameObject **)
	*(dword*)&GetClientObjectByPlayerId = 0x0042CD30; // CServerExoApp::GetClientObjectByPlayerId(unsigned long, unsigned char)

	*(dword*)&CNWMessage__CreateWriteMessage = 0x00507E30; // CNWMessage::CreateWriteMessage(unsigned long, unsigned long, int)
	*(dword*)&CNWSMessage__WriteGameObjUpdate_UpdateObject = 0x00445160; // CNWSMessage::WriteGameObjUpdate_UpdateObject(CNWSPlayer *, CNWSObject *, CLastUpdateObject *, unsigned long, unsigned long)
	*(dword*)&CNWMessage__GetWriteMessage = 0x00508B80; // CNWMessage::GetWriteMessage(unsigned char **, unsigned long *)
	*(dword*)&CNWSMessage__SendServerToPlayerMessage = 0x00449F40; // int __cdecl CNWSMessage__SendServerToPlayerMessage(int, int, char, char, void *ptr, int)
	*(dword*)&CNWSMessage__SendServerToPlayerPlayerList_All = 0x0044A4C0; // CNWSMessage::SendServerToPlayerPlayerList_All(CNWSPlayer *)

	ppServer = (dword *) 0x0066C050;  //CAppManager *g_pAppManager
	*(dword*)&pScriptThis = (dword)((char*)ppServer-0x8);

	*(dword*)&pRunScript = (dword)((char*)0x005BF9D0);

	HookCode((PVOID) org_GetName, GetNameHookProc, (PVOID*) &GetNameNextHook);
	HookCode((PVOID) org_ExamineCreature, ExamineCreatureHookProc, (PVOID*) &ExamineCreatureNextHook);
	HookCode((PVOID) org_MsgSetOID, MsgSetOIDHookProc, (PVOID*) &MsgSetOIDNextHook);

	return 1;
}
