#pragma once

#include <stddef.h>

#include "OrdnanceShell.h"

// VTABLE: SWBF2 0x007ad138
class OrdnanceStickyClass : public OrdnanceShellClass
{
public:
    OrdnanceStickyClass(unsigned int uHash);
    OrdnanceStickyClass(OrdnanceStickyClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    GameSound mSoundCollisionWater;
    GameSound mSoundCollisionLeaf;
    GameSound mSoundCollisionOther;
    bool mStartTimerOnContact;
    bool mAlignVertical;
    int mStickTypes;

    static PblHash rttiHashOrdnanceStickyClass;
    static char rttiNameOrdnanceStickyClass[];
};
  
 
 
 
 
 
 

class StickInfo;

class OrdnanceSticky : public OrdnanceShell
{
public:
    OrdnanceSticky(OrdnanceStickyClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceSticky();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool CheckCollision();
    virtual bool Collide(CollisionObject* pObject, const PblVector3& vPos,
                         const PblVector3& vNormal, int iBodyId);
    virtual void SoundEvent();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    virtual bool Update(float fDeltaTime);

public:
    StickInfo* mStickInfo;
    unsigned char mNumBounces;
    float mLastEventTime;
    float mBeginTime;

    static PblHash rttiHashOrdnanceSticky;
    static char rttiNameOrdnanceSticky[];
};
 
 
 
 
 
