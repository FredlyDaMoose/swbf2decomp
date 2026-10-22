#include "LockOnManager.h"
#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "FlagItem.h"
#include "GameObject.h"
#include "GameSound.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "NetGame.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "AIUtil.h"
#include "EntityCarrier.h"
#include "GameModel.h"
#include "TargetManager.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"

#include <math.h>

#include "EntityDroideka.h"
#include "EntityHover.h"
#include "EntitySoldier.h"
#include "EntityWalker.h"
#include "MountedTurret.h"
#include <float.h>
#include "CollisionManager.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// GLOBAL: SWBF2 0x01e573f8
unsigned int LockOnManager::sNumLockOnManagers;

// GLOBAL: SWBF2 0x01e57400
LockOnManager* LockOnManager::sLockOnManagers[4];

// GLOBAL: SWBF2 0x01ec7d30
// SYNTHETIC: SWBF2 0x00404180
// `dynamic initializer for 'LockOnManager::sTargetLockSound''
// SYNTHETIC: SWBF2 0x00768c80
// `dynamic atexit destructor for 'LockOnManager::sTargetLockSound''
GameSound LockOnManager::sTargetLockSound;

static inline void ClearHandle(PblHandle<GameObject>& handle)
{
    handle.mObject = 0;
    handle.mSavedHandleId = 0;
}

// FUNCTION: SWBF2 0x0057acf0
LockOnManager::LockOnManager(int iPlayerIndex)
    : mPlayerIndex(iPlayerIndex), mPlayerCharacter(0)
{
    ClearHandle(mNewAttacker);
    ClearHandle(mLockedObj);
    ClearHandle(mForceNewLockTarget);

    mPlayerCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mPlayerIndex));

    ClearHandle(mNewAttacker);
    mAutoLoseVisibilityCheck = false;
    mAutoLoseLockedVisibilityCheck = false;
    mNumNearestTargets = 0;
    mNumAttackerTargets = 0;
    mNewNumNearestTargets = 0;
    mNewNumAttackerTargets = 0;
    mTargetsAllowedFilter = 0;

    mAutoLoseTime = 0.0f;
    mAutoLoseDotProd = -1.0f;
    mAutoLoseDistSquare = 1e10f;
    mAutoLoseLockedTime = 0.0f;
    mAutoLoseLockedDotProd = -1.0f;
    mAutoLoseLockedDistSquare = 1e10f;
    mLockDotProdFilter = -1.0f;
    mLockDotProdFilterLocked = -1.0f;
    mLockDistFilter = 10000.0f;
    mLockDistSquareFilter = 1e8f;
    mSafeZone = RedInterfaceScreen::GetScreenSafeZone();
    mLockVisibleFilter = false;
    mAutoTarget = true;
    mAutoLock = false;

    ClearHandle(mLockedObj);
    Controllable* pControllable = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;
    if (pControllable)
    {
        ClearHandle(pControllable->mTargetLockedObj);
    }

    mLockedObjChangedTurn = 0;
    mForceNewLock = 0;
    ClearHandle(mForceNewLockTarget);
    mDisplayIconsOnEverything = false;
    mTimeSinceLastForceLock = 0.0f;
    mTargetVisibilityUpdate = true;

    ++sNumLockOnManagers;
    sLockOnManagers[mPlayerIndex] = this;
}

// FUNCTION: SWBF2 0x0057afa0
LockOnManager::~LockOnManager()
{
    --sNumLockOnManagers;
    sLockOnManagers[mPlayerIndex] = 0;
}

inline void LockOnManager::Target::SetObj(GameObject* const pObject, TargetInfo* const pInfo)
{
    obj = pObject;
    if (pInfo)
    {
        pInfo->mAttacker = false;
        pInfo->mLocked = false;
        pInfo->mHintTarget = false;
        pInfo->mPositionValid = false;
    }
    if (!(GameObject*)obj)
    {
        teamMask = 0;
    }
}

inline void LockOnManager::TargetHistory::Add(GameObject* const pObject)
{
    if (mCount < 64)
    {
        mCount++;
    }
    mHistory[mNextIndexToFill++] = pObject;
    if (mNextIndexToFill >= 64)
    {
        mNextIndexToFill = 0;
    }
}

// FUNCTION: SWBF2 0x0057d5a0
void LockOnManager::SetDisplayIconsOnEverything(bool bEverything)
{
    if (!bEverything)
    {
        for (unsigned int i = 0; i < 64; i++)
        {
            mTargetPlayer[i].SetObj(0, &mTargetPlayerInfo[i]);
            if (mPlayerIndex >= 0)
            {
                HUD::Event event;
                event.mClass = HUD::GameEvents::gPlayerEvents[mPlayerIndex].targetResetPlayer;
                event.mData.uintValue = i;
                event.Send();
            }
        }
    }
    if (NetGame::IsEnabled())
    {
        mDisplayIconsOnEverything = false;
    }
    else
    {
        mDisplayIconsOnEverything = bEverything;
    }
}

// FUNCTION: SWBF2 0x0057d8f0
void LockOnManager::CreateAll()
{
    sTargetLockSound.SetID(0x6a942216u, false, false);
    for (unsigned int i = 0; i < CameraManager::Instance()->GetNumCameras(); i++)
    {
        new LockOnManager(i);
    }
    for (int i = 0; i < 32; i++)
    {
        sTargetCommon[i].obj = 0;
        sTargetCommon[i].teamMask = 0;
    }
}

