#include "HookHandlers.h"

extern nwn_objid_t nExCrePlayerObjID;
extern nwn_objid_t nExCreCreatureObjID;
extern nwn_objid_t nPartyInviteeObjID;

HANDLER(CNWSCreatureStats__GetFullName)
{
	INIT_OUTPUTS();

	int test_offset_1 = 0x50 / 4; // (call from CNWSMessage::HandlePlayerToServerParty(CNWSPlayer *,uchar))
	// int test_offset_2 = 0xA8 / 4; // (call from CNWSMessage::HandlePlayerToServerMessage(ulong,uchar *,ulong))
	int test_offset_3 = 0x68 / 4; // This is a hack. Argh. Why is the first param to HandlePlayerToServerParty broken?
	if(*(secondlevel_base + test_offset_1) == 0x00543E11)
	{
		nPlayerObjID = nPartyInviteeObjID;
		// I have no idea why the arguments are getting stomped. Maybe I've been staring at this too long.
		CNWSPlayer *pInviter = *(CNWSPlayer **)(secondlevel_base + test_offset_3);
		nCreatureObjID = pInviter->m_oidGameObjID;
	}
	else
	{
		// Apparently there are plenty of cases here we just don't care about,
		// so we won't even bother printing a log message.
		return 0;
	}

	// SET_OUTPUTS();
	*nPlayerObjID_out = nPlayerObjID;
	*nCreatureObjID_out = nCreatureObjID;
	*bBroadcast_out = bBroadcast;
	return 1;
}


