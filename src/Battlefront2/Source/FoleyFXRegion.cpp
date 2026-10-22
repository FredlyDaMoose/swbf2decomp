#include <stdio.h>

#include "FoleyFXRegion.h"
#include "FoleyFX.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"

// FUNCTION: SWBF2 0x0052cb20
FoleyFXRegionFactory::FoleyFXRegionFactory()
    : RedRegionFactory("foleyfx")
{
}

// FUNCTION: SWBF2 0x0052cae0
FoleyFXRegion::FoleyFXRegion(RedRegionDesc& desc, FoleyFXGroup* const pGroup)
    : RedRegion(desc), mGroup(pGroup)
{
    smList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0052cb70
void* FoleyFXRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    char acGroupName[512];
    char acFormat[512];

    strlcpy(acFormat, GetPrefix(), sizeof(acFormat));
    strlcat(acFormat, " %s", sizeof(acFormat));
    sscanf(pcName, acFormat, acGroupName);

    FoleyFXGroup* const pGroup = FoleyFXGroup::FindByID(PblHash(acGroupName));
    return new FoleyFXRegion(desc, pGroup);
}

// GLOBAL: SWBF2 0x007eb8e8
// SYNTHETIC: SWBF2 0x00403ae0
// `dynamic initializer for 'FoleyFXRegion::smList''
// SYNTHETIC: SWBF2 0x007687f0
// `dynamic atexit destructor for 'FoleyFXRegion::smList''
PblListSingle FoleyFXRegion::smList;
