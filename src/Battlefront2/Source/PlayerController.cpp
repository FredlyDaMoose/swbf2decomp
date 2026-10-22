#include "PlayerController.h"
#include "AIUtil.h"
#include "Character.h"
#include "CollisionManager.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "EntityCarrier.h"
#include "EntitySoldier.h"
#include "FLInputManager.h"
#include "FLJoystick.h"
#include "GameModel.h"
#include "GameObject.h"
#include "HUDElementMap.h"
#include "NetComm.h"
#include "NetGame.h"
#include "PauseMenu.h"
#include "ToolTips.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "Aimer.h"
#include "CollisionModel.h"
#include "EntityGeometry.h"
#include "EventInfo.h"
#include "GameLoop.h"
#include "Team.h"
#include "UnitController.h"

#include <float.h>
#include <math.h>

 
 

 
 
 

class EntityDroideka
{
public:
    static PblHash rttiHashEntityDroideka;

    bool IsRollingOrTransition() const;
};
class MountedTurret { public: static PblHash rttiHashMountedTurret; };
class EntityBuildingArmed { public: static PblHash rttiHashEntityBuildingArmed; };
class EntityTrap { public: static PblHash rttiHashEntityTrap; };
class EntityItem { public: static PblHash rttiHashEntityItem; };
class EntityMine { public: static PblHash rttiHashEntityMine; };

namespace PopupText
{

    enum PopupType { DISABLED = 0, HINT = 1, OBJECTIVE = 2, SELECTION = 3, MAX_POPUP_TYPES = 4 };
    bool IsEnabled();
    void ShowPopup(PopupType eType, unsigned int uID);
    void SetPreventJump(bool bPrevent);
    bool GetPreventJump();
}

void ClearFireTriggerBlock();
bool IsFireTriggerBlocked();
namespace ObjectiveManager { int GetCurrentPopupText(); }
void RecordRecentPlayerObject(int iPlayer, const GameObject* pObject);

extern int gSpawnInvicibility;

extern bool netHeroesEnabled;
extern bool netHeroUnlocked[3];
extern int netHeroNextPlayerID[3];
extern int netHeroPlayerID[3];
extern bool netIsLocalTurn;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);
extern "C" PblMatrix* __stdcall D3DXMatrixInverse(PblMatrix* pOut, float* pDeterminant, const PblMatrix* pM);

static const PblMatrix& GetObjectMatrix(const GameObject* pObject)
{
    return static_cast<const CollisionObject*>(pObject)->GetMatrix();
}

static float GetInvisibility(const GameObject* pObject)
{
    return (float)static_cast<const EntitySoldier*>(pObject)->GetInvisibilityAlpha() * (1.0f / 255.0f);
}

// GLOBAL: SWBF2 0x01ef7f4c
// SYNTHETIC: SWBF2 0x00405f00
// `dynamic initializer for 'PlayerController::rttiHashPlayerController''
PblHash PlayerController::rttiHashPlayerController("PlayerController");
char PlayerController::rttiNamePlayerController[] = "PlayerController";

// GLOBAL: SWBF2 0x01ef7f50
// SYNTHETIC: SWBF2 0x00405f10
// `dynamic initializer for 'PlayerController::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769330
// `dynamic atexit destructor for 'PlayerController::sMemoryPool''
MemoryPool PlayerController::sMemoryPool("PlayerController", sizeof(PlayerController));

// FUNCTION: SWBF2 0x0061a060
bool PlayerController::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashPlayerController || Controller::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0061a090
unsigned int PlayerController::GetDerivedRtti() const
{
    return rttiHashPlayerController;
}

// FUNCTION: SWBF2 0x0061a0a0
const char* PlayerController::GetDerivedRttiName() const
{
    return rttiNamePlayerController;
}

