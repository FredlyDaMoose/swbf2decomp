#include "VehicleSpawn.h"
#include "NetGame.h"

#include <stdio.h>
#include <stdlib.h>

#include "CommandPostManager.h"
#include "EntityFlyer.h"
#include "SpawnManager.h"
#include "Team.h"
#include "BonusManager.h"
#include "BoundaryManager.h"
#include "Character.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "EntityCarrier.h"
#include "EntityHover.h"
#include "EntityWalker.h"
#include "Explosion.h"
#include "GameModel.h"
#include "GameObject.h"
#include "ProceduralAnimation.h"
#include "StringDB.h"
#include "RedEngineFL/Graphics/RedModel.h"

#include <math.h>

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// GLOBAL: SWBF2 0x01f9a278
// SYNTHETIC: SWBF2 0x00406f10
// `dynamic initializer for 'VehicleSpawn::VehicleTracker::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769760
// `dynamic atexit destructor for 'VehicleSpawn::VehicleTracker::sMemoryPool''
MemoryPool VehicleSpawn::VehicleTracker::sMemoryPool("VehicleSpawn::VehicleTracker", 0x1c);

static unsigned int HandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

static bool IsAlive(const GameObject* pObject)
{
    return pObject->mIsAlive;
}

static int GetTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}

