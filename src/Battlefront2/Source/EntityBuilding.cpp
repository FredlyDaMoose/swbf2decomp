#include "EntityBuilding.h"
#include "PebbleFL/Common/PblVector.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include "CameraManager.h"
#include "Character.h"
#include "CollisionBody.h"
#include "CollisionMesh.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "EntityHologram.h"
#include "EntityBuildingAnimated.h"
#include "Explosion.h"
#include "GameLoop.h"
#include "GameModel.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PathFinder.h"
#include "ProceduralAnimation.h"

#include "TargetManager.h"
#include "Team.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Graphics/RedModel.h"

// FUNCTION: SWBF2 0x00475ec0
const PblMatrix& EntityBuilding::GetBBoxMatrix() const
{
    return mMatrix;
}

extern PblVector3 gVectorZero;
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// FUNCTION: SWBF2 0x00493690
EntityBuildingClass::EntityBuildingClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    mExplosionOffset.x = 0.0f;
    mExplosionOffset.y = 0.0f;
    mExplosionOffset.z = 0.0f;
    mHealthType = HEALTH_TYPE_BUILDING;
    mDestroyedModel = 0;
    mUnbuiltModel = 0;
    mLodType = 1;
    mLODClass = 1;
    mExplosion = 0;
    mRespawnTime = FLT_MAX;
    m_pHologramClass = 0;
    mAutoCreateBarrier = false;
    mDisableBarrierWhenDead = false;
    mColorizeByTeam = false;
    mBarrierZ = -1.0f;
    mBarrierX = -1.0f;
    mBarrierOZ = 0.0f;
    mBarrierOX = 0.0f;
    mIsDestructable = (uHash == PblHash("destructablebuilding"));
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x00475ed0
const PblBox& EntityBuilding::GetBBox() const
{
    return GetModel()->_mModel->_TotalBoundingBox;
}

// GLOBAL: SWBF2 0x01eb986c
// SYNTHETIC: SWBF2 0x00402890
// `dynamic initializer for 'EntityBuildingClass::rttiHashEntityBuildingClass''
PblHash EntityBuildingClass::rttiHashEntityBuildingClass("EntityBuildingClass");
char EntityBuildingClass::rttiNameEntityBuildingClass[] = "EntityBuildingClass";

// GLOBAL: SWBF2 0x01ebb874
// SYNTHETIC: SWBF2 0x00402880
// `dynamic initializer for 'EntityBuilding::rttiHashEntityBuilding''
PblHash EntityBuilding::rttiHashEntityBuilding("EntityBuilding");
char EntityBuilding::rttiNameEntityBuilding[] = "EntityBuilding";

// FUNCTION: SWBF2 0x00475ef0
bool EntityBuildingClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityBuildingClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00492390
unsigned int EntityBuildingClass::GetDerivedRtti()
{
    return rttiHashEntityBuildingClass;
}

// FUNCTION: SWBF2 0x004923a0
const char* EntityBuildingClass::GetDerivedRttiName()
{
    return rttiNameEntityBuildingClass;
}

// FUNCTION: SWBF2 0x004923b0
bool EntityBuildingClass::IsDestructableBuilding()
{
    return mIsDestructable;
}

// FUNCTION: SWBF2 0x00493800
EntityBuildingClass::EntityBuildingClass(EntityBuildingClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash)
{
    mDestroyedModel = pBase->mDestroyedModel;
    mUnbuiltModel = pBase->mUnbuiltModel;
    mLodType = pBase->mLodType;
    mLODClass = mLodType;
    mExplosion = pBase->mExplosion;
    mExplosionOffset = pBase->mExplosionOffset;
    mRespawnTime = pBase->mRespawnTime;
    m_pHologramClass = pBase->m_pHologramClass;
    mAutoCreateBarrier = pBase->mAutoCreateBarrier;
    mDisableBarrierWhenDead = pBase->mDisableBarrierWhenDead;
    mColorizeByTeam = pBase->mColorizeByTeam;
    mBarrierX = pBase->mBarrierX;
    mBarrierZ = pBase->mBarrierZ;
    mBarrierOX = pBase->mBarrierOX;
    mBarrierOZ = pBase->mBarrierOZ;
    mIsDestructable = pBase->mIsDestructable;
}

