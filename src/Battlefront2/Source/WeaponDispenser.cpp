#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WeaponDispenser.h"
#include "Entity.h"
#include "NetGame.h"
#include "Trigger.h"
#include "Character.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameObject.h"
#include "SoldierAnimatorClass.h"
#include "Aimer.h"
#include "Damageable.h"
#include "EntitySoldier.h"
#include "EntityItem.h"
#include "EntityMine.h"
#include "EntityPortableTurret.h"
#include "EventManager_Character.h"
#include "UnitController.h"
#include "UnitStationaryController.h"
#include "CollisionManager.h"

namespace gf { bool isInitialized(); bool checkError(); }
void ProcessGalaxyData();

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

class AmmoCounter
{
public:
    bool CanExpendRounds(float fRounds) const;
    void ExpendRounds(float fRounds);
};

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

namespace GameLoop { extern float sClientDeltaTime; }

// GLOBAL: SWBF2 0x01f9a8c0
// SYNTHETIC: SWBF2 0x00407150
// `dynamic initializer for 'WeaponDispenserClass::rttiHashWeaponDispenserClass''
PblHash WeaponDispenserClass::rttiHashWeaponDispenserClass("WeaponDispenserClass");
char WeaponDispenserClass::rttiNameWeaponDispenserClass[] = "WeaponDispenserClass";

// FUNCTION: SWBF2 0x00682430
bool WeaponDispenserClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponDispenserClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006836d0
unsigned int WeaponDispenserClass::GetDerivedRtti()
{
    return rttiHashWeaponDispenserClass;
}

// FUNCTION: SWBF2 0x006836e0
const char* WeaponDispenserClass::GetDerivedRttiName()
{
    return rttiNameWeaponDispenserClass;
}

// FUNCTION: SWBF2 0x00684670
WeaponDispenserClass::WeaponDispenserClass(WeaponDispenserClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
    mDispenseClass = pBase->mDispenseClass;
    mVelocity = pBase->mVelocity;
    mShotDelay = pBase->mShotDelay;
    mMaxPressedTime = pBase->mMaxPressedTime;
    mMinStrength = pBase->mMinStrength;
    mMaxStrength = pBase->mMaxStrength;
    mInitialSalvoDelay[0] = pBase->mInitialSalvoDelay[0];
    mInitialSalvoDelay[1] = pBase->mInitialSalvoDelay[1];
    mMaxItems = pBase->mMaxItems;
    mHideOnFire = pBase->mHideOnFire;
    mHideOnEmpty = pBase->mHideOnEmpty;
    mFirePointExtraOffset = pBase->mFirePointExtraOffset;
    m_avFireOffset[0] = pBase->m_avFireOffset[0];
    m_avFireOffset[1] = pBase->m_avFireOffset[1];
    m_avFireOffset[2] = pBase->m_avFireOffset[2];
}

// SYNTHETIC: SWBF2 0x00684640
// WeaponDispenserClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00684760
// WeaponDispenserClass::~WeaponDispenserClass

// FUNCTION: SWBF2 0x00684770
void WeaponDispenserClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x399BF05D:
    case 0xF8899C5E:
        mDispenseClass = EntityClass::Find(PblHash(pszValue));
        break;
    case 0x32741C32:
        sscanf(pszValue, "%f", &mVelocity);
        break;
    case 0xD9356908:
        sscanf(pszValue, "%f", &mShotDelay);
        if (mShotDelay < 0.01f)
        {
            mShotDelay = 0.01f;
        }
        break;
    case 0xC8388D9D:
        sscanf(pszValue, "%f", &mInitialSalvoDelay[0]);
        break;
    case 0x36F0BFCD:
        sscanf(pszValue, "%f", &mInitialSalvoDelay[1]);
        break;
    case 0x2819FFA0:
        sscanf(pszValue, "%f", &mMaxPressedTime);
        break;
    case 0x4D832102:
        sscanf(pszValue, "%f", &mMinStrength);
        break;
    case 0x40BD9B1C:
        sscanf(pszValue, "%f", &mMaxStrength);
        break;
    case 0x54C3C993:
        sscanf(pszValue, "%d", &mMaxItems);
        break;
    case 0x5578D44C:
        mHideOnFire = atoi(pszValue) != 0;
        break;
    case 0xDD65D239:
        mHideOnEmpty = atoi(pszValue) != 0;
        break;
    case 0x389C8E46:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mFirePointExtraOffset.x = x;
        mFirePointExtraOffset.y = y;
        mFirePointExtraOffset.z = z;
        break;
    }
    default:
        WeaponClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00684960
