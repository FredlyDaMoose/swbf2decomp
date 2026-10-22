#include "OrdnanceHaywire.h"
#include "GameObject.h"
#include "StickInfo.h"

#include <stdio.h>

// FUNCTION: SWBF2 0x00600d50
bool OrdnanceHaywire::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceHaywire || OrdnanceSticky::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00600d90
unsigned int OrdnanceHaywire::GetDerivedRtti() const
{
    return rttiHashOrdnanceHaywire;
}

// FUNCTION: SWBF2 0x00600da0
const char* OrdnanceHaywire::GetDerivedRttiName() const
{
    return rttiNameOrdnanceHaywire;
}

// FUNCTION: SWBF2 0x00600e40
OrdnanceHaywire::OrdnanceHaywire(OrdnanceHaywireClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceSticky(pClass, desc)
{
}

// FUNCTION: SWBF2 0x00600e70
OrdnanceHaywire::~OrdnanceHaywire()
{
}

// SYNTHETIC: SWBF2 0x006010e6
// [thunk]:OrdnanceHaywire::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x006010ee
// [thunk]:OrdnanceHaywire::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x00600db0
// OrdnanceHaywire::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00600e90
bool OrdnanceHaywire::Update(float fDeltaTime)
{
    if (OrdnanceSticky::Update(fDeltaTime))
    {
        return true;
    }
    if (mStickInfo && mStickInfo->mCollHandle.mObject)
    {
        GameObject* pObject = mStickInfo->mCollHandle.mObject;
        if (pObject->mHandleId == mStickInfo->mCollHandle.mSavedHandleId)
        {
            pObject->mDisableTime = ((OrdnanceHaywireClass*)mClass)->mDisableTime;
        }
        else
        {
            mStickInfo->mCollHandle.mObject = 0;
            mStickInfo->mCollHandle.mSavedHandleId = 0;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00601050
Ordnance* OrdnanceHaywireClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.IsFull())
    {
        return 0;
    }
    OrdnanceHaywire* pOrdnance = new OrdnanceHaywire(this, desc);
    pOrdnance->ActivateThread(-40, PER_LOCAL_TURN);
    pOrdnance->AddOrdnanceToCollisionManager();
    return pOrdnance;
}

// GLOBAL: SWBF2 0x01ef2644
// SYNTHETIC: SWBF2 0x00405ae0
// `dynamic initializer for 'OrdnanceHaywire::rttiHashOrdnanceHaywire''
PblHash OrdnanceHaywire::rttiHashOrdnanceHaywire("OrdnanceHaywire");
char OrdnanceHaywire::rttiNameOrdnanceHaywire[] = "OrdnanceHaywire";

// GLOBAL: SWBF2 0x01ef2640
// SYNTHETIC: SWBF2 0x00405af0
// `dynamic initializer for 'OrdnanceHaywireClass::rttiHashOrdnanceHaywireClass''
PblHash OrdnanceHaywireClass::rttiHashOrdnanceHaywireClass("OrdnanceHaywireClass");
char OrdnanceHaywireClass::rttiNameOrdnanceHaywireClass[] = "OrdnanceHaywireClass";

// FUNCTION: SWBF2 0x00600f00
OrdnanceHaywireClass::OrdnanceHaywireClass(unsigned int uHash)
    : OrdnanceStickyClass(uHash)
{
    mDisableTime = 10.0f;
}

// FUNCTION: SWBF2 0x00600f60
OrdnanceHaywireClass::OrdnanceHaywireClass(OrdnanceHaywireClass* pBase, unsigned int uHash)
    : OrdnanceStickyClass(pBase, uHash)
{
    mDisableTime = pBase->mDisableTime;
}

// FUNCTION: SWBF2 0x00600de0
bool OrdnanceHaywireClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceHaywireClass || OrdnanceStickyClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00600e20
unsigned int OrdnanceHaywireClass::GetDerivedRtti()
{
    return rttiHashOrdnanceHaywireClass;
}

// FUNCTION: SWBF2 0x00600e30
const char* OrdnanceHaywireClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceHaywireClass;
}

// FUNCTION: SWBF2 0x00600fa0
void OrdnanceHaywireClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x1C1694FA)
    {
        sscanf(pszValue, "%f", &mDisableTime);
    }
    else
    {
        OrdnanceStickyClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00600fe0
OrdnanceClass* OrdnanceHaywireClass::Derive(unsigned int uHash)
{
    return new OrdnanceHaywireClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x00600f90
// OrdnanceHaywireClass::~OrdnanceHaywireClass

// SYNTHETIC: SWBF2 0x00600f30
// OrdnanceHaywireClass::`scalar deleting destructor'