// SYNTHETIC: SWBF2 0x00493920
// EntityBuildingClass::~EntityBuildingClass

// SYNTHETIC: SWBF2 0x004937d0
// EntityBuildingClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00493990
EntityClass* EntityBuildingClass::Derive(unsigned int uHash)
{
    return new EntityBuildingClass(this, uHash);
}

// FUNCTION: SWBF2 0x00493a00
Entity* EntityBuildingClass::Build(const EntityDesc& desc)
{
    if (!mModel)
    {
        return 0;
    }

    EntityBuilding* pBuilding = new EntityBuilding(this, desc);
    if (pBuilding)
    {
        static_cast<CollisionObject*>(pBuilding)->AddStaticToCollisionManager();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            ((EntityGeometry*)pBuilding)->BuildEffects();
        }
    }
    return (Entity*)pBuilding;
}

// FUNCTION: SWBF2 0x00493ac0
void EntityBuildingClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x47c86b4a:
    {
        GameObjectClass::SetProperty(uProperty, pszValue);
        const CollisionMesh* pMesh = CollisionMesh::_HashTable.Find(PblHash(pszValue));
        if (pMesh)
        {
            mCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "CollisionMesh", _IdentityMatrix, 0));
            PblBox box = pMesh->GetWorldspaceAABB(_IdentityMatrix);
            if (mBarrierX <= 0.0f)
            {
                mBarrierX = box._vMax.x - box._vMin.x;
                mBarrierOX = (box._vMin.x + box._vMax.x) * 0.5f;
            }
            if (mBarrierZ <= 0.0f)
            {
                mBarrierZ = box._vMax.z - box._vMin.z;
                mBarrierOZ = (box._vMin.z + box._vMax.z) * 0.5f;
            }
        }
        break;
    }

    case 0x5dc6e917:
    {
        mDestroyedModel = (GameModel*)GameModel::Find(PblHash(pszValue));
        const CollisionMesh* pMesh = CollisionMesh::_HashTable.Find(PblHash(pszValue));
        if (pMesh)
        {
            mDestroyedCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "DestroyedCollisionMesh", _IdentityMatrix, 0));
        }

        CollisionBodyPrimitive* pPrimitive;
        int i = 0;
        do
        {
            char acName[128];
            _snprintf(acName, 127, "%s%d", pszValue, i);
            pPrimitive = CollisionBodyPrimitive::_HashTable.Find(PblHash(acName));
            if (pPrimitive)
            {
                mDestroyedCollisionModel.AddBody(pPrimitive);
            }
            i++;
        } while (pPrimitive);
        break;
    }

    case 0x97c931c5:
    {
        mUnbuiltModel = GameModel::_HashTable.Find(PblHash(pszValue));
        const CollisionMesh* pMesh = CollisionMesh::_HashTable.Find(PblHash(pszValue));
        if (pMesh)
        {
            mUnbuiltCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "UnbuiltCollisionMesh", _IdentityMatrix, 0));
        }
        break;
    }

    case 0x48542041:
        m_pHologramClass = EntityHologramClass::Find(PblHash(pszValue));
        break;

    case 0x02bb1fe0:
    case 0x9b30e763:
        if (*pszValue)
        {
            mExplosion = ExplosionClass::Find(PblHash(pszValue));
        }
        else
        {
            mExplosion = 0;
        }
        break;

    case 0xc5637083:
        if (*pszValue)
        {
            float x = 0.0f, y = 0.0f, z = 0.0f;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            mExplosionOffset.x = x;
            mExplosionOffset.y = y;
            mExplosionOffset.z = z;
        }
        break;

    case 0xbd94b44e:
        sscanf(pszValue, "%f", &mRespawnTime);
        break;

    case 0x66aab312:
        mBuiltSound.SetID(pszValue, false, false);
        break;

    case 0xeba16fe4:
        mAmbientBuildingSound.SetID(pszValue, false, false);
        break;

    case 0xc1f062c6:
        mAutoCreateBarrier = atoi(pszValue) != 0;
        break;

    case 0x43de19e2:
        mDisableBarrierWhenDead = atoi(pszValue) != 0;
        break;

    case 0x22f50f8a:
    {
        int iValue = 0;
        sscanf(pszValue, "%i", &iValue);
        mColorizeByTeam = iValue != 0;
        break;
    }

    case 0xf52eced4:
        if (*pszValue)
        {
            sscanf(pszValue, "%f %f", &mBarrierX, &mBarrierZ);
            mBarrierOZ = 0.0f;
            mBarrierOX = 0.0f;
        }
        break;

    default:
        GameObjectClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00475db0
