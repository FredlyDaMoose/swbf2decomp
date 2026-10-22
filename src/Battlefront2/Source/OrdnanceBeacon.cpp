#include "OrdnanceBeacon.h"

#include <stdio.h>

#include "Ordnance.h"
#include "GameObject.h"
#include "NetGame.h"
#include "StickInfo.h"
#include "PebbleFL/Common/PblRandom.h"

#include <d3dx9math.h>

extern bool netIsLocalTurn;

namespace
{

    GameObject* ResolveHandle(PblHandle<GameObject>& handle)
    {
        GameObject* pObject = handle.mObject;
        if (pObject &&
            pObject->mHandleId !=
                handle.mSavedHandleId)
        {
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
            pObject = 0;
        }
        return pObject;
    }
}

// GLOBAL: SWBF2 0x01ef2610
// SYNTHETIC: SWBF2 0x00405a10
// `dynamic initializer for 'OrdnanceBeacon::rttiHashOrdnanceBeacon''
PblHash OrdnanceBeacon::rttiHashOrdnanceBeacon("OrdnanceBeacon");
char OrdnanceBeacon::rttiNameOrdnanceBeacon[] = "OrdnanceBeacon";

// GLOBAL: SWBF2 0x01ef257c
// SYNTHETIC: SWBF2 0x00405a20
// `dynamic initializer for 'OrdnanceBeaconClass::rttiHashOrdnanceBeaconClass''
PblHash OrdnanceBeaconClass::rttiHashOrdnanceBeaconClass("OrdnanceBeaconClass");
char OrdnanceBeaconClass::rttiNameOrdnanceBeaconClass[] = "OrdnanceBeaconClass";

// FUNCTION: SWBF2 0x005f8b10
const PblVector3& OrdnanceBullet::GetOldPosition() const
{
    return mOldPosition;
}

// FUNCTION: SWBF2 0x005f8b20
const PblVector3& OrdnanceBullet::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x005f8b60
float OrdnanceBulletClass::GetVelocity()
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x005f8b70
float OrdnanceBulletClass::GetRange()
{
    return mVelocity * mLifeSpan;
}

// FUNCTION: SWBF2 0x005f8be0
float OrdnanceShellClass::GetGravity()
{
    return mGravity;
}

// FUNCTION: SWBF2 0x005f8c70
bool OrdnanceBeacon::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceBeacon || OrdnanceSticky::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005f8cb0
unsigned int OrdnanceBeacon::GetDerivedRtti() const
{
    return rttiHashOrdnanceBeacon;
}

// FUNCTION: SWBF2 0x005f8cc0
const char* OrdnanceBeacon::GetDerivedRttiName() const
{
    return rttiNameOrdnanceBeacon;
}

