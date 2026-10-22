#include "EntityTrap.h"
#include "PebbleFL/Common/PblVector.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionMesh.h"
#include "CollisionObject.h"
#include "Damageable.h"
#include "EntityHologram.h"
#include "EntityProp.h"
#include "FLEffect.h"
#include "GameModel.h"
#include "NetGame.h"
#include "Team.h"
#include "Thread.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "Zephyr/Source/ZephyrAnim.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x004fe100
EntityTrapClass::EntityTrapClass(unsigned int uHash)
    : EntityBuildingClass(uHash)
{
    mAnimObj = 0;
    mAnimBuildTrap = 0;
    mBuildOffset = _IdentityMatrix;
    mBuildBoxOdf = 0;
    mAnimTriggerTrap = 0;
    mTriggerSide = 0;
    mResetSide = 0;
    mTriggerNode = 0;
    mTriggerOffset.x = 0.0f;
    mTriggerOffset.y = 0.0f;
    mTriggerOffset.z = 0.0f;
    mTriggerRadius = 3.0f;
    mMinEnemyRadius = 0.0f;
    mDeathTime = -1.0f;
    mDeathEffect = 0;
    mDeathEffectOffset.x = 0.0f;
    mDeathEffectOffset.y = 0.0f;
    mDeathEffectOffset.z = 0.0f;
    mCollisionOnTrigger = false;
    mCollisionOnBuilt = false;
    mHideUnbuiltModel = false;
    mDeathOnCollision = false;
    mRayTriggerWidth = -1.0f;
    mRayTriggerMinSpeed = -1.0f;
}

// GLOBAL: SWBF2 0x01ebc254
// SYNTHETIC: SWBF2 0x00402eb0
// `dynamic initializer for 'EntityTrapClass::rttiHashEntityTrapClass''
PblHash EntityTrapClass::rttiHashEntityTrapClass("EntityTrapClass");
char EntityTrapClass::rttiNameEntityTrapClass[] = "EntityTrapClass";

// GLOBAL: SWBF2 0x01ebc258
// SYNTHETIC: SWBF2 0x00402ea0
// `dynamic initializer for 'EntityTrap::rttiHashEntityTrap''
PblHash EntityTrap::rttiHashEntityTrap("EntityTrap");
char EntityTrap::rttiNameEntityTrap[] = "EntityTrap";

static inline CollisionObject* AsCollisionObject(EntityTrap* pTrap)
{
    return static_cast<CollisionObject*>(pTrap);
}

static inline Damageable* AsDamageable(EntityTrap* pTrap)
{
    return static_cast<Damageable*>(pTrap);
}

// FUNCTION: SWBF2 0x004fcd90
EntityTrap::EntityTrap(EntityTrapClass* pClass, const EntityDesc& desc)
    : EntityBuilding(pClass, desc)
{
    mAnimObj = pClass->mAnimObj;
    mTrapClass = pClass;
    mZephyrSkeleton.Open(*pClass->mAnimObj->GetZephyrSkeletonShared());
    mZephyrPoseDyn.Open(mZephyrSkeleton);
    mCurrAnim = 0;
    mBuildBox = 0;
    mAnimationTime = 0.0f;
    mBuildingTrap = false;
    mUnbuiltTime = 0.0f;
    mTrapCollisionSphere = GetCollisionSphere();
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x004fcf00
EntityTrap::~EntityTrap()
{
    NetGame::Destroyed(this);
}

// SYNTHETIC: SWBF2 0x004fcd00
// EntityTrap::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004ff4ab
// [thunk]:EntityTrap::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x004ff4b6
// [thunk]:EntityTrap::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004ff4be
// [thunk]:EntityTrap::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x004ff4c9
// [thunk]:EntityTrap::`scalar deleting destructor' adjustor{608}
// SYNTHETIC: SWBF2 0x004ff4d4
// [thunk]:EntityTrap::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x004fcc70
bool EntityTrap::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityTrap || EntityBuilding::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004fccc0
unsigned int EntityTrap::GetDerivedRtti() const
{
    return rttiHashEntityTrap;
}

// FUNCTION: SWBF2 0x004fccd0
const char* EntityTrap::GetDerivedRttiName() const
{
    return rttiNameEntityTrap;
}

// FUNCTION: SWBF2 0x004fcce0
const RedPose* EntityTrap::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x004fccf0
EntityType EntityTrap::GetEntityType() const
{
    return ENT_TRAP;
}

// FUNCTION: SWBF2 0x004fcfb0
void EntityTrap::SetAnimation(const ZephyrAnim* pAnim)
{
    mCurrAnim = const_cast<ZephyrAnim*>(pAnim);
    mTrapClass->SetupAnimation(mZephyrPoseDyn, pAnim, mAnimationTime);

    ZephyrPoseStatic<64> kPose;
    mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    kPose.Set(mZephyrPoseDyn, true);
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);
}