// FUNCTION: SWBF2 0x0057b030
void LockOnManager::Update(float fDeltaTime)
{
    if (fDeltaTime == 0.0f)
    {
        return;
    }

    if (FlagItem::sFlagGameplayType != FlagItem::FLAG_GAMEPLAY_1FLAG &&
        FlagItem::sFlagGameplayType != FlagItem::FLAG_GAMEPLAY_2FLAG)
    {
        if (mNewNumNearestTargets > 1 && mNewNumNearestTargets <= 3)
        {
            mNewNumNearestTargets = 1;
        }
    }

    const unsigned int uTeamMask = 1 << (mPlayerCharacter ? mPlayerCharacter->mTeamNumber : 0);
    RedCamera* pCamera = CameraManager::Instance()->GetCamera(mPlayerIndex);
    const bool bMyFrame = (unsigned int)RedRenderer::GetFrameNumber() % CameraManager::Instance()->GetNumCameras() ==
                     (unsigned int)mPlayerIndex;
    Controllable* pCtrl = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;

    if (bMyFrame || mForceNewLock)
    {
        if (!pCtrl)
        {
            mNewNumNearestTargets = 0;
            mNewNumAttackerTargets = 0;
            LockOnManager::ClearPositionValid(mTargetPlayerInfo, 64);
            LockOnManager::ClearPositionValid(mTargetCommonInfo, 32);
        }
        if (mNewNumAttackerTargets != mNumAttackerTargets || mNewNumNearestTargets != mNumNearestTargets)
        {
            for (int i = 0; i < 64; i++)
            {
                mTargetPlayer[i].SetObj(0, &mTargetPlayerInfo[i]);
                mTargetPlayerInfo[i].Clean();
            }
            mNumAttackerTargets = mNewNumAttackerTargets;
            mNumNearestTargets = mNewNumNearestTargets;
            ClearHandle(mNewAttacker);
            mForceNewLock = 0;
            SetLockedTarget(0, 0);
        }
        if (pCtrl)
        {
            GameObject* pObject = pCtrl->GetGameObject();

            if (mNumNearestTargets)
            {
                SetupNearestTargets(pCtrl, pObject, 0, mNumNearestTargets, fDeltaTime, uTeamMask);
            }
        }
    }

    PblVector3 vEye = pCamera->GetMatrix().GetTranslation();
    bool bOffFrame = !bMyFrame;
    unsigned int uChanged = 0;
    if (mTargetVisibilityUpdate && pCtrl)
    {
        for (int i = 0; i < 64; i++)
        {
            UpdateTargetVisibility(pCtrl, pCamera, vEye, mTargetPlayer[i], mTargetPlayerInfo[i], false, uTeamMask,
                                   bOffFrame, uChanged);
        }
        for (int i = 0; i < 32; i++)
        {
            UpdateTargetVisibility(pCtrl, pCamera, vEye, sTargetCommon[i], mTargetCommonInfo[i], true, uTeamMask,
                                   bOffFrame, uChanged);
        }
    }
    if (!bOffFrame && uChanged)
    {
        LockOnManager::ClearVisibilityChecked(mTargetPlayerInfo, 64);
        LockOnManager::ClearVisibilityChecked(mTargetCommonInfo, 32);
    }
}

// FUNCTION: SWBF2 0x0057bd80
void LockOnManager::ClearPositionValid(LockOnManager::TargetInfo* const pInfo, unsigned int uCount)
{
    for (unsigned int i = 0; i < uCount; i++)
    {
        pInfo[i].Clean();
    }
}

// FUNCTION: SWBF2 0x0057d9f0
void LockOnManager::UpdateAll(float fDeltaTime)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        if (sLockOnManagers[i])
        {
            sLockOnManagers[i]->Update(fDeltaTime);
        }
    }
}

extern bool netFrameLock;
namespace GameLoop { extern bool mbHistoricalRules; }

// FUNCTION: SWBF2 0x0057b430
void LockOnManager::CalculateScreenCoordinates(Controllable* const pCtrl, RedCamera* const pCamera, const PblVector3* pPos, float* pScreenX, float* pScreenY, bool* pOffScreen, bool bAimRay)
{
    LockOnManager* pThis = (LockOnManager*)this;
    PblVector3 vCamera;
    D3DXVec3TransformCoord(&vCamera, pPos, &pCamera->GetMatrixInverse());
    const PblVector3 vProjected = pCamera->TransformCameraPointToProjectionSpace(vCamera);

    PblVector3 vToTarget;
    PblVector3 vForward;
    if (bAimRay)
    {
        const PblMatrix& mCamera = pCamera->GetMatrix();
        vForward = PblVector3(-mCamera.m[2][0], -mCamera.m[2][1], -mCamera.m[2][2]);
        vToTarget = PblVector3(pPos->x - mCamera.m[3][0], pPos->y - mCamera.m[3][1],
                               pPos->z - mCamera.m[3][2]);
    }
    else
    {
        vForward = pCtrl->mEyeDir;
        vToTarget = PblVector3(pPos->x - pCtrl->mEyePoint.x,
                               pPos->y - pCtrl->mEyePoint.y, pPos->z - pCtrl->mEyePoint.z);
    }

    D3DXVec3Normalize(&vToTarget, &vToTarget);
    const float fDot = vForward.x * vToTarget.x + vForward.y * vToTarget.y +
                       vForward.z * vToTarget.z;

    *pScreenX = vCamera.z > 0.0f ? -vProjected.x : vProjected.x;
    *pScreenY = vCamera.z > 0.0f ? -vProjected.y : vProjected.y;
    const float fAbsX = fabsf(*pScreenX);
    float fAbsY = fabsf(*pScreenY);
    GameObject* pObject = pCtrl->GetGameObject();
    if (!pObject || !pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        if (fDot < 0.2f && fAbsY > fAbsX * 0.3f)
        {
            float fLimit = fAbsX * 0.3f;
            if (fDot > 0.0f)
            {
                fLimit += (fAbsY - fLimit) * (fDot / 0.2f);
            }
            const float fScale = fLimit / fAbsY;
            *pScreenY *= fScale;
            fAbsY = fScale * fAbsY;
        }
    }

    if (fDot <= 0.0f || fAbsX > pThis->mSafeZone || fAbsY > pThis->mSafeZone)
    {
        const float fMax = fAbsX < fAbsY ? fAbsY : fAbsX;
        const float fScale = pThis->mSafeZone / fMax;
        *pScreenX *= fScale;
        *pScreenY *= fScale;
        *pOffScreen = true;
    }
    else
    {
        *pOffScreen = false;
    }
}

