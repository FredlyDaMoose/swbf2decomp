#pragma once

#include <stddef.h>

#include "OrdnanceBullet.h"

class pcRedShader;

// VTABLE: SWBF2 0x007ad19c
class OrdnanceTowCableClass : public OrdnanceBulletClass
{
public:
    OrdnanceTowCableClass(unsigned int uHash);
    OrdnanceTowCableClass(OrdnanceTowCableClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mInitialCableLimit;
    float mCableLimit;

    static PblHash rttiHashOrdnanceTowCableClass;
    static char rttiNameOrdnanceTowCableClass[];
};
  
 
 

class WeaponTowCable;

enum TowCableState
{
    TOW_ATTACHING = 0,
    TOW_WRAP = 1,
    TOW_FALL = 2,
    TOW_DAMAGE = 3,
};

struct TowCableAttachPointData
{
    TowCableAttachPointData() {}

    float mFinalHeight;
    float mFallSpeed;
    bool mIsAttached;
    int mBodyID;
    PblVector3 mWorldBodyOffset;
    PblVector3 mWorldInitialBodyOffset;
    PblVector3 mBodyOffset;
    PblVector3 mInitialBodyOffset;
    PblVector3 mFinalSlope;
    PblVector3 mSlopeIn;
    PblVector3 mSlopeOut;
};
 
 

class TowCableRenderObject
{
public:
    TowCableRenderObject();
    ~TowCableRenderObject() {}

    void Clear();
    void AddAttachPoint(int iBodyID, const PblVector3& vOffset, const PblVector3& vSlope);
    void UpdateAttachPoints(float fDeltaTime, GameObject* pObject);
    void Render(GameObject* pObject, unsigned char ucViewport, float fDist, unsigned int uiFlags);

    TowCableState mTowState;
    TowCableAttachPointData mAttachPoints[13];
};
 

// VTABLE: SWBF2 0x007ad1cc
// VTABLE: SWBF2 0x007ad20c
// VTABLE: SWBF2 0x007ad274
class OrdnanceTowCable : public OrdnanceBullet
{
public:
    OrdnanceTowCable(OrdnanceTowCableClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceTowCable();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool Collide(CollisionObject* pObject, const PblVector3& vPos,
                         const PblVector3& vNormal, int iBodyId);
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    void SetFakeCable(bool bFake) { mFakeCable = bFake; }

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }

protected:
    virtual bool Update(float fDeltaTime);

    void CalculateWorldSpacePoints();
    void ConvertPointsToWorldSpace();
    void RecalculateCenterAndRadius();
    void TowCableFailure();

public:
    WeaponTowCable* mFireWeapon;
    float mTowCableFallTime;
    PblVector3 mLastOwnerFirePos;
    PblVector3 mLastOwnerFireDir;
    TowCableRenderObject mTowCable;
    PblHandle<GameObject> mHitWalker;
    bool mFakeCable;
    bool mDeleteNextUpdate;

    static PblHash rttiHashOrdnanceTowCable;
    static char rttiNameOrdnanceTowCable[];
    static MemoryPool sMemoryPool;

    static void PlatformInit();
    static pcRedShader* sShader;
    static void PlatformCleanup();
    static unsigned int sTextureHash;
    static float sCableThickness;
};
 
 
 
 
 
 
 
 
