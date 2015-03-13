/***************************************************************************
    NWNXNames.cpp - Implementation of the CNWNXNames class.
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

#include "NWNXNames.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNWNXNames::CNWNXNames()
{
}

CNWNXNames::~CNWNXNames()
{
}

BOOL CNWNXNames::OnCreate (const char* LogDir)
{
	// call the base class function
	char log[MAX_PATH];
	sprintf (log, "%s\\nwnx_names.txt", LogDir);
	this->confKey = "NAMES";
	if (!CNWNXBase::OnCreate(log))
		return false;
	fprintf(m_fFile, "NWNX Names V.0.0.1 for Windows\n");
	fprintf(m_fFile, "Copyright 2006-2010 virusman (virusman@virusman.ru)\n");
	fprintf(m_fFile, "Windows port by Zebranky (andrew@mercuric.net)\n");
	fprintf(m_fFile, "visit us at http://www.nwnx.org\n\n");
	
	CIniFile iniFile ("nwnx.ini");

	; // TODO: grab event_script from ini

	if (HookFunctions())
	{
		bHooked=1;
		fprintf(m_fFile, "* Module loaded successfully.\n");
	}
	else
	{
		bHooked=0;
		fprintf(m_fFile, "* Module loaded successfully.\n");
		fprintf(m_fFile, "* Signature recognition failed. Some functions will be disabled.\n");
		//return false;
	}
	fflush(m_fFile);

	if(!this->Names.data)
	{
		Log(0, "Unable to construct names array\n");
		return false;
	}

	return true;
}

void CNWNXNames::InitPlayerList(char *value)
{
	Log(2,"Player %08lX entered the game.\n",this->nGameObjectID);
	dword oPlayer = nGameObjectID;
	int nUnkStyle=atoi(value);
	if(nUnkStyle<0||nUnkStyle>2) nUnkStyle=1;
	if(Names.FindPlayerID(oPlayer)==-1)
	{
		Names.InsertPlayer(oPlayer, nUnkStyle, 1);
	}
}

void CNWNXNames::EnableDisableNames(char *value)
{
	CPlayerNames *pPlayerEntry = Names.FindPlayerEntry(nGameObjectID);
	if(!pPlayerEntry) return;
	pPlayerEntry->bEnabled = (bool) atoi(value);
}

char *CNWNXNames::GetDynamicName(char *value)
{
	dword oPlayer = nGameObjectID;
	dword oObject = strtol(value, NULL, 16);
	char *sName;
	CPlayerNames *pPlayerEntry = Names.FindPlayerEntry(oPlayer);
	if(pPlayerEntry)
	{
		sName = Names.FindCustomName(oPlayer, oObject);
		if(sName)
		{
			char *sNameRet = new char[strlen(sName)+1];
			strncpy(sNameRet, sName, strlen(sName));
			sNameRet[strlen(sName)] = 0;
			return sNameRet;
		}
	}
	value[0] = 0;
	return NULL;
}

void CNWNXNames::SetDynamicName(char *value)
{
	dword oPlayer, oObject;
	oPlayer = nGameObjectID;
	int nParamLen = strlen(value);
	char *nLastDelimiter = strrchr(value, '¬');
	if (!nLastDelimiter || (nLastDelimiter-value)<0)
	{
		Log(3, "o nLastDelimiter error\n");
		return;
	}
	char *sName = new char[nParamLen-(nLastDelimiter-value)+1];
	if(sscanf(value, "%x¬%s", &oObject, sName)<2)
	{
		Log(3, "o sscanf error\n");
		return;
	}
	strcpy(sName, nLastDelimiter+1);
	if(!oPlayer||!oObject)
	{
		Log(3, "o invalid object\n");
		return;
	}
	Names.InsertCustomName(oPlayer, oObject, sName);
	SendNewName(oPlayer, oObject);
}

void CNWNXNames::UpdateDynamicName(char *value)
{
	dword oPlayer = nGameObjectID;
	dword oObject = strtol(value, NULL, 16);
	SendNewName(oPlayer, oObject);
}

void CNWNXNames::UpdatePlayerList(char *value)
{
	dword oPlayer = nGameObjectID;
	SendPlayerList(oPlayer);
}

void CNWNXNames::DeleteDynamicName(char *value)
{
	dword oPlayer = nGameObjectID;
	dword oObject = strtol(value, NULL, 16);
	CPlayerNames *pPlayerEntry = Names.FindPlayerEntry(oPlayer);
	if(pPlayerEntry)
	{
		pPlayerEntry->DeleteByObjectID(oObject);
	}
}

void CNWNXNames::ClearPlayerList(char *value)
{
	Log(2,"Player %08lX is exiting.\n",this->nGameObjectID);
	dword oPlayer = nGameObjectID;
	Names.DeletePlayer(oPlayer);
}

char* CNWNXNames::OnRequest (char* gameObject, char* Request, char* Parameters)
{
	;
	Log(2,"Request: \"%s\"\n",Request);
	Log(2,"Params:  \"%s\"\n",Parameters);
	this->pGameObject = (CNWSObject*)gameObject;
	this->nGameObjectID = pGameObject->m_oid;
	if (strncmp(Request, "INITPLAYERNAMELIST", 18) == 0) 	
	{
		InitPlayerList(Parameters);
		return NULL;
	}
	else if (strncmp(Request, "GETDYNAMICNAME", 14) == 0) 	
	{
		return GetDynamicName(Parameters);
	}
	else if (strncmp(Request, "SETDYNAMICNAME", 14) == 0) 	
	{
		SetDynamicName(Parameters);
		return NULL;
	}
	else if (strncmp(Request, "UPDATEDYNAMICNAME", 17) == 0) 	
	{
		UpdateDynamicName(Parameters);
		return NULL;
	}
	else if (strncmp(Request, "UPDATEPLAYERLIST", 16) == 0) 	
	{
		UpdatePlayerList(Parameters);
		return NULL;
	}
	else if (strncmp(Request, "DELETEDYNAMICNAME", 17) == 0) 	
	{
		DeleteDynamicName(Parameters);
		return NULL;
	}
	else if (strncmp(Request, "CLEARPLAYERNAMELIST", 19) == 0) 	
	{
		ClearPlayerList(Parameters);
		return NULL;
	}
	else if (strncmp(Request, "SETNAMESENABLED", 15) == 0) 	
	{
		EnableDisableNames(Parameters);
		return NULL;
	}
	return NULL;
}

BOOL CNWNXNames::OnRelease ()
{
	Log (0, "o Shutdown.\n");
	fclose(PacketData);
	return CNWNXBase::OnRelease();
}