// FUNCTION: SWBF2 0x004fd070
void EntityTrap::Render(unsigned char uFlags, float fDist, unsigned int uMask)
{
    if (mTrapClass->mHideUnbuiltModel && mState == STATE_UNBUILT &&
        AsCollisionObject(this)->mCollisionModel == &mTrapClass->mBuildBoxCollisionModel)
    {
        return;
    }
    EntityBuilding::Render(uFlags, fDist, uMask);
}

// FUNCTION: SWBF2 0x004fde40
bool EntityTrap::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor)
{
    if (mTrapClass->mDeathOnCollision && mState == STATE_ALIVE && mTriggered)
    {
        GameObject* pObject = pOther->GetGameObject();
        if (pObject && Team::sTeams[pObject->GetTeam()]->GetSide() == mTrapClass->mTriggerSide)
        {
            Damageable* pDamageable = static_cast<Damageable*>(pObject);
            pDamageable->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
        }
    }
    return AsCollisionObject(this)->CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x004fdef0
void EntityTrap::HealthChangeCallback(bool bArg, float fDelta)
{
    typedef void(__thiscall * SetModelFn)(void*, const GameModel*);
    typedef void(__thiscall * DeactivateFn)(void*);

    Damageable* pDamageable = AsDamageable(this);
    if (mState == STATE_UNBUILT && pDamageable->mCurHealth >= pDamageable->mMaxHealth)
    {
        CollisionObject* pCollision = AsCollisionObject(this);
        if (mTrapClass->mBuildBoxOdf)
        {
            pCollision->SetCollisionModel(&mTrapClass->mTrapCollisionModel);
            pCollision->SetCollisionCenter(mTrapCollisionSphere.GetCenter());
            pCollision->SetCollisionRadius(mTrapCollisionSphere.GetRadius());
            pCollision->Set2DCollisionRadius(mTrapCollisionSphere.GetRadius());
            pCollision->RecalculateAABB();
            if (pCollision->IsInCollisionList())
            {
                pCollision->RemoveFromCollisionManager();
            }
        }
        if (mTrapClass->mCollisionOnBuilt)
        {
            if (!pCollision->IsInCollisionList())
            {
                pCollision->AddStaticToCollisionManager();
            }
        }
        else if (pCollision->IsInCollisionList())
        {
            pCollision->RemoveFromCollisionManager();
        }

        pDamageable->mIsAlive = true;
        mState = STATE_ALIVE;

        SetModel(mTrapClass->mModel);
        if (m_pHologram)
        {
            m_pHologram->TurnOnNow();
        }
        if (mBuildBox->IsRtti(EntityProp::rttiHashEntityProp))
        {

            EntityProp* pProp = static_cast<EntityProp*>(mBuildBox);
            static_cast<RedSceneObject*>(pProp)->Deactivate();
            pProp->RemoveFromCollisionManager();
        }
        pDamageable->mHealthType = HEALTH_TYPE_BUILDING;
        mBuildingTrap = true;
        mTriggered = false;
        pDamageable->mCurHealth = pDamageable->mMaxHealth;
    }
    pDamageable->Damageable::HealthChangeCallback(bArg, fDelta);
    if (!pDamageable->mIsAlive)
    {
        mTriggered = true;
    }
}

// FUNCTION: SWBF2 0x004fe090
bool EntityTrap::IsVisibleToMe(const GameObject* pObject) const
{
    return Team::sTeams[pObject->GetTeam()]->GetSide() != mTrapClass->mTriggerSide;
}

// FUNCTION: SWBF2 0x004fe0d0
int EntityTrap::GetTargetBodyID(const PblVector3&, const PblVector3&) const
{
    return -1;
}

// FUNCTION: SWBF2 0x004fe0e0
PblVector3 EntityTrap::GetTargetPoint(const PblVector3&, const PblVector3&, int) const
{
    return mBuildBoxCollisionSphere.GetCenter();
}

// SYNTHETIC: SWBF2 0x004fe260
// EntityTrapClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fe290
EntityTrapClass::EntityTrapClass(EntityTrapClass* pBase, unsigned int uHash)
    : EntityBuildingClass(pBase, uHash)
{
    mAnimObj = pBase->mAnimObj;
    mAnimBuildTrap = pBase->mAnimBuildTrap;
    mBuildOffset = pBase->mBuildOffset;
    mBuildBoxOdf = pBase->mBuildBoxOdf;
    mAnimTriggerTrap = pBase->mAnimTriggerTrap;
    mTriggerSide = pBase->mTriggerSide;
    mResetSide = pBase->mResetSide;
    mTriggerNode = pBase->mTriggerNode;
    mTriggerOffset = pBase->mTriggerOffset;
    mTriggerRadius = pBase->mTriggerRadius;
    mMinEnemyRadius = pBase->mMinEnemyRadius;
    mDeathTime = pBase->mDeathTime;
    mDeathEffect = pBase->mDeathEffect;
    mDeathEffectOffset = pBase->mDeathEffectOffset;
    mTriggerSound = pBase->mTriggerSound;
    mCollisionOnBuilt = pBase->mCollisionOnBuilt;
    mCollisionOnTrigger = pBase->mCollisionOnTrigger;
    mDeathOnCollision = pBase->mDeathOnCollision;
    mRayTriggerWidth = pBase->mRayTriggerWidth;
    mRayTriggerMinSpeed = pBase->mRayTriggerMinSpeed;
    mHideUnbuiltModel = pBase->mHideUnbuiltModel;
}

// FUNCTION: SWBF2 0x004fe460
EntityTrapClass::~EntityTrapClass()
{
}

// FUNCTION: SWBF2 0x004fe4d0
bool EntityTrapClass::InitAnimations(const char* pszName)
{
    mAnimObj = RedAnimation::Find(PblHash(pszName));
    return mAnimObj != 0;
}

// FUNCTION: SWBF2 0x004fe510
bool EntityTrapClass::SetupAnimation(ZephyrPoseDyn<64>& kPose, const ZephyrAnim* pAnim, float fTime)
{
    if (!pAnim)
    {
        return false;
    }
    kPose.SetLoop(0);
    kPose.SetAnimation(pAnim, 30.0f);
    kPose.SetCurTime(fTime);
    kPose.Update(0.0f);
    return true;
}

// FUNCTION: SWBF2 0x004fe5b0
void EntityTrapClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x98557a5e:
        InitAnimations(pszValue);
        break;

    case 0x2fd8a07b:
        if (mAnimObj)
        {
            mAnimBuildTrap = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
        }
        break;

    case 0xf7a5d75b:
        if (mAnimObj)
        {
            mAnimTriggerTrap = (ZephyrAnim*)mAnimObj->FindAnimation(pszValue);
        }
        break;

    case 0x619005ab:
    {
        GameModel* pModel = mUnbuiltModel ? mUnbuiltModel : mModel;
        pModel->GetRedModel(0)->GetBoneMatrix(PblTEMPHash(pszValue), mBuildOffset);
        break;
    }

    case 0x138b5187:
        mBuildBoxOdf = EntityClass::Find(PblHash(pszValue));
        break;

    case 0x47c86b4a:
    {
        EntityBuildingClass::SetProperty(uProperty, pszValue);
        const CollisionMesh* pMesh = CollisionMesh::Find(PblHash(pszValue));
        if (pMesh)
        {
            mTrapCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "CollisionMesh", _IdentityMatrix, 0));
        }

        CollisionBodyPrimitive* pPrimitive;
        int i = 0;
        do
        {
            char acName[128];
            _snprintf(acName, 127, "%s%d", pszValue, i);
            acName[127] = 0;
            pPrimitive = CollisionBodyPrimitive::_HashTable.Find(PblHash(acName));
            if (pPrimitive)
            {
                mTrapCollisionModel.AddBody(pPrimitive);
            }
            i++;
        } while (pPrimitive);
        break;
    }

    case 0xabcb950a:
        switch (PblTEMPHash(pszValue))
        {
        case 0x335dfa3a: mTriggerSide = 1; break;
        case 0x969c1fca: mTriggerSide = 2; break;
        case 0x0df20559: mTriggerSide = 3; break;
        case 0xd38f258c: mTriggerSide = 4; break;
        default:         mTriggerSide = 0; break;
        }
        break;

    case 0x2e4c2b79:
        switch (PblTEMPHash(pszValue))
        {
        case 0x335dfa3a: mResetSide = 1; break;
        case 0x969c1fca: mResetSide = 2; break;
        case 0x0df20559: mResetSide = 3; break;
        case 0xd38f258c: mResetSide = 4; break;
        default:         mResetSide = 0; break;
        }
        break;

    case 0x58fe8213:
        mTriggerNode = PblTEMPHash(pszValue);
        break;

    case 0xfe7c5230:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mTriggerOffset.x = x;
        mTriggerOffset.y = y;
        mTriggerOffset.z = z;
        break;
    }

    case 0xc461d811:
        sscanf(pszValue, "%f", &mTriggerRadius);
        break;

    case 0x479e5b63:
        sscanf(pszValue, "%f", &mMinEnemyRadius);
        break;

    case 0xb4c6360c:
        sscanf(pszValue, "%f", &mDeathTime);
        break;

    case 0xf138015c:
        mDeathEffect = PblHash(pszValue);
        break;

    case 0x29824727:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mDeathEffectOffset.x = x;
        mDeathEffectOffset.y = y;
        mDeathEffectOffset.z = z;
        break;
    }

    case 0x836ff8c2:
        mTriggerSound.SetID(pszValue, false, false);
        break;

    case 0x146ffe85:
        if (atoi(pszValue))
        {
            mCollisionOnTrigger = true;
        }
        break;

    case 0x06b4bbb5:
        if (atoi(pszValue))
        {
            mCollisionOnBuilt = true;
        }
        break;

    case 0x3fd818d0:
        if (atoi(pszValue))
        {
            mDeathOnCollision = true;
        }
        break;

    case 0x42e950ab:
        sscanf(pszValue, "%f", &mRayTriggerWidth);
        break;

    case 0xdc6a93e6:
        sscanf(pszValue, "%f", &mRayTriggerMinSpeed);
        break;

    case 0x0d689977:
        if (atoi(pszValue))
        {
            mHideUnbuiltModel = true;
        }
        break;

    default:
        EntityBuildingClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004fecb0