void WeaponDispenserClass::PostReadInit()
{
    mTrackItemPool = 0;
    memset(mTrackItemArray, 0, sizeof(mTrackItemArray));
    memset(mTrackItemCount, 0, sizeof(mTrackItemCount));
    if (mMaxItems > 0)
    {
        mTrackItemPool = new PblHandle<GameObject>[600 * mMaxItems];
        PblHandle<GameObject>* pItem = mTrackItemPool;
        for (int i = 0; i < 600; ++i)
        {
            mTrackItemArray[i] = pItem;
            pItem += mMaxItems;
        }
    }
}

// FUNCTION: SWBF2 0x00684a50
WeaponClass* WeaponDispenserClass::Derive(unsigned int uHash)
{
    return new WeaponDispenserClass(this, uHash);
}

// FUNCTION: SWBF2 0x00684ac0
Weapon* WeaponDispenserClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    return new WeaponDispenser(this, desc);
}

// FUNCTION: SWBF2 0x00684b30
bool WeaponDispenserClass::PostLoadInit()
{
    if (!WeaponClass::PostLoadInit())
    {
        return false;
    }

    PblMatrix mBone;
    if (mIsOffhand)
    {
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY,
                0xB7EC1D31, mInitialSalvoDelay[0], mBone))
        {
            m_avFireOffset[0] = mBone.GetTranslation();
        }
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY2,
                0xB7EC1D31, mInitialSalvoDelay[1], mBone))
        {
            m_avFireOffset[1] = mBone.GetTranslation();
        }
    }
    else
    {
        PblVector3 vPos;
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY,
                0x2B960099, mInitialSalvoDelay[0], mBone))
        {
            D3DXVec3TransformCoord(&vPos, &mFirePointOffset, &mBone);
            m_avFireOffset[0] = vPos;
        }
        if (SoldierAnimatorClass::Instance()->GetAnimationBoneMatrix(
                SoldierAnimationBank::HUMAN_RIFLE, SoldierAnimType::STAND, SoldierAnimType::SHOOT_SECONDARY2,
                0x2B960099, mInitialSalvoDelay[1], mBone))
        {
            D3DXVec3TransformCoord(&vPos, &mFirePointOffset, &mBone);
            m_avFireOffset[1] = vPos;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00684570
WeaponDispenserClass::WeaponDispenserClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mLockOnAngle = 0.0f;
    mReloadTime = 0.6f;

    mDispenseClass = 0;
    mVelocity = 10.0f;
    mShotDelay = 0.125f;
    mMaxPressedTime = 0.0f;
    mMinStrength = 0.3f;
    mMaxStrength = 1.0f;
    mInitialSalvoDelay[0] = 0.5f;
    mInitialSalvoDelay[1] = 0.584f;
    mMaxItems = 0;

    mHideOnFire = true;
    mHideOnEmpty = true;

    mFirePointExtraOffset.x = 0.0f;
    mFirePointExtraOffset.y = 0.0f;
    mFirePointExtraOffset.z = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        m_avFireOffset[i].x = 0.0f;
        m_avFireOffset[i].y = 0.0f;
        m_avFireOffset[i].z = 0.0f;
    }
}

// GLOBAL: SWBF2 0x01f9a8bc
// SYNTHETIC: SWBF2 0x00407140
// `dynamic initializer for 'WeaponDispenser::rttiHashWeaponDispenser''
PblHash WeaponDispenser::rttiHashWeaponDispenser("WeaponDispenser");
char WeaponDispenser::rttiNameWeaponDispenser[] = "WeaponDispenser";

// FUNCTION: SWBF2 0x00682400
bool WeaponDispenser::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponDispenser || reinterpret_cast<const Weapon*>(this)->Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006836b0
unsigned int WeaponDispenser::GetDerivedRtti() const
{
    return rttiHashWeaponDispenser;
}

// FUNCTION: SWBF2 0x006836c0
const char* WeaponDispenser::GetDerivedRttiName() const
{
    return rttiNameWeaponDispenser;
}

// FUNCTION: SWBF2 0x006836f0
WeaponDispenser::WeaponDispenser(const WeaponDispenserClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mNetFiredFlag = false;
    mFired = false;
    mItemArray = 0;
    mItemCount = 0;
}

// SYNTHETIC: SWBF2 0x00683730
// WeaponDispenser::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00683760
WeaponDispenser::~WeaponDispenser()
{
}

