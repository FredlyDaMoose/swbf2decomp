#pragma once

#include <stddef.h>

#include "Ordnance.h"
#include "Battlefront2Lod.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class Aimer;
class FLEffectClass;

// VTABLE: SWBF2 0x007ac684
class OrdnanceBeamClass : public OrdnanceClass
{
public:
    OrdnanceBeamClass(unsigned int uHash);

    OrdnanceBeamClass(OrdnanceBeamClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    static char rttiNameOrdnanceBeamClass[];
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetRange();

    float mRange;
    float mBeamWidth;
    unsigned int mBeamTextureId;
    RedColor mBeamColor;
    float mGlowWidth;
    unsigned int mGlowTextureId;
    RedColor mGlowColor;
    bool mPassThrough : 1;
    FLEffectClass* mImpactEffect;

    static PblHash rttiHashOrdnanceBeamClass;
};
  
 
 
 
 
 
 
 
 

class BeamLodData : public RedLodData
{
public:
    BeamLodData() { mLowRezFlags = 5; }
};
 

class OrdnanceBeam : public Ordnance, public RedSceneObject
{
public:
    OrdnanceBeam(OrdnanceBeamClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceBeam();

    virtual bool Update(float fDeltaTime);
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                                GameObject* pObject) const;
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    void SetDead();

    PblVector3 mDirection;
    float mLength;
    Aimer* mAttachAimer;
    OrdnanceBeam* m_pRicochet;
    bool mDeadFlag;

    static PblHash rttiHashOrdnanceBeam;
    static char rttiNameOrdnanceBeam[];
};
 
 
 
 
 
 
