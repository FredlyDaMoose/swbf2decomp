#include "PowerupStation.h"

#include <math.h>
#include <stdio.h>

#include "CameraManager.h"
#include "Character.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "GameModel.h"
#include "NetGame.h"
#include "PowerupUtility.h"
#include "ToolTips.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Util/RedRegion.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x00625cf0
PowerupStationClass::PowerupStationClass(unsigned int uHash)
    : EntityBuildingClass(uHash)
{
    mPowerupDelay = 0.5f;
    for (int i = 0; i < 8; i++)
    {
        mAddHealth[i] = 0.0f;
        mAddAmmo[i] = 0.0f;
        mAddEnergy[i] = 0.0f;
    }
    mBuffOffenseTimer = 0.0f;
    mBuffOffenseMult = 0.0f;
    mBuffDefenseTimer = 0.0f;
    mBuffDefenseMult = 0.0f;
    mBuffHealthTimer = 0.0f;
    mBuffHealthRate = 0.0f;
    mDebuffDamageTimer = 0.0f;
    mDebuffDamageRate = 0.0f;
    mIdleRotateSpeed = 0.0f;
    mIdleWaitTime = 0.0f;
    mIdleWobbleHash = 0;
    mBaseWobbleMatrix = _IdentityMatrix;
    mIdleWobbleFactor = 0.0f;
    mIdleWobbleLeftFootHash = 0;
    mIdleWobbleRightFootHash = 0;
    for (int i = 0; i < 4; i++)
    {
        mActiveRotateHash[i] = 0;
    }
    mActiveSpinHash = 0;
}

// GLOBAL: SWBF2 0x01efa228
// SYNTHETIC: SWBF2 0x00405fb0
// `dynamic initializer for 'PowerupStationClass::rttiHashPowerupStationClass''
PblHash PowerupStationClass::rttiHashPowerupStationClass("PowerupStationClass");
char PowerupStationClass::rttiNamePowerupStationClass[] = "PowerupStationClass";

// FUNCTION: SWBF2 0x00625e80
PowerupStationClass::PowerupStationClass(PowerupStationClass* pBase, unsigned int uHash)
    : EntityBuildingClass(pBase, uHash)
{
    mPowerupDelay = pBase->mPowerupDelay;
    for (int i = 0; i < 8; i++)
    {
        mAddHealth[i] = pBase->mAddHealth[i];
        mAddAmmo[i] = pBase->mAddAmmo[i];
        mAddEnergy[i] = pBase->mAddEnergy[i];
    }
    mBuffOffenseTimer = pBase->mBuffOffenseTimer;
    mBuffOffenseMult = pBase->mBuffOffenseMult;
    mBuffDefenseTimer = pBase->mBuffDefenseTimer;
    mBuffDefenseMult = pBase->mBuffDefenseMult;
    mBuffHealthTimer = pBase->mBuffHealthTimer;
    mBuffHealthRate = pBase->mBuffHealthRate;
    mDebuffDamageTimer = pBase->mDebuffDamageTimer;
    mDebuffDamageRate = pBase->mDebuffDamageRate;
    mPowerupSound = pBase->mPowerupSound;
    mAmbientSound = pBase->mAmbientSound;
    mIdleRotateSpeed = pBase->mIdleRotateSpeed;
    mIdleWaitTime = pBase->mIdleWaitTime;
    mIdleWobbleHash = pBase->mIdleWobbleHash;
    mBaseWobbleMatrix = pBase->mBaseWobbleMatrix;
    mIdleWobbleFactor = pBase->mIdleWobbleFactor;
    mIdleWobbleLeftFootHash = pBase->mIdleWobbleLeftFootHash;
    mIdleWobbleRightFootHash = pBase->mIdleWobbleRightFootHash;
    for (int i = 0; i < 4; i++)
    {
        mActiveRotateHash[i] = pBase->mActiveRotateHash[i];
    }
    mActiveSpinHash = pBase->mActiveSpinHash;
}

// FUNCTION: SWBF2 0x00626080
PowerupStationClass::~PowerupStationClass()
{
}