// SYNTHETIC: SWBF2 0x005f95d6
// [thunk]:OrdnanceBeacon::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x005f95e1
// [thunk]:OrdnanceBeacon::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x005f8cd0
// OrdnanceBeacon::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f8d60
OrdnanceBeacon::OrdnanceBeacon(OrdnanceBeaconClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceSticky(pClass, desc)
{
    mSalvoCount = pClass->mOrdnanceClass ? pClass->mSalvoCount : 0;
}

// FUNCTION: SWBF2 0x005f8dc0
OrdnanceBeacon::~OrdnanceBeacon()
{
}

// FUNCTION: SWBF2 0x005f8de0
bool OrdnanceBeacon::Update(float fDeltaTime)
{
    while (mSalvoCount > 0 && fDeltaTime > mLifeTime)
    {
        OrdnanceBeaconClass* pClass = static_cast<OrdnanceBeaconClass*>(mClass);

        D3DXMATRIXA16 aim;
        D3DXMATRIXA16 yaw;
        D3DXMatrixRotationX(&aim, -pClass->mAimElevation);
        D3DXMatrixMultiply(&aim, &aim, D3DXMatrixRotationY(&yaw, pClass->mAimAzimuth));

        OrdnanceDesc desc;
        desc.mPosition = mPosition;
        const float fDist = pClass->mAimDistance;
        desc.mPosition.x += aim._31 * fDist;
        desc.mPosition.y += aim._32 * fDist;
        desc.mPosition.z += aim._33 * fDist;

        PblRandom& rand = PblRandom::GetSim();
        const float fRight = (rand.GetFloat() - rand.GetFloat()) * pClass->mScatterDistance;
        desc.mPosition.x += aim._11 * fRight;
        desc.mPosition.y += aim._12 * fRight;
        desc.mPosition.z += aim._13 * fRight;
        const float fUp = (rand.GetFloat() - rand.GetFloat()) * pClass->mScatterDistance;
        desc.mPosition.x += aim._21 * fUp;
        desc.mPosition.y += aim._22 * fUp;
        desc.mPosition.z += aim._23 * fUp;

        desc.mDirection = PblVector3(-aim._31, -aim._32, -aim._33);
        desc.mStrength = 1.0f;
        desc.mOwner = ResolveHandle(mOwner);
        desc.mDamageOwner = mDamageOwner;
        desc.mTarget = mStickInfo ? ResolveHandle(mStickInfo->mCollHandle) : 0;
        desc.mBodyID = mStickInfo ? mStickInfo->mCollBodyID : -1;

        if (NetGame::IsEnabled() && !NetGame::IsClient())
        {
            NetGame::CreateEvent_CreateOrdnance(pClass->mOrdnanceClass, desc);
        }
        if (!NetGame::IsEnabled() || !NetGame::IsClient() || netIsLocalTurn)
        {
            pClass->mOrdnanceClass->Build(desc);
        }

        --mSalvoCount;
        mLifeTime += pClass->mSalvoDelay;
    }
    return OrdnanceSticky::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x005f8d00
bool OrdnanceBeaconClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceBeaconClass || OrdnanceStickyClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005f8d40
unsigned int OrdnanceBeaconClass::GetDerivedRtti()
{
    return rttiHashOrdnanceBeaconClass;
}

// FUNCTION: SWBF2 0x005f8d50
const char* OrdnanceBeaconClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceBeaconClass;
}

// FUNCTION: SWBF2 0x005f9270
OrdnanceBeaconClass::OrdnanceBeaconClass(unsigned int uHash)
    : OrdnanceStickyClass(uHash)
{
    mAimAzimuth = 0.0f;
    mAimElevation = 1.5707964f;
    mAimDistance = 100.0f;
    mSalvoCount = 1;
    mSalvoDelay = 0.0f;
    mScatterDistance = 0.0f;
    mOrdnanceClass = 0;
}

// SYNTHETIC: SWBF2 0x005f92e0
// OrdnanceBeaconClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f9310
OrdnanceBeaconClass::OrdnanceBeaconClass(OrdnanceBeaconClass* pBase, unsigned int uHash)
    : OrdnanceStickyClass(pBase, uHash)
{
    mAimAzimuth = pBase->mAimAzimuth;
    mAimElevation = pBase->mAimElevation;
    mAimDistance = pBase->mAimDistance;
    mSalvoCount = pBase->mSalvoCount;
    mSalvoDelay = pBase->mSalvoDelay;
    mScatterDistance = pBase->mScatterDistance;
    mOrdnanceClass = pBase->mOrdnanceClass;
}

// SYNTHETIC: SWBF2 0x005f9390
// OrdnanceBeaconClass::~OrdnanceBeaconClass

// FUNCTION: SWBF2 0x005f93a0
void OrdnanceBeaconClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x3186FBFE:
        if (sscanf(pszValue, "%f", &mAimAzimuth))
        {
            mAimAzimuth *= 0.017453292f;
        }
        break;
    case 0x8339FB35:
        if (sscanf(pszValue, "%f", &mAimElevation))
        {
            mAimElevation *= 0.017453292f;
        }
        break;
    case 0xECDB9D1D:
        sscanf(pszValue, "%f", &mAimDistance);
        break;
    case 0x9260EA63:
        sscanf(pszValue, "%i", &mSalvoCount);
        break;
    case 0xD76F37DB:
        sscanf(pszValue, "%f", &mSalvoDelay);
        break;
    case 0xEBD81A5E:
        sscanf(pszValue, "%f", &mScatterDistance);
        break;
    case 0x399BF05D:
    case 0xF8899C5E:
        mOrdnanceClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(PblHash(pszValue));
        break;
    default:
        OrdnanceStickyClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x005f94d0
OrdnanceClass* OrdnanceBeaconClass::Derive(unsigned int uHash)
{
    return new OrdnanceBeaconClass(this, uHash);
}

// FUNCTION: SWBF2 0x005f9540
Ordnance* OrdnanceBeaconClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceBeacon* pBeacon = new OrdnanceBeacon(this, desc);
    pBeacon->ActivateThread(-40, PER_LOCAL_TURN);
    pBeacon->AddOrdnanceToCollisionManager();
    return pBeacon;
}