EntityClass* EntityTrapClass::Derive(unsigned int uHash)
{
    return new EntityTrapClass(this, uHash);
}

// FUNCTION: SWBF2 0x004fcd30
bool EntityTrapClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityTrapClass || EntityBuildingClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004fcd70
unsigned int EntityTrapClass::GetDerivedRtti()
{
    return rttiHashEntityTrapClass;
}

// FUNCTION: SWBF2 0x004fcd80
const char* EntityTrapClass::GetDerivedRttiName()
{
    return rttiNameEntityTrapClass;
}

// FUNCTION: SWBF2 0x004fd0b0
bool EntityTrap::Update(float fDeltaTime)
{
    if (GetTeam() == 0 && mTrapClass->mResetSide)
    {
        int iTeam = 0;
        for (int i = 0; i < Team::GetCount(); ++i)
        {
            if (Team::sTeams[i]->GetSide() == mTrapClass->mResetSide)
            {
                iTeam = i;
            }
        }
        SetTeam(iTeam);
        SetPerceivedTeam(iTeam);
    }

    if (!mCurrAnim && mTrapClass->mAnimTriggerTrap)
    {
        mAnimationTime = mTrapClass->mAnimTriggerTrap->GetNumFrames() / 30.0f;
        SetAnimation(mTrapClass->mAnimTriggerTrap);
    }

    if (mState == STATE_UNBUILT)
    {
        mUnbuiltTime += fDeltaTime;
        if (mUnbuiltTime > 10.0f)
        {
            if (mTrapClass->mBuildBoxOdf)
            {
                SetCollisionModel(&mTrapClass->mBuildBoxCollisionModel);
                SetCollisionCenter(mBuildBoxCollisionSphere.GetCenter());
                SetCollisionRadius(mBuildBoxCollisionSphere.GetRadius());
                Set2DCollisionRadius(mBuildBoxCollisionSphere.GetRadius());
                RecalculateAABB();
                if (IsInCollisionList())
                {
                    RemoveFromCollisionManager();
                }
            }
            if (!IsInCollisionList())
            {
                AddStaticToCollisionManager();
            }
        }
        return true;
    }

    if (mState != STATE_ALIVE)
    {
        return true;
    }

    if (mBuildingTrap)
    {
        if (mCurrAnim != mTrapClass->mAnimBuildTrap)
        {
            mCurrAnim = mTrapClass->mAnimBuildTrap;
            mAnimationTime = 0.0f;
            if (mTrapClass->mAnimBuildTrap)
            {
                SetAnimation(mTrapClass->mAnimBuildTrap);
            }
        }
        mAnimationTime += fDeltaTime;
        mZephyrPoseDyn.Update(fDeltaTime);

        ZephyrPoseStatic<64> kPose;
        mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
        kPose.Open(mZephyrSkeleton);
        kPose.Set(mZephyrPoseDyn, true);
        mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
        mPose.ConvertFromZephyrPose(mZephyrSkeleton);

        if (mTrapClass->mAnimBuildTrap)
        {
            if (mAnimationTime >= mTrapClass->mAnimBuildTrap->GetNumFrames() / 30.0f)
            {
                mBuildingTrap = false;
                if (mTrapClass->mAnimTriggerTrap)
                {
                    mAnimationTime = 0.0f;
                    SetAnimation(mTrapClass->mAnimTriggerTrap);
                }
            }
        }
        else if (mTrapClass->mAnimTriggerTrap)
        {
            mBuildingTrap = false;
            mAnimationTime = 0.0f;
            SetAnimation(mTrapClass->mAnimTriggerTrap);
        }
        return true;
    }

    if (!mTriggered)
    {
        PblMatrix mNode = _IdentityMatrix;
        if (mTrapClass->mTriggerNode)
        {
            GetModel()->GetRedModel(0)->GetBoneMatrix(mTrapClass->mTriggerNode, mNode);
        }
        PblMatrix mOffset = _IdentityMatrix;
        mOffset.m[3][0] = mTrapClass->mTriggerOffset.x;
        mOffset.m[3][1] = mTrapClass->mTriggerOffset.y;
        mOffset.m[3][2] = mTrapClass->mTriggerOffset.z;
        mOffset.m[3][3] = 1.0f;
        PblMatrix mLocal;
        D3DXMatrixMultiply(&mLocal, &mOffset, &mNode);
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, &mLocal, &mMatrix);

        PblVector3 vTrigger = mWorld.GetTranslation();
        if (RedTerrain::_bEnable)
        {
            vTrigger.y = RedTerrain::GetInterpolatedHeight(vTrigger.x, vTrigger.z);
        }

        PblVector3 vRight = mMatrix.GetXAxis();
        PblVector3 vForward = mMatrix.GetZAxis();
        const PblVector3& vPos = mMatrix.GetTranslation();

        GameObject* apObjects[5];
        int iCount;
        if (mTrapClass->mRayTriggerWidth > 0.0f)
        {
            PblBox kBox = GetModel()->_mModel->_TotalBoundingBox;
            float fHalfWidth = (kBox._vMax.x - kBox._vMin.x) * 0.5f;
            PblVector3 vFrom(vPos.x - vRight.x * fHalfWidth, vPos.y - vRight.y * fHalfWidth,
                             vPos.z - vRight.z * fHalfWidth);
            PblVector3 vTo(vPos.x + vRight.x * fHalfWidth, vPos.y + vRight.y * fHalfWidth,
                           vPos.z + vRight.z * fHalfWidth);
            iCount = CollisionManager::GetObjectsInFatRay(vTo, vFrom, mTrapClass->mRayTriggerWidth, apObjects,
                                                         5, 0, 10);
        }
        else
        {
            iCount = CollisionManager::GetObjectsInRange(vTrigger, mTrapClass->mTriggerRadius, apObjects, 5, 0,
                                                        10);
        }

        for (int i = 0; i < iCount; ++i)
        {
            CollisionObject* pCollision = apObjects[i]->GetCollisionObject();
            if (!pCollision || pCollision->GetCollisionRadius() <= mTrapClass->mMinEnemyRadius)
            {
                continue;
            }
            if (mTrapClass->mRayTriggerWidth > 0.0f)
            {
                const PblVector3& vCenter = pCollision->GetCollisionCenter();
                float fDist = (float)fabs((vPos.x - vCenter.x) * vForward.x + vForward.y * (vPos.y - vCenter.y) +
                                          vForward.z * (vPos.z - vCenter.z));
                if (fDist > pCollision->GetCollisionRadius() + mTrapClass->mRayTriggerWidth)
                {
                    continue;
                }
                PblVector3 vVelocity = pCollision->GetGameObject()->GetVelocity();
                if (mTrapClass->mRayTriggerMinSpeed >
                    (float)fabs(vVelocity.x * vForward.x + vVelocity.y * vForward.y + vVelocity.z * vForward.z))
                {
                    continue;
                }
                const PblVector3& vTheirs = pCollision->GetCollisionCenter();
                const PblVector3& vMine = GetCollisionCenter();
                if ((vMine.x - vTheirs.x) * vVelocity.x + (vMine.y - vTheirs.y) * vVelocity.y +
                        (vMine.z - vTheirs.z) * vVelocity.z < 0.0f)
                {
                    continue;
                }
            }
            if (Team::sTeams[apObjects[i]->GetTeam()]->GetSide() != mTrapClass->mTriggerSide)
            {
                continue;
            }

            if (mTrapClass->mCollisionOnTrigger)
            {
                if (!IsInCollisionList())
                {
                    AddStaticToCollisionManager();
                }
            }
            else if (IsInCollisionList())
            {
                RemoveFromCollisionManager();
            }
            mTriggered = true;
            CreateChunkCluster(1.0f);
            mTrapClass->mTriggerSound.Play(&GetMatrix().GetTranslation(), 0, false, true);
            break;
        }
        if (!mTriggered)
        {
            return true;
        }
    }

    if (mCurrAnim != mTrapClass->mAnimTriggerTrap)
    {
        mCurrAnim = mTrapClass->mAnimTriggerTrap;
        mAnimationTime = 0.0f;
        if (mTrapClass->mAnimTriggerTrap)
        {
            SetAnimation(mTrapClass->mAnimTriggerTrap);
        }
    }

    float fDeathTime = mTrapClass->mDeathTime;
    if (fDeathTime > 0.0f && fDeathTime > mAnimationTime && mAnimationTime + fDeltaTime >= fDeathTime)
    {
        bool bKilled = false;
        PblMatrix mNode(_IdentityMatrix);
        if (mTrapClass->mTriggerNode)
        {
            GetModel()->GetRedModel(0)->GetBoneMatrix(mTrapClass->mTriggerNode, mNode);
        }
        PblMatrix mOffset(_IdentityMatrix);
        mOffset.m[3][0] = mTrapClass->mTriggerOffset.x;
        mOffset.m[3][1] = mTrapClass->mTriggerOffset.y;
        mOffset.m[3][2] = mTrapClass->mTriggerOffset.z;
        mOffset.m[3][3] = 1.0f;
        PblMatrix mLocal;
        D3DXMatrixMultiply(&mLocal, &mOffset, &mNode);
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, &mLocal, &mMatrix);

        PblVector3 vTrigger = mWorld.GetTranslation();
        if (RedTerrain::_bEnable)
        {
            vTrigger.y = RedTerrain::GetInterpolatedHeight(vTrigger.x, vTrigger.z);
        }

        GameObject* apObjects[5];
        int iCount = CollisionManager::GetObjectsInRange(vTrigger, mTrapClass->mTriggerRadius, apObjects, 5, 0, 10);
        for (int i = 0; i < iCount; ++i)
        {
            CollisionObject* pCollision = apObjects[i]->GetCollisionObject();
            if (pCollision && pCollision->GetCollisionRadius() > mTrapClass->mMinEnemyRadius)
            {
                static_cast<Damageable*>(apObjects[i])->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
                bKilled = true;
            }
        }
        if (bKilled)
        {
            FLEffectClass* pEffectClass = FLEffect::Get(mTrapClass->mDeathEffect);
            if (pEffectClass)
            {
                PblMatrix mEffect = _IdentityMatrix;
                mEffect.m[3][0] = mTrapClass->mDeathEffectOffset.x;
                mEffect.m[3][1] = mTrapClass->mDeathEffectOffset.y;
                mEffect.m[3][2] = mTrapClass->mDeathEffectOffset.z;
                mEffect.m[3][3] = 1.0f;
                FLEffectObject* pEffect = pEffectClass->BuildEffect();
                if (pEffect)
                {
                    pEffect->AttachEffectToObject(this, mEffect, true);
                }
            }
        }
    }

    mAnimationTime += fDeltaTime;
    mZephyrPoseDyn.Update(fDeltaTime);

    ZephyrPoseStatic<64> kPose;
    mZephyrSkeleton.Open(*mZephyrSkeleton.m_pShared, kPose);
    kPose.Open(mZephyrSkeleton);
    kPose.Set(mZephyrPoseDyn, true);
    mZephyrSkeleton.BuildWorldMatrices(_IdentityMatrix);
    mPose.ConvertFromZephyrPose(mZephyrSkeleton);

    if (!mTrapClass->mAnimTriggerTrap ||
        mAnimationTime >= mTrapClass->mAnimTriggerTrap->GetNumFrames() / 30.0f)
    {
        mIsAlive = false;
        mCurHealth = 0.0f;
        mHealthType = HEALTH_TYPE_BUILDING_UNBUILT;
        mState = STATE_UNBUILT;
        SetModel(mTrapClass->mUnbuiltModel);
        if (m_pHologram)
        {
            m_pHologram->TurnOn();
        }
        mUnbuiltTime = 0.0f;
        if (mBuildBox->IsRtti(EntityProp::rttiHashEntityProp))
        {
            EntityProp* pProp = static_cast<EntityProp*>(mBuildBox);
            static_cast<RedSceneObject*>(pProp)->Activate();
            pProp->AddStaticToCollisionManager();
        }
        mBuildingTrap = false;
        mTriggered = false;
    }
    return true;
}

