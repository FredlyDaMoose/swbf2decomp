#pragma once

#include <stddef.h>

#include "GameObject.h"

class ExplosionClass;

// VTABLE: SWBF2 0x00796e98
class AsteroidClass : public GameObjectClass
{
public:
    AsteroidClass(unsigned int uHash);

    AsteroidClass(AsteroidClass* pBase, unsigned int uHash);
    virtual ~AsteroidClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    void FillRegion(const char* pcRegion, int iCount, PblVector3 vMin, PblVector3 vMax);
    void FillPath(const char* pcPath, float fDensity, int iCount,
                    PblVector3 vMin, PblVector3 vMax);

    static PblHash rttiHashAsteroidClass;
    static char rttiNameAsteroidClass[];

    struct ChildAsteroidDesc
    {
        unsigned int mClassHash;
        unsigned int mHardPointHash;
    };

    float mMinSpeed;
    float mMaxSpeed;
    float mMinOmega;
    float mMaxOmega;
    int mCollidable;

    ChildAsteroidDesc mChildren[10];
    int mNumChildren;
    ExplosionClass* mExplosionBreak;
    ExplosionClass* mExplosionDestruct;
};
  
 
 
 
 
 
 
 
 
 

#include "Thread.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// VTABLE: SWBF2 0x00796d00
// VTABLE: SWBF2 0x00796cac CollisionObject
// VTABLE: SWBF2 0x00796e20 RedSceneObject
// VTABLE: SWBF2 0x00796c80 Damageable
// VTABLE: SWBF2 0x00796e88 PblHandled
// VTABLE: SWBF2 0x00796c68 Thread
class Asteroid : public GameObject, public Thread
{
public:

    Asteroid(AsteroidClass* pClass, const EntityDesc& desc);

    virtual ~Asteroid();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Init();
    virtual const PblVector3& GetVelocity() const;
    virtual void SetVelocity(const PblVector3& vVelocity);
    virtual void ClientKill();
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual void ActivatePhysics();
    virtual void DeactivatePhysics();
    virtual GameObjectType GetGameObjectType();
    virtual EntityType GetEntityType() const;
    virtual bool AddToTeamManager() const;

    virtual void WriteCreate(NetPktGroup& group);
    virtual void ReadCreate(NetPktGroup& group);
    virtual void AddSends(void (*pfnSend)(GameObject*, int), int iArg);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);

    void Move(const PblMatrix& matrix);
    virtual const PblMatrix& GetSmoothedMatrix() const;

    virtual void SetPosition(const PblVector3& vPos);
    virtual void SetMatrix(const PblMatrix& matrix);
    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);
    virtual const PblMatrix& GetMatrix() const;

    virtual void Render(unsigned char ucLod, float fDist, unsigned int uMask);

    virtual void Kill();

    virtual bool Update(float fDeltaTime);

    PblVector3 mVelocity;
    PblVector3 mOmega;
    AsteroidClass* mClass;

    static MemoryPool sMemoryPool;
    static PblHash rttiHashAsteroid;
    static char rttiNameAsteroid[];
};
 
 
 
 
