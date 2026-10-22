#include <stdio.h>
#include <string.h>

#include "Battlefront2/Source/FirstPerson.h"
#include "Battlefront2/Source/LoadUtil.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"

// FUNCTION: SWBF2 0x00411d20
void FirstPerson::LoadLVLFile(char const* pName)
{
    char acPath[256];
    char acName[256];

    strncpy(acName, pName, sizeof(acName) - 1);
    acName[sizeof(acName) - 1] = 0;
    *strchr(acName, ';') = 0;

    _snprintf(acPath, sizeof(acPath) - 1, "%s%s.lvl", "FPM\\", acName);
    acPath[sizeof(acPath) - 1] = 0;

    LoadUtil::ReadDataFile(acPath, 0, 0, 0, 0);
}

// FUNCTION: SWBF2 0x00411da0
void FirstPerson::InitPlatform()
{
    for (int i = 0; i < FirstPerson::sModelCount; i++)
    {
        LoadLVLFile(sModelName[i]);
    }
}

// FUNCTION: SWBF2 0x00411cb0
const RedModel* FirstPerson::LoadDynamicModel(const char* pszName, bool bArg)
{
    char acName[256];
    strncpy(acName, pszName, sizeof(acName) - 1);
    acName[sizeof(acName) - 1] = 0;
    char* pFirst = strchr(acName, ';');
    char* pSecond = strchr(pFirst, ';');
    return RedModel::_HashTable.Find(PblHash(bArg ? pFirst + 1 : pSecond));
}