// SYNTHETIC: SWBF2 0x00625e50
// PowerupStationClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006245a0
bool PowerupStationClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashPowerupStationClass || EntityBuildingClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006245e0
unsigned int PowerupStationClass::GetDerivedRtti()
{
    return rttiHashPowerupStationClass;
}

// FUNCTION: SWBF2 0x006245f0
const char* PowerupStationClass::GetDerivedRttiName()
{
    return rttiNamePowerupStationClass;
}

// FUNCTION: SWBF2 0x00626090
void PowerupStationClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float f;
    switch (uProperty)
    {
    case 0x299acaba:
        sscanf(pszValue, "%f", &mPowerupDelay);
        break;
    case 0xf33b41df:
        sscanf(pszValue, "%f", &f);
        mAddHealth[HEALTH_TYPE_PERSON] = f;
        mAddHealth[HEALTH_TYPE_DROID] = f;
        break;
    case 0xf51ad206:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_PERSON]);
        break;
    case 0x60d746d7:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x9622c73b:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_DROID]);
        break;
    case 0x14e381c9:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xa6c59e9f:
        sscanf(pszValue, "%f", &mAddHealth[HEALTH_TYPE_BUILDING]);
        break;
    case 0xc3da529f:
        sscanf(pszValue, "%f", &f);
        mAddAmmo[HEALTH_TYPE_PERSON] = f;
        mAddAmmo[HEALTH_TYPE_DROID] = f;
        break;
    case 0x738f412e:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_PERSON]);
        break;
    case 0xbfe8e397:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x0fda4dbb:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_DROID]);
        break;
    case 0xe728d955:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xa25f865f:
        sscanf(pszValue, "%f", &mAddAmmo[HEALTH_TYPE_BUILDING]);
        break;
    case 0x0adc5145:
        sscanf(pszValue, "%f", &f);
        mAddEnergy[HEALTH_TYPE_PERSON] = f;
        mAddEnergy[HEALTH_TYPE_DROID] = f;
        break;
    case 0x84622134:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_PERSON]);
        break;
    case 0xfb0507ad:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_ANIMAL]);
        break;
    case 0x3fe43479:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_DROID]);
        break;
    case 0x56238a9b:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_VEHICLE]);
        break;
    case 0xda2c3905:
        sscanf(pszValue, "%f", &mAddEnergy[HEALTH_TYPE_BUILDING]);
        break;
    case 0x70b204af:
        sscanf(pszValue, "%f", &mBuffOffenseTimer);
        break;
    case 0xce532e64:
        sscanf(pszValue, "%f", &mBuffOffenseMult);
        break;
    case 0x7705fcc1:
        sscanf(pszValue, "%f", &mBuffDefenseTimer);
        break;
    case 0xb25fb62e:
        sscanf(pszValue, "%f", &mBuffDefenseMult);
        break;
    case 0x460b143b:
        sscanf(pszValue, "%f", &mBuffHealthTimer);
        break;
    case 0xb6639618:
        sscanf(pszValue, "%f", &mBuffHealthRate);
        break;
    case 0x805b51e7:
        sscanf(pszValue, "%f", &mDebuffDamageTimer);
        break;
    case 0x9834a7b4:
        sscanf(pszValue, "%f", &mDebuffDamageRate);
        break;
    case 0xede68c8b:
        sscanf(pszValue, "%f", &mIdleRotateSpeed);
        break;
    case 0x044894ab:
        sscanf(pszValue, "%f", &mIdleWaitTime);
        break;
    case 0x1b5b62ea:
    {
        mIdleWobbleHash = PblTEMPHash(pszValue);
        RedPose pose;
        mModel->GetRedModel(0)->SetupPose(pose);
        const PblMatrix* pMatrix = pose._Table.Find(mIdleWobbleHash);
        if (pMatrix)
        {
            mBaseWobbleMatrix = *pMatrix;
        }
        break;
    }
    case 0x1c1b9c83:
        mIdleWobbleLeftFootHash = PblTEMPHash(pszValue);
        break;
    case 0x6eefb998:
        mIdleWobbleRightFootHash = PblTEMPHash(pszValue);
        break;
    case 0x19e0afd9:
        sscanf(pszValue, "%f", &mIdleWobbleFactor);
        break;
    case 0x2c70cf18:
        for (int i = 0; i < 4; i++)
        {
            if (mActiveRotateHash[i] == 0)
            {
                mActiveRotateHash[i] = PblTEMPHash(pszValue);
                break;
            }
        }
        break;
    case 0x18275a73:
        mActiveSpinHash = PblTEMPHash(pszValue);
        break;
    case 0xa2a034ea:
        mPowerupSound.SetID(pszValue, false, false);
        break;
    case 0x65a920ba:
        mAmbientSound.SetID(pszValue, false, false);
        break;
    default:
        EntityBuildingClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00626540
