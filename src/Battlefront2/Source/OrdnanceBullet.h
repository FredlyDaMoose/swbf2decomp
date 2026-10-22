#pragma once

#include <stddef.h>

#include "Ordnance.h"
#include "Battlefront2Lod.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include "PebbleFL/Common/PblHashTable.h"

class RedModel;

// VTABLE: SWBF2 0x007ac6e0
class OrdnanceBulletClass : public OrdnanceClass
{
public:
    OrdnanceBulletClass(unsigned int uHash);
    OrdnanceBulletClass(OrdnanceBulletClass* pBase, unsigned int uHash);

    virtual OrdnanceClass* Derive(unsigned int uHash);
    virtual Ordnance* Build(const OrdnanceDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual float GetVelocity();
    virtual float GetRange();

    RedModel* mModel;
    bool mStraightBullet;
    bool mFadeColorLight;
    RedLodData mLodData;
    float mVelocity;
    unsigned int mImpactEffectSoft;
    unsigned int mImpactEffectRigid;
    unsigned int mImpactEffectStatic;
    unsigned int mImpactEffectTerrain;
    unsigned int mImpactEffectWater;
    unsigned int mImpactEffectShieldSmall;
    unsigned int mImpactEffectShieldLarge;
    unsigned int mImpactDecalStatic;
    unsigned int mImpactDecalTerrain;
    unsigned int m_uiLightOdfNameHash;
    RedColor m_colorLight;
    RedColor m_endColorLight;
    float m_fRadiusLight;

    static PblHash rttiHashOrdnanceBulletClass;
    static char rttiNameOrdnanceBulletClass[];
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class EntityLight;

class OrdnanceBullet : public Ordnance, public RedSceneObject
{
public:
    OrdnanceBullet(OrdnanceBulletClass* pClass, const OrdnanceDesc& desc);
    virtual ~OrdnanceBullet();

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
    virtual void CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                                GameObject* pObject) const;
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    PblVector3 mOldPosition;
    PblVector3 mVelocity;
    PblVector3 mFinalCollisionNormal;
    CollisionObject* mFinalCollision;
    PblHandle<GameObject> mFinalCollisionHandle;
    bool mFinalCollisionOnNextUpdate;
    EntityLight* m_pLight;
    float mOffscreenTime;

    static PblHash rttiHashOrdnanceBullet;
    static char rttiNameOrdnanceBullet[];
};
 
 
 
