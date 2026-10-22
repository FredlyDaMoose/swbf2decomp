#include "FoleyFX.h"
#include "CollisionObject.h"
#include "PebbleFL/Common/PblHashTable.h"

// FUNCTION: SWBF2 0x0052c420
FoleyFXCollideeClass::FoleyFXCollideeClass()
{
    mGroup = 0;
}

// FUNCTION: SWBF2 0x0052c400
FoleyFXCollidee::FoleyFXCollidee(FoleyFXCollideeClass* pClass)
{
    mCollideeClass = pClass;
}

// FUNCTION: SWBF2 0x0052c410
FoleyFXCollideeClass* FoleyFXCollidee::GetFoleyFXCollideeClass() const
{
    return mCollideeClass;
}

// FUNCTION: SWBF2 0x0052c430
FoleyFXCollideeClass::FoleyFXCollideeClass(const FoleyFXCollideeClass* pClass)
{
    mGroup = pClass->mGroup;
}

// FUNCTION: SWBF2 0x0052c450
bool FoleyFXCollideeClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0xa1c9cbc5)
    {
        return false;
    }
    mGroup = FoleyFXGroup::FindByID(PblHash(pszValue));
    return true;
}

// FUNCTION: SWBF2 0x0052c440
FoleyFXGroup* FoleyFXCollideeClass::GetFoleyFXGroup() const
{
    return mGroup;
}