// FUNCTION: SWBF2 0x0057b720
void LockOnManager::UpdateTargetVisibility(Controllable* const pCtrl, RedCamera* const pCamera, PblVector3& vEye,
                                           Target& rTarget, TargetInfo& rInfo, bool bCommon,
                                           unsigned int uTeamMask, bool& bArg8, unsigned int& uArg9)
{
    GameObject* pObject = rTarget.obj;
    if (!pObject)
    {
        return;
    }
    if (!(rTarget.teamMask & uTeamMask))
    {
        return;
    }

    PblVector3 vPos = pObject->GetCollisionCenter();
    if (pObject->GetObjectData(0) == COLL_ATTACHED)
    {
        const PblMatrix& mObject = pObject->GetMatrix();
        vPos.x += mObject.m[1][0] * 0.4f;
        vPos.y += mObject.m[1][1] * 0.4f;
        vPos.z += mObject.m[1][2] * 0.4f;
    }
    if (const RedPose* pPose = pObject->GetPose())
    {
        if (const PblMatrix* pBone = pPose->Find(0x3483A489u))
        {
            D3DXVec3TransformCoord(&vPos, (const PblVector3*)pBone->m[3], &pObject->GetMatrix());
        }
    }
    else if (const GameModel* pModel = pObject->GetModel())
    {
        PblMatrix mBone;
        if (pModel->GetRedModel(0) && pModel->GetRedModel(0)->GetBoneMatrix(0x3483A489u, mBone))
        {
            D3DXVec3TransformCoord(&vPos, (const PblVector3*)mBone.m[3], &pObject->GetMatrix());
        }
    }

    PblVector3 vLift;
    if (bCommon)
    {
        PblVector3 vUp(0.0f, 1.3f, 0.0f);
        D3DXVec3TransformNormal(&vLift, &vUp, &pObject->GetMatrix());
    }
    else if (rInfo.mLocked)
    {
        vLift = PblVector3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        PblVector3 vUp;
        if (pObject == pCtrl->GetGameObject())
        {
            vUp = PblVector3(0.0f, 0.3f, pObject->GetCollisionRadius() * -0.2f);
        }
        else
        {
            float fUp = pObject->GetCollisionRadius() * 0.7f;
            vUp = PblVector3(0.0f, 4.0f > fUp ? fUp : 4.0f, 0.0f);
        }
        D3DXVec3TransformNormal(&vLift, &vUp, &pObject->GetMatrix());
    }

    float fDist = sqrtf((vPos.y - vEye.y) * (vPos.y - vEye.y) + (vPos.x - vEye.x) * (vPos.x - vEye.x) +
                        (vPos.z - vEye.z) * (vPos.z - vEye.z));
    if (fDist > rTarget.maxDistTrack)
    {
        return;
    }

    GameObject* pVehicle = pObject;
    if (pObject->GetObjectData(0) == COLL_ATTACHED)
    {
        pVehicle = Controllable::GetVehicleIfAttached(pObject);
        if (pVehicle == pObject)
        {
            pVehicle = 0;
        }
    }
    PblVector3 vLag(0.0f, 0.0f, 0.0f);
    if (pVehicle && pVehicle->IsInScene())
    {
        PblMatrix mSmoothed = pVehicle->GetSmoothedMatrix();
        if (!netFrameLock && NetGame::IsEnabled())
        {
            gameObjectInterpolator.GetMatrix(pVehicle->mNetUniqueId, mSmoothed);
        }
        const PblMatrix& mVehicle = pVehicle->GetMatrix();
        vLag = PblVector3(mSmoothed.m[3][0] - mVehicle.m[3][0], mSmoothed.m[3][1] - mVehicle.m[3][1],
                          mSmoothed.m[3][2] - mVehicle.m[3][2]);
    }

    bool bAimRay = true;
    if (rInfo.mLocked)
    {
        bAimRay = pCtrl->GetEntity() && pCtrl->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer);
    }

    PblVector3 vPoint(vLag.x + vPos.x + vLift.x, vLag.y + vPos.y + vLift.y, vLag.z + vPos.z + vLift.z);
    float fScreenX;
    float fScreenY;
    bool bOffScreen;
    CalculateScreenCoordinates(pCtrl, pCamera, &vPoint, &fScreenX, &fScreenY, &bOffScreen, bAimRay);
    ++uArg9;
    if (!bArg8 && !rInfo.mVisibilityChecked)
    {
        rInfo.mVisible = AIUtil::CanSeePosition(vEye, pCtrl, vPos, pObject);
        rInfo.mVisibilityChecked = true;
        rInfo.mPositionValid = true;
        bArg8 = true;
    }
    rInfo.mPosition = PblVector3(fScreenX, fScreenY, fDist);
    rInfo.mOnScreen = !bOffScreen;

    bool bFlagCarrier = false;
    if (bCommon)
    {
        TargetManager::Marker* pMarker = 0;
        for (unsigned int i = 0; i < TargetManager::MAX_MARKERS; ++i)
        {
            if (pObject == (GameObject*)TargetManager::gMarkers[i].actor)
            {
                pMarker = &TargetManager::gMarkers[i];
                break;
            }
        }
        if (pMarker && ((GameObject*)pMarker->actor)->IsRtti(FlagItem::rttiHashFlagItem) && !GameLoop::mbHistoricalRules)
        {
            bFlagCarrier = true;
        }
    }
    else
    {
        bFlagCarrier = pObject->GetCarriedFlagItem() != 0;
    }
    rInfo.mFlagCarrier = bFlagCarrier;

    rInfo.mFriendly = pCtrl->GetGameObject() && pCtrl->GetGameObject()->GetAffiliationToMe(pObject) >= 0;
}