const PblVector3& EntityGeometry::GetVelocity() const
{
    return gVectorZero;
}

// FUNCTION: SWBF2 0x00475dd0
bool GameObject::AddToTeamManager() const
{
    return true;
}

// FUNCTION: SWBF2 0x00475de0
bool EntityBuilding::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityBuilding || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004923e0
EntityBuilding::EntityBuilding(EntityBuildingClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_STATIC, &pClass->mModel->_mLodData)
{
    mClass = pClass;
    if (pClass->mUnbuiltModel)
    {
        mState = STATE_UNBUILT;
        mHealthType = HEALTH_TYPE_BUILDING_UNBUILT;
        SetModel(mClass->mUnbuiltModel);
        SetCollisionModel(&mClass->mUnbuiltCollisionModel);

        PblVector3 vCenter;
        D3DXVec3TransformCoord(&vCenter, &mModel->_mModel->_BoundingSphere.GetCenter(), &mMatrix);
        SetCollisionCenter(vCenter);
        float fRadius = mModel->_mModel->_BoundingSphere.GetRadius();
        if (!(fRadius > mClass->mPotentialCollisionRadius))
        {
            fRadius = mClass->mPotentialCollisionRadius;
        }
        SetCollisionRadius(fRadius);
        Set2DCollisionRadius(fRadius);
        RecalculateAABB();
        SetRenderPos(GetCollisionCenter());
        SetRenderRadius(GetCollisionRadius());

        mIsAlive = false;
        SetCurHealth(0.0f, false, 0);
        mAddHealth = 0.0f;
        SetCurShield(0.0f, false, 0);
        mMaxShield = 0.0f;
        mAddShield = 0.0f;

        if (pClass->m_pHologramClass)
        {
            EntityDesc hologramDesc;
            PblMatrix mBone;
            if (mClass->mUnbuiltModel->_mLod[0]->GetBoneMatrix(0xD1E4CC7A, mBone))
            {
                PblMatrix mWorld;
                D3DXMatrixMultiply(&mWorld, &mBone, &GetMatrix());
                hologramDesc.mMatrix = mWorld;
            }
            else
            {
                hologramDesc.mMatrix = GetMatrix();
            }
            m_pHologram = (EntityHologram*)pClass->m_pHologramClass->Build(hologramDesc);
        }
        else
        {
            m_pHologram = 0;
        }
    }
    else
    {
        mState = STATE_ALIVE;
        m_pHologram = 0;
    }

    TargetManager::AddTarget(this, false);

    mpShadow = 0;
    if ((!NetGame::IsEnabled() || NetGame::IsClient()) && !mClass->mDestroyedModel && !mClass->mUnbuiltModel)
    {
        mpShadow = new RedShadowProxy;
        mpShadow->BuildData(mModel->GetRedModel(0), &mMatrix, GetCollisionRadius(), GetCollisionCenter());
    }

    RecalculateAABB();
    SetTestBBox(GetCollisionRadius() > 10.0f);
    AddToScene();
    PAAnimationGroup::GeometryCreated(desc.mNameId, this, mMatrix);

    mObstacle = 0;
    if (mClass->mAutoCreateBarrier)
    {
        PathFinder::CreateGameObjectBarrier(mObstacle, this, mClass->mBarrierX * 0.5f, mClass->mBarrierZ * 0.5f,
                                            mClass->mBarrierOX, mClass->mBarrierOZ, 1);
    }
    if (mClass->mIsDestructable)
    {
        NetGame::Created(this);
    }
}

