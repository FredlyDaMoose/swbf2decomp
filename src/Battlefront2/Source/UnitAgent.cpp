#include "UnitAgent.h"

#include <new>

#include "AILowLevel.h"
#include "AIUtil.h"
#include "AttackHelper.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "ControllerManager.h"
#include "Entity.h"
#include "EventInfo.h"
#include "GameObject.h"
#include "EntitySoldier.h"
#include "FlyRegion.h"
#include "GameLoop.h"
#include "Team.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"

#include "UnitThreatManager.h"

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

class EntityDroideka
{
public:
    static PblHash rttiHashEntityDroideka;
};

namespace AI
{

enum AIAffil
{
    AFFIL_ENEMY = 1,
    AFFIL_NEUTRAL = 2,
    AFFIL_ALLY = 4,
    AFFIL_ALLYNEUTRAL = 6,
    AFFIL_ALL = 7
};

enum VOType
{
    VOT_FriendlyFire = 2,
    VOT_SC_FollowResponse = 23,
    VOT_SC_GetInResponse = 29
};

namespace VOHelper
{

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget, const GameObject* pOther, float fDelay);
}
}

namespace AIUtil
{

    bool IsVehicle(const GameObject* pObj);

    bool IsValidEnemy(const GameObject* pMe, const GameObject* pObj);
}

namespace CombatUtil
{
    bool HasNoWeapons(const Controllable* pMe);

    bool EngageInCombat(const UnitController* pCtrl, const GameObject* pTarget, bool bFlag);

    bool CanFaceDisturbance(const GameObject* pObj, const PblVector3& vPos);
}

namespace CombatUtil { float GetCombatRange(const Controllable* pMe); }

namespace TeamManager
{

    int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                          int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);

    int GetApproxObjectsAlongLine(const PblVector3& vStart, const PblVector3& vEnd, float fRadius,
                                  GameObject** const ppObjects, int iMax, const GameObject* pIgnore,
                                  AI::AIAffil eAffil);
}

namespace ReserveManager
{

    int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType);

    void Reserve(const GameObject* pObj, const GameObject* pReserver, ReserveType eType);
}

namespace VisionManager
{

    float MaxVisibleDist(const GameObject* pSeer, const GameObject* pTarget);
}

namespace ControllerManager
{

    void BroadcastEvent(AI::EventInfo* pInfo, const PblVector3& vPos, float fRadius,
                        const UnitController* pCtrl, const Team* pTeam, AI::AIAffil eAffil);
}

namespace
{

    enum
    {
        AILOD_NORMAL = 3
    };

    enum
    {
        SOLDIER_REPAIR = 2,
        SOLDIER_PILOT = 3
    };

    const PblVector3& GetTrans(const PblMatrix& m)
    {
        return *reinterpret_cast<const PblVector3*>(&m.m[3][0]);
    }

    int GetTeam(const GameObject* pObj)
    {
    return pObj->GetTeam();
    }

    unsigned int GetHandleId(const GameObject* pObj)
    {
        return pObj->mHandleId;
    }

    GameObject* ResolveHandle(PblHandle<GameObject>& handle)
    {
        GameObject* pObj = handle.mObject;
        if (pObj)
        {
            if (GetHandleId(pObj) == handle.mSavedHandleId)
            {
                return pObj;
            }
            handle.mObject = 0;
            handle.mSavedHandleId = 0;
        }
        return 0;
    }

    PblHandle<const GameObject> MakeHandle(const GameObject* pObj)
    {
        PblHandle<const GameObject> handle;
        handle.mObject = pObj;
        handle.mSavedHandleId = pObj ? GetHandleId(pObj) : 0;
        return handle;
    }

    const Entity* AsEntity(const GameObject* pObj)
    {
        return reinterpret_cast<const Entity*>(pObj);
    }

    Entity* AsEntity(GameObject* pObj)
    {
        return reinterpret_cast<Entity*>(pObj);
    }

    const CollisionObject* GetCollisionObject(const GameObject* pObj)
    {
        return static_cast<const CollisionObject*>(pObj);
    }

    float GetMaxSpeed(const GameObject* pObj)
    {
        return pObj->GetMaxSpeed();
    }