// FUNCTION: SWBF2 0x0057bda0
void LockOnManager::SetLockedTarget(GameObject* const pObject, TargetInfo* const pInfo)
{
    mLockedObj = pObject;
    Controllable* pCtrl = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;
    if (pCtrl)
    {
        pCtrl->mTargetLockedObj = pObject;
    }
    mLockedObjChangedTurn = NetGame::GetClientTurnNumber();
    if (!pInfo)
    {
        return;
    }
    if (pObject)
    {
        sTargetLockSound.Play(0, 0, false, true);
        for (int i = 0; i < 64; ++i)
        {
            mTargetPlayerInfo[i].mLocked = (pInfo == &mTargetPlayerInfo[i]);
        }
    }
    else
    {
        pInfo->mVisibilityChecked = false;
        pInfo->mLocked = false;
    }
}

// FUNCTION: SWBF2 0x0057b320
void LockOnManager::RemoveTargetLock()
{
    if (!(GameObject*)mLockedObj)
    {
        return;
    }

    TargetInfo* pInfo = 0;
    for (unsigned int i = 0; i < 64; i++)
    {
        if ((GameObject*)mLockedObj == (GameObject*)mTargetPlayer[i].obj)
        {
            mTargetPlayer[i].SetObj(0, 0);
            pInfo = &mTargetPlayerInfo[i];
            pInfo->Clean();
            break;
        }
    }
    SetLockedTarget(0, pInfo);
    mForceNewLock = 0;
    mForceNewLockTarget = 0;
}