// FUNCTION: SWBF2 0x004928f0
EntityBuilding::~EntityBuilding()
{
    NetGame::Destroyed(this);
    if (mObstacle)
    {
        PathFinder::DeleteGameObjectBarrier(mObstacle);
    }
    TargetManager::RemoveTarget(this);
    delete mpShadow;
    delete m_pHologram;
}

// SYNTHETIC: SWBF2 0x00492360
// EntityBuilding::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004940b5
// [thunk]:EntityBuilding::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x004940c0
// [thunk]:EntityBuilding::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004940c8
// [thunk]:EntityBuilding::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x004940d3
// [thunk]:EntityBuilding::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x00492330
unsigned int EntityBuilding::GetDerivedRtti() const
{
    return rttiHashEntityBuilding;
}

// FUNCTION: SWBF2 0x00492340
const char* EntityBuilding::GetDerivedRttiName() const
{
    return rttiNameEntityBuilding;
}

// FUNCTION: SWBF2 0x00475e90
bool EntityBuilding::IsUnbuilt() const
{
    return mState == STATE_UNBUILT;
}

// FUNCTION: SWBF2 0x00475e20
float EntityBuilding::GetMaxSpeed() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x00475e30
float EntityBuilding::GetMaxStrafe() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x00475e60
Controllable* EntityBuilding::GetControllable()
{
    if (mControllableList.GetCount() == 1)
    {
        return mControllableList.GetFirst()->_pObject;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00475e40
const Controllable* EntityBuilding::GetControllable() const
{
    if (mControllableList.GetCount() == 1)
    {
        return mControllableList.GetFirst()->_pObject;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00475e80
GameObjectType EntityBuilding::GetGameObjectType()
{
    return GO_BUILDING;
}

// FUNCTION: SWBF2 0x00492350
EntityType EntityBuilding::GetEntityType() const
{
    return ENT_BUILDING;
}

// FUNCTION: SWBF2 0x00475ea0
const PblMatrix* EntityBuilding::GetRenderMatrix() const
{
    return &mMatrix;
}

// FUNCTION: SWBF2 0x00475eb0
const RedPose* EntityBuilding::GetRenderPose() const
{
    return static_cast<const CollisionObject*>(this)->GetPose();
}

static inline Damageable* AsDamageable(EntityBuilding* pBuilding)
{
    return static_cast<Damageable*>(pBuilding);
}

static GameObject* GetHandleObject(PblHandle<GameObject>& handle)
{
    if (handle.mObject &&
        handle.mObject->mHandleId !=
            handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

// FUNCTION: SWBF2 0x00492a50
void EntityBuilding::ClientKill()
{
    EntityBuildingClass* pClass = mClass;
    if (!pClass->mExplosion)
    {
        return;
    }
    DamageOwner& damageOwner = AsDamageable(this)->mDamageOwner;
    DamageOwner owner(damageOwner.mCharacter, GetHandleObject(damageOwner.mGameObject), damageOwner.mTeam, pClass,
                      0, 1.0f);
    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &mClass->mExplosionOffset, &mMatrix);
    pClass->mExplosion->CreateExplosion(vPos, owner, false, COLL_UNKNOWN);
}

// FUNCTION: SWBF2 0x004929b0
void EntityBuilding::HealthChangeCallback(bool bArg, float fDelta)
{
    Damageable* pDamageable = AsDamageable(this);
    bool bClient = NetGame::IsEnabled() && NetGame::IsClient();
    if (pDamageable->mCurHealth > 0.0f)
    {
        if (pDamageable->mCurHealth >= pDamageable->mMaxHealth)
        {
            pDamageable->mCurHealth = pDamageable->mMaxHealth;
            if (!bClient && mState != STATE_ALIVE)
            {
                pDamageable->Respawn();
            }
        }
    }
    else
    {
        pDamageable->mCurHealth = 0.0f;
        if (mState == STATE_ALIVE)
        {
            pDamageable->Kill();
        }
    }
    pDamageable->Damageable::HealthChangeCallback(bArg, fDelta);
}

// FUNCTION: SWBF2 0x00492b00
void EntityBuilding::Kill()
{
    if (mState == STATE_DEAD || mState == STATE_UNBUILT)
    {
        return;
    }

    mIsAlive = false;
    mCreatedTime = GameLoop::GetMissionTime();

    if (mClass->mDestroyedModel || mClass->mUnbuiltModel)
    {
        if (mClass->mDestroyedModel)
        {
            mState = STATE_DEAD;
            if (mClass->mHealthType == HEALTH_TYPE_BUILDING)
            {
                mHealthType = HEALTH_TYPE_BUILDING_DEAD;
            }
            SetModel(mClass->mDestroyedModel);
            SetCollisionModel(&mClass->mDestroyedCollisionModel);
            if (m_pHologram)
            {
                m_pHologram->TurnOnNow();
            }
        }
        else
        {
            mState = STATE_UNBUILT;
            if (mClass->mHealthType == HEALTH_TYPE_BUILDING)
            {
                mHealthType = HEALTH_TYPE_BUILDING_UNBUILT;
            }
            SetModel(mClass->mUnbuiltModel);
            SetCollisionModel(&mClass->mUnbuiltCollisionModel);
            if (m_pHologram)
            {
                m_pHologram->TurnOn();
            }
        }

        SetMatrix(mMatrix);
        PblVector3 vCenter;
        D3DXVec3TransformCoord(&vCenter, &mModel->_mModel->_BoundingSphere.GetCenter(), &mMatrix);
        SetCollisionCenter(vCenter);
        float fRadius = mModel->_mModel->_BoundingSphere.GetRadius();
        if (!(fRadius > mClass->mPotentialCollisionRadius))
        {
            fRadius = mClass->mPotentialCollisionRadius;
        }
        SetCollisionRadius(fRadius);
        Set2DCollisionRadius(fRadius);
        RecalculateAABB();
        SetRenderPos(GetCollisionCenter());
        SetRenderRadius(GetCollisionRadius());
        if (TreeGrid* pTreeGrid = CollisionManager::GetTreeGrid())
        {
            pTreeGrid->Update(this, GetCollisionCenter(), GetCollisionRadius() * 2.0f);
        }
        if (mpShadow)
        {
            mpShadow->Reset();
            mpShadow->BuildData(mModel->GetRedModel(0), &mMatrix, GetCollisionRadius(), GetCollisionCenter());
        }
    }
    else
    {

        mClass->mSoundDeath.Play(&mMatrix.GetTranslation(), 0, false, true);
        mAmbientSound.Stop(true);
        mState = STATE_DEAD;
        if (mClass->mHealthType == HEALTH_TYPE_BUILDING)
        {
            mHealthType = HEALTH_TYPE_BUILDING_DEAD;
        }
        RemoveFromScene();
        RemoveFromCollisionManager();
    }

    EntityBuildingClass* pClass = mClass;
    if (pClass->mExplosion)
    {
        DamageOwner& damageOwner = mDamageOwner;
        DamageOwner owner(damageOwner.mCharacter, GetHandleObject(damageOwner.mGameObject), damageOwner.mTeam,
                          pClass, 0, 1.0f);
        PblVector3 vPos;
        D3DXVec3TransformCoord(&vPos, &mClass->mExplosionOffset, &mMatrix);
        pClass->mExplosion->CreateExplosion(vPos, owner, false, COLL_UNKNOWN);
    }

    CreateChunkCluster(1.0f);
    mAddHealth = mMaxHealth / mClass->mRespawnTime;
    SetCurShield(0.0f, false, 0);
    mMaxShield = 0.0f;
    mAddShield = 0.0f;
    if (mObstacle && mClass->mDisableBarrierWhenDead)
    {
        mObstacle->SetEnabled(false);
    }
    GameObject::Kill();
}

// FUNCTION: SWBF2 0x004922b0
void __fastcall RestoreDamageableHealth(Damageable* pDamageable)
{
    pDamageable->mCurHealth = pDamageable->mMaxHealth;
    pDamageable->mCurShield = pDamageable->mMaxShield;
    pDamageable->mDamageOwner = DamageOwner(0, 0, 0, 0, 0, 1.0f);
}

// FUNCTION: SWBF2 0x00492f30
void EntityBuilding::Respawn()
{
    if (mState == STATE_ALIVE)
    {
        return;
    }

    mIsAlive = true;
    mState = STATE_ALIVE;
    mCreatedTime = GameLoop::GetMissionTime();

    mClass->mBuiltSound.Play(&mMatrix.GetTranslation(), 0, false, true);
    if (mClass->mHealthType == HEALTH_TYPE_BUILDING)
    {
        mHealthType = HEALTH_TYPE_BUILDING;
    }
    if (mObstacle && mClass->mDisableBarrierWhenDead)
    {
        mObstacle->SetEnabled(true);
    }

    if (mClass->mDestroyedModel || mClass->mUnbuiltModel)
    {
        SetModel(mClass->mModel);
        SetCollisionModel(&mClass->mCollisionModel);
        if (mpShadow)
        {
            mpShadow->Reset();
            mpShadow->BuildData(mModel->GetRedModel(0), &mMatrix, GetCollisionRadius(), GetCollisionCenter());
        }
        if (m_pHologram)
        {
            m_pHologram->TurnOnNow();
        }

        SetMatrix(mMatrix);
        PblVector3 vCenter;
        D3DXVec3TransformCoord(&vCenter, &mModel->_mModel->_BoundingSphere.GetCenter(), &mMatrix);
        SetCollisionCenter(vCenter);
        float fRadius = mModel->_mModel->_BoundingSphere.GetRadius();
        if (!(fRadius > mClass->mPotentialCollisionRadius))
        {
            fRadius = mClass->mPotentialCollisionRadius;
        }
        SetCollisionRadius(fRadius);
        Set2DCollisionRadius(fRadius);
        RecalculateAABB();
        SetRenderPos(GetCollisionCenter());
        SetRenderRadius(GetCollisionRadius());
        if (TreeGrid* pTreeGrid = CollisionManager::GetTreeGrid())
        {
            pTreeGrid->Update(this, GetCollisionCenter(), GetCollisionRadius() * 2.0f);
        }
    }
    else
    {
        AddToScene();
        AddStaticToCollisionManager();
    }

    mAddHealth = mClass->mAddHealth;
    RestoreDamageableHealth(AsDamageable(this));
    SetCurShield(mClass->mAddShield > 0.0f ? 0.0f : mClass->mMaxShield, false, 0);
    mMaxShield = mClass->mMaxShield;
    mAddShield = mClass->mAddShield;
    GameObject::Respawn();
}

// FUNCTION: SWBF2 0x00493630
bool EntityBuilding::ApplyDamage(const DamageDesc& damage, const PblVector3& vDir, const PblVector3& vPos,
                                 int iBodyPart, unsigned int uFlags)
{
    if (NetGame::IsPreGame())
    {
        return true;
    }
    if (!Damageable::ApplyDamage(damage, vDir, vPos, iBodyPart, uFlags))
    {
        return false;
    }

    mClass->mSoundHurt.Play(&mMatrix.GetTranslation(), 0, false, true);
    return true;
}

// FUNCTION: SWBF2 0x004923c0
TreeGrid* CollisionManager::GetTreeGrid()
{
    return sInstance ? sInstance->mTreeGrid : 0;
}

// FUNCTION: SWBF2 0x004439c0
const GameModel* GameModel::Find(unsigned int uHash)
{
    return _HashTable.Find(uHash);
}

// FUNCTION: SWBF2 0x004920c0
CollisionBodyPrimitive* CollisionBodyPrimitive::Find(unsigned int uHash)
{
    return _HashTable.Find(uHash);
}

// FUNCTION: SWBF2 0x00493280
void EntityBuilding::SetPosition(const PblVector3& vPos)
{
    EntityGeometry::SetPosition(vPos);
}

// FUNCTION: SWBF2 0x00493290
void EntityBuilding::SetMatrix(const PblMatrix& matrix)
{
    EntityGeometry::SetMatrix(matrix);
}

// FUNCTION: SWBF2 0x004932a0
void EntityBuilding::RecalculateAABB()
{
    mAABB = mModel->_mModel->_TotalBoundingBox.GetTransformedBox(mMatrix);
}

// FUNCTION: SWBF2 0x004932e0
void EntityBuilding::Update(float fDeltaTime)
{
    bool bPlaying = mAmbientSound.GetPlaying();
    PblVector3 vPos = mMatrix.GetTranslation();
    if (IsAlive())
    {
        if (!bPlaying)
        {
            return;
        }
        mAmbientSound.Play(&mClass->mAmbientBuildingSound, &vPos, NULL, true);
    }
    if (bPlaying)
    {
        mAmbientSound.Update(&mClass->mAmbientBuildingSound, &vPos, NULL);
    }
}

// FUNCTION: SWBF2 0x00493360
void EntityBuilding::Render(unsigned char uFlags, float fDist, unsigned int uMask)
{

    Update(0.0f);

    if (mClass->m_bDynamicLighting)
    {
        uMask &= ~0x80;
    }
    else
    {
        uMask |= 0x80;
    }
    if (mpShadow && mpShadow->IsInScene())
    {
        uMask &= ~0x10000;
    }
    else
    {
        SetupShadow(this, mModel, uFlags, uMask, 0);
    }

    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(int)(fDist * 255.0f);
    if (mClass->mColorizeByTeam)
    {
        int iPlayer = NetGame::GetLocalIndex(CameraManager::sInstance->mUpdatedCameraId);
        int iSpectator = NetGame::GetSpectator(iPlayer);
        if (iSpectator >= 0)
        {
            iPlayer = iSpectator;
        }
        Character* pCharacter = Character::FindPlayer(iPlayer);
        Team* pTeam = pCharacter ? pCharacter->mTeamPtr : Team::sTeams[1];

        color *= pTeam->mColor[GetGameObject()->GetTeam()];
    }

    if (!mModel)
    {
        return;
    }

    mModel->Render(uFlags, fDist, GetRenderMatrix(), GetRenderPose(), color, uMask | 0x1000000, NULL);
    if (uMask & 0x200000)
    {
        return;
    }

    PblMatrix mReflection;
    if (FLRenderer::IsReflected(GetCollisionCenter(), GetCollisionRadius(), mReflection, false))
    {
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, GetRenderMatrix(), &mReflection);
        mReflection = mWorld;
        mModel->Render(uFlags, fDist, &mReflection, GetRenderPose(), color,
                       (uMask & ~0x10000) | 0x10000100, NULL);
    }
}
