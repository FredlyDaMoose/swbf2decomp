#include "FoleyFX.h"
#include "CollisionObject.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Application/RedWater.h"

// FUNCTION: SWBF2 0x0052c660
FoleyFXColliderClass::FoleyFXColliderClass()
{
    mClassID = 0;
}

// FUNCTION: SWBF2 0x0052c670
FoleyFXColliderClass::FoleyFXColliderClass(const FoleyFXColliderClass* pClass)
{
    mClassID = pClass->mClassID;
}

// FUNCTION: SWBF2 0x0052c680
unsigned int FoleyFXColliderClass::GetFoleyFXClassID() const
{
    return mClassID;
}

// FUNCTION: SWBF2 0x0052c690
bool FoleyFXColliderClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty != 0x99D55922)
    {
        return false;
    }
    mClassID = PblHash(pszValue);
    return true;
}

// FUNCTION: SWBF2 0x0052c500
void FoleyFXCollider::SetupFoleyFX(FoleyFXGroup* const pGroup)
{
    if (pGroup && mColliderClass && pGroup != mFoleyFXGroup &&
        mColliderClass->GetFoleyFXClassID())
    {
        FoleyFX* pFoleyFX = pGroup->FindFoleyFX(mColliderClass->GetFoleyFXClassID());
        mFoleyFXGroup = pGroup;
        if (pFoleyFX)
        {
            mFoleyFX = pFoleyFX;
        }
    }
}

// FUNCTION: SWBF2 0x0052c4c0
FoleyFX* FoleyFXCollider::GetFoleyFX(unsigned int uRtti) const
{
    if (mFoleyFX && mFoleyFX->GetDerivedRtti() != uRtti)
    {
        return 0;
    }
    return mFoleyFX;
}

// FUNCTION: SWBF2 0x0052c4f0
FoleyFXColliderClass* FoleyFXCollider::GetFoleyFXColliderClass() const
{
    return mColliderClass;
}

// FUNCTION: SWBF2 0x0052c490
FoleyFXCollider::FoleyFXCollider(FoleyFXColliderClass* pClass)
{
    mColliderClass = pClass;
    mFoleyFX = 0;
    mFoleyFXGroup = 0;
    SetupFoleyFX(FoleyFXGroup::GetTerrainFX());
}

// FUNCTION: SWBF2 0x0052c540
bool FoleyFXCollider::CollisionCallback(CollisionObjectType eType,
                                        FoleyFXCollidee* const pCollidee,
                                        const PblVector3* const pPos)
{
    float fWaterHeight;
    if (RedWater::GetWaterHeight(*pPos, fWaterHeight) && fWaterHeight > pPos->y)
    {
        eType = COLL_WATER;
    }

    switch (eType)
    {
    case COLL_TERRAIN:
        SetupFoleyFX(FoleyFXGroup::GetTerrainFX());
        return true;

    case COLL_WATER:
        SetupFoleyFX(FoleyFXGroup::GetWaterFX());
        return true;

    case COLL_ITEM:
    case COLL_SOFT:
    case COLL_ASTEROID:
    case COLL_RIGID:
    case COLL_STATIC:
    case COLL_ATTACHED:
    case COLL_LEAFPATCH:
    case COLL_ORDNANCE:
    case COLL_TOWCABLE:
    case COLL_SHIELD:
    case COLL_TARGETABLE:
    {
        FoleyFXCollideeClass* pClass = pCollidee->GetFoleyFXCollideeClass();
        if (pClass)
        {
            SetupFoleyFX(pClass->GetFoleyFXGroup());
        }
        return true;
    }

    default:
        return false;
    }
}