EntityClass* PowerupStationClass::Derive(unsigned int uHash)
{
    return new PowerupStationClass(this, uHash);
}

// FUNCTION: SWBF2 0x006265b0
Entity* PowerupStationClass::Build(const EntityDesc& desc)
{
    PowerupStation* pStation = new PowerupStation(this, desc);
    if (pStation)
    {
        static_cast<CollisionObject*>(pStation)->AddStaticToCollisionManager();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            ((EntityGeometry*)pStation)->BuildEffects();
        }
    }
    return (Entity*)pStation;
}

// GLOBAL: SWBF2 0x01efa22c
// SYNTHETIC: SWBF2 0x00405fa0
// `dynamic initializer for 'PowerupStation::rttiHashPowerupStation''
PblHash PowerupStation::rttiHashPowerupStation("PowerupStation");
char PowerupStation::rttiNamePowerupStation[] = "PowerupStation";

// FUNCTION: SWBF2 0x006244f0
bool PowerupStation::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashPowerupStation || EntityBuilding::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00624540
unsigned int PowerupStation::GetDerivedRtti() const
{
    return rttiHashPowerupStation;
}

// FUNCTION: SWBF2 0x00624550
const char* PowerupStation::GetDerivedRttiName() const
{
    return rttiNamePowerupStation;
}

// FUNCTION: SWBF2 0x00624560
EntityType PowerupStation::GetEntityType() const
{
    return ENT_POWERUPSTATION;
}

// FUNCTION: SWBF2 0x00624590
bool PowerupStation::AddToTeamManager() const
{
    return false;
}

// FUNCTION: SWBF2 0x00624580
const PblMatrix* PowerupStation::GetRenderMatrix() const
{
    return &mRenderMatrix;
}

// FUNCTION: SWBF2 0x00624570
const RedPose* PowerupStation::GetRenderPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x00624600
PowerupStation::PowerupStation(PowerupStationClass* pClass, const EntityDesc& desc)
    : EntityBuilding(pClass, desc)
{
    mClass = pClass;
    mEffectRegion = 0;
    mPowerupTimer = 0.0f;
    ActivateThread(0, PER_LOCAL_TURN);
    NetGame::Created(this);
    mPowerupState = PS_IDLE;
    mRenderMatrix = mMatrix;
    mRotateWaitTime = 0.0f;
    mRotateDir = 1;
    mIdleWobblePeriodicity = 0.0f;
}

// FUNCTION: SWBF2 0x00624790
PowerupStation::~PowerupStation()
{
    NetGame::Destroyed(this);
    DeactivateThread();
    mAmbientSound.Stop(true);
}

// SYNTHETIC: SWBF2 0x00624760
// PowerupStation::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00626650
// [thunk]:PowerupStation::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x0062665b
// [thunk]:PowerupStation::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x00626663
// [thunk]:PowerupStation::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x0062666e
// [thunk]:PowerupStation::`scalar deleting destructor' adjustor{608}
// SYNTHETIC: SWBF2 0x00626679
// [thunk]:PowerupStation::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x00624850
void PowerupStation::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x0DBA4CB3:
    {
        float fRadius = 0.0f;
        sscanf(pszValue, "%f", &fRadius);
        if (fRadius > 0.0f)
        {
            RedRegionDesc desc;
            desc.mType = PblRegion::type_invalid;
            desc.mMatrix = GetMatrix();
            desc.mNameId = mId;
            desc.mTypeId = 0xAFD98518;
            desc.mSize.w = fRadius;
            if (!mEffectRegion)
            {
                mEffectRegion = new RedRegion(desc);
            }
        }
        break;
    }
    case 0xA4F267B8:
        mEffectRegion = RedRegion::Find(PblHash(pszValue));
        break;
    default:
        GameObject::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00625c50