    const Controllable* GetControllable(const GameObject* pObj)
    {
        return pObj->GetControllable();
    }

    void HeardEnemy(GameObject* pObj)
    {
        pObj->HeardEnemy();
    }

    void Charge(GameObject* pObj)
    {
        pObj->Charge();
    }

    int GetSoldierType(const GameObject* pObj)
    {
        return pObj->GetGameObjectClass()->GetSoldierType();
    }

    const Controllable* FirstControllable(const GameObject* pObj)
    {
        const PblList<Controllable>& list =
            pObj->mControllableList;
        return list.GetFirst()->_pObject;
    }

    float GetVehicleBlockTime(const Character* pChr)
    {
        return pChr->mVehicleBlockTime;
    }

    void SetVehicleBlockTime(Character* pChr, float fTime)
    {
        pChr->mVehicleBlockTime = fTime;
    }

    AcklayData* GetAcklayData(const Entity* pEntity)
    {
        return static_cast<const EntitySoldier*>(pEntity)->mAcklayData;
    }

    const AttackHelper& GetAttackHelper(const UnitAgent* pAgent)
    {
        return *reinterpret_cast<const AttackHelper*>(pAgent->mAttackHelper);
    }
}

// GLOBAL: SWBF2 0x01f7d0dc
// SYNTHETIC: SWBF2 0x00406bf0
// `dynamic initializer for 'UnitAgent::rttiHashUnitAgent''
PblHash UnitAgent::rttiHashUnitAgent("UnitAgent");
char UnitAgent::rttiNameUnitAgent[] = "UnitAgent";

// GLOBAL: SWBF2 0x01f9a1a8
// SYNTHETIC: SWBF2 0x00406c00
// `dynamic initializer for 'UnitAgent::sMemoryPool''
// SYNTHETIC: SWBF2 0x007696b0
// `dynamic atexit destructor for 'UnitAgent::sMemoryPool''
MemoryPool UnitAgent::sMemoryPool("UnitAgent", sizeof(UnitAgent));

bool Agent::IsRtti(unsigned int hash) const
{
    return hash == rttiHashAgent;
}

unsigned int Agent::GetDerivedRtti() const
{
    return rttiHashAgent;
}

const char* Agent::GetDerivedRttiName() const
{
    return rttiNameAgent;
}

// FUNCTION: SWBF2 0x0065dce0
bool UnitAgent::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashUnitAgent || Agent::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0065dd10
unsigned int UnitAgent::GetDerivedRtti() const
{
    return rttiHashUnitAgent;
}

// FUNCTION: SWBF2 0x0065dd20
const char* UnitAgent::GetDerivedRttiName() const
{
    return rttiNameUnitAgent;
}

// FUNCTION: SWBF2 0x0065dd40
bool UnitAgent::ShouldQueueCurCmd(const UnitCmdInfo& cmd) const
{
    return true;
}

// SYNTHETIC: SWBF2 0x0065dd50
// UnitAgent::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0065dd80
UnitAgent::UnitAgent(UnitController* pCtrl)
    : mCtrl(pCtrl),
      mCheckNearbyUnitsTime(-1.0f)
{
    new (mAttackHelper) AttackHelper(this);
}

// FUNCTION: SWBF2 0x0065ddb0
UnitAgent::~UnitAgent()
{
}

// FUNCTION: SWBF2 0x0065ddc0
void UnitAgent::Kill()
{
    Fsm::Kill();
}

// FUNCTION: SWBF2 0x0065ddd0
float UnitAgent::GetUpdateRate() const
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x0065dde0
void UnitAgent::UpdateState(float dt)
{
    UpdateStatePostVision(dt);
}

const Controllable* UnitAgent::GetMe() const
{
    return mCtrl->mOwner;
}

Controllable* UnitAgent::GetMe()
{
    return mCtrl->mOwner;
}

AILowLevel& UnitAgent::GetLowLevel()
{
    return *reinterpret_cast<AILowLevel*>(mCtrl->mLowLevel);
}

