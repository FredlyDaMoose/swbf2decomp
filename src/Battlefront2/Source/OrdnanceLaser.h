#pragma once

#include <stddef.h>

#include "OrdnanceBullet.h"

// VTABLE: SWBF2 0x007acd0c
class OrdnanceLaserClass : public OrdnanceBulletClass
{
public:
    OrdnanceLaserClass(unsigned int uHash);
    virtual ~OrdnanceLaserClass();
    OrdnanceLaserClass(OrdnanceLaserClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float mLaserWidth;
    float mLaserLength;
    unsigned int mTextureId;
    RedColor mGlowColor;
    RedColor mEndGlowColor;
    float mGlowLength;
    float mBlurLength;
    float mFadeOutSpeed;

    static PblHash rttiHashOrdnanceLaserClass;
    static char rttiNameOrdnanceLaserClass[];
};
  
 
 
 
 
 
 
 
 

class OrdnanceLaser : public OrdnanceBullet
{
public:
    OrdnanceLaser(OrdnanceLaserClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceLaser();

protected:
    virtual bool Update(float fDeltaTime);

public:
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void KillOrdnance();
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    PblVector3 mStartPos;
    float mGrowLength;
    float mFadeOutTime;
    bool mDeadFlag;
    float mMaxLength;

    static PblHash rttiHashOrdnanceLaser;
    static char rttiNameOrdnanceLaser[];
};
 
 
 
 
 
 
