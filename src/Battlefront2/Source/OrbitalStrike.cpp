#include "OrbitalStrike.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblRandom.h"

#include <d3dx9math.h>
#include <stdio.h>

extern bool netIsLocalTurn;

typedef PblList<OrdnanceClass>::Itor OrdnanceItor;

// FUNCTION: SWBF2 0x005f6f80
OrbitalStrikeClass::OrbitalStrikeClass(unsigned int uHash)
{
    (void)uHash;

    mAimAzimuth = 0.0f;
    mAimElevation = 1.5707964f;
    mAimDistance = 100.0f;
    mSalvoCount = 1;
    mSalvoDelay = 0.0f;
    mInitialSalvoDelay = 0.0f;
    mScatterDistance = 0.0f;
    mStrikeClass = 0;
}

// FUNCTION: SWBF2 0x005f6fd0
OrbitalStrikeClass::OrbitalStrikeClass(OrbitalStrikeClass* pBase, unsigned int uHash)
{
    (void)uHash;
    *this = *pBase;
}

// FUNCTION: SWBF2 0x005f6aa0
OrbitalStrike::OrbitalStrike(const OrbitalStrikeClass* pClass, OrbitalStrikeDesc& desc)
    : Thread(), PblHandled(), mClass(const_cast<OrbitalStrikeClass*>(pClass))
{
    mPosition = desc.mPosition;
    mDamageOwner = desc.mDamageOwner;
    mSalvoCount = pClass->mSalvoCount;
    mSalvoDelay = pClass->mInitialSalvoDelay;
    ActivateThread(-40, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x005f6b80
OrbitalStrike::~OrbitalStrike()
{
}

// SYNTHETIC: SWBF2 0x005f6b50
// OrbitalStrike::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005f7193
// [thunk]:OrbitalStrike::`scalar deleting destructor' adjustor{24}

// FUNCTION: SWBF2 0x005f6ba0
bool OrbitalStrike::Update(float fDeltaTime)
{
    while (fDeltaTime > 0.00012207031f)
    {
        if (mSalvoCount <= 0)
        {
            return false;
        }

        while (mSalvoDelay <= 0.0f)
        {
            D3DXMATRIXA16 aim;
            D3DXMATRIXA16 yaw;
            D3DXMatrixRotationX(&aim, -mClass->mAimElevation);
            D3DXMatrixMultiply(&aim, &aim, D3DXMatrixRotationY(&yaw, mClass->mAimAzimuth));
            aim._41 = mPosition.x;
            aim._42 = mPosition.y;
            aim._43 = mPosition.z;
            aim._44 = 1.0f;

            OrdnanceDesc desc;
            PblRandom& rand = PblRandom::GetSim();
            const D3DXVECTOR3 offset((rand.GetFloat() - rand.GetFloat()) * mClass->mScatterDistance,
                                     (rand.GetFloat() - rand.GetFloat()) * mClass->mScatterDistance,
                                     mClass->mAimDistance);
            D3DXVECTOR3 pos;
            D3DXVec3TransformCoord(&pos, &offset, &aim);
            desc.mPosition = PblVector3(pos.x, pos.y, pos.z);
            desc.mDirection = PblVector3(-aim._31, -aim._32, -aim._33);
            desc.mDamageOwner = mDamageOwner;

            if (NetGame::IsEnabled() && !NetGame::IsClient())
            {
                NetGame::CreateEvent_CreateOrdnance(mClass->mStrikeClass, desc);
            }
            if (!NetGame::IsEnabled() || !NetGame::IsClient() || netIsLocalTurn)
            {
                mClass->mStrikeClass->Build(desc);
            }

            --mSalvoCount;
            mSalvoDelay += mClass->mSalvoDelay;
            if (mSalvoCount <= 0)
            {
                break;
            }
        }

        const float fStep = (mSalvoDelay > fDeltaTime) ? fDeltaTime : mSalvoDelay;
        fDeltaTime -= fStep;
        mSalvoDelay -= fStep;
    }
    return true;
}

// FUNCTION: SWBF2 0x005f7140
template OrdnanceClass* Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(unsigned int);

// FUNCTION: SWBF2 0x005f7010
bool OrbitalStrikeClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x3186FBFE:
        if (sscanf(pszValue, "%f", &mAimAzimuth))
        {
            mAimAzimuth *= 0.017453292f;
        }
        return true;
    case 0x8339FB35:
        if (sscanf(pszValue, "%f", &mAimElevation))
        {
            mAimElevation *= 0.017453292f;
        }
        return true;
    case 0xECDB9D1D:
        sscanf(pszValue, "%f", &mAimDistance);
        return true;
    case 0x9260EA63:
        sscanf(pszValue, "%i", &mSalvoCount);
        return true;
    case 0xD76F37DB:
        sscanf(pszValue, "%f", &mSalvoDelay);
        return true;
    case 0xC8388D9D:
        sscanf(pszValue, "%f", &mInitialSalvoDelay);
        return true;
    case 0xEBD81A5E:
        sscanf(pszValue, "%f", &mScatterDistance);
        return true;
    case 0x6FD92BE8:
    case 0x0E52B6A3:
        mStrikeClass = Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Find(PblHash(pszValue));
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x005d3b50
OrdnanceItor* InitOrdnanceItor(OrdnanceItor* pItor, PblList<OrdnanceClass>::Node* pNode)
{
    pItor->_pNode = pNode;
    return pItor;
}

// FUNCTION: SWBF2 0x005d3b70
OrdnanceClass* GetOrdnanceItorObjectForCheck(const OrdnanceItor* pItor)
{
    return *pItor;
}

// FUNCTION: SWBF2 0x005d3b90
OrdnanceClass* GetOrdnanceItorObjectForReturn(const OrdnanceItor* pItor)
{
    return *pItor;
}

// FUNCTION: SWBF2 0x005d3bb0
OrdnanceClass* GetOrdnanceItorObjectForEndTest(const OrdnanceItor* pItor)
{
    return *pItor;
}

// FUNCTION: SWBF2 0x005d3bd0
OrdnanceItor* AdvanceOrdnanceItor(OrdnanceItor* pItor)
{
    return &++*pItor;
}