// FUNCTION: SWBF2 0x0065de00
void UnitAgent::FaceTargetCommon(ControllableHeight eHeight, bool bFlag, const PblVector3& vTarget)
{
    if (eHeight != HEIGHT_COUNT)
    {
        GetLowLevel().SetHeight(eHeight);
    }

    const AttackHelper& attack = GetAttackHelper(this);
    if (attack.GetVisibleTarget())
    {
        GetLowLevel().SetHeading(attack.GetVisibleTarget());
    }
    else if (attack.HasTarget())
    {
        const PblVector3& vLastSeen = attack.GetLastSeenTargetPos();
        GetLowLevel().SetHeading(PblVector3(vLastSeen.x + 0.0f, vLastSeen.y + 1.6f, vLastSeen.z + 0.0f), 0.0f);
    }
    else if (bFlag)
    {
        GetLowLevel().SetHeading(vTarget, 0.0f);
    }
    else
    {
        GetLowLevel().SetHeadingDefault();
    }
}

// FUNCTION: SWBF2 0x0065dee0
void UnitAgent::FaceTarget()
{
    FaceTargetCommon(HEIGHT_COUNT, false, PblVector3(0, 0, 0));
}

// FUNCTION: SWBF2 0x0065df10
void UnitAgent::FaceTarget(ControllableHeight eHeight)
{
    FaceTargetCommon(eHeight, false, PblVector3(0, 0, 0));
}

// FUNCTION: SWBF2 0x0065df40
void UnitAgent::FaceTarget(ControllableHeight eHeight, const PblVector3& vTarget)
{
    FaceTargetCommon(eHeight, true, vTarget);
}