// FUNCTION: SWBF2 0x0061a0b0
PlayerController* PlayerControllerPtr(Controller* pCtrl)
{
    if (pCtrl && pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
    {
        return static_cast<PlayerController*>(pCtrl);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0061a0d0
const PlayerController* PlayerControllerPtr(const Controller* pCtrl)
{
    if (pCtrl && pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
    {
        return static_cast<const PlayerController*>(pCtrl);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0061a0f0
PlayerController* PlayerControllerPtr(Controllable* pControllable)
{
    return pControllable ? PlayerControllerPtr(pControllable->mCtrl) : 0;
}

const PlayerController* PlayerControllerPtr(const Controllable* pControllable)
{
    return pControllable ? PlayerControllerPtr(static_cast<const Controller*>(pControllable->mCtrl)) : 0;
}

// FUNCTION: SWBF2 0x0061a120
PlayerController::PlayerController(Controllable* pOwner)
    : Controller(pOwner)
{
    mJoystick = NetGame::GetJoystick(pOwner->mPlayerId);
    mLanding = false;
    mMouseTurnBuildup = 0.0f;
    mAutoAimScale = 0.0f;
    mNextCritHitTime = 0.0f;
    if (pOwner->mPlayerId < 0)
    {
        mIsFireButtonDebounced = true;
        mUseButtonDebounced = true;
    }
    else
    {
        mIsFireButtonDebounced = false;
        mUseButtonDebounced = false;
    }
    for (int i = 0; i < 4; ++i)
    {
        mEnemyDeaths[i] = -1000.0f;
        mAllyDeaths[i] = -1000.0f;
        mTeamKills[i] = -1000.0f;
    }
    mAutoFollowTime = 0.0f;
}

// SYNTHETIC: SWBF2 0x0061a270
// PlayerController::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0061a2a0
float PlayerController::GetMouseBuildup()
{
    return mMouseTurnBuildup;
}

// FUNCTION: SWBF2 0x0061a2b0
void PlayerController::Update(float dt)
{
    if (NetGame::IsSkipped(mOwner->GetGameObject(), dt) || dt <= 0.0f)
    {
        return;
    }

    UpdateTargetBubbleSize(dt);
    UpdateTotalVisFactor();

    bool bIsSoldier = mOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier);
    bool bIsPlane = mOwner->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer) &&
                    mOwner->GetGameObject()->GetMaxStrafe() < 0.0001f;
    bool bIsDroideka = mOwner->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka);
    bool bIsJedi = mOwner->GetWeaponPtr(0) && mOwner->GetWeaponPtr(0)->IsMelee();
    bool bIsTurret = mOwner->GetEntity()->IsRtti(MountedTurret::rttiHashMountedTurret) ||
                     mOwner->GetEntity()->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed);

    int iInputMode;
    if (bIsPlane)
    {
        iInputMode = 2;
    }
    else if (bIsJedi)
    {
        iInputMode = 3;
    }
    else if (bIsTurret)
    {
        iInputMode = 4;
    }
    else
    {
        iInputMode = !bIsSoldier;
    }

    int iJoystick = NetGame::GetJoystickIndex(mOwner->mPlayerId);
    if (iJoystick >= 0)
    {
        if (!NetGame::IsEnabled() || NetGame::IsClient() || (NetGame::IsHost() && NetGame::IsOneWorld()))
        {
            if (!PauseMenu::GetPaused(iJoystick))
            {
                RawControllerInputs* pRawInputs = FLInputManager::s_instance.GetPlayerJoystick(iJoystick);
                pRawInputs->SetInputMode(iInputMode);
                ToolTipsDisplay::sInstance->SetControllerDefault(pRawInputs->IsDefaultBindings(iInputMode));
            }
        }
        if (((1 << iJoystick) & PlayerController::sEnabled) == 0)
        {
            mOwner->ClearControllable();
            return;
        }
    }

    bool bMouseMode = NetGame::IsMouseMode(mOwner->mPlayerId);
    float fStrafe = mJoystick->GetStrafeAxis();
    float fMove = mJoystick->GetMoveAxis();
    float fTurn = mJoystick->GetTurnAxis();
    float fPitch = mJoystick->GetPitchAxis();

    float fBuildup = mMouseTurnBuildup + fTurn;
    if (fBuildup < -1.0f)
    {
        fBuildup = -1.0f;
    }
    else if (fBuildup > 1.0f)
    {
        fBuildup = 1.0f;
    }
    mMouseTurnBuildup = fBuildup;

    if (fStrafe < -1.0f)
    {
        fStrafe = -1.0f;
    }
    else if (fStrafe > 1.0f)
    {
        fStrafe = 1.0f;
    }
    if (fMove < -1.0f)
    {
        fMove = -1.0f;
    }
    else if (fMove > 1.0f)
    {
        fMove = 1.0f;
    }

    if (bMouseMode && bIsSoldier)
    {
        if (fTurn < -16.0f)
        {
            fTurn = -16.0f;
        }
        else if (fTurn > 16.0f)
        {
            fTurn = 16.0f;
        }
        if (fPitch < -16.0f)
        {
            fPitch = -16.0f;
        }
        else if (fPitch > 16.0f)
        {
            fPitch = 16.0f;
        }
    }
    else
    {
        if (fTurn < -1.0f)
        {
            fTurn = -1.0f;
        }
        else if (fTurn > 1.0f)
        {
            fTurn = 1.0f;
        }
        if (fPitch < -1.0f)
        {
            fPitch = -1.0f;
        }
        else if (fPitch > 1.0f)
        {
            fPitch = 1.0f;
        }
    }

    if (!(bMouseMode && bIsSoldier))
    {
        if (fPitch * fPitch + fTurn * fTurn >= 0.9f)
        {
            float fTurnBuildup = mOwner->GetTurnBuildup() + dt;
            mOwner->SetTurnBuildup(fTurnBuildup < 1.0f ? fTurnBuildup : 1.0f);
            float fPitchBuildup = mOwner->GetPitchBuildup() + dt;
            mOwner->SetPitchBuildup(fPitchBuildup < 1.0f ? fPitchBuildup : 1.0f);
        }
        else
        {
            mOwner->SetTurnBuildup(0.0f);
            mOwner->SetPitchBuildup(0.0f);
        }
    }

    if (fStrafe > 0.0f && fStrafe < 0.3f)
    {
        fStrafe = 0.3f;
    }
    else if (fStrafe < 0.0f && fStrafe > -0.3f)
    {
        fStrafe = -0.3f;
    }
    if (fMove > 0.0f && fMove < 0.3f)
    {
        fMove = 0.3f;
    }
    else if (fMove < 0.0f && fMove > -0.3f)
    {
        fMove = -0.3f;
    }

    if (fMove != 0.0f)
    {
        if (fMove != 1.0f)
        {
            mOwner->mControlThrustFwd.ClearUntilPressed();
        }
        if (fMove != -1.0f)
        {
            mOwner->mControlThrustBack.ClearUntilPressed();
        }
    }
    if (fStrafe != 0.0f)
    {
        if (fStrafe != 1.0f)
        {
            mOwner->mControlStrafeRight.ClearUntilPressed();
        }
        if (fStrafe != -1.0f)
        {
            mOwner->mControlStrafeLeft.ClearUntilPressed();
        }
    }
    mOwner->mControlThrustFwd.Update(dt, fMove >= 0.7f);
    mOwner->mControlThrustBack.Update(dt, fMove <= -0.7f);
    mOwner->mControlStrafeLeft.Update(dt, fStrafe <= -0.7f);
    mOwner->mControlStrafeRight.Update(dt, fStrafe >= 0.7f);

    float fLength = sqrtf(fStrafe * fStrafe + fMove * fMove);
    if (fLength > 1.0f)
    {
        fStrafe /= fLength;
        fMove /= fLength;
    }
    mOwner->mControlStrafe = fStrafe;
    mOwner->mControlMove = fMove;

    if (!(bMouseMode && bIsSoldier))
    {
        fLength = sqrtf(fPitch * fPitch + fTurn * fTurn);
        if (fLength > 1.0f)
        {
            fPitch /= fLength;
            fTurn /= fLength;
        }
    }
    mOwner->mControlPitch = fPitch;
    mOwner->mControlTurn = fTurn;

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mOwner->mPlayerId >= 0)
    {
        Character* pCharacter = mOwner->mCharacter;
        if (pCharacter && pCharacter->mUnit && netHeroesEnabled)
        {
            int iTeam = pCharacter->mTeamNumber;
            if (netHeroUnlocked[iTeam] && netHeroPlayerID[iTeam] == -1 &&
                netHeroNextPlayerID[iTeam] == pCharacter->mPlayerId)
            {
                if (mJoystick->IsAcceptHeroPressed())
                {
                    NetGame::AcceptHero(iTeam, pCharacter->mPlayerId);
                    GameObject* pUnitObject = pCharacter->mUnit->GetGameObject();
                    if (pCharacter->mRemote)
                    {
                        pCharacter->mUnit->ExitRemote();
                    }
                    if (pCharacter->mVehicle)
                    {
                        pCharacter->mUnit->ExitVehicle(true, true);
                    }
                    pUnitObject->mIsAlive = false;
                    pUnitObject->mVanishing = true;
                    return;
                }
                if (mJoystick->IsDeclineHeroPressed())
                {
                    NetGame::DeclineHero(iTeam, pCharacter->mPlayerId);
                }
            }
        }
    }

    mOwner->mTriggerControlSwitch[0].Update(dt, mJoystick->IsPrimarySwitchButtonPressed());
    mOwner->mTriggerControlSwitch[1].Update(dt, mJoystick->IsSecondarySwitchButtonPressed());

    bool bCanUse = true;
    bool bCanExit = true;
    if (NetGame::IsEnabled())
    {
        Character* pCharacter;
        if (mOwner->mPlayerId >= 0 && (pCharacter = mOwner->mCharacter) != 0 &&
            (int)(NetComm::GetTime() - pCharacter->GetSpawnTime()) < gSpawnInvicibility)
        {
            if (mJoystick->IsPrimaryFireButtonDown() || mJoystick->IsSecondaryFireButtonDown())
            {
                pCharacter->SetSpawnTime(NetComm::GetTime() - (float)gSpawnInvicibility - 0.1f);
                NetGame::CreateEvent_UpdateInvincibility(pCharacter);
            }
            else if (!bIsDroideka)
            {
                bCanUse = false;
            }
        }
        if (!NetGame::IsClient())
        {
            bCanExit = bCanExit && NetGame::CanExitVehicle(mOwner->mPlayerId);
        }
    }

    mOwner->SetSwitch(mJoystick->IsPrimarySwitchButtonPressed(), 0);
    mOwner->SetSwitch(mJoystick->IsSecondarySwitchButtonPressed(), 1);

    if (mIsFireButtonDebounced && !IsFireTriggerBlocked())
    {
        mOwner->mControlFire[0].Update(dt, mJoystick->IsPrimaryFireButtonDown());
    }
    else
    {
        mOwner->mControlFire[0].Update(dt, false);
        if (!mJoystick->IsPrimaryFireButtonDown())
        {
            ClearFireTriggerBlock();
            mIsFireButtonDebounced = true;
        }
    }
    mOwner->mControlFire[1].Update(dt, mJoystick->IsSecondaryFireButtonDown());
    mOwner->mControlReload.Update(dt, mJoystick->IsReloadButtonDown());
    mOwner->mControlCrouch.Update(dt, mJoystick->IsCrouchButtonDown());

    mOwner->mControlSpecialMove.Update(dt, (mJoystick->bits.mBoolInputs & 0x20000000) != 0);
    mOwner->mControlSprint.Update(dt, mJoystick->IsSprintButtonDown());
    mOwner->mControlZoom.Update(dt, mJoystick->IsZoomButtonDown());
    mOwner->mControlView.Update(dt, mJoystick->IsViewButtonDown());
    mOwner->mControlSquadCommand.Update(dt, mJoystick->IsSquadCommandButtonDown());
    mOwner->mControlLockTarget.Update(dt, mJoystick->IsLockTargetButtonPressed());
    mOwner->mControlUse.Update(dt, bCanUse && bCanExit && mJoystick->IsUseButtonPressed());

    if (PopupText::GetPreventJump())
    {
        mOwner->mControlJump.Update(dt, false);
        if (!mJoystick->IsJumpButtonDown() && !PopupText::IsEnabled())
        {
            PopupText::SetPreventJump(false);
        }
    }
    else
    {
        mOwner->mControlJump.Update(dt, mJoystick->IsJumpButtonDown());
    }

    HUD::ElementMap* pMap;
    if ((iJoystick >= 0 && (pMap = HUD::ElementMap::FindByPlayerIndex(iJoystick)) != 0 &&
         pMap->GetMode() == HUD::ElementMap::mode_Large) ||
        PopupText::IsEnabled())
    {
        mOwner->mControlUse.Update(dt, false);
        mOwner->mControlLockTarget.Update(dt, false);
        if (!mJoystick->IsUseButtonPressed())
        {
            mUseButtonDebounced = true;
        }
        if (!PopupText::IsEnabled())
        {
            if (mUseButtonDebounced && mJoystick->IsUseButtonPressed())
            {
                mUseButtonDebounced = false;
                int iID = ObjectiveManager::GetCurrentPopupText();
                if (iID)
                {
                    PopupText::ShowPopup(PopupText::OBJECTIVE, iID);
                }
            }
            else if (mJoystick->IsLockTargetButtonPressed() && ObjectiveManager::GetCurrentPopupText())
            {
                PopupText::ShowPopup(PopupText::HINT, 0);
            }
        }
    }

    if (mOwner->mControlFire[0].IsDown() || mOwner->mControlFire[1].IsDown() || mOwner->mControlJump.IsDown() ||
        mOwner->mControlSquadCommand.IsDown())
    {
        SetForceAudible(true);
    }

    if (mOwner->GetWeaponCount() > 0 && mOwner->GetWeaponPtr(0)->IsMelee() && mOwner->mControlReload.IsDown())
    {
        ToolTipsDisplay::sInstance->CancelTimeoutTip(TTT_JediForcePower, 0);
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()) ||
        (NetGame::GetJoystickIndex(mOwner->mPlayerId) >= 0 && netIsLocalTurn))
    {
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mOwner->GetWeaponIndex(iChannel);
            if (iIndex < 0)
            {
                continue;
            }
            Weapon* pWeapon = mOwner->GetWeaponPtr(iIndex);
            if (!pWeapon)
            {
                continue;
            }

            WeaponClass* pClass = pWeapon->mClass;
            const GameObject* pTarget = pWeapon->mTarget;
            const GameObject* pAITarget = pWeapon->mTarget;
            int iBodyID = pWeapon->mTargetBodyID;
            bool bLocked = pTarget != 0;
            GameObject* pMe = mOwner->GetGameObject();

            if (pTarget && pTarget->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier &&
                GetInvisibility(pTarget) > 0.0f && pMe->GetAffiliationToMe(pTarget) < 0)
            {
                pTarget = 0;
                iBodyID = -1;
                pAITarget = 0;
                bLocked = false;
            }

            float fAIPrio = FLT_MAX;
            PblVector3 vStart = mOwner->mTargetInfo.mAimStart;
            PblVector3 vDir = mOwner->mEyeDir;
            float fLockOnRange = pClass->mLockOnRange;
            float fLockOnAngle = pClass->mLockOnAngle;
            float fRange = pClass->mExtremeRange;
            int iTargetSides = pClass->mTargetSides;
            int iTargetTypes = pClass->mTargetTypes;
            float fTargetPrio = FLT_MAX;

            if (pTarget)
            {
                bool bHoldLock;
                float fAngle;
                if (pClass->mLockOffAngle > 0.0f && pWeapon->IsLocked())
                {
                    bHoldLock = true;
                    fAngle = pClass->mLockOffAngle;
                }
                else
                {
                    bHoldLock = false;
                    if (mOwner->mControlFire[iChannel].IsDown())
                    {
                        fAngle = fLockOnAngle + 0.78539819f;
                    }
                    else
                    {
                        fAngle = fLockOnAngle;
                    }
                }

                PblVector3 vCenter;
                float fRadius;
                if (pTarget->mHealthType == HEALTH_TYPE_BUILDING_UNBUILT &&
                    pTarget->IsRtti(EntityTrap::rttiHashEntityTrap))
                {
                    vCenter = pTarget->GetCollisionObject()->GetCollisionCenter();
                    fRadius = pTarget->GetCollisionObject()->GetCollisionRadius();
                }
                else
                {
                    const PblBox& box = pTarget->GetModel()->_mModel->_TotalBoundingBox;
                    PblVector3 vMid((box._vMax.x + box._vMin.x) * 0.5f, (box._vMax.y + box._vMin.y) * 0.5f,
                                    (box._vMax.z + box._vMin.z) * 0.5f);
                    D3DXVec3TransformCoord(&vCenter, &vMid, &GetObjectMatrix(pTarget));
                    float dx = box._vMax.x - box._vMin.x;
                    float dy = box._vMax.y - box._vMin.y;
                    float dz = box._vMax.z - box._vMin.z;
                    fRadius = sqrtf(dy * dy + dx * dx + dz * dz) * 0.5f;
                }

                float fPrio = TargetInfo::GetTargetPrio(vStart, vDir, vCenter, fRadius, fAngle, fRange, false);
                bool bKeep = false;

                if (fPrio >= 0.0f &&
                    (pTarget->mIsAlive || pTarget->mHealthType == HEALTH_TYPE_BUILDING_DEAD ||
                     pTarget->mHealthType == HEALTH_TYPE_BUILDING_UNBUILT) &&
                    pTarget->mIsTargetable && pTarget->IsVisibleToMe(pMe))
                {
                    if (!bHoldLock)
                    {
                        iBodyID = pTarget->GetTargetBodyID(vStart, vDir);
                    }
                    PblVector3 vPoint = pTarget->GetTargetPoint(vStart, vDir, iBodyID);
                    PblVector3 vBack(vStart.x - vPoint.x, vStart.y - vPoint.y, vStart.z - vPoint.z);
                    PblVector3 vBackDir;
                    D3DXVec3Normalize(&vBackDir, &vBack);
                    vPoint.x += fRadius * vBackDir.x;
                    vPoint.y += vBackDir.y * fRadius;
                    vPoint.z += vBackDir.z * fRadius;
                    const GameObject* apIgnore[2] = { pMe, pTarget };
                    if (!CollisionManager::RayTest(vStart, vPoint, apIgnore, 2, 0x90))
                    {
                        bKeep = true;
                        float dx = vCenter.x - vStart.x;
                        float dy = vCenter.y - vStart.y;
                        float dz = vCenter.z - vStart.z;
                        bLocked = ((1 << (pMe->GetAffiliationToMe(pTarget) + 1)) & iTargetSides) != 0 &&
                                  ((1 << pTarget->mHealthType) & iTargetTypes) != 0 &&
                                  fRadius + fLockOnRange >= sqrtf(dx * dx + dy * dy + dz * dz);
                        if (bHoldLock)
                        {
                            fTargetPrio = 0.0f;
                        }
                        else
                        {
                            fTargetPrio = fPrio;
                            fAIPrio = fPrio;
                        }
                    }
                }
                if (!bKeep)
                {
                    pTarget = 0;
                    bLocked = false;
                    iBodyID = -1;
                }
            }

            if (fTargetPrio > 0.0f)
            {
                PblVector3 vSearch(vDir.x * fRange * 0.5f + vStart.x, vDir.y * fRange * 0.5f + vStart.y,
                                   vDir.z * fRange * 0.5f + vStart.z);
                PblMatrix mInverse;
                D3DXMatrixInverse(&mInverse, 0, &GetObjectMatrix(pMe));
                GameObject* apObjects[300];
                int iCount = CollisionManager::GetObjectsInRange(vSearch, fRange, apObjects, 300, pMe, 0x3b);
                for (int i = 0; i < iCount; ++i)
                {
                    GameObject* pObject = apObjects[i];
                    if (pObject->IsRtti(EntityItem::rttiHashEntityItem) &&
                        !pObject->IsRtti(EntityMine::rttiHashEntityMine))
                    {
                        continue;
                    }
                    if (!(pObject->mIsAlive || pObject->mHealthType == HEALTH_TYPE_BUILDING_DEAD ||
                          pObject->mHealthType == HEALTH_TYPE_BUILDING_UNBUILT) ||
                        !pObject->mIsTargetable || !pObject->IsVisibleToMe(pMe))
                    {
                        continue;
                    }
                    if (mOwner->mPlayerId >= 0 &&
                        static_cast<GameObjectClass*>(pObject->mEntityClass)->IsNotTargetableByPlayer())
                    {
                        continue;
                    }
                    if (pObject->GetDerivedRtti() == EntitySoldier::rttiHashEntitySoldier &&
                        GetInvisibility(pObject) > 0.0f && pMe->GetAffiliationToMe(pObject) < 0)
                    {
                        continue;
                    }

                    bool bLockable = ((1 << (pMe->GetAffiliationToMe(pObject) + 1)) & iTargetSides) != 0 &&
                                     ((1 << pObject->mHealthType) & iTargetTypes) != 0;
                    if (bLocked && !bLockable)
                    {
                        continue;
                    }

                    PblVector3 vCenter;
                    float fRadius;
                    if (pObject->mHealthType == HEALTH_TYPE_BUILDING_UNBUILT &&
                        pObject->IsRtti(EntityTrap::rttiHashEntityTrap))
                    {
                        vCenter = pObject->GetCollisionObject()->GetCollisionCenter();
                        fRadius = pObject->GetCollisionObject()->GetCollisionRadius();
                    }
                    else
                    {
                        const PblBox& box = pObject->GetModel()->_mModel->_TotalBoundingBox;
                        PblVector3 vMid((box._vMin.x + box._vMax.x) * 0.5f, (box._vMin.y + box._vMax.y) * 0.5f,
                                        (box._vMin.z + box._vMax.z) * 0.5f);
                        D3DXVec3TransformCoord(&vCenter, &vMid, &GetObjectMatrix(pObject));
                        float dx = box._vMax.x - box._vMin.x;
                        float dy = box._vMax.y - box._vMin.y;
                        float dz = box._vMax.z - box._vMin.z;
                        fRadius = sqrtf(dx * dx + dy * dy + dz * dz) * 0.5f;
                    }

                    Controllable* pControllable = pObject->GetControllable();
                    if (pControllable && pControllable->mCharacter && pControllable->mCharacter->mVehicle &&
                        pControllable->mCharacter->mVehicle->GetGameObject() == pMe)
                    {
                        continue;
                    }

                    if (bLockable)
                    {
                        float dx = vCenter.x - vStart.x;
                        float dy = vCenter.y - vStart.y;
                        float dz = vCenter.z - vStart.z;
                        bLockable = fRadius + fLockOnRange >= sqrtf(dx * dx + dy * dy + dz * dz);
                    }

                    float fPrio = TargetInfo::GetTargetPrio(vStart, vDir, vCenter, fRadius, fLockOnAngle, fRange, false);
                    float fPrioAI = TargetInfo::GetTargetPrio(vStart, vDir, vCenter, fRadius, 0.25f, fRange, false);
                    if (fPrioAI >= 0.0f && fAIPrio > fPrioAI)
                    {
                        fAIPrio = fPrioAI;
                        pAITarget = pObject;
                    }
                    if (fPrio < 0.0f || fTargetPrio <= fPrio)
                    {
                        continue;
                    }

                    int iObjectBodyID = pObject->GetTargetBodyID(vStart, vDir);
                    PblVector3 vPoint = pObject->GetTargetPoint(vStart, vDir, iObjectBodyID);
                    PblVector3 vBack(vStart.x - vPoint.x, vStart.y - vPoint.y, vStart.z - vPoint.z);
                    PblVector3 vBackDir;
                    D3DXVec3Normalize(&vBackDir, &vBack);
                    vPoint.x += fRadius * vBackDir.x;
                    vPoint.y += vBackDir.y * fRadius;
                    vPoint.z += vBackDir.z * fRadius;
                    const GameObject* apIgnore[2] = { pMe, pObject };
                    if (!CollisionManager::RayTest(vStart, vPoint, apIgnore, 2, 0x90))
                    {
                        iBodyID = iObjectBodyID;
                        pTarget = pObject;
                        bLocked = bLockable;
                        fTargetPrio = fPrio;
                    }
                }
            }

            if (bLocked)
            {
                pWeapon->mTarget = pTarget;
                pWeapon->mTargetBodyID = iBodyID;
            }
            else
            {
                pWeapon->mTarget = 0;
                pWeapon->mTargetBodyID = -1;
                if (pTarget && pTarget->mCurHealth <= 0.0f)
                {
                    pTarget = 0;
                }
            }

            RecordRecentPlayerObject(mOwner->mPlayerId, pTarget);
            mOwner->mReticuleTarget[iChannel] = pTarget;
            mOwner->mAIReticuleTarget[iChannel] = pAITarget;
            if (pTarget)
            {
                mOwner->mReticuleAffiliation[iChannel] = pMe->GetAffiliationToMe(pTarget);
            }
            else
            {
                mOwner->mReticuleAffiliation[iChannel] = 0;
            }
        }
    }

    DeployCargo();
    CheckForVisibleCriticalPts();
    CheckForAutoFollow(dt);
}