HANDLER(CNWSMessage__SendServerToPlayerExamineGui_CreatureData)
{
	INIT_OUTPUTS();

	// Ugh. The nwnx_events examine creature hook may mess up the stack, so we have to hook
	// the upstream function and store our data. We should probably do this for every hook
	// anyway, but not right now! --Zeb

	if(secondlevel==0x00446D09) name_type = eFirstName;
	else if(secondlevel==0x00446D1D) name_type = eLastName;

	if(nExCrePlayerObjID != OBJECT_INVALID && nExCreCreatureObjID != OBJECT_INVALID)
	{
		nPlayerObjID = nExCrePlayerObjID;
		nCreatureObjID = nExCreCreatureObjID;
	}
	else
	{
		names.Log(1, "Invalid state in CNWSMessage::SendServerToPlayerExamineGui_CreatureData handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}


HANDLER(CNWSMessage__SendServerToPlayerPlayerList_All)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0044A76F) name_type = eFirstName;
	else if(secondlevel==0x0044A77D) name_type = eLastName;

	int test_offset_1 = 0xD8 / 4; // (call from CNWSMessage::HandlePlayerToServerMessage(ulong,uchar *,ulong))
	int test_offset_2 = 0x94 / 4; // (call from CNWSMessage::HandlePlayerToServerModuleMessage(CNWSPlayer *,uchar))
	int test_offset_3 = 0xA8 / 4; // (call from CNWSMessage::HandlePlayerToServerMessage(ulong,uchar *,ulong))
	if(*(secondlevel_base + test_offset_1) == 0x0054283C && *(secondlevel_base + test_offset_2) == 0x0054330E)
	{
		CNWSPlayer *pPlayer = *(CNWSPlayer **)(secondlevel_base + test_offset_1 + 1);
		nCreatureObjID = *(secondlevel_base + test_offset_2 - 0x70/4);

		nPlayerObjID = pPlayer->m_oidGameObjID;

		names.Log(3, "Got data\n");
	}
	else if(*(secondlevel_base + test_offset_3) == 0x00542A18 && *(secondlevel_base + test_offset_2) == 0x0054366D)
	{
		CNWSPlayer *pPlayer = *(CNWSPlayer **)(secondlevel_base + test_offset_3 + 1);
		nCreatureObjID = *(secondlevel_base + test_offset_2 - 0x70/4);
		nPlayerObjID = pPlayer->m_oidGameObjID;
		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::SendServerToPlayerPlayerList_All handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}

HANDLER(CNWSMessage__SendServerToPlayerPlayModuleCharacterListResponse)
{
	INIT_OUTPUTS();

	// TODO: this will get stomped by nwnx_fixes when I fix those fixes, so I'll need to fix it, or we'll be in a fix.
	// For now, though, we'll use the easy approach. --Zeb

	if(secondlevel==0x00450FC3) name_type = eFirstName;
	else if(secondlevel==0x00450FD0) name_type = eLastName;

	int test_offset_1 = 0x54 / 4; // (call from CNWSMessage::HandlePlayerToServerPlayModuleCharacterList_Start(CNWSPlayer *))
	if(*(secondlevel_base + test_offset_1) == 0x00546600)
	{
		nCreatureObjID = *(secondlevel_base + test_offset_1 + 2);
		nPlayerObjID = nCreatureObjID;
		bBroadcast = 1;
		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::SendServerToPlayerPlayModuleCharacterListResponse handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}


HANDLER(CNWSMessage__SendServerToPlayerChat_Party)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0043D8DE) name_type = eFirstName;
	else if(secondlevel==0x0043D8F3) name_type = eLastName;

	dword test_offset = 0x3C / 4;
	if(*(secondlevel_base + test_offset) == 0x004E9A1D)
	{
		dword nClientID = *(secondlevel_base + test_offset + 1);
		nCreatureObjID = *(secondlevel_base + test_offset + 2);
		CNWSPlayer *pl = GetPlayerByClientID(nClientID);
		if(!pl) return 0;
		nPlayerObjID = pl->m_oidGameObjID;
		names.Log(3, "Got data\n");
	}
	else if(*(secondlevel_base + test_offset) == 0x0043D0CC) // message is going to DMs
	{
		return 0;
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::SendServerToPlayerChat_Party handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}

HANDLER(CNWSMessage__WriteGameObjUpdate_UpdateAppearance)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0043EA88) name_type = eFirstName;
	else if(secondlevel==0x0043EA9B) name_type = eLastName;

	dword test_offset = 0xC0 / 4;
	if(*(secondlevel_base + test_offset) == 0x0043F7F1)
	{
		CNWSPlayer *pl = *(CNWSPlayer **)(secondlevel_base + test_offset + 1);
		CNWSCreature *cre = *(CNWSCreature **)(secondlevel_base + test_offset + 2);

		nPlayerObjID = pl->m_oidGameObjID;
		nCreatureObjID = cre->m_oid;

		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::WriteGameObjUpdate_UpdateAppearance handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}
HANDLER(CNWSMessage__SendServerToPlayerPlayerList_Add)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0044A3A8) name_type = eFirstName;
	else if(secondlevel==0x0044A3B6) name_type = eLastName;

	int test_offset_1 = 0xCC / 4;
	int test_offset_2 = 0x84 / 4;
	int test_offset_3 = 0x9C / 4;
	if(*(secondlevel_base + test_offset_1) == 0x0054283C
		&& (*(secondlevel_base + test_offset_2) == 0x005432F1 || *(secondlevel_base + test_offset_2) == 0x00543304))
	{
		CNWSPlayer *pPlayer = *(CNWSPlayer **)(secondlevel_base + test_offset_1 + 1);
		dword nClientID = *(secondlevel_base + test_offset_2 + 1);
		nCreatureObjID = *(secondlevel_base + test_offset_2 + 2);

		names.Log(3, "Message type: %08lX\n", nClientID);
		if(nClientID >= PLAYERID_ALL_SERVERADMINS && nClientID != PLAYERID_ALL_GAMEMASTERS)
		{
			bBroadcast=1;
			nPlayerObjID = nCreatureObjID;
		}
		else
		{
			if(!pPlayer) return 0;
			nPlayerObjID = pPlayer->m_oidGameObjID;
		}
		names.Log(3, "Got data\n");
	}
	else if((*(secondlevel_base + test_offset_2) == 0x0054364F || *(secondlevel_base + test_offset_2) == 0x00543663)
		&& *(secondlevel_base + test_offset_3) == 0x00542A18)
	{
		CNWSPlayer *pPlayer = *(CNWSPlayer **)(secondlevel_base + test_offset_3 + 1);
		dword nClientID = *(secondlevel_base + test_offset_2 + 1);
		nCreatureObjID = *(secondlevel_base + test_offset_2 + 2);

		names.Log(3, "Message type: %08lX\n", nClientID);
		if(nClientID >= PLAYERID_ALL_SERVERADMINS && nClientID != PLAYERID_ALL_GAMEMASTERS)
		{
			bBroadcast=1;
			nPlayerObjID = nCreatureObjID;
		}
		else
		{
			if(!pPlayer) return 0;
			nPlayerObjID = pPlayer->m_oidGameObjID;
		}
		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::SendServerToPlayerPlayerList_Add handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}