// FUNCTION: SWBF2 0x0065df60
bool UnitAgent::RunForVehicle(const GameObject* pVehicle) const
{
    PblVector3 vVehiclePos = GetTrans(GetCollisionObject(pVehicle)->GetMatrix());
    const GameObject* pMe = GetMe()->GetGameObject();
    GameObject* pObjects[30];
    int iCount = TeamManager::GetApproxObjectsAlongLine(GetTrans(GetMe()->GetMatrix()), vVehiclePos, 8.0f,
                                                        pObjects, 30, pMe, AI::AFFIL_ENEMY);
    for (int i = 0; i < iCount; ++i)
    {
        if (AIUtil::IsValidEnemy(pMe, pObjects[i]))
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0065e000
bool UnitAgent::AreNearbyUnitsInCombat()
{
    if (GameLoop::GetMissionTime() > mCheckNearbyUnitsTime)
    {
        mCheckNearbyUnitsTime = GameLoop::GetMissionTime();
        mNearbyUnitsInCombat = false;

        GameObject* pMe = GetMe()->GetGameObject();
        if (AsEntity(pMe)->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            GameObject* pObjects[30];
            int iCount = TeamManager::GetObjectsInRange(GetTrans(GetMe()->GetMatrix()), 7.0f, pObjects, 30,
                                                        pMe, AI::AFFIL_ALLY);
            int iInCombat = 0;
            for (int i = 0; i < iCount && iInCombat < 3; ++i)
            {
                const GameObject* pObj = pObjects[i];
                if (AsEntity(pObj)->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
                    GetControllable(pObj) && GetControllable(pObj)->mCtrl &&
                    GetControllable(pObj)->mCtrl->IsRtti(UnitController::rttiHashUnitController) &&
                    static_cast<const UnitController*>(GetControllable(pObj)->mCtrl)->IsInCombat())
                {
                    ++iInCombat;
                }
            }
            if (iInCombat == 3)
            {
                Charge(GetMe()->GetGameObject());
                mNearbyUnitsInCombat = true;
            }
        }
    }
    return mNearbyUnitsInCombat;
}

// FUNCTION: SWBF2 0x0065e140
bool UnitAgent::ShouldAgentGetInNearbyVehicle(const GameObject* pVehicle) const
{
    if (!GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return false;
    }
    if (!GetMe()->CanEnterVehicle(pVehicle))
    {
        return false;
    }

    int iTeam = GetMe()->mCharacter->mTeamNumber;
    if (ReserveManager::GetOpenReservableSlots(pVehicle, iTeam, RT_BOARD) < 1)
    {
        return false;
    }

    Team* pTeam = Team::sTeams[iTeam];
    if (pTeam && pTeam->GetAIMembersIgnoreVehicles())
    {
        return false;
    }

    if (GameLoop::GetMissionTime() < GetVehicleBlockTime(GetMe()->mCharacter))
    {
        return false;
    }

    if (GetMaxSpeed(pVehicle) > 0.0f)
    {
        return true;
    }

    if (AIUtil::gStayInTurrets)
    {
        int eType = GetSoldierType(GetMe()->GetGameObject());
        return eType != SOLDIER_REPAIR && eType != SOLDIER_PILOT;
    }

    if (GetAttackHelper(this).HasTarget())
    {
        return CombatUtil::CanFaceDisturbance(pVehicle, GetAttackHelper(this).GetLastSeenTargetPos());
    }
    return false;
}

// FUNCTION: SWBF2 0x0065e280
void UnitAgent::PlayDamageEventVO(AI::EventInfo* pEvent)
{
    AI::EventInfo_Damaged* pInfo = static_cast<AI::EventInfo_Damaged*>(pEvent);
    GameObject* pMe = GetMe()->GetGameObject();
    const Character* pOwner = pInfo->mDamageOwner;
    if (pOwner->mTeamPtr->GetAffiliation(GetTeam(pMe)) > 0 && pOwner->GetControllable())
    {
        AI::VOHelper::Play(AI::VOT_FriendlyFire, pMe, 0, pOwner->GetControllable()->GetGameObject(), 0.0f);
    }
}

// FUNCTION: SWBF2 0x0065e320
int UnitAgent::DamagedEventPlayerCentricness(AI::EventInfo* pEvent)
{
    AI::EventInfo_Damaged* pInfo = static_cast<AI::EventInfo_Damaged*>(pEvent);
    if (pInfo->mType != AI::EVT_Damaged)
    {
        return 0;
    }

    if (!pInfo->mIsPropagated && pInfo->mDamageOwner && pInfo->mDamageOwner->GetControllable())
    {
        const PblVector3& vAttacker = GetTrans(pInfo->mDamageOwner->GetControllable()->GetMatrix());
        const PblVector3& vMe = GetTrans(GetMe()->GetMatrix());
        float fDX = vAttacker.x - vMe.x;
        float fDY = vAttacker.y - vMe.y;
        float fDZ = vAttacker.z - vMe.z;
        if (fDY * fDY + fDX * fDX + fDZ * fDZ > 2500.0f)
        {
            AI::EventInfo_Damaged info(*pInfo);
            info.mIsPropagated = true;
            ControllerManager::BroadcastEvent(&info, vMe, 20.0f, mCtrl, 0, AI::AFFIL_ALL);
        }
    }

    if (!pInfo->mDamageOwner || !pInfo->mDamageOwner->GetControllable())
    {
        return 0;
    }
    if (pInfo->mDamageOwner->mPlayerId < 0)
    {
        return 0;
    }

    GameObject* pMe = GetMe()->GetGameObject();
    if (Team::sTeams[GetTeam(pMe)]->GetAffiliation(pInfo->mDamageOwner->mTeamNumber) >= 0)
    {
        return 0;
    }

    GameObject* pAttacker = pInfo->mDamageOwner->GetControllable()->GetGameObject();
    reinterpret_cast<AI::UnitThreatManager*>(mCtrl->mThreatManager)->AddThreat(pAttacker);
    mCtrl->ForceSetAttackHelperTarget(pAttacker);
    return 1;
}

// SYNTHETIC: SWBF2 0x0065e4c0
// AI::EventInfo_Damaged::EventInfo_Damaged(const AI::EventInfo_Damaged&)

// FUNCTION: SWBF2 0x0065e530
int UnitAgent::HandleDamagedEvent(AI::EventInfo* pEvent)
{
    AI::EventInfo_Damaged* pInfo = static_cast<AI::EventInfo_Damaged*>(pEvent);
    DamagedEventPlayerCentricness(pInfo);
    PlayDamageEventVO(pInfo);

    GameObject* pMe = GetMe()->GetGameObject();
    const Character* pOwner = pInfo->mDamageOwner;
    if (pOwner && pOwner->mTeamPtr->GetAffiliation(GetTeam(pMe)) >= 0)
    {
        return 0;
    }

    if (GetMe()->GetControllableClass() &&
        !GetMe()->GetControllableClass()->DoesCombatInterrupt())
    {
        return 0;
    }

    if (GetMe()->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        PblVector3 vPos = GetTrans(GetMe()->GetMatrix());
        float fRadius = AIUtil::GetCollisionRadius(GetMe()->GetGameObject());
        if (FlyRegionManager::IsInside(vPos, 15.0f > fRadius ? 15.0f : fRadius))
        {
            return 0;
        }
    }

    if (CombatUtil::HasNoWeapons(GetMe()) || !pOwner || !pOwner->GetControllable())
    {
        return 0;
    }

    const GameObject* pAttacker = pOwner->GetControllable()->GetGameObject();
    PblVector3 vMyPos = GetTrans(GetMe()->GetMatrix());
    PblVector3 vAttackerPos = GetTrans(pOwner->GetControllable()->GetMatrix());
    float fRange = VisionManager::MaxVisibleDist(pAttacker, pMe);
    float fDX = vAttackerPos.x - vMyPos.x;
    float fDY = vAttackerPos.y - vMyPos.y;
    float fDZ = vAttackerPos.z - vMyPos.z;
    if (fRange * fRange > fDY * fDY + fDX * fDX + fDZ * fDZ &&
        CombatUtil::EngageInCombat(mCtrl, pAttacker, false))
    {
        pInfo->mNextCmd = UnitCombatInfo(UnitCombatInfo::TURN, vAttackerPos, true);
        if (pMe)
        {
            if (GetMaxSpeed(pMe) == 0.0f && !AIUtil::gStayInTurrets && !CombatUtil::CanFaceDisturbance(pMe, vAttackerPos))
            {
                GetLowLevel().SetExitVehicle(0);
                pInfo->mNextCmd = UnitCmdInfo(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                                              PblHandle<GameObject>());
            }
        }
        reinterpret_cast<AI::UnitThreatManager*>(mCtrl->mThreatManager)->AddThreat(pInfo->mDamageOwner->GetControllable()->GetGameObject());
        return 1;
    }

    if (AIUtil::GetEntityType(AsEntity(GetMe()->GetGameObject())) != AIUtil::ENTITY_SOLDIER)
    {
        return 0;
    }

    bool bAllowRoll = pInfo->mAmountLost > 0.0f;
    PblVector3 vRand = PblRandom::GetGlobalVector3();
    pInfo->mNextCmd = UnitCombatInfo(UnitCombatInfo::FLEE_EVENT,
                                     PblVector3(vRand.x + vMyPos.x, vRand.y + vMyPos.y, vRand.z + vMyPos.z),
                                     bAllowRoll);
    return 1;
}

// FUNCTION: SWBF2 0x0065e880
int UnitAgent::HandleSquadCommandEvent(AI::EventInfo* pInfo)
{
    for (int i = CMD_SC_START; i <= CMD_SC_END; ++i)
    {
        mCtrl->RemoveQueuedCommands(i);
    }

    if (pInfo->mType == AI::EVT_SC_FollowChr || pInfo->mType == AI::EVT_SC_FollowChrLoose)
    {
        pInfo->mNextCmd = UnitFollowInfo(pInfo->mGameObject, pInfo->mGameObject,
                                         pInfo->mType == AI::EVT_SC_FollowChr);
        Controllable* pMe = GetMe();
        AI::VOHelper::Play(AI::VOT_SC_FollowResponse, pMe->GetGameObject(), 0, 0,
                           AIUtil::GetRandomFloat(0.5f, 1.2f) + GameLoop::GetMissionTime());
        return 1;
    }

    if (pInfo->mType == AI::EVT_SC_GetIntoVehicle)
    {
        int iVOHandle = -1;
        if (FirstControllable(ResolveHandle(pInfo->mGameObject)) &&
            FirstControllable(ResolveHandle(pInfo->mGameObject))->mPlayerId >= 0)
        {
            Controllable* pMe = GetMe();
            GameObject* pVehicle = ResolveHandle(pInfo->mGameObject);
            iVOHandle = AI::VOHelper::Play(AI::VOT_SC_GetInResponse, pMe->GetGameObject(), 0, pVehicle,
                                           AIUtil::GetRandomFloat(0.5f, 1.2f) + GameLoop::GetMissionTime());
        }
        pInfo->mNextCmd = UnitBoardInfo(pInfo->mGameObject, iVOHandle);
        return 1;
    }

    if (pInfo->mType == AI::EVT_SC_GetOutOfVehicle)
    {
        pInfo->mNextCmd = UnitUnBoardInfo();
        if (GetMe() && GetMe()->mCharacter)
        {
            SetVehicleBlockTime(GetMe()->mCharacter, GameLoop::GetMissionTime() + 20.0f);
        }
        return 1;
    }

    return 0;
}

// FUNCTION: SWBF2 0x0065eab0
int UnitAgent::Handle_Vision_CanSeeEnemy(AI::EventInfo* pInfo)
{
    GameObject* pEnemy = ResolveHandle(pInfo->mGameObject);

    if (GetMe()->GetControllableClass() &&
        !GetMe()->GetControllableClass()->DoesCombatInterrupt())
    {
        return AI::EVT_Ignored;
    }

    if (AIUtil::GetEntityType(GetMe()->GetEntity()) == AIUtil::ENTITY_DROIDEKA)
    {
        const PblVector3& vMe = GetTrans(GetMe()->GetMatrix());
        const PblVector3& vEnemy = GetTrans(GetCollisionObject(pEnemy)->GetMatrix());
        float fDX = vEnemy.x - vMe.x;
        float fDY = vEnemy.y - vMe.y;
        float fDZ = vEnemy.z - vMe.z;
        if (fDY * fDY + fDX * fDX + fDZ * fDZ > 100.0f)
        {
            return 0;
        }
    }

    if (!CombatUtil::EngageInCombat(static_cast<const UnitController*>(GetMe()->mCtrl), pEnemy, false))
    {
        return 0;
    }

    pInfo->mNextCmd = UnitCombatInfo(UnitCombatInfo::ENGAGE, MakeHandle(pEnemy));
    return 1;
}

// FUNCTION: SWBF2 0x0065ebe0
int UnitAgent::Handle_Audible_CanHearEnemy(AI::EventInfo* pInfo)
{
    if (AIUtil::IsVehicle(GetMe()->GetGameObject()))
    {
        return AI::EVT_Ignored;
    }

    if (GetMe()->GetControllableClass() &&
        !GetMe()->GetControllableClass()->DoesCombatInterrupt())
    {
        return AI::EVT_Ignored;
    }

    if (GetMe()->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        PblVector3 vPos = GetTrans(GetMe()->GetMatrix());
        float fRadius = AIUtil::GetCollisionRadius(GetMe()->GetGameObject());
        if (FlyRegionManager::IsInside(vPos, 15.0f > fRadius ? 15.0f : fRadius))
        {
            return AI::EVT_Ignored;
        }
    }

    if (CombatUtil::HasNoWeapons(GetMe()))
    {
        return AI::EVT_Ignored;
    }

    if (CombatUtil::EngageInCombat(mCtrl, ResolveHandle(pInfo->mGameObject), false))
    {
        pInfo->mNextCmd = UnitCombatInfo(UnitCombatInfo::INVESTIGATE, MakeHandle(ResolveHandle(pInfo->mGameObject)));
        HeardEnemy(GetMe()->GetGameObject());
        return 1;
    }

    PblVector3 vMyPos = GetTrans(GetMe()->GetMatrix());
    PblVector3 vEnemyPos = GetTrans(GetCollisionObject(ResolveHandle(pInfo->mGameObject))->GetMatrix());
    PblVector3 vDiff(vEnemyPos.x - vMyPos.x, vEnemyPos.y - vMyPos.y, vEnemyPos.z - vMyPos.z);
    float fRange = CombatUtil::GetCombatRange(GetMe());
    if (AIUtil::GetEntityType(GetMe()->GetEntity()) == AIUtil::ENTITY_DROIDEKA)
    {
        fRange *= 0.5f;
    }
    if (fRange * fRange > vDiff.z * vDiff.z + vDiff.x * vDiff.x)
    {
        GameObject* pMe = GetMe()->GetGameObject();
        if (pMe && GetMaxSpeed(pMe) == 0.0f && !AIUtil::gStayInTurrets && !CombatUtil::CanFaceDisturbance(pMe, vEnemyPos))
        {
            GetLowLevel().SetExitVehicle(0);
            return 1;
        }
    }
    return AI::EVT_Ignored;
}

// FUNCTION: SWBF2 0x0065ee60
int UnitAgent::EventHandler(AI::EventInfo* pInfo)
{
    switch (pInfo->mType)
    {
    case AI::EVT_EmptyVehicle:
        if (!ShouldAgentGetInNearbyVehicle(ResolveHandle(pInfo->mGameObject)))
        {
            return 0;
        }
        pInfo->mNextCmd = UnitBoardInfo(pInfo->mGameObject, -1);
        ReserveManager::Reserve(ResolveHandle(pInfo->mGameObject), GetMe()->GetGameObject(), RT_BOARD);
        return 1;

    case AI::EVT_Grenade:
        if (GetMe()->GetControllableClass() &&
            !GetMe()->GetControllableClass()->DoesCombatInterrupt())
        {
            return 0;
        }
        if (AIUtil::GetEntityType(AsEntity(GetMe()->GetGameObject())) != AIUtil::ENTITY_SOLDIER)
        {
            return 0;
        }
        if (GetAcklayData(AsEntity(GetMe()->GetGameObject())->GetEntity()) != 0)
        {
            return AI::EVT_Ignored;
        }
        if (CombatUtil::HasNoWeapons(GetMe()))
        {
            return 0;
        }
        pInfo->mNextCmd = UnitCombatInfo(UnitCombatInfo::FLEE_EVENT, pInfo->mPos, true);
        return 1;

    case AI::EVT_Damaged:
        return HandleDamagedEvent(pInfo);

    case AI::EVT_Vehicle_WaitForPickUp:
        pInfo->mNextCmd = UnitWaitSecondaryInfo(pInfo->mFloat, false);
        return 1;

    case AI::EVT_Vision_CanSeeEnemy:
        return Handle_Vision_CanSeeEnemy(pInfo);

    case AI::EVT_Audible_CanHearEnemy:
        return Handle_Audible_CanHearEnemy(pInfo);

    case AI::EVT_Path_PathFollowerStuck:
        if (GetMe()->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) ||
            GetMe()->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka))
        {
            pInfo->mNextCmd = UnitRandomInfo();
            return 1;
        }
        return 0;

    case AI::EVT_Path_PathResult:
        GetLowLevel().ReceivePathResult(static_cast<AIPath*>(pInfo->mVoidPtr));
        return 1;

    case AI::EVT_Path_DynamicCongraphChange:
        if (!GetMe() || !GetMe()->GetGameObject() || !AIUtil::IsVehicle(GetMe()->GetGameObject()))
        {
            return AI::EVT_Ignored;
        }
        pInfo->mNextCmd = UnitCmdInfo(CMD_WAIT, PblHandle<GameObject>(), PblVector3(0, 0, 0), PblHandle<GameObject>());
        return 1;

    case AI::EVT_Nav_NavigatorFinished:
        if (mCtrl->mLodState >= AILOD_NORMAL)
        {
            ControllerManager::RequestControllerUpdateASAP(mCtrl);
        }
        return 1;

    case AI::EVT_SC_FollowChr:
    case AI::EVT_SC_FollowChrLoose:
    case AI::EVT_SC_GetIntoVehicle:
    case AI::EVT_SC_GetOutOfVehicle:
        return HandleSquadCommandEvent(pInfo);

    case AI::EVQ_GetSkillModifier:
        pInfo->mInt = 0;
        return 0;

    case AI::EVT_EndInvisibility:
        pInfo->mNextCmd = UnitCombatInfo(UnitCombatInfo::TURN,
                                         GetCollisionObject(ResolveHandle(pInfo->mGameObject))->GetPosition(), true);
        return 1;
    }
    return 0;
}