// FUNCTION: SWBF2 0x0057db80
LockOnManager::Target* LockOnManager::FindTargetByGameObject(Target* const pTargets, unsigned int uNumTargets,
                                                             const GameObject* const pObject)
{
    for (unsigned int i = 0; i < uNumTargets; ++i)
    {
        if ((GameObject*)pTargets[i].obj == pObject)
        {
            return &pTargets[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0057b300
bool LockOnManager::IsGameObjectTargeted(const GameObject* const pObject)
{
    return FindTargetByGameObject(mTargetPlayer, 64, pObject) != 0;
}

// FUNCTION: SWBF2 0x0057dbd0
bool LockOnManager::IsGameObjectTargetedAll(const GameObject* const pObject)
{
    if (!pObject)
    {
        return false;
    }
    if (FindTargetByGameObject(sTargetCommon, 32, pObject))
    {
        return true;
    }
    for (unsigned int i = 0; i < 4; ++i)
    {
        if (sLockOnManagers[i] && sLockOnManagers[i]->IsGameObjectTargeted(pObject))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0057d680
bool LockOnManager::GetDisplayIconsEverything() const
{
    return !NetGame::IsEnabled() && mDisplayIconsOnEverything;
}

// FUNCTION: SWBF2 0x0057d6b0
bool LockOnManager::IsTargetObjectTypeAllowed(GameObject* gameObject, GameObject* me, bool noLockon,
                                              unsigned int targetsAllowedFilter, unsigned int newNumNearestTargets,
                                              bool displayIconsOnEverything)
{
    if (!gameObject || !gameObject->mIsTargetable || !(gameObject->mCurHealth > 0.0f) || !gameObject->IsAlive())
    {
        return false;
    }

    if (noLockon)
    {
        if ((FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_1FLAG ||
             FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_2FLAG) &&
            gameObject->GetCarriedFlagItem())
        {
            if (gameObject != me)
            {
                return true;
            }
            if (me->GetControllable() && me->GetControllable()->mCharacter &&
                me->GetControllable()->CheckFirstPerson(me->GetControllable()->mCharacter->mPlayerId))
            {
                return false;
            }
            return true;
        }
        if (newNumNearestTargets <= 3)
        {
            return false;
        }
    }

    if (gameObject == me)
    {
        return false;
    }

    if (gameObject->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier)
    {
        if (static_cast<EntitySoldier*>(gameObject)->GetInvisibilityFactor() > 0.0f &&
            me->GetAffiliationToMe(gameObject) < 0)
        {
            return false;
        }
    }

    int healthType = gameObject->mHealthTypeForLockOn;
    if (healthType == 8)
    {
        healthType = gameObject->mHealthType;
    }

    bool typeAllowed = ((1 << healthType) & targetsAllowedFilter) != 0;

    if (noLockon && displayIconsOnEverything && !NetGame::IsEnabled() && me->GetAffiliationToMe(gameObject) < 0)
    {
        if (gameObject->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
            gameObject->IsRtti(EntityDroideka::rttiHashEntityDroideka) ||
            gameObject->IsRtti(EntityWalker::rttiHashEntityWalker) ||
            gameObject->IsRtti(EntityFlyer::rttiHashEntityFlyer) ||
            gameObject->IsRtti(EntityHover::rttiHashEntityHover) ||
            gameObject->GetEntity()->IsRtti(MountedTurret::rttiHashMountedTurret))
        {
            return true;
        }
    }

    if (typeAllowed)
    {
        int affiliationToMe = me->GetAffiliationToMe(gameObject);
        if (affiliationToMe == 0)
        {
            return false;
        }
        if (affiliationToMe > 0)
        {
            if (!noLockon)
            {
                return false;
            }
            bool isFlyer = gameObject->IsRtti(EntityFlyer::rttiHashEntityFlyer);
            if (!isFlyer)
            {
                return false;
            }
        }
    }
    return typeAllowed;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0057be70
void LockOnManager::SetupNearestTargets(Controllable* const playerControllable, GameObject* const playerObject,
                                        unsigned int arrayStartIndex, unsigned int arrayEndIndex, float deltaTime,
                                        unsigned int teamMask)
{
    mForceNewLock--;
    if (mForceNewLock == 0)
    {
        mForceNewLockTarget = NULL;
    }
    else if (mForceNewLock < 0)
    {
        mForceNewLock = 0;
    }

    if (mForceNewLockTarget)
    {
        GameObject* vehicle = Controllable::GetVehicleIfAttached(mForceNewLockTarget);
        if (vehicle && vehicle != mForceNewLockTarget)
        {
            bool vehicleAllowed = IsTargetObjectTypeAllowed(vehicle, playerObject, false, mTargetsAllowedFilter,
                                                            mNewNumNearestTargets, GetDisplayIconsEverything());
            if (vehicleAllowed)
            {
                mForceNewLockTarget = vehicle;
                mForceNewLock = 5;
            }
            else if (mForceNewLockTarget->GetCollisionType() == COLL_ATTACHED)
            {
                mForceNewLock = IsTargetObjectTypeAllowed(mForceNewLockTarget, playerObject, false,
                                                          mTargetsAllowedFilter, mNewNumNearestTargets,
                                                          GetDisplayIconsEverything())
                                    ? 5
                                    : 0;
            }
            else
            {
                mForceNewLock = 0;
            }
        }
        else
        {
            mForceNewLock = IsTargetObjectTypeAllowed(mForceNewLockTarget, playerObject, false, mTargetsAllowedFilter,
                                                      mNewNumNearestTargets, GetDisplayIconsEverything())
                                ? 5
                                : 0;
        }
        if (!mForceNewLock)
        {
            mForceNewLockTarget = NULL;
        }
    }

    if (!playerObject)
    {
        return;
    }

    for (unsigned int i = arrayStartIndex; i < arrayEndIndex; ++i)
    {
        GameObject* targetObj = mTargetPlayer[i].obj;
        if (targetObj)
        {
            if (mLockedObj && targetObj != mLockedObj && mTargetPlayerInfo[i].mLocked)
            {
                mTargetPlayerInfo[i].SetLocked(false);
            }

            for (unsigned int j = i + 1; j < arrayEndIndex; ++j)
            {
                if (mTargetPlayer[j].obj == targetObj)
                {
                    mTargetPlayer[j].SetObj(NULL, NULL);
                    mTargetPlayerInfo[j].Clean();
                }
            }

            GameObject* vehicleTarget = Controllable::GetVehicleIfAttached(targetObj);
            if (vehicleTarget && vehicleTarget != targetObj)
            {
                bool vehicleAllowed =
                    IsTargetObjectTypeAllowed(vehicleTarget, playerObject, targetObj != mLockedObj,
                                              mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything());
                if (vehicleAllowed)
                {
                    if (targetObj == mLockedObj)
                    {
                        SetLockedTarget(vehicleTarget, &mTargetPlayerInfo[i]);
                    }
                    mTargetPlayer[i].SetObj(vehicleTarget, NULL);
                    mTargetPlayerInfo[i].ClearPositionValid();
                    mTargetPlayerInfo[i].SetFriendly(playerControllable && playerControllable->GetGameObject() &&
                                                     playerControllable->GetGameObject()->GetAffiliationToMe(
                                                         vehicleTarget) >= 0);
                    mTargetPlayerInfo[i].SetFlagCarrier(vehicleTarget->GetCarriedFlagItem() != NULL);
                }
                else if (targetObj->GetCollisionType() != COLL_ATTACHED ||
                         !IsTargetObjectTypeAllowed(targetObj, playerObject, targetObj != mLockedObj,
                                                    mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
                {
                    if (targetObj == mLockedObj)
                    {
                        SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                    }
                    mTargetPlayer[i].SetObj(NULL, NULL);
                    mTargetPlayerInfo[i].Clean();
                }
            }
            else if (!IsTargetObjectTypeAllowed(targetObj, playerObject, targetObj != mLockedObj,
                                                mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
            {
                if (targetObj == mLockedObj)
                {
                    SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                }
                mTargetPlayer[i].SetObj(NULL, NULL);
                mTargetPlayerInfo[i].Clean();
            }
        }
        else
        {
            if (mTargetPlayerInfo[i].mLocked)
            {
                mTargetPlayerInfo[i].SetLocked(false);
            }
            if (mTargetPlayerInfo[i].mPositionValid)
            {
                mTargetPlayerInfo[i].ClearPositionValid();
            }
            if (mTargetPlayerInfo[i].mHintTarget)
            {
                mTargetPlayerInfo[i].SetHint(false);
            }
            if (mTargetPlayerInfo[i].GetFlagCarrier())
            {
                mTargetPlayerInfo[i].SetFlagCarrier(false);
            }
            if (mTargetPlayerInfo[i].mAttacker)
            {
                mTargetPlayerInfo[i].SetAttacker(false);
            }
            if (mTargetPlayerInfo[i].mOnScreen)
            {
                mTargetPlayerInfo[i].SetOnScreen(false);
            }
        }
    }

    if (mLockedObj && !mForceNewLock)
    {
        mTimeSinceLastForceLock += deltaTime;
    }
    else
    {
        mTimeSinceLastForceLock = 0.0f;
    }

    if (mForceNewLock && mForceNewLockTarget)
    {
        mTargetHistory.Clear();
    }
    else
    {
        if (!mLockedObj || mTimeSinceLastForceLock > 0.5f)
        {
            mTargetHistory.Clear();
        }
        if (mLockedObj)
        {
            mTargetHistory.Add(mLockedObj);
        }
    }

    const PblVector3& playerPosition = playerObject->GetMatrix().GetTranslation();
    float maxDot = 0.0f;
    unsigned int maxDotIndex = 64;
    unsigned int emptySlot = 64;
    for (unsigned int i = arrayStartIndex; i < arrayEndIndex; ++i)
    {
        bool targetIsVisible = mTargetPlayerInfo[i].GetVisible();
        if (IsTargetObjectTypeAllowed(mTargetPlayer[i].obj, playerObject, mLockedObj != mTargetPlayer[i].obj,
                                      mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything()))
        {
            bool targetIsLocked = mLockedObj == mTargetPlayer[i].obj;
            float autoLoseTime;
            float autoLoseDotProd;
            float autoLoseDistSquare;
            bool autoLoseVisibilityCheck;
            if (targetIsLocked)
            {
                autoLoseTime = mAutoLoseLockedTime;
                autoLoseDotProd = mAutoLoseLockedDotProd;
                autoLoseDistSquare = mAutoLoseLockedDistSquare;
                autoLoseVisibilityCheck = mAutoLoseLockedVisibilityCheck;
            }
            else
            {
                autoLoseTime = mAutoLoseTime;
                autoLoseDotProd = mAutoLoseDotProd;
                autoLoseDistSquare = mAutoLoseDistSquare;
                autoLoseVisibilityCheck = mAutoLoseVisibilityCheck;
            }

            if (0.0f > autoLoseTime)
            {
                mTargetPlayer[i].timer = 0.0f;
            }
            else
            {
                const PblVector3& targetPos = mTargetPlayer[i].obj->GetMatrix().GetTranslation();
                PblVector3 toTargetVector(targetPos.x - playerControllable->mEyePoint.x,
                                          targetPos.y - playerControllable->mEyePoint.y,
                                          targetPos.z - playerControllable->mEyePoint.z);
                float dot;
                if (mTargetPlayer[i].obj == playerObject)
                {
                    dot = 1.0f;
                }
                else
                {
                    PblVector3 direction;
                    D3DXVec3Normalize(&direction, &toTargetVector);
                    dot = direction.x * playerControllable->mEyeDir.x + direction.y * playerControllable->mEyeDir.y +
                          direction.z * playerControllable->mEyeDir.z;
                }

                if (dot < autoLoseDotProd ||
                    autoLoseDistSquare < toTargetVector.x * toTargetVector.x + toTargetVector.y * toTargetVector.y +
                                             toTargetVector.z * toTargetVector.z ||
                    (autoLoseVisibilityCheck && !targetIsVisible))
                {
                    mTargetPlayer[i].timer += deltaTime;
                }
                else
                {
                    mTargetPlayer[i].timer = 0.0f;
                }

                if (mTargetPlayer[i].timer >= autoLoseTime)
                {
                    if (targetIsLocked)
                    {
                        SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                    }
                    mTargetPlayer[i].SetObj(NULL, NULL);
                    mTargetPlayerInfo[i].Clean();
                }
                else if (dot > 0.98f && mForceNewLock && !mForceNewLockTarget &&
                         IsTargetObjectTypeAllowed(mTargetPlayer[i].obj, playerObject, false, mTargetsAllowedFilter,
                                                   mNewNumNearestTargets, GetDisplayIconsEverything()) &&
                         (mTargetHistory.IsEmpty() || !mTargetHistory.Check(mTargetPlayer[i].obj)))
                {
                    if (maxDotIndex == 64 || dot > maxDot)
                    {
                        maxDot = dot;
                        maxDotIndex = i;
                    }
                }
            }
        }
        else if (mTargetPlayer[i].obj)
        {
            if (mLockedObj == mTargetPlayer[i].obj)
            {
                SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
            }
            mTargetPlayer[i].SetObj(NULL, NULL);
            mTargetPlayerInfo[i].Clean();
        }

        if (emptySlot == 64 && !mTargetPlayer[i].obj)
        {
            emptySlot = i;
        }
    }

    if (maxDotIndex < 64)
    {
        SetLockedTarget(mTargetPlayer[maxDotIndex].obj, &mTargetPlayerInfo[maxDotIndex]);
        mForceNewLock = 0;
        mTargetHistory.Add(mLockedObj);
    }

    unsigned int startIdx = 0;
    unsigned int endIdx = 0;
    if (mForceNewLock)
    {
        startIdx = emptySlot < 64 ? emptySlot : arrayStartIndex;
        endIdx = startIdx + 1;
    }
    else if (emptySlot < 64 && mAutoTarget)
    {
        startIdx = arrayStartIndex;
        endIdx = arrayEndIndex;
    }

    if (startIdx != endIdx)
    {
        GameObject* objectArray[384];
        unsigned int objectCount;
        if (mForceNewLock && mForceNewLockTarget)
        {
            objectArray[0] = mForceNewLockTarget;
            objectCount = 1;
        }
        else
        {
            objectCount = CollisionManager::GetObjectsInRange(playerObject->GetMatrix().GetTranslation(),
                                                              mLockDistFilter, objectArray, 384, NULL,
                                                              COLL_SOFT | COLL_RIGID | COLL_STATIC | COLL_ATTACHED);
        }

        if (objectCount)
        {
            int objectLeft = objectCount;
            for (unsigned int i = startIdx; i < endIdx && objectLeft > 0; ++i)
            {
                Target& target = mTargetPlayer[i];
                if (target.obj && !mForceNewLock)
                {
                    continue;
                }

                float maxDistSquare = FLT_MAX;
                int newGameObjectIdx = -1;
                objectLeft = 0;
                for (unsigned int objI = 0; objI < objectCount; ++objI)
                {
                    if (objectArray[objI])
                    {
                        GameObject* targetGameObjectVehicle = Controllable::GetVehicleIfAttached(objectArray[objI]);
                        if (targetGameObjectVehicle && targetGameObjectVehicle != objectArray[objI])
                        {
                            bool vehicleAllowed =
                                IsTargetObjectTypeAllowed(targetGameObjectVehicle, playerObject, !mForceNewLock && !mAutoLock,
                                                          mTargetsAllowedFilter, mNewNumNearestTargets, GetDisplayIconsEverything());
                            if (vehicleAllowed)
                            {
                                objectArray[objI] = targetGameObjectVehicle;
                            }
                            else if (objectArray[objI]->GetCollisionType() != COLL_ATTACHED ||
                                     !IsTargetObjectTypeAllowed(objectArray[objI], playerObject,
                                                                !mForceNewLock && !mAutoLock, mTargetsAllowedFilter,
                                                                mNewNumNearestTargets, GetDisplayIconsEverything()))
                            {
                                objectArray[objI] = NULL;
                            }
                        }
                        else if (!IsTargetObjectTypeAllowed(objectArray[objI], playerObject, !mForceNewLock && !mAutoLock,
                                                            mTargetsAllowedFilter, mNewNumNearestTargets,
                                                            GetDisplayIconsEverything()))
                        {
                            objectArray[objI] = NULL;
                        }

                        if (objectArray[objI] && (!mForceNewLock || !mTargetHistory.Check(objectArray[objI])))
                        {
                            if (!mForceNewLock)
                            {
                                for (unsigned int k = arrayStartIndex; k < arrayEndIndex; ++k)
                                {
                                    if (objectArray[objI] == mTargetPlayer[k].obj)
                                    {
                                        objectArray[objI] = NULL;
                                        break;
                                    }
                                }
                            }

                            if (objectArray[objI])
                            {
                                PblVector3 targetPos = objectArray[objI]->GetCollisionCenter();
                                if (objectArray[objI]->GetCollisionType() == COLL_ATTACHED)
                                {
                                    const PblMatrix& objMatrix = objectArray[objI]->GetMatrix();
                                    targetPos.x += objMatrix.m[1][0] * 0.4f;
                                    targetPos.y += objMatrix.m[1][1] * 0.4f;
                                    targetPos.z += objMatrix.m[1][2] * 0.4f;
                                }

                                float distSquare = (targetPos.x - playerPosition.x) * (targetPos.x - playerPosition.x) +
                                                   (targetPos.y - playerPosition.y) * (targetPos.y - playerPosition.y) +
                                                   (targetPos.z - playerPosition.z) * (targetPos.z - playerPosition.z);
                                float dot;
                                if (objectArray[objI] == playerObject)
                                {
                                    dot = 1.0f;
                                }
                                else
                                {
                                    PblVector3 toTarget(targetPos.x - playerControllable->mEyePoint.x,
                                                        targetPos.y - playerControllable->mEyePoint.y,
                                                        targetPos.z - playerControllable->mEyePoint.z);
                                    PblVector3 direction;
                                    D3DXVec3Normalize(&direction, &toTarget);
                                    dot = direction.x * playerControllable->mEyeDir.x +
                                          direction.y * playerControllable->mEyeDir.y +
                                          direction.z * playerControllable->mEyeDir.z;
                                }

                                if (mLockDistSquareFilter >= distSquare &&
                                    ((dot >= mLockDotProdFilter && mAutoLoseDistSquare > distSquare &&
                                      !mForceNewLock && !mAutoLock) ||
                                     (dot >= mLockDotProdFilterLocked && mAutoLoseLockedDistSquare > distSquare &&
                                      (mForceNewLock || mAutoLock))))
                                {
                                    float dotDiv = dot > 0.1f ? dot : 0.1f;
                                    if (dotDiv > 0.998f)
                                    {
                                        distSquare *= 1e-8f;
                                    }
                                    if (dotDiv > 0.98f)
                                    {
                                        distSquare *= 1e-6f;
                                    }
                                    dotDiv *= dotDiv;
                                    distSquare /= dotDiv;
                                    if (maxDistSquare > distSquare)
                                    {
                                        if (!mLockVisibleFilter ||
                                            AIUtil::CanSeePosition(playerControllable->mEyePoint, playerControllable,
                                                                   targetPos, objectArray[objI]))
                                        {
                                            maxDistSquare = distSquare;
                                            newGameObjectIdx = objI;
                                        }
                                        else
                                        {
                                            objectArray[objI] = NULL;
                                        }
                                    }
                                }
                                else
                                {
                                    objectArray[objI] = NULL;
                                }
                            }
                        }
                    }
                    if (objectArray[objI])
                    {
                        objectLeft++;
                    }
                }

                if (newGameObjectIdx >= 0 && objectArray[newGameObjectIdx])
                {
                    if (mLockedObj == target.obj)
                    {
                        SetLockedTarget(NULL, &mTargetPlayerInfo[i]);
                    }
                    if (mForceNewLock)
                    {
                        for (unsigned int k = arrayStartIndex; k < arrayEndIndex; ++k)
                        {
                            if (objectArray[newGameObjectIdx] == mTargetPlayer[k].obj)
                            {
                                mTargetPlayer[k].SetObj(NULL, NULL);
                                mTargetPlayerInfo[k].Clean();
                            }
                        }
                    }

                    bool isFriendly = playerControllable->GetGameObject() &&
                                      playerControllable->GetGameObject()->GetAffiliationToMe(
                                          objectArray[newGameObjectIdx]) >= 0;
                    target.InitPlayer(objectArray[newGameObjectIdx], teamMask, mTargetPlayerInfo[i], isFriendly);
                    if (mPlayerIndex >= 0)
                    {

                        HUD::Event resetTargetPlayer;
                        resetTargetPlayer.mClass = HUD::GameEvents::gPlayerEvents[mPlayerIndex].targetResetPlayer;
                        resetTargetPlayer.mData.uintValue = i;
                        resetTargetPlayer.Send();
                    }

                    if (mForceNewLock || mAutoLock)
                    {
                        if (!mTargetHistory.Check(objectArray[newGameObjectIdx]))
                        {
                            mTargetHistory.Add(objectArray[newGameObjectIdx]);
                        }
                        SetLockedTarget(objectArray[newGameObjectIdx], &mTargetPlayerInfo[i]);
                        mForceNewLock = 0;
                        mForceNewLockTarget = NULL;
                    }

                    objectArray[newGameObjectIdx] = NULL;
                    objectLeft--;
                }
            }
        }
    }

    if (mForceNewLock)
    {
        if (mForceNewLockTarget)
        {
            mForceNewLockTarget = NULL;
            mForceNewLock = 0;
        }
        else if (mTargetHistory.IsEmpty())
        {
            mForceNewLock = 0;
        }
        else
        {
            mTargetHistory.Clear();
        }
    }
}

// FUNCTION: SWBF2 0x0057ab70
bool LockOnManager::TargetHistory::Check(GameObject* const pObject) const
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        if ((GameObject*)mHistory[i] == pObject)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0057abf0
void LockOnManager::Target::InitPlayer(GameObject* const initObj, unsigned int initTeamMask,
                                       TargetInfo& targetInfo, bool isFriendly)
{
    obj = initObj;
    teamMask = initTeamMask;
    timer = 0.0f;
    GameObjectClass* pClass = initObj ? static_cast<GameObjectClass*>(initObj->mEntityClass) : 0;
    maxDistTrack = pClass ? pClass->mLockOnMaxTrackDist : 200000.0f;
    maxDist = pClass ? pClass->mLockOnMaxDist : 500.0f;
    minDist = pClass ? pClass->mLockOnMinDist : 100.0f;
    targetInfo.mVisibilityChecked = false;
    targetInfo.mVisible = false;
    targetInfo.mPositionValid = true;
    targetInfo.mFriendly = isFriendly;
    targetInfo.mFlagCarrier = initObj && initObj->GetCarriedFlagItem();
}

// FUNCTION: SWBF2 0x0057bd60
void LockOnManager::ClearVisibilityChecked(LockOnManager::TargetInfo* const pInfo, unsigned int uCount)
{
    for (unsigned int i = 0; i < uCount; ++i)
    {
        pInfo[i].mVisibilityChecked = false;
    }
}

// FUNCTION: SWBF2 0x0057d9a0
void LockOnManager::DestroyAll()
{
    sTargetLockSound.SetID(0u, false, false);

    for (unsigned int i = 0; i < 4; ++i)
    {
        if (sLockOnManagers[i])
        {
            delete sLockOnManagers[i];
            sLockOnManagers[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0057db10
void LockOnManager::RemoveTargetCommon(GameObject* pActor, unsigned int uTeamMask)
{
    for (unsigned int i = 0; i < 32; ++i)
    {
        LockOnManager::Target& target = LockOnManager::sTargetCommon[i];
        if ((GameObject*)target.obj != pActor)
        {
            continue;
        }
        const unsigned int uTaken = target.teamMask & uTeamMask;
        if (!uTaken)
        {
            continue;
        }
        uTeamMask &= ~uTaken;
        target.teamMask &= ~uTaken;
        if (!target.teamMask)
        {
            target.obj.mObject = 0;
            target.obj.mSavedHandleId = 0;
            target.teamMask = 0;
        }
        if (!uTeamMask)
        {
            break;
        }
    }
}

// GLOBAL: SWBF2 0x01ec7d50
// SYNTHETIC: SWBF2 0x004041a0
// `dynamic initializer for 'LockOnManager::sTargetCommon''
// SYNTHETIC: SWBF2 0x00768c90
// `dynamic atexit destructor for 'LockOnManager::sTargetCommon''
LockOnManager::Target LockOnManager::sTargetCommon[32];