int PowerupStation::GetObjectsInRegion(RedRegion* pRegion, GameObject** const ppObjects, int iMaxObjects) const
{
    if (!pRegion)
    {
        return 0;
    }
    int iNumFound = CollisionManager::GetObjectsInRange(pRegion->mMatrix.GetTranslation(), pRegion->mSize.w,
                                                        ppObjects, iMaxObjects, this, 0x1a);
    int iNumInside = 0;
    for (int i = 0; i < iNumFound; ++i)
    {
        if (IsInsideRegion(pRegion, ppObjects[i]))
        {
            ppObjects[iNumInside++] = ppObjects[i];
        }
    }
    return iNumInside;
}

// FUNCTION: SWBF2 0x00625cc0
bool PowerupStation::IsInsideRegion(RedRegion* pRegion, const GameObject* pObject) const
{
    if (!pRegion || !pObject)
    {
        return false;
    }
    return pRegion->IsPointInside(pObject->GetMatrix().GetTranslation());
}

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
static PblMatrix PblMatrix_FromQuaternion(const PblQuaternion& q, const PblVector3& vPos)
{
    PblQuaternion qUnit;
    PblMatrix m;
    D3DXMatrixRotationQuaternion(&m, D3DXQuaternionNormalize(&qUnit, &q));
    m.m[3][0] = vPos.x;
    m.m[3][1] = vPos.y;
    m.m[3][2] = vPos.z;
    m.m[3][3] = 1.0f;
    return m;
}

class EntityHover
{
public:
    static PblHash rttiHashEntityHover;
};
class EntityWalker
{
public:
    static PblHash rttiHashEntityWalker;
};
class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

static inline PblQuaternion PblQuaternion_FromAxisAngle(const PblVector3& vAxis, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle * 0.5f);
    PblQuaternion q;
    q._fX = vAxis.x * fSin;
    q._fY = vAxis.y * fSin;
    q._fZ = vAxis.z * fSin;
    q._fS = fCos;
    return q;
}

