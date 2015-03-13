#if !defined(HOOKHANDLERS_H_)
#define HOOKHANDLERS_H_

#include "NWNXNames.h"

#define HANDLER(x) int Handler_##x##(dword secondlevel, \
	 dword *secondlevel_base,\
	 dword *nPlayerObjID_out,\
	 dword *nCreatureObjID_out,\
	 bool *bBroadcast_out,\
	 int *name_type_out)

#define INIT_OUTPUTS() dword nPlayerObjID = 0;\
	dword nCreatureObjID = 0;\
	bool bBroadcast = 0;\
	int name_type

#define SET_OUTPUTS() *nPlayerObjID_out = nPlayerObjID;\
	*nCreatureObjID_out = nCreatureObjID;\
	*bBroadcast_out = bBroadcast;\
	*name_type_out = name_type

#define CALL_HANDLER(x) Handler_##x##(secondlevel,\
	secondlevel_base,\
	&nPlayerObjID,\
	&nCreatureObjID,\
	&bBroadcast,\
	&name_type)\

HANDLER(CNWSMessage__WriteGameObjUpdate_UpdateAppearance);
HANDLER(CNWSMessage__WriteGameObjUpdate_UpdateObject);
HANDLER(CNWSMessage__SendServerToPlayerPlayerList_Add);
HANDLER(CNWSMessage__SendServerToPlayerPlayerList_All);
HANDLER(CNWSMessage__SendServerToPlayerPlayModuleCharacterListResponse);
HANDLER(CNWSMessage__SendServerToPlayerExamineGui_CreatureData);
HANDLER(CNWSMessage__CreateNewLastUpdateObject);
HANDLER(CNWSMessage__SendServerToPlayerChat_Party);
HANDLER(CNWSMessage__SendServerToPlayerChat_Tell);
HANDLER(CNWSMessage__SendServerToPlayerChat_Shout);
HANDLER(CNWVirtualMachineCommands__ExecuteCommandGetName);
HANDLER(CNWSCreatureStats__GetFullName);

#endif