static GameObject* GetObject(const PblHandle<GameObject>& handle)
{
    PblHandle<GameObject>& h = const_cast<PblHandle<GameObject>&>(handle);
    if (h.mObject && HandleId(h.mObject) != h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

static const GameObject* GetPostObject(const ConstCommandPostHandle& post)
{
    const CommandPost* pPost = post;
    return GetObject(pPost->mObject);
}

// FUNCTION: SWBF2 0x0066ff80
VehicleSpawnClass::VehicleSpawnClass(unsigned int uHash, bool bIsPad)
    : EntityClass(uHash)
{
    mIsPad = bIsPad;
}

// FUNCTION: SWBF2 0x0066ffe0
VehicleSpawnClass::VehicleSpawnClass(VehicleSpawnClass* pParent, unsigned int uHash)
    : EntityClass(pParent, uHash)
{
    mIsPad = pParent->mIsPad;
}

// FUNCTION: SWBF2 0x00670010
VehicleSpawnClass::~VehicleSpawnClass()
{
}

// SYNTHETIC: SWBF2 0x0066ffb0
// VehicleSpawnClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00670020
void VehicleSpawnClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityClass::SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00670030
EntityClass* VehicleSpawnClass::Derive(unsigned int uHash)
{
    return new VehicleSpawnClass(this, uHash);
}

// FUNCTION: SWBF2 0x006700a0
Entity* VehicleSpawnClass::Build(const EntityDesc& desc)
{
    return new VehicleSpawn(this, desc);
}

// GLOBAL: SWBF2 0x0078bc40
static const unsigned int sSideLabel[Team::side_MAX + 1][3] = {
    { 0x8cdb433e, 0x8cdb433e, 0x8cdb433e },
    { 0x6a183212, 0xdd29eddc, 0x2a456b61 },
    { 0x7e58741b, 0x54d2fc8b, 0x07b77f06 },
    { 0x494a707f, 0x24f75004, 0x61c30279 },
    { 0x477791dc, 0xa2c909c6, 0x9e973303 },
    { 0xd118d857, 0xd92c6ca7, 0xcaa92a12 },
    { 0xfcc25b7b, 0xe2c47a79, 0x9836edac },
    { 0, 0, 0 },
    { 0, 0, 0 },
};

// FUNCTION: SWBF2 0x0066e820
VehicleSpawn::VehicleSpawn(VehicleSpawnClass* pClass, const EntityDesc& desc)
    : mMatrix(desc.mMatrix),
      mClass(pClass),
      mCommandPost(0),
      mNameId(desc.mNameId),
      mSpawnCount(1),
      mSpawnTime(10.0f),
      mExpireTimeEnemy(10.0f),
      mExpireTimeField(20.0f),
      mDecayTime(10.0f),
      mVehicleTeam(-1),
      mSpawnTeam(-1),
      mSpawnTrigger(true),
      mSpawnTimer(0.0f),
      mCycleTimer(0.0f)
{
    VehicleTracker::AddMemoryPoolCount(mSpawnCount);
    for (int i = 0; i < 8; ++i)
    {
        mSpawnClass[i] = 0;
        mFlyerClass[i] = 0;
        mUseCarrier[i] = false;
    }
    ActivateThread(0, PER_LOCAL_TURN);
    sVehicleSpawnList.AddLast(&m_ListNode, this);
}

// SYNTHETIC: SWBF2 0x00670155
// [thunk]:VehicleSpawn::`scalar deleting destructor' adjustor{4}
// SYNTHETIC: SWBF2 0x0066e9f0
// VehicleSpawn::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0066ea20
VehicleSpawn::~VehicleSpawn()
{
    sVehicleSpawnList.Remove(&m_ListNode);
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0066e810
const PblMatrix& VehicleSpawn::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0066ead0
void VehicleSpawn::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x4e99b371:
        sscanf(pszValue, "%f", &mSpawnTime);
        return;
    case 0x1c098b3c:
        sscanf(pszValue, "%f", &mDecayTime);
        return;
    case 0x447c6db0:
        mCommandPost = CommandPostManager::Instance()->FindPost(pszValue);
        if (mCommandPost)
        {
            mSpawnTeam = GetTeam(mCommandPost->GetGameObject());
        }
        return;
    case 0x88923ff7:
        VehicleTracker::AddMemoryPoolCount(-mSpawnCount);
        mSpawnCount = atoi(pszValue);
        VehicleTracker::AddMemoryPoolCount(mSpawnCount);
        return;
    case 0xa2fd7d0c:
        mSpawnTeam = atoi(pszValue);
        return;
    }

    for (int iTeam = 0; iTeam < 8; ++iTeam)
    {
        Team* pTeam = Team::GetPtr(iTeam);
        int iSide = pTeam->GetSide();
        if ((iTeam == 1 || iTeam == 2) && SpawnManager::Instance()->IsHistorical())
        {
            iSide = Team::side_MAX;
        }
        if (uProperty == sSideLabel[iSide][0] || uProperty == sSideLabel[iSide][iTeam <= 2 ? iTeam : 0])
        {
            EntityClass* pSpawnClass = EntityClass::Find(PblHash(pszValue));
            mSpawnClass[iTeam] = pSpawnClass;
            mFlyerClass[iTeam] = 0;
            if (pSpawnClass && mClass->mIsPad)
            {
                if (pSpawnClass->IsRtti(EntityFlyerClass::rttiHashEntityFlyerClass))
                {
                    mFlyerClass[iTeam] = static_cast<EntityFlyerClass*>(mSpawnClass[iTeam]);
                    mUseCarrier[iTeam] = false;
                }
                else if (pTeam->GetCarrierPtr())
                {
                    mFlyerClass[iTeam] = static_cast<EntityFlyerClass*>(pTeam->GetCarrierPtr());
                    mUseCarrier[iTeam] = true;
                }
            }
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0066f2f0
void VehicleSpawn::gCalcBeginLandPos(const EntityFlyerClass& flyerClass, PblMatrix* pMatrix)
{
    const float fTime = flyerClass.mLandingTime;
    const PblVector3 vOffset(0.0f, fTime * 0.5f * flyerClass.mLandingSpeed + flyerClass.mLandedHeight,
                             fTime * -0.5f * flyerClass.mMinFlyerSpeed);
    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &vOffset, pMatrix);
    pMatrix->m[3][0] = vPos.x;
    pMatrix->m[3][1] = vPos.y;
    pMatrix->m[3][2] = vPos.z;
    pMatrix->m[3][3] = 1.0f;
}

static bool IsCurrentlySelfPiloted(const Controllable* pControllable)
{
    return pControllable->mPilotType == PILOT_SELF ||
           (pControllable->mPilotType == PILOT_VEHICLESELF && pControllable->mPilot == pControllable);
}

static const Controllable* GetPilot(const Controllable* pControllable)
{
    return IsCurrentlySelfPiloted(pControllable) ? 0 : pControllable->mPilot;
}

// FUNCTION: SWBF2 0x0066ecf0
void VehicleSpawn::UpdateLive(float fDeltaTime, VehicleTracker& tracker)
{
    if (GetObject(mCarrier))
    {
        EntityCarrier* pCarrier = reinterpret_cast<EntityCarrier*>(GetObject(mCarrier));
        EntityFlyer* pFlyer = reinterpret_cast<EntityFlyer*>(pCarrier);
        if (pFlyer->GetState() == EntityFlyer::LANDED)
        {
            pCarrier->DetachCargo(0);
            pFlyer->TakeOff();
            return;
        }
        if (pFlyer->GetState() == EntityFlyer::FLYING)
        {
            delete GetObject(mCarrier);
            mCarrier = 0;
        }
    }

    GameObject* pVehicle = GetObject(tracker.mVehicle);
    if (!BoundaryManager::sInstance->IsOutOfBounds(pVehicle->GetMatrix().GetTranslation()))
    {
        const CommandPost* pPost = mCommandPost.mPost;
        if (!pPost || !GetObject(pPost->mObject) || GetTeam(GetObject(tracker.mVehicle)) != 0)
        {
            tracker.mEmptyTimer = 0.0f;
            return;
        }
    }

    for (PblList<Controllable>::Itor it(GetObject(tracker.mVehicle)->GetControllableList()); it; ++it)
    {
        const Controllable* pControllable = it;
        if (GetPilot(pControllable))
        {
            tracker.mEmptyTimer = 0.0f;
            return;
        }
    }

    float fExpireTime = mExpireTimeField;
    const CommandPost* pPost = mCommandPost.mPost;
    if (pPost && GetObject(pPost->mObject) && IsAlive(GetObject(pPost->mObject)) &&
        pPost->IsInsideControlRegion(GetObject(tracker.mVehicle)))
    {
        if (mSpawnClass[mVehicleTeam] != mSpawnClass[mSpawnTeam])
        {
            fExpireTime = mExpireTimeEnemy;
        }
        else
        {
            if (mVehicleTeam != mSpawnTeam && GetObject(tracker.mVehicle) &&
                !GetObject(tracker.mVehicle)->GetGameObjectClass()->AvailableForAnyTeam())
            {
                GetObject(tracker.mVehicle)->SetOwningTeam(mSpawnTeam);
            }
            return;
        }
    }

    mCycleTimer -= fDeltaTime;
    if (mCycleTimer >= 0.0f)
    {
        return;
    }
    mCycleTimer += 1.0f;

    const CollisionObject* pCollision = GetObject(tracker.mVehicle)->GetCollisionObject();
    const PblVector3& vCenter = pCollision->GetCollisionCenter();
    const float fRadius = pCollision->GetCollisionRadius();
    int iNearby = 0;
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        if (pCharacter->mPlayerId < 0)
        {
            continue;
        }
        Controllable* pControllable = pCharacter->GetControllable();
        if (!pControllable || !pControllable->CanEnterVehicle(GetObject(tracker.mVehicle)))
        {
            continue;
        }
        const CollisionObject* pOther = pControllable->GetGameObject()->GetCollisionObject();
        const PblVector3& vOther = pOther->GetCollisionCenter();
        const float dx = vOther.x - vCenter.x;
        const float dy = vOther.y - vCenter.y;
        const float dz = vOther.z - vCenter.z;
        const float fReach = pOther->GetCollisionRadius() + fRadius + 10.0f;
        if (fReach * fReach >= dy * dy + dx * dx + dz * dz)
        {
            ++iNearby;
        }
    }
    if (iNearby)
    {
        return;
    }

    tracker.mEmptyTimer += 1.0f;
    if (tracker.mEmptyTimer > fExpireTime)
    {
        GameObject* pDecaying = GetObject(tracker.mVehicle);
        const DamageDesc damage(DamageOwner(0, 0, 0, 0, 0, 1.0f), pDecaying->mMaxHealth / mDecayTime);
        static_cast<Damageable*>(GetObject(tracker.mVehicle))->ApplyDamage(damage, (unsigned int)-1);
    }
}

// FUNCTION: SWBF2 0x0066f370
void VehicleSpawn::UpdateSpawn(float fDeltaTime)
{
    const PblBox& box =
        static_cast<EntityGeometryClass*>(mSpawnClass[mSpawnTeam])->GetModel()->_mModel->_TotalBoundingBox;
    const PblVector3 vCenter((box._vMax.x + box._vMin.x) * 0.5f, (box._vMax.y + box._vMin.y) * 0.5f,
                             (box._vMax.z + box._vMin.z) * 0.5f);
    const PblVector3 vExtents((box._vMax.x - box._vMin.x) * 0.5f, (box._vMax.y - box._vMin.y) * 0.5f,
                              (box._vMax.z - box._vMin.z) * 0.5f);
    mSpawnRegion.mMatrix = mMatrix;
    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &vCenter, &mMatrix);
    mSpawnRegion.mMatrix.m[3][0] = vPos.x;
    mSpawnRegion.mMatrix.m[3][1] = vPos.y;
    mSpawnRegion.mMatrix.m[3][2] = vPos.z;
    mSpawnRegion.mMatrix.m[3][3] = 1.0f;

    mSpawnRegion.mSize.x = vExtents.x;
    mSpawnRegion.mSize.y = vExtents.y;
    mSpawnRegion.mSize.z = vExtents.z;
    mSpawnRegion.mSize.w = sqrtf(vExtents.x * vExtents.x + vExtents.y * vExtents.y + vExtents.z * vExtents.z);
    mSpawnRegion.mType = PblRegion::type_box;

    const PblMatrix mInverse = mMatrix.GetInverseFast();
    (void)mInverse;

    for (PblList<VehicleTracker>::Itor it(mTrackerList); it; ++it)
    {
        GameObject* pVehicle = it->mVehicle;
        if (pVehicle &&
            mSpawnRegion.DoesSphereIntersect(pVehicle->GetCollisionCenter(), pVehicle->GetCollisionRadius()))
        {
            return;
        }
    }

    if (!mSpawnTrigger)
    {
        mSpawnTrigger = true;
        mSpawnTimer = mSpawnTime;
        if (mFlyerClass[mSpawnTeam])
        {
            mSpawnTimer -= mFlyerClass[mSpawnTeam]->mLandingTime;
            if (mSpawnTimer < 0.0f)
            {
                mSpawnTimer = 0.0f;
            }
        }
    }
    mSpawnTimer -= fDeltaTime;
    if (mSpawnTimer > 0.0f)
    {
        return;
    }

    EntityDesc desc;
    desc.mMatrix = mMatrix;
    desc.mNameId = mNameId;
    int iIndex = 0;
    while (EntityEx::GetEntityMap().Find(desc.mNameId))
    {
        char acName[256];
        _snprintf(acName, 255, "%s%d", StringDB::Find(desc.mNameId), ++iIndex);
        acName[255] = 0;
        desc.mNameId = PblHash(acName);
        StringDB::Store(desc.mNameId, acName);
    }

    PblVector3& vTranslation = *(PblVector3*)&desc.mMatrix.m[3][0];
    const PblVector3 vStart(vTranslation.x + gVectorUnitY.x, vTranslation.y + gVectorUnitY.y,
                            vTranslation.z + gVectorUnitY.z);
    const PblVector3 vDir(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
    CollisionObject* pHit;
    const float fHit = CollisionManager::RayHit(vStart, vDir, 2.0f, pHit, 0, 0, 0, 0x90, false);
    if (fHit < 1.0f)
    {
        const float fDist = fHit * 2.0f;
        desc.mMatrix.m[3][3] = 1.0f;
        vTranslation.x = vDir.x * fDist + vStart.x;
        vTranslation.y = vDir.y * fDist + vStart.y;
        vTranslation.z = vDir.z * fDist + vStart.z;
    }

    if (mFlyerClass[mSpawnTeam])
    {
        if (mUseCarrier[mSpawnTeam] && EntityCarrier::sMemoryPool.IsFull())
        {
            mSpawnTimer = 1.0f;
            return;
        }
        gCalcBeginLandPos(*mFlyerClass[mSpawnTeam], &desc.mMatrix);
    }

    GameObject* apInRegion[1];
    if (CollisionManager::GetObjectsInRegion(mSpawnRegion, apInRegion, 1, 0, 8))
    {
        mSpawnTimer = 1.0f;
        return;
    }
    if (NetGame::IsEnabled() && !NetGame::IsHost())
    {
        mSpawnTimer = 1.0f;
        return;
    }

    Entity* pEntity = mSpawnClass[mSpawnTeam]->Build(desc);
    if (!pEntity)
    {
        mSpawnTimer = 1.0f;
        return;
    }
    GameObject* pObject = pEntity->GetGameObject();
    if (!NetGame::IsEnabled())
    {
        PAAnimationGroup::GeometryCreated(desc.mNameId, pObject, pObject->GetMatrix());
    }

    if (mFlyerClass[mSpawnTeam])
    {
        if (mUseCarrier[mSpawnTeam])
        {
            const PblBox& rCargo = pObject->GetModel()->_mModel->_TotalBoundingBox;
            const float fHeight = rCargo._vMax.y - rCargo._vMin.y;
            desc.mMatrix.m[3][3] = 1.0f;
            vTranslation.x += 0.0f;
            vTranslation.y += fHeight;
            vTranslation.z += 0.0f;
            Entity* pCarrier = mFlyerClass[mSpawnTeam]->Build(desc);
            mCarrier = pCarrier->GetGameObject();
            mCarrier->SetTeam(mSpawnTeam);
            mCarrier->SetPerceivedTeam(mSpawnTeam);
            mCarrier->Init();

            reinterpret_cast<EntityCarrier*>(pCarrier)->AttachCargo(0, pObject);
            reinterpret_cast<EntityFlyer*>(pCarrier)->InitAsFlying();
            reinterpret_cast<EntityFlyer*>(pCarrier)->Land();
        }
        else
        {
            reinterpret_cast<EntityFlyer*>(pObject)->InitAsFlying();
            reinterpret_cast<EntityFlyer*>(pObject)->Land();
        }
    }

    bool bSetTeam = true;
    if (pObject->GetGameObjectClass() && pObject->GetGameObjectClass()->IsNeutralWhenEmpty())
    {
        bSetTeam = false;
        for (PblList<Controllable>::Itor it(pObject->GetControllableList()); it; ++it)
        {
            const Controllable* pControllable = it;
            if (GetPilot(pControllable) || IsCurrentlySelfPiloted(pControllable))
            {
                bSetTeam = true;
                break;
            }
        }
    }
    if (bSetTeam)
    {
        pObject->SetTeam(mSpawnTeam);
        pObject->SetPerceivedTeam(mSpawnTeam);
    }
    if (!pObject->GetGameObjectClass()->AvailableForAnyTeam())
    {
        pObject->SetOwningTeam(mSpawnTeam);
    }
    pObject->Init();

    if (pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) || pObject->IsRtti(EntityWalker::rttiHashEntityWalker) ||
        pObject->IsRtti(EntityHover::rttiHashEntityHover))
    {
        if (mSpawnTeam >= 1 && mSpawnTeam <= 2)
        {
            for (int iTeam = 1; iTeam <= 2; ++iTeam)
            {
                if (iTeam == mSpawnTeam)
                {
                    continue;
                }
                if (!BonusManager::Instance()->IsTeamBonusActive(iTeam, 0xd265e905))
                {
                    continue;
                }
                const float fHealth = pObject->mCurHealth - pObject->mMaxHealth * 0.75f;
                if (ExplosionClass* pExplosion = pObject->GetExplosionClass())
                {
                    pExplosion->CreateExplosion(pObject->GetMatrix().GetTranslation(),
                                                DamageOwner(0, pObject, pObject->GetTeam(), 0, 0, 1.0f), false,
                                                COLL_UNKNOWN);
                }
                pObject->SetCurHealth(fHealth, true, 0);
                break;
            }
        }
    }

    VehicleTracker* pTracker = new VehicleTracker;
    mTrackerList.AddLast(pTracker, pTracker);
    pTracker->mVehicle = pObject;
    mVehicleTeam = mSpawnTeam;
    mSpawnTrigger = false;
}

// FUNCTION: SWBF2 0x0066fd00
bool VehicleSpawn::Update(float fDeltaTime)
{
    if ((netInShell ? netEnabled : netEnabledNext) && NetGame::JustEndedPreGame())
    {
        mSpawnTrigger = true;
        mSpawnTimer = 1.0f;
    }

    if (mCommandPost)
    {
        int iTeam = IsAlive(GetPostObject(mCommandPost)) ? GetTeam(GetPostObject(mCommandPost)) : 0;
        if (mSpawnTeam != iTeam)
        {
            mSpawnTeam = GetTeam(GetPostObject(mCommandPost));
            mSpawnTimer = mSpawnTime;
        }
    }

    for (PblList<VehicleTracker>::Itor it(mTrackerList); it; ++it)
    {
        VehicleTracker* pTracker = it;
        if (GetObject(pTracker->mVehicle) && IsAlive(GetObject(pTracker->mVehicle)))
        {
            UpdateLive(fDeltaTime, *pTracker);
        }
        else
        {
            --it;
            mTrackerList.Remove(pTracker);
            delete pTracker;
        }
    }

    if (mTrackerList.GetCount() < mSpawnCount && mSpawnClass[mSpawnTeam])
    {
        UpdateSpawn(fDeltaTime);
    }

    return true;
}

// FUNCTION: SWBF2 0x0066eac0
void VehicleSpawn::Reset()
{
    sVehicleSpawnList.RemoveAll();
}

// FUNCTION: SWBF2 0x00670110
template void PblList<VehicleSpawn>::RemoveAll();

// FUNCTION: SWBF2 0x0066e7e0
const CommandPost* ConstCommandPostHandle::operator->() const
{
    return *this;
}

// GLOBAL: SWBF2 0x007ebebc
PblList<VehicleSpawn> VehicleSpawn::sVehicleSpawnList;
