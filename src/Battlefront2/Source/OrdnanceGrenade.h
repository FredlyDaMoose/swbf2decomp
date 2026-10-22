#pragma once

#include <stddef.h>

#include "OrdnanceShell.h"

// VTABLE: SWBF2 0x007aca48
class OrdnanceGrenadeClass : public OrdnanceShellClass
{
public:
    OrdnanceGrenadeClass(unsigned int uHash);
    OrdnanceGrenadeClass(OrdnanceGrenadeClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mRebound;
    float mFriction;
    GameSound mSoundCollisionLeaf;
    GameSound mSoundCollisionOther;
    unsigned int mImpactEffectWater;
    bool mStartTimerOnContact;
    bool mAlignVertical;

    static PblHash rttiHashOrdnanceGrenadeClass;
    static char rttiNameOrdnanceGrenadeClass[];
};
  
 
 
 
 
 
 
 

class OrdnanceGrenade : public OrdnanceShell
{
public:
    OrdnanceGrenade(OrdnanceGrenadeClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceGrenade();

    virtual bool Update(float fDeltaTime);
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool CheckCollision();
    virtual bool Collide(CollisionObject* pObject, const PblVector3& vPos,
                         const PblVector3& vNormal, int iBodyId);
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);
    virtual void SoundEvent();

    int mNumBounces;
    float mBeginTime;
    float mLastEventTime;

    static PblHash rttiHashOrdnanceGrenade;
    static char rttiNameOrdnanceGrenade[];
};
 
 
 
 
