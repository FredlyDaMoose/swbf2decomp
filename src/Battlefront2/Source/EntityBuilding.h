#pragma once

#include <stddef.h>

#include "CollisionModel.h"
#include "GameObject.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class CollisionMesh;
class Controllable;
class EntityHologram;
class EntityHologramClass;
class ExplosionClass;
class Obstacle;
class RedPose;
class RedShadowProxy;
struct EntityDesc;

// VTABLE: SWBF2 0x00799540
class EntityBuildingClass : public GameObjectClass
{
public:
    EntityBuildingClass(unsigned int uHash);

    EntityBuildingClass(EntityBuildingClass* pBase, unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    bool IsDestructableBuilding();

    static PblHash rttiHashEntityBuildingClass;
    static char rttiNameEntityBuildingClass[];

    int mLodType;
    GameModel* mDestroyedModel;
    CollisionModel mDestroyedCollisionModel;
    GameModel* mUnbuiltModel;
    CollisionModel mUnbuiltCollisionModel;
    EntityHologramClass* m_pHologramClass;
    ExplosionClass* mExplosion;
    PblVector3 mExplosionOffset;
    float mRespawnTime;
    GameSound mBuiltSound;
    GameSound mAmbientBuildingSound;
    bool mAutoCreateBarrier;
    bool mDisableBarrierWhenDead;
    bool mColorizeByTeam;
    float mBarrierX;
    float mBarrierZ;
    float mBarrierOX;
    float mBarrierOZ;
    bool mIsDestructable;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class EntityBuilding : public GameObject
{
public:
    enum STATE
    {
        STATE_ALIVE = 0,
        STATE_DEAD = 1,
        STATE_UNBUILT = 2,
        STATE_DYING = 3,
    };

    EntityBuilding(EntityBuildingClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityBuilding();
    virtual void ClientKill();
    virtual bool IsUnbuilt() const;
    virtual float GetMaxSpeed() const;
    virtual float GetMaxStrafe() const;
    virtual const Controllable* GetControllable() const;
    virtual Controllable* GetControllable();
    virtual GameObjectType GetGameObjectType();
    virtual EntityType GetEntityType() const;
    virtual const PblMatrix* GetRenderMatrix() const;
    virtual const RedPose* GetRenderPose() const;

    virtual void Render(unsigned char uFlags, float fDist, unsigned int uMask);
    virtual const PblMatrix& GetBBoxMatrix() const;
    virtual const PblBox& GetBBox() const;

    virtual void SetPosition(const PblVector3& vPos);

    virtual void SetMatrix(const PblMatrix& matrix);
    virtual void RecalculateAABB();

    virtual void Kill();
    virtual void Respawn();

    virtual bool ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                             int iBodyPart, unsigned int uFlags);

    virtual void HealthChangeCallback(bool bArg, float fDelta);

    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);
    virtual GameObject* AddToClient();

    void Update(float fDeltaTime);

    static PblHash rttiHashEntityBuilding;
    static char rttiNameEntityBuilding[];

    EntityBuildingClass* mClass;
    STATE mState;
    CollisionMesh* mCollision;
    RedShadowProxy* mpShadow;
    EntityHologram* m_pHologram;
    Obstacle* mObstacle;
    GameSoundControllable mAmbientSound;
};
 
 
 
 
 
 
 
 
 
 