// FUNCTION: SWBF2 0x006249a0
bool PowerupStation::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime) || mState != STATE_ALIVE)
    {
        return true;
    }

    if (mAmbientSound.GetPlaying())
    {
        mAmbientSound.Update(&mClass->mAmbientSound, &mMatrix.GetTranslation(), 0);
    }
    else
    {
        mAmbientSound.Play(&mClass->mAmbientSound, &mMatrix.GetTranslation(), 0, true);
    }

    mPowerupTimer += fDeltaTime;
    if (mPowerupTimer >= mClass->mPowerupDelay)
    {
        mPowerupTimer -= mClass->mPowerupDelay;

        GameObject* apObjects[64];
        int iNumObjects = GetObjectsInRegion(mEffectRegion, apObjects, 64);
        bool bApplied = false;
        for (int i = 0; i < iNumObjects; i++)
        {
            GameObject* pObject = apObjects[i];
            if (!pObject->GetControllable())
            {
                continue;
            }

            int iHealthType = pObject->mHealthType;
            float fHealth = mClass->mAddHealth[iHealthType];
            float fAmmo = mClass->mAddAmmo[iHealthType];
            float fEnergy = mClass->mAddEnergy[iHealthType];

            bool bUsed = PowerupUtility::ApplyBuff(pObject, mClass->mBuffOffenseTimer, mClass->mBuffOffenseMult,
                                                   mClass->mBuffDefenseTimer, mClass->mBuffDefenseMult,
                                                   mClass->mBuffHealthTimer, mClass->mBuffHealthRate,
                                                   mClass->mDebuffDamageTimer, mClass->mDebuffDamageRate);
            bUsed |= PowerupUtility::ApplyHealth(pObject, fHealth, 0, false);
            bUsed |= PowerupUtility::ApplyAmmo(pObject, fAmmo, false);
            bUsed |= PowerupUtility::ApplyEnergy(pObject, fEnergy);
            if (bUsed)
            {
                bApplied = true;
                PblVector3 vPos = GetPosition();

                pObject->GetMatrix();
                mClass->mPowerupSound.Play(&vPos, 0, false, true);
            }

            if (ToolTipsDisplay::sInstance->ShouldTestForToolTipState(TTT_ApproachHealthGonk) ||
                ToolTipsDisplay::sInstance->ShouldTestForToolTipState(TTT_ApproachAmmoGonk) ||
                ToolTipsDisplay::sInstance->ShouldTestForToolTipState(TTT_ApproachVehicleGonk))
            {
                Character* pCharacter = Character::FindPlayer(NetGame::GetLocalIndex(0));
                if (pCharacter && pObject->GetControllable() &&
                    pObject->GetControllable()->mCharacter == pCharacter)
                {
                    if (pObject->IsRtti(EntityHover::rttiHashEntityHover) ||
                        pObject->IsRtti(EntityWalker::rttiHashEntityWalker) ||
                        pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
                    {
                        ToolTipsDisplay::sInstance->ShowTip(TTT_ApproachVehicleGonk, 0);
                    }
                    else
                    {
                        if (fHealth != 0.0f)
                        {
                            ToolTipsDisplay::sInstance->ShowTip(TTT_ApproachHealthGonk, 0);
                        }
                        if (fAmmo != 0.0f)
                        {
                            ToolTipsDisplay::sInstance->ShowTip(TTT_ApproachAmmoGonk, 0);
                        }
                    }
                }
            }
        }

        if (bApplied)
        {
            if (mPowerupState != PS_APPLY && mPowerupState != PS_FACE)
            {
                mPowerupState = PS_APPLY;
            }
        }
        else
        {
            if (mPowerupState != PS_WAIT && mPowerupState != PS_IDLE)
            {
                mPowerupState = PS_WAIT;
            }
        }
    }

    if (!CameraManager::sInstance->IsInsideAnyCamera(mMatrix.GetTranslation(), 60.0f, 70.0f))
    {
        return true;
    }

    mModel->GetRedModel(0)->SetupPose(mPose);

    int iNumProcedural = 0;
    PblMatrix* pWobbleMatrix = 0;
    if (mClass->mIdleWobbleHash)
    {
        const PblMatrix* pMatrix = mPose._Table.Find(mClass->mIdleWobbleHash);
        if (pMatrix)
        {
            pWobbleMatrix = &mProceduralMatrix[0];
            iNumProcedural = 1;
            if (pMatrix != pWobbleMatrix)
            {
                mPose.Remove(mClass->mIdleWobbleHash);
                mPose.Store(mClass->mIdleWobbleHash, pWobbleMatrix);
            }
            *pWobbleMatrix = mClass->mBaseWobbleMatrix;
        }
    }

    switch (mPowerupState)
    {
    case PS_IDLE:
    case PS_FACE:
    {
        mRotateWaitTime -= fDeltaTime;
        if (mRotateWaitTime < 0.0f)
        {
            mRotateWaitTime = 0.0f;
            mPowerupState = (mPowerupState != PS_IDLE) ? PS_APPLY : PS_WAIT;
        }

        PblQuaternion q = PblQuaternion(mRenderMatrix);
        PblVector3 vPos = mMatrix.GetTranslation();
        float fAngle = (float)mRotateDir * mClass->mIdleRotateSpeed * fDeltaTime;
        if (fabs(fAngle) > 0.0001f)
        {
            q = q * PblQuaternion_FromAxisAngle(gVectorUnitY, fAngle);
        }
        mRenderMatrix = PblMatrix_FromQuaternion(q, vPos);

        if (mClass->mIdleWobbleHash)
        {
            float fWobble = PblMath::Sin(mIdleWobblePeriodicity * mRotateWaitTime) * mClass->mIdleWobbleFactor;
            if (pWobbleMatrix)
            {
                PblVector3 vWobblePos = pWobbleMatrix->GetTranslation();
                *pWobbleMatrix = _IdentityMatrix;
                PblQuaternion qWobble = PblQuaternion(*pWobbleMatrix);
                if (fabs(fWobble) > 0.0001f)
                {
                    qWobble = qWobble * PblQuaternion_FromAxisAngle(gVectorUnitZ, fWobble);
                }
                *pWobbleMatrix = PblMatrix_FromQuaternion(qWobble, vWobblePos);
            }

            if (mClass->mIdleWobbleLeftFootHash)
            {
                const PblMatrix* pFoot = mPose.Find(mClass->mIdleWobbleLeftFootHash);
                if (pFoot && iNumProcedural < 5)
                {
                    PblMatrix* pMatrix = &mProceduralMatrix[iNumProcedural++];
                    if (pFoot != pMatrix)
                    {
                        mPose.Remove(mClass->mIdleWobbleLeftFootHash);
                        mPose.Store(mClass->mIdleWobbleLeftFootHash, pMatrix);
                        *pMatrix = *pFoot;
                    }
                    pMatrix->m[3][3] = 1.0f;
                }
            }
            if (mClass->mIdleWobbleRightFootHash)
            {
                const PblMatrix* pFoot = mPose.Find(mClass->mIdleWobbleRightFootHash);
                if (pFoot && iNumProcedural < 5)
                {
                    PblMatrix* pMatrix = &mProceduralMatrix[iNumProcedural++];
                    if (pFoot != pMatrix)
                    {
                        mPose.Remove(mClass->mIdleWobbleRightFootHash);
                        mPose.Store(mClass->mIdleWobbleRightFootHash, pMatrix);
                        *pMatrix = *pFoot;
                    }
                    pMatrix->m[3][3] = 1.0f;
                }
            }
        }

        if (mPowerupState == PS_WAIT)
        {
            mRotateWaitTime = mClass->mIdleWaitTime;
        }
        return true;
    }

    case PS_WAIT:
        mRotateWaitTime -= fDeltaTime;
        if (mRotateWaitTime < 0.0f)
        {
            mRotateDir = -mRotateDir;
            mPowerupState = PS_IDLE;
            if (mClass->mIdleRotateSpeed > 0.0001f)
            {
                mRotateWaitTime =
                    (PblRandom::GetGlobalFloat() * 0.5f + 0.75f) * PblMath::fPi / mClass->mIdleRotateSpeed;
                if (mClass->mIdleWobbleHash)
                {
                    mIdleWobblePeriodicity = PblMath::fTwoPi * 6.0f / mRotateWaitTime;
                }
            }
            else
            {
                mRotateWaitTime = 0.0f;
            }
        }
        return true;

    case PS_APPLY:
    {
        mRotateWaitTime -= fDeltaTime;
        if (mClass->mIdleWobbleHash)
        {
            if (pWobbleMatrix)
            {
                PblVector3 vWobblePos = pWobbleMatrix->GetTranslation();
                *pWobbleMatrix = _IdentityMatrix;
                float fCos = PblMath::Cos(mRotateWaitTime * PblMath::fTwoPi);
                vWobblePos.y += (fCos - 1.0f) * (mClass->mIdleWobbleFactor * 0.5f);
                pWobbleMatrix->m[3][0] = vWobblePos.x;
                pWobbleMatrix->m[3][1] = vWobblePos.y;
                pWobbleMatrix->m[3][2] = vWobblePos.z;
                pWobbleMatrix->m[3][3] = 1.0f;
            }

            if (mClass->mIdleWobbleLeftFootHash)
            {
                const PblMatrix* pFoot = mPose.Find(mClass->mIdleWobbleLeftFootHash);
                if (pFoot && iNumProcedural < 5)
                {
                    PblMatrix* pMatrix = &mProceduralMatrix[iNumProcedural++];
                    if (pFoot != pMatrix)
                    {
                        mPose.Remove(mClass->mIdleWobbleLeftFootHash);
                        mPose.Store(mClass->mIdleWobbleLeftFootHash, pMatrix);
                        *pMatrix = *pFoot;
                    }
                    pMatrix->m[3][1] = 0.0f;
                    pMatrix->m[3][3] = 1.0f;
                }
            }
            if (mClass->mIdleWobbleRightFootHash)
            {
                const PblMatrix* pFoot = mPose.Find(mClass->mIdleWobbleRightFootHash);
                if (pFoot && iNumProcedural < 5)
                {
                    PblMatrix* pMatrix = &mProceduralMatrix[iNumProcedural++];
                    if (pFoot != pMatrix)
                    {
                        mPose.Remove(mClass->mIdleWobbleRightFootHash);
                        mPose.Store(mClass->mIdleWobbleRightFootHash, pMatrix);
                        *pMatrix = *pFoot;
                    }
                    pMatrix->m[3][1] = 0.0f;
                    pMatrix->m[3][3] = 1.0f;
                }
            }
        }

        const RedSkeleton* pSkeleton;
        PblMatrix* pNext = &mProceduralMatrix[iNumProcedural];
        for (int i = 0; i < 4; i++)
        {
            unsigned int uHash = mClass->mActiveRotateHash[i];
            if (!uHash)
            {
                continue;
            }
            const PblMatrix* pNode = mPose._Table.Find(uHash);
            if (!pNode || iNumProcedural >= 5)
            {
                continue;
            }
            iNumProcedural++;
            PblMatrix* pMatrix = pNext++;
            if (pNode != pMatrix)
            {
                mPose.Remove(uHash);
                mPose.Store(uHash, pMatrix);
                *pMatrix = *pNode;
            }

            PblMatrix mDefault(_IdentityMatrix);
            pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
            for (unsigned int uBone = 0; uBone < 128; uBone++)
            {
                if (pSkeleton->GetBoneNameHash(uBone) == uHash)
                {
                    mDefault = pSkeleton->GetDefaultPoseMatrix(uBone);
                    break;
                }
            }

            float fAngle = PblMath::Sin((mRotateWaitTime - (float)i) * PblMath::fTwoPi) * 0.15f;
            PblVector3 vNodePos = pMatrix->GetTranslation();
            *pMatrix = mDefault;
            PblQuaternion q = PblQuaternion(*pMatrix);
            if (fabs(fAngle) > 0.0001f)
            {
                q = q * PblQuaternion_FromAxisAngle(gVectorUnitX, fAngle);
            }
            *pMatrix = PblMatrix_FromQuaternion(q, vNodePos);
        }

        unsigned int uSpinHash = mClass->mActiveSpinHash;
        if (!uSpinHash)
        {
            return true;
        }
        const PblMatrix* pNode = mPose._Table.Find(uSpinHash);
        if (!pNode || iNumProcedural >= 5)
        {
            return true;
        }
        PblMatrix* pMatrix = &mProceduralMatrix[iNumProcedural];
        if (pNode != pMatrix)
        {
            mPose.Remove(uSpinHash);
            mPose.Store(uSpinHash, pMatrix);
            *pMatrix = *pNode;
        }

        PblMatrix mDefault(_IdentityMatrix);
        pSkeleton = mModel->GetRedModel(0)->GetSkeleton();
        for (unsigned int uBone = 0; uBone < 128; uBone++)
        {
            if (pSkeleton->GetBoneNameHash(uBone) == uSpinHash)
            {
                mDefault = pSkeleton->GetDefaultPoseMatrix(uBone);
                break;
            }
        }

        float fAngle = PblMath::Sin(mRotateWaitTime * PblMath::fTwoPi) * PblMath::fPi;
        PblVector3 vNodePos = pMatrix->GetTranslation();
        *pMatrix = mDefault;
        PblQuaternion q = PblQuaternion(*pMatrix);
        if (fabs(fAngle) > 0.0001f)
        {
            q = q * PblQuaternion_FromAxisAngle(gVectorUnitZ, fAngle);
        }
        *pMatrix = PblMatrix_FromQuaternion(q, vNodePos);
        return true;
    }

    default:
        return true;
    }
}
