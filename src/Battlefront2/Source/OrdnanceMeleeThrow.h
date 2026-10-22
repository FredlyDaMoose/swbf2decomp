#pragma once

#include <stddef.h>

#include "Ordnance.h"
#include "Battlefront2Lod.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include "PebbleFL/Common/PblMatrix.h"

// VTABLE: SWBF2 0x007ace2c
class OrdnanceMeleeThrowClass : public OrdnanceClass
{
public:
    OrdnanceMeleeThrowClass(unsigned int uHash);
    OrdnanceMeleeThrowClass(OrdnanceMeleeThrowClass* pBase, unsigned int uHash);
    virtual ~OrdnanceMeleeThrowClass();

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetVelocity();
    virtual float GetRange();

    RedLodData mLodData;
    float mVelocity;
    float m_fReturnDuration;
    float m_fOmega;
    float m_fDamageLength;
    float m_fDamageWidth;
    unsigned int m_bDamageLengthFromEdge : 1;
    unsigned int m_bDamageWidthFromEdge : 1;
    unsigned int z_uiReserved0 : 30;
    unsigned int m_uiLightOdfNameHash;
    RedColor m_colorLight;
    float m_fRadiusLight;
    GameSound m_OnSound;

    static PblHash rttiHashOrdnanceMeleeThrowClass;
    static char rttiNameOrdnanceMeleeThrowClass[];
};
  
 
 
 
 
 
 
 
 
 
 

class EntityLight;
class WeaponMeleeThrow;

class OrdnanceMeleeThrow : public Ordnance, public RedSceneObject
{
public:
    OrdnanceMeleeThrow(OrdnanceMeleeThrowClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceMeleeThrow();

protected:
    virtual bool Update(float fDeltaTime);

public:
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual const PblVector3& GetOldPosition() const;
    virtual const PblVector3& GetVelocity() const;
    virtual bool Collide(CollisionObject* pObject, const PblVector3& vPos,
                         const PblVector3& vNormal, int iBodyId);
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

protected:
    void ApplyDamage(float fDeltaTime);
    void Return();

public:
    PblHandle<GameObject> m_pOwner;
    WeaponMeleeThrow* m_pWeapon;
    PblMatrix m_mMatrix;
    PblVector3 mOldPosition;
    PblVector3 mVelocity;
    PblVector3 m_vRotationAxis;
    void* m_pDamageData;
    EntityLight* m_pLight;
    bool m_bLastFrameIndicator;
    float mOffscreenTime;
    float m_fReturnTime;

    static PblHash rttiHashOrdnanceMeleeThrow;
    static char rttiNameOrdnanceMeleeThrow[];
};
 
 
 
 
 
 
 
 
 
 
 
 