// FUNCTION: SWBF2 0x00683770
bool WeaponDispenser::Update(float fDeltaTime)
{
    if (Character* pCharacter = mOwner->mCharacter)
    {
        const int iIndex = pCharacter - Character::sCharacters;
        WeaponDispenserClass* pClass = (WeaponDispenserClass*)mStart;
        while (pClass)
        {
            int iCount = 0;
            for (int i = 0; i < pClass->mTrackItemCount[iIndex]; ++i)
            {
                PblHandle<GameObject>& hItem = pClass->mTrackItemArray[iIndex][i];
                if (hItem && hItem->IsAlive())
                {
                    if (iCount != i)
                    {
                        pClass->mTrackItemArray[iIndex][iCount] = pClass->mTrackItemArray[iIndex][i];
                    }
                    ++iCount;
                }
            }
            pClass->mTrackItemCount[iIndex] = iCount;

            WeaponDispenserClass* pNext = (WeaponDispenserClass*)pClass->mNextCharge;
            if (pNext == pClass || pNext == (WeaponDispenserClass*)mStart)
            {
                break;
            }
            pClass = pNext;
        }
        WeaponDispenserClass* pMyClass = (WeaponDispenserClass*)mClass;
        mItemArray = &pMyClass->mTrackItemArray[iIndex];
        mItemCount = &pMyClass->mTrackItemCount[iIndex];
    }
    return Weapon::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x006838c0
bool WeaponDispenser::IsBusy() const
{
    if (mState != FIRE && mState != FIRE2)
    {
        return false;
    }
    return !mFired || mStateLimit > mStateTimer + GameLoop::sClientDeltaTime;
}

// FUNCTION: SWBF2 0x00683900
void WeaponDispenser::SignalFire()
{
    const bool bFiredFlag = mFiredFlag;
    Weapon::SignalFire();
    mFiredFlag = bFiredFlag;
}

// FUNCTION: SWBF2 0x00683930
void WeaponDispenser::EnterFire()
{
    const WeaponDispenserClass* pClass = (const WeaponDispenserClass*)mClass;

    if (pClass->mHideOnFire)
    {
        mHideWeapon = 1;
    }
    mFired = false;

    if (pClass->mMaxPressedTime > 0.0f)
    {
        mStrength = mStateTimer / pClass->mMaxPressedTime;
    }
    else
    {
        mStrength = 1.0f;
    }

    mFiredFlag = 1;
    mStateTimer = -1.0f;
    mNetFiredFlag = true;

    if (pClass->mTriggerSingle)
    {
        mTrigger->ClearUntilPressed();
    }
}

// FUNCTION: SWBF2 0x006839b0
bool WeaponDispenser::UpdateFire(float fDeltaTime)
{
    if (mStateTimer < 0.0f)
    {
        mStateTimer = 0.0f;
    }
    else
    {
        mStateTimer += fDeltaTime;
    }

    if (!mSelectedFlag)
    {
        mFired = true;
    }

    if (!mFired)
    {
        const WeaponDispenserClass* pClass = (const WeaponDispenserClass*)mClass;
        if (mStateTimer >= pClass->mInitialSalvoDelay[m_iSoldierState == 2])
        {
            if (m_pAmmoCounter->CanExpendRounds(pClass->mRoundsPerShot) &&
                !(m_pEnergyBar->IsOverheating() && pClass->mHeatPerShot > 0.0f) && Fire())
            {
                m_pAmmoCounter->ExpendRounds(mClass->mRoundsPerShot);
                m_pEnergyBar->SpendEnergy(mClass->mHeatPerShot);
            }
            mStateLimit = ((const WeaponDispenserClass*)mClass)->mShotDelay;
            mNetFiredFlag = false;
            mFired = true;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00683aa0
void WeaponDispenser::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00683ab0
void WeaponDispenser::EnterCharge()
{
    mStateTimer = 0.0f;
    mStateLimit = ((const WeaponDispenserClass*)mClass)->mMaxPressedTime;
    Weapon::EnterCharge();
}

// FUNCTION: SWBF2 0x00683ad0
bool WeaponDispenser::UpdateCharge(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
    }
    Weapon::UpdateCharge(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00683b20
void WeaponDispenser::ExitCharge()
{
    Weapon::ExitCharge();
}

// FUNCTION: SWBF2 0x00683b30
void WeaponDispenser::ExitReload()
{
    mHideWeapon = 0;
    Weapon::ExitReload();
}

// FUNCTION: SWBF2 0x00683b40
void WeaponDispenser::EnterEmpty()
{
    const WeaponDispenserClass* pClass = (const WeaponDispenserClass*)mClass;
    if (pClass->mHideOnEmpty)
    {
        mHideWeapon = 1;
    }
    Weapon::EnterEmpty();
    mStateLimit = ((const WeaponDispenserClass*)mClass)->mShotDelay;
}

// FUNCTION: SWBF2 0x00683b70
void WeaponDispenser::ExitEmpty()
{
    mHideWeapon = 0;
}

// FUNCTION: SWBF2 0x00683b80
bool WeaponDispenser::CheckCharge() const
{
    if (!mTrigger || !(((const WeaponDispenserClass*)mClass)->mMaxPressedTime > 0.0f))
    {
        return false;
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00683ba0
bool WeaponDispenser::CheckFire() const
{
    if (NetGame::IsPreGame())
    {
        return false;
    }
    const WeaponDispenserClass* pClass = (const WeaponDispenserClass*)mClass;
    if (!pClass->mDispenseClass || !mTrigger || reinterpret_cast<const Weapon*>(this)->CheckOverheat())
    {
        return false;
    }
    if (((const WeaponDispenserClass*)mClass)->mMaxPressedTime > 0.0f)
    {
        return mTrigger->IsRelease();
    }
    return mTrigger->IsDown();
}

// FUNCTION: SWBF2 0x00683c00
bool WeaponDispenser::FireDone() const
{
    return mFired && mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x00683c20
Entity* WeaponDispenser::Fire()
{
    if (!mOwner->mCharacter)
    {
        return 0;
    }
    const WeaponDispenserClass* pClass = (const WeaponDispenserClass*)mClass;
    if (!pClass->mDispenseClass)
    {
        return 0;
    }

    if (!NetGame::IsEnabled())
    {
        PlayFireSound(false);
    }
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }

    if (pClass->mMaxItems > 0 && *mItemCount >= pClass->mMaxItems)
    {
        GameObject* pOldest = (*mItemArray)[0];
        if (pOldest->IsRtti(EntityMine::rttiHashEntityMine))
        {
            reinterpret_cast<EntityMine*>(pOldest)->mLifeTimer = 0.0f;
        }
        else
        {
            pOldest->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), false);
        }

        --*mItemCount;
        for (int i = 0; i < *mItemCount; ++i)
        {
            (*mItemArray)[i] = (*mItemArray)[i + 1];
        }
    }

    EntityDesc desc;
    PblVector3 vPos;
    if (mOwner)
    {
        vPos = pClass->m_avFireOffset[m_iSoldierState == 2];
        if (mOwner->GetGameObject()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            float fScale =
                static_cast<EntitySoldier*>(mOwner->GetGameObject())->mClass->mSkeletonRootScale;
            if (fScale != 1.0f)
            {
                vPos.x *= fScale;
                vPos.y *= fScale;
                vPos.z *= fScale;
            }
        }
        PblVector3 vWorld;
        D3DXVec3TransformCoord(&vWorld, &vPos, &mOwner->GetMatrix());
        vPos = vWorld;
    }
    else
    {
        vPos = mAimer->mFirePos;
    }

    PblVector3 vExtra;
    D3DXVec3TransformNormal(&vExtra, &pClass->mFirePointExtraOffset, &mOwner->GetMatrix());
    vPos.x = vExtra.x + vPos.x;
    vPos.y = vExtra.y + vPos.y;
    vPos.z = vExtra.z + vPos.z;

    if (const GameObject* pMe = mOwner->GetGameObject())
    {
        PblVector3 vCenter = pMe->GetCollisionCenter();
        PblVector3 vEnd = vPos;
        PblVector3 vDelta(vPos.x - vCenter.x, vPos.y - vCenter.y, vPos.z - vCenter.z);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vDelta);
        const GameObject* pIgnore = pMe;
        vEnd.x = vDir.x + vEnd.x;
        vEnd.y = vDir.y + vEnd.y;
        vEnd.z = vDir.z + vEnd.z;
        CollisionObject* pHit;
        PblVector3 vNormal;
        if (CollisionManager::RayHit(vCenter, vEnd, pHit, &vNormal, &pIgnore, 1, 0x98, false) < 1.0f)
        {
            vPos = vCenter;
        }
    }

    desc.mMatrix = DirectionMatrix(mAimer->mDirection, mOwner->GetMatrix().GetYAxis(), vPos);
    Entity* pEntity = pClass->mDispenseClass->Build(desc);
    if (!pEntity)
    {
        if (pClass->mMaxItems <= 0)
        {
            return pEntity;
        }

        GameObject* pMe = mOwner->GetGameObject();
        GameObject* pOldest = 0;
        PblHandle<GameObject>* pHandle = pClass->mTrackItemPool;
        for (int i = 0; i < 600 * pClass->mMaxItems; ++i, ++pHandle)
        {
            GameObject* pItem = *pHandle;
            if (pItem && pItem->IsAlive())
            {
                pItem = *pHandle;
                if (pItem->GetAffiliationToMe(pMe) >= 0)
                {
                    if (!pOldest || pOldest->mCreatedTime > ((GameObject*)*pHandle)->mCreatedTime)
                    {
                        pOldest = *pHandle;
                    }
                }
            }
        }
        if (!pOldest)
        {
            return pEntity;
        }

        pOldest->Kill();
        delete pOldest;
        pEntity = pClass->mDispenseClass->Build(desc);
        if (!pEntity)
        {
            return pEntity;
        }
    }

    GameObject* pObject = pEntity->GetGameObject();
    if (!pObject)
    {
        return pEntity;
    }

    float fStrength = (pClass->mMaxStrength - pClass->mMinStrength) * mStrength + pClass->mMinStrength;
    float fVelocity = pClass->mVelocity;
    const PblVector3& vOwnerVel = mOwner->GetGameObject()->GetVelocity();
    float fSpeed = fVelocity * fStrength;
    PblVector3 vVel(mAimer->mDirection.x * fSpeed + vOwnerVel.x, mAimer->mDirection.y * fSpeed + vOwnerVel.y,
                    mAimer->mDirection.z * fSpeed + vOwnerVel.z);
    pObject->SetVelocity(vVel);

    int iTeam = mOwner->mCharacter->mTeamNumber;
    pObject->SetTeam(iTeam);
    pObject->SetPerceivedTeam(iTeam);
    if (pObject->GetEntity()->IsRtti(EntityPortableTurret::rttiHashEntityPortableTurret))
    {
        reinterpret_cast<EntityPortableTurret*>(pObject)->mOwnerChar = mOwner->mCharacter;
    }
    pObject->Init();

    if (pObject->IsRtti(EntityItem::rttiHashEntityItem))
    {
        static_cast<EntityItem*>(pObject)->mOwner = mOwner->mCharacter;
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            EventManager::CharacterEvent::mDispensePowerup.Trigger(*mOwner->mCharacter, *pObject);
        }
    }
    else
    {
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            EventManager::CharacterEvent::mDispenseControllable.Trigger(*mOwner->mCharacter, *pObject);
        }

        Controllable* pControllable = pObject->GetControllable();
        if (pControllable &&
            (pControllable->mPilotType == PILOT_SELF || pControllable->mPilotType == PILOT_VEHICLESELF) &&
            !pControllable->GetPilot())
        {
            Controller* pController = pControllable->mCtrl;
            if (pController && pController->IsRtti(UnitController::rttiHashUnitController) &&
                !pController->IsRtti(UnitStationaryController::rttiHashUnitStationaryController))
            {
                const GameObject* pTarget = mTarget;
                GameObject* pOwnerObject = mOwner->GetGameObject();
                if (!pTarget)
                {
                    pTarget = pOwnerObject;
                }
                static_cast<UnitController*>(pController)
                    ->ForceCommand(DroidRepairInfo(pTarget, pOwnerObject));
            }
        }
    }

    if (pClass->mMaxItems > 0)
    {
        (*mItemArray)[(*mItemCount)++] = pObject;
    }
    return pEntity;
}

// FUNCTION: SWBF2 0x00684510
int* WeaponDispenser::GetItemCount() const
{
    return mItemCount;
}

// FUNCTION: SWBF2 0x00684520
const GameObject* WeaponDispenser::GetItem(int iIndex) const
{
    if (iIndex < 0 || iIndex >= *mItemCount)
    {
        return 0;
    }
    PblHandle<GameObject>& h = (*mItemArray)[iIndex];
    if (h.mObject && h.mObject->mHandleId != h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

// FUNCTION: SWBF2 0x005d5a40
bool PumpGalaxyMatchmaking()
{
    if (!gf::isInitialized())
    {
        return false;
    }
    ProcessGalaxyData();
    return !gf::checkError();
}
