#include "typedefs.h"

#ifndef NWNXStructures_h_
#define NWNXStructures_h_

struct CVirtualMachine;
struct CGameObjectArray;
struct CExoLinkedList;
struct CResRef;
struct CExoLocString;
struct CNWSObject;
struct CNWSCreature;
struct CNWSCreatureStats;
struct CServerExoApp;
struct CServerExoAppInternal;
struct CNWSMessage;
struct CNWSCreatureAppearanceInfo;
struct CLastUpdateObject;
struct CNWSPlayer;

#include "CNWSCreature.h"
#include "CNWSCreatureStats.h"
#include "CNWSObject.h"
#include "CNWSPlayer.h"

struct CVirtualMachine {
    uint32_t                   field_00;
};

struct CGameObjectArray {
    uint32_t                   field_00;
};

struct CExoString
{
	char *Text;
	dword Length;
};

struct CNWObjectVarListElement
{
    CExoString sVarName;
    dword      nVarType;
    dword       nVarValue;
};

struct CNWObjectVarList
{
	CNWObjectVarListElement *VarList;
	dword                    VarCount;
};

struct CExoLinkedList
{
  /* 0x0/0 */ void *Header;
  /* 0x4/4 */ unsigned long Count;
};
struct CResRef
{
  /* 0x0/0 */ char ResRef[16];
};
struct CExoLocString
{
  /* 0x0/0 */ CExoLinkedList List;
};
struct CServerExoApp
{
  /* 0x0/0 */ void *vt;
  /* 0x4/4 */ CServerExoAppInternal *Internal;
};
struct CServerExoAppInternal
{
	uint32_t unknown;
};

struct CNWSMessage
{
  /* 0x0/0 */ unsigned long field_0;
};

struct CNWSCreatureAppearanceInfo
{
	uint32_t unknown;
};
struct CLastUpdateObject
{
	uint32_t unknown;
};

#endif