HANDLER(CNWSMessage__CreateNewLastUpdateObject)
{
	INIT_OUTPUTS();

	if(secondlevel==0x00440626) name_type = eFirstName;
	else if(secondlevel==0x00440902) name_type = eLastName;

	dword test_offset = 0x9C / 4;
	if(*(secondlevel_base + test_offset) == 0x0043F9B0)
	{
		CNWSPlayer *pl = *(CNWSPlayer **)(secondlevel_base + test_offset + 1);
		CNWSCreature *cre = *(CNWSCreature **)(secondlevel_base + test_offset + 2);

		nPlayerObjID = pl->m_oidGameObjID;
		nCreatureObjID = cre->m_oid;

		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::CreateNewLastUpdateObject!\n");
		return 0;
	}

	return 0;

	SET_OUTPUTS();
	return 1;
}

HANDLER(CNWSMessage__SendServerToPlayerChat_Tell)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0043DDDE) name_type = eFirstName;
	else if(secondlevel==0x0043DDF3) name_type = eLastName;

	int test_offset = 0x3C / 4;
	if(*(secondlevel_base + test_offset) == 0x0043CB2E) // CNWSMessage::SendServerToPlayerChatMessage+???
	{
		names.Log(3, "CNWSMessage::SendServerToPlayerChat_Tell called from CNWSMessage::SendServerToPlayerChatMessage\n");
		dword nClientID = *(secondlevel_base + test_offset + 1); // first arg
		nCreatureObjID = *(secondlevel_base + test_offset + 2); // second arg
		CNWSPlayer *pPlayer = (CNWSPlayer *)GetPlayerByClientID(nClientID);
		if(!pPlayer)
			return 0;
		nPlayerObjID = pPlayer->m_oidGameObjID;

		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown CNWSMessage::SendServerToPlayerChat_Tell caller!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}

HANDLER(CNWSMessage__SendServerToPlayerChat_Shout)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0043E0AB) name_type = eFirstName;
	else if(secondlevel==0x0043E0C0) name_type = eLastName;

	int test_offset = 0x44 / 4;
	if(*(secondlevel_base + test_offset) == 0x0043CB5A) // CNWSMessage::SendServerToPlayerChatMessage+???
	{
		names.Log(3, "CNWSMessage::SendServerToPlayerChat_Shout called from CNWSMessage::SendServerToPlayerChatMessage\n");

		nCreatureObjID = *(secondlevel_base + test_offset + 2); // second arg
		bBroadcast = 1;
		nPlayerObjID = nCreatureObjID;

		names.Log(3, "Got data\n");
	}
	else
	{
		names.Log(1, "Unknown CNWSMessage::SendServerToPlayerChat_Shout caller!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}


HANDLER(CNWVirtualMachineCommands__ExecuteCommandGetName)
{
	INIT_OUTPUTS();

	// This hook doesn't actually do anything in Linux, so we won't do anything here either!

	return 0;

	SET_OUTPUTS();
	return 1;
}

HANDLER(CNWSMessage__WriteGameObjUpdate_UpdateObject)
{
	INIT_OUTPUTS();

	if(secondlevel==0x0044671B || secondlevel==0x00446609) name_type = eFirstName;
	else if(secondlevel==0x0044672E || secondlevel==0x0044661C) name_type = eLastName;

	dword test_offset_1 = 0x84 / 4;
	if(*(secondlevel_base + test_offset_1) == 0x0043F7F1 || *(secondlevel_base + test_offset_1) == 0x00444F28)
	{
		CNWSPlayer *pl = *(CNWSPlayer**)(secondlevel_base + test_offset_1 + 1);
		CNWSCreature *cre = *(CNWSCreature**)(secondlevel_base + test_offset_1 + 2);
		nPlayerObjID = pl->m_oidGameObjID;
		nCreatureObjID = cre->m_oid;
		names.Log(3, "Got data\n");
		if(nCreatureObjID == 0)
		{
			names.Log(3, "Umm, nCreatureObjID is 0?\n");
		}
	}
	else
	{
		names.Log(1, "Unknown caller in CNWSMessage::WriteGameObjUpdate_UpdateObject handler!\n");
		return 0;
	}

	SET_OUTPUTS();
	return 1;
}