// FUNCTION: SWBF2 0x004fed20
Entity* EntityTrapClass::Build(const EntityDesc& desc)
{
    EntityTrap* pTrap = new EntityTrap(this, desc);
    if (mBuildBoxOdf)
    {
        EntityDesc kBoxDesc;
        D3DXMatrixMultiply(&kBoxDesc.mMatrix, &mBuildOffset, &desc.mMatrix);
        PblVector3 vPos = kBoxDesc.mMatrix.GetTranslation();
        if (RedTerrain::_bEnable)
        {
            vPos.y = RedTerrain::GetInterpolatedHeight(vPos.x, vPos.z);
        }
        kBoxDesc.mMatrix.m[3][0] = vPos.x;
        kBoxDesc.mMatrix.m[3][1] = vPos.y;
        kBoxDesc.mMatrix.m[3][2] = vPos.z;
        kBoxDesc.mMatrix.m[3][3] = 1.0f;
        pTrap->mBuildBox = mBuildBoxOdf->Build(kBoxDesc);

        if (pTrap->m_pHologram)
        {
            PblMatrix mHologram = kBoxDesc.mMatrix;
            if (pTrap->mBuildBox->IsRtti(EntityProp::rttiHashEntityProp))
            {
                EntityProp* pProp = static_cast<EntityProp*>(pTrap->mBuildBox);
                if (pProp->GetModel())
                {
                    PblMatrix mBone;
                    if (pProp->GetModel()->GetRedModel(0)->GetBoneMatrix(0xD1E4CC7A, mBone))
                    {
                        D3DXMatrixMultiply(&mHologram, &mBone, &pProp->GetMatrix());
                    }
                    else
                    {
                        mHologram = pProp->GetMatrix();
                    }
                }
            }
            pTrap->m_pHologram->SetStoredMatrix(mHologram);
        }

        CollisionPrimitive kPrimitive;
        PblVector3 vHalfSize(0.75f, 0.75f, 0.75f);
        if (pTrap->mBuildBox->IsRtti(EntityProp::rttiHashEntityProp))
        {
            EntityProp* pProp = static_cast<EntityProp*>(pTrap->mBuildBox);
            if (pProp->GetModel())
            {
                PblBox kBox = pProp->GetModel()->_mModel->_TotalBoundingBox;
                vHalfSize.x = (kBox._vMax.x - kBox._vMin.x) * 0.5f;
                vHalfSize.y = (kBox._vMax.y - kBox._vMin.y) * 0.5f;
                vHalfSize.z = (kBox._vMax.z - kBox._vMin.z) * 0.5f;
            }
        }
        kPrimitive.SetBox(vHalfSize.x, vHalfSize.y, vHalfSize.z, -1.0f);

        PblVector3 vOffset(0.0f, vHalfSize.y * 0.5f, 0.0f);
        PblMatrix mBody = kBoxDesc.mMatrix;
        mBody.m[3][0] = kBoxDesc.mMatrix.m[3][0] + vOffset.x;
        mBody.m[3][1] = kBoxDesc.mMatrix.m[3][1] + vOffset.y;
        mBody.m[3][2] = kBoxDesc.mMatrix.m[3][2] + vOffset.z;
        mBody.m[3][3] = 1.0f;
        PblMatrix mInverse = pTrap->GetMatrix().GetInverseFast();
        PblMatrix mLocal;
        D3DXMatrixMultiply(&mLocal, &mBody, &mInverse);

        mBuildBoxCollisionModel.AddBody(new CollisionBodyPrimitive(kPrimitive, "main_body", mLocal, 0));
        pTrap->SetCollisionModel(&mBuildBoxCollisionModel);
        mBuildBoxCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(0));
        mBuildBoxCollisionModel.SetCollisionMask(COLL_RIGID, CollisionMask(0));

        pTrap->mBuildBoxCollisionSphere.SetRadius(
            (float)sqrt(vHalfSize.x * vHalfSize.x + vHalfSize.y * vHalfSize.y + vHalfSize.z * vHalfSize.z));
        pTrap->mBuildBoxCollisionSphere.SetCenter(PblVector3(kBoxDesc.mMatrix.m[3][0] + vOffset.x,
                                                             kBoxDesc.mMatrix.m[3][1] + vOffset.y,
                                                             kBoxDesc.mMatrix.m[3][2] + vOffset.z));
        pTrap->SetCollisionCenter(pTrap->mBuildBoxCollisionSphere.GetCenter());
        pTrap->SetCollisionRadius(pTrap->mBuildBoxCollisionSphere.GetRadius());
        pTrap->Set2DCollisionRadius(pTrap->mBuildBoxCollisionSphere.GetRadius());
        pTrap->RecalculateAABB();
    }

    pTrap->AddStaticToCollisionManager();
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        pTrap->BuildEffects();
    }
    static_cast<Thread*>(pTrap)->ActivateThread(0, PER_FRAME);
    pTrap->mCurHealth = pTrap->mMaxHealth;
    pTrap->HealthChangeCallback(false, 0.0f);
    if (mAnimBuildTrap)
    {
        pTrap->mAnimationTime = mAnimBuildTrap->GetNumFrames() / 30.0f;
    }
    return pTrap;
}