// FUNCTION: SWBF2 0x0061bc70
void PlayerController::DeployCargo()
{
    GameObject* pObject = mOwner->GetGameObject();
    if (pObject->GetControllable() != mOwner ||
        static_cast<GameObjectClass*>(pObject->mEntityClass)->GetVehicleType() != 5)
    {
        return;
    }

    EntityFlyer* pFlyer = mOwner ? static_cast<EntityFlyer*>(mOwner) : 0;
    EntityFlyer::State eState = pFlyer->mState;
    if (eState == EntityFlyer::LANDING)
    {
        mLanding = true;
        return;
    }
    if (mLanding && eState == EntityFlyer::LANDED)
    {
        const CommandPost* pPost = 0;
        if (AIUtil::IsNearNonAllyCommandPost(pObject, &pPost))
        {
            AIUtil::DeployCargo(static_cast<GameObject*>(pFlyer), pPost);
        }
    }
    mLanding = false;
}

namespace ReserveManager
{
int GetReservationCountForObject(const GameObject* pObj, int iTeam, ReserveType eType);
}

namespace TeamManager { int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
               const Team* pTeam, int iAffilMask); }

namespace AI
{
enum VOType
{
    VOT_NotShootingCriticalHit = 33,
    VOT_NearbyFriendlySlaughter = 35,
    VOT_NearbyEnemySlaughter = 36,
    VOT_NearbyTeamKillSlaughter = 37
};
namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

// FUNCTION: SWBF2 0x0061c050
void PlayerController::LogVODeath(int iVictimTeam, int iKillerTeam)
{
    if (!mOwner || !mOwner->mCharacter || !mOwner->GetGameObject())
    {
        return;
    }

    const Team* pTeam = mOwner->mCharacter->mTeamPtr;
    float* pDeaths;
    AI::VOType eType;
    if (pTeam->GetAffiliation(iVictimTeam) <= 0)
    {
        pDeaths = mEnemyDeaths;
        eType = AI::VOT_NearbyEnemySlaughter;
    }
    else if (pTeam->GetAffiliation(iKillerTeam) <= 0)
    {
        pDeaths = mAllyDeaths;
        eType = AI::VOT_NearbyFriendlySlaughter;
    }
    else
    {
        pDeaths = mTeamKills;
        eType = AI::VOT_NearbyTeamKillSlaughter;
    }

    const float fTime = GameLoop::GetMissionTime();
    const float fWindow = fTime - 8.0f;
    int i;
    for (i = 0; i < 4; ++i)
    {
        if (fWindow > pDeaths[i])
        {
            pDeaths[i] = fTime;
            break;
        }
    }
    for (i = 0; i < 4; ++i)
    {
        if (fWindow > pDeaths[i])
        {
            break;
        }
    }
    if (i == 4)
    {
        AI::VOHelper::Play(eType, 0, mOwner->GetGameObject(), 0, 0.0f);
        pDeaths[0] = -1000.0f;
        pDeaths[1] = -1000.0f;
        pDeaths[2] = -1000.0f;
        pDeaths[3] = -1000.0f;
    }
}

// FUNCTION: SWBF2 0x0061bd00
void PlayerController::CheckForVisibleCriticalPts()
{
    Controllable* pOwner = mOwner;
    if (!pOwner->mControlFire[0].IsDown() && !pOwner->mControlFire[1].IsDown())
    {
        return;
    }
    float fTime = GameLoop::GetMissionTime();
    if (mNextCritHitTime > fTime)
    {
        return;
    }
    mNextCritHitTime = fTime + 5.0f;
    if (!pOwner)
    {
        return;
    }

    GameObject* pMine = pOwner->GetGameObject();
    if (pMine && pMine->IsRtti(EntityDroideka::rttiHashEntityDroideka))
    {
        if (!mOwner->mControlFire[0].IsDown() || ((const EntityDroideka*)pMine)->IsRollingOrTransition())
        {
            return;
        }
    }

    const GameObject* pTarget = pOwner->mReticuleTarget[0];
    if (!pTarget || !AIUtil::IsVehicle(pTarget))
    {
        return;
    }
    const EntityGeometryClass* pClass = (const EntityGeometryClass*)pTarget->mEntityClass;
    int iNumHitLocations = pClass->mNumHitLocations;
    if (iNumHitLocations <= 0)
    {
        return;
    }
    if (!pOwner->GetAimer(0))
    {
        return;
    }
    PblMatrix mFire = pOwner->GetAimer(0)->GetFireMatrix();
    const PblVector3& vForward = *(const PblVector3*)mFire.m[2];
    const PblVector3& vFirePos = pOwner->GetAimer(0)->mFirePos;
    CollisionModel* pModel = pTarget->mCollisionModel;
    if (!pModel)
    {
        return;
    }
    CollisionMask mask = pModel->GetCollisionMask(COLL_ORDNANCE);

    for (int i = 0; i < iNumHitLocations; ++i)
    {
        int iBody = pClass->mHitLocations[i].mBodyId;
        if (!mask.IsSet(iBody))
        {
            continue;
        }
        PblMatrix mBody = pModel->GetWorldMatrix(iBody, pTarget->GetMatrix(), pTarget->GetPose());
        float fX = mBody.m[3][0] - vFirePos.x;
        float fY = mBody.m[3][1] - vFirePos.y;
        float fZ = mBody.m[3][2] - vFirePos.z;
        float fDist = sqrtf(fX * fX + fY * fY + fZ * fZ);
        if (fDist > 50.0f)
        {
            return;
        }
        float fInv = 1.0f / fDist;
        PblVector3 vDir(fX * fInv, fY * fInv, fZ * fInv);
        if (vForward.y * vDir.y + vDir.x * vForward.x + vForward.z * vDir.z < 0.99026805f)
        {
            continue;
        }
        PblVector3 vHit;
        int iHitBody = -1;
        float fHit = ((GameObject*)pTarget)->GetCollisionObject()->RayHit(vFirePos, vDir, fDist, &vHit,
                                                                         &iHitBody, true, 0.0f);
        if (fHit < 1.0f && iHitBody == iBody)
        {
            return;
        }
    }
    AI::VOHelper::Play(AI::VOT_NotShootingCriticalHit, 0, mOwner->GetGameObject(), pTarget, 0.0f);
}

// FUNCTION: SWBF2 0x0061c150
void PlayerController::CheckForAutoFollow(float dt)
{
    if (!mOwner || !mOwner->GetGameObject())
    {
        mAutoFollowTime = 0.0f;
        return;
    }
    GameObject* pSoldier = mOwner->GetGameObject();
    if (!pSoldier->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        mAutoFollowTime = 0.0f;
        return;
    }
    if (ReserveManager::GetReservationCountForObject(pSoldier, pSoldier->GetTeam(), RT_FORMATION) >= 2)
    {
        return;
    }

    const PblVector3& vVelocity = mOwner->GetGameObject()->GetVelocity();
    float fSpeedSq = vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z;
    if (pSoldier->GetMaxSpeed() * pSoldier->GetMaxSpeed() * 0.9f * 0.9f > fSpeedSq)
    {
        float fTime = mAutoFollowTime - dt * 4.0f;
        mAutoFollowTime = 0.0f > fTime ? 0.0f : fTime;
        return;
    }

    const PblMatrix& mMatrix = mOwner->GetMatrix();
    mAutoFollowTime += dt;
    if (4.0f > mAutoFollowTime)
    {
        return;
    }
    float fTime = mAutoFollowTime - 1.0f;
    mAutoFollowTime = 0.0f > fTime ? 0.0f : fTime;

    const Team* pTeam = Team::GetPtr(pSoldier->GetTeam());
    PblVector3 vBehind(mMatrix.m[2][0] * vVelocity.x * 10.0f * 0.5f, mMatrix.m[2][1] * vVelocity.y * 10.0f * 0.5f,
                       mMatrix.m[2][2] * vVelocity.z * 10.0f * 0.5f);
    const PblVector3& vPos = pSoldier->GetPosition();
    PblVector3 vCenter(vPos.x - vBehind.x, vPos.y - vBehind.y, vPos.z - vBehind.z);
    GameObject* apObjects[15];
    int iCount = TeamManager::GetObjectsInRange(vCenter, 10.0f, apObjects, 15, pTeam, 4);
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObject = apObjects[i];
        if (!pObject->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            continue;
        }
        UnitController* pUnit = UnitControllerPtr(pObject->GetControllable());
        if (!pUnit)
        {
            continue;
        }
        AI::EventInfo query(AI::EVQ_SC_IsFollowingChr, pSoldier);
        if (pUnit->SendEvent(&query))
        {
            continue;
        }
        AI::EventInfo follow(AI::EVT_SC_FollowChrLoose, pSoldier);
        if (pUnit->SendEvent(&follow) == 1)
        {
            return;
        }
    }
}
