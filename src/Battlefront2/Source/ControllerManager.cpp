#include <stdlib.h>

#include "ControllerManager.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "PlayerController.h"
#include "Thread.h"
#include "UnitController.h"
#include "AIUtil.h"
#include "AIVisRegion.h"
#include "Adrenaline.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "Entity.h"
#include "GameObject.h"
#include "Team.h"
#include "PathManager.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "AIDifficulty.h"
#include "FlagItem.h"
#include "PlayerProfile.h"
#include "SpaceAssault.h"
#include "SpawnManager.h"
#include "CameraManager.h"
#include "EntityCarrier.h"
#include "WeaponCannonInstance.h"
#include "Damageable.h"
#include "Ordnance.h"
#include "CombatUtil.h"

namespace ControllerManager {
// GLOBAL: SWBF2 0x01e30930
PblList<PlayerController>* sPlayerCtrls;
// GLOBAL: SWBF2 0x01e30934
PblOrderedList<UnitController, float>* sAICtrls;
// GLOBAL: SWBF2 0x01e30928
int sFrameCount;
// GLOBAL: SWBF2 0x01e3093c
float sPeriodicUpdateTime;
// GLOBAL: SWBF2 0x01e30924
int sHostFrameCount;
// GLOBAL: SWBF2 0x01e30938
float sHostPeriodicUpdateTime;
// GLOBAL: SWBF2 0x01e30920
int sClientFrameCount;
// GLOBAL: SWBF2 0x01e3092c
float sClientPeriodicUpdateTime;
}

// GLOBAL: SWBF2 0x01f9c290
PblList<PlayerController>* sHostPlayerCtrls;
// GLOBAL: SWBF2 0x01f9c284
PblOrderedList<UnitController, float>* sHostAICtrls;
// GLOBAL: SWBF2 0x01f9c288
PblList<PlayerController>* sClientPlayerCtrls;
// GLOBAL: SWBF2 0x01f9c28c
PblOrderedList<UnitController, float>* sClientAICtrls;

static PblOrderedList<UnitController, float>::Node* GetManagerNode(UnitController* pCtrl)
{
    return (PblOrderedList<UnitController, float>::Node*)pCtrl->mManagerNode;
}

// FUNCTION: SWBF2 0x00485b30
void ControllerManager::EnterClient()
{
    sPlayerCtrls = sClientPlayerCtrls;
    sAICtrls = sClientAICtrls;
    sHostPeriodicUpdateTime = sPeriodicUpdateTime;
    sHostFrameCount = sFrameCount;
    sFrameCount = sClientFrameCount;
    sPeriodicUpdateTime = sClientPeriodicUpdateTime;
}

// FUNCTION: SWBF2 0x00485b80
void ControllerManager::LeaveClient()
{
    sPlayerCtrls = sHostPlayerCtrls;
    sAICtrls = sHostAICtrls;
    sClientPeriodicUpdateTime = sPeriodicUpdateTime;
    sClientFrameCount = sFrameCount;
    sFrameCount = sHostFrameCount;
    sPeriodicUpdateTime = sHostPeriodicUpdateTime;
}

// FUNCTION: SWBF2 0x00485bd0
void ControllerManager::Init()
{
    sPlayerCtrls = new PblList<PlayerController>;
    sAICtrls = new PblOrderedList<UnitController, float>;
    sFrameCount = 0;
    sPeriodicUpdateTime = 0.0f;
    sHostPlayerCtrls = sPlayerCtrls;
    sHostAICtrls = sAICtrls;
    EnterClient();

    sPlayerCtrls = new PblList<PlayerController>;
    sAICtrls = new PblOrderedList<UnitController, float>;
    sClientPlayerCtrls = sPlayerCtrls;
    sClientAICtrls = sAICtrls;
    LeaveClient();
}

// FUNCTION: SWBF2 0x00485ce0
void ControllerManager::Reset()
{
    sPlayerCtrls = 0;
    sAICtrls = 0;
}

// FUNCTION: SWBF2 0x00485d00
void ControllerManager::AddController(Controller* pCtrl)
{
    if (pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
    {
        PlayerController* pPlayer = (PlayerController*)pCtrl;
        sPlayerCtrls->AddLast(&pPlayer->mManagerNode, pPlayer);
    }
    else if (pCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        UnitController* pUnit = (UnitController*)pCtrl;
        sAICtrls->Add(GetManagerNode(pUnit), pUnit, 0.0f);
    }
}

// FUNCTION: SWBF2 0x00485d60
void ControllerManager::RemoveController(Controller* pCtrl)
{
    if (pCtrl->IsRtti(PlayerController::rttiHashPlayerController))
    {
        PlayerController* pPlayer = (PlayerController*)pCtrl;
        sPlayerCtrls->Remove(&pPlayer->mManagerNode);
    }
    else if (pCtrl->IsRtti(UnitController::rttiHashUnitController))
    {
        sAICtrls->Remove(GetManagerNode((UnitController*)pCtrl));
    }
}

// FUNCTION: SWBF2 0x004862f0
void ControllerManager::RequestControllerUpdateASAP(UnitController* pCtrl)
{
    float fTime = GameLoop::GetMissionTime() + 0.0001f;
    if (fTime > pCtrl->mNextUpdateTime)
    {
        fTime = pCtrl->mNextUpdateTime;
    }
    pCtrl->mNextUpdateTime = fTime;

    PblOrderedList<UnitController, float>::Node* pNode = GetManagerNode(pCtrl);
    sAICtrls->Remove(pNode);
    sAICtrls->Add(pNode, pCtrl, pCtrl->mNextUpdateTime);
}

// FUNCTION: SWBF2 0x004865a0
void ControllerManager::BroadcastEvent(AI::EventInfo* pInfo, int iMaxCount)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }

    PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext;
    while (UnitController* pCtrl = pNode->_pObject)
    {
        pNode = pNode->_pNext;
        if (pCtrl->SendEvent(pInfo) == 1 && --iMaxCount <= 0)
        {
            break;
        }
    }
}

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
}

class EntitySoldier
{
public:
    static PblHash rttiHashEntitySoldier;
};

namespace VehicleManager
{

void Update();
}

namespace TeamManager
{

int GetObjectsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const GameObject* pIgnore, AI::AIAffil eAffil);
}

namespace AI { namespace VOHelper { void Update(); } }
namespace AIDifficulty { void UpdateAutoBalance(); }
namespace VisionManager { bool RayTests(int i); }

namespace AI
{
enum VOType
{
    VOT_NONE = 0
};

namespace VOHelper
{
struct QueuedSound
{
    VOType mType;
    PblHandle<const GameObject> mSpeaker;
    PblHandle<const GameObject> mPlayer;
    PblHandle<const GameObject> mTarget;
    PblVector3 mPos;
    bool mUseTarget;
    float mTime;
    int mHandle;

    ~QueuedSound();
};
extern QueuedSound mSndQueue[20];

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);

int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const PblVector3& vPos, float fDelay);
}
}

namespace ControllerManager
{
int GetPlayersInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                      int iMax, const Team* pTeam, AI::AIAffil eAffil);
int GetAIInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                 int iMax, const Team* pTeam, AI::AIAffil eAffil);
int GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                    int iMax, const Team* pTeam, AI::AIAffil eAffil);
void BroadcastEvent(AI::EventInfo* pInfo, const PblVector3& vPos, float fRadius,
                    const UnitController* pSender, const Team* pTeam, AI::AIAffil eAffil);

template <class T> void UpdateControllerAudibles(T* pCtrl);
}

namespace AudibleManager
{
void UpdateAudience(Controller* pController);
}

namespace
{

int GetTeam(const GameObject* pObject)
{
    return pObject->mTeam;
}

int GetPerceivedTeam(const GameObject* pObject)
{
    return pObject->mPerceivedTeam;
}

const CollisionObject* GetCollisionObject(const GameObject* pObject)
{
    return static_cast<const CollisionObject*>(pObject);
}

Controllable* GetControllable(GameObject* pObject)
{
    return pObject->GetControllable();
}

PblList<Controllable>& GetControllableList(GameObject* pObject)
{
    return pObject->mControllableList;
}

float DistanceSq(const PblVector3& a, const float* b)
{
    float dx = b[0] - a.x;
    float dy = b[1] - a.y;
    float dz = b[2] - a.z;
    return dx * dx + dy * dy + dz * dz;
}
}

namespace ControllerManager
{
// FUNCTION: SWBF2 0x00485e00
static void sUpdateFriendlyFire()
{
    if (sPlayerCtrls->GetCount() > 8)
    {
        return;
    }

    for (PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext;
         UnitController* pCtrl = pNode->_pObject; pNode = pNode->_pNext)
    {
        pCtrl->ResetFriendlyList();
    }

    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); (PlayerController*)it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (!pOwner || !pOwner->GetGameObject())
        {
            continue;
        }

        GameObject* pPlayer = it->mOwner->GetGameObject();
        GameObject* apObjects[20];
        int iCount = TeamManager::GetObjectsInRange(
            *reinterpret_cast<const PblVector3*>(GetCollisionObject(pPlayer)->GetMatrix().m[3]),
            50.0f, apObjects, 20, pPlayer, AI::AFFIL_ALLYNEUTRAL);
        for (int i = 0; i < iCount; ++i)
        {
            GameObject* pObject = apObjects[i];
            if (!GetControllable(pObject) || !GetControllable(pObject)->mCtrl ||
                !GetControllable(pObject)->mCtrl->IsRtti(UnitController::rttiHashUnitController))
            {
                continue;
            }

            for (PblList<Controllable>::Itor itC(GetControllableList(pObject)); (Controllable*)itC; ++itC)
            {
                if (itC->mCtrl && itC->mCtrl->IsRtti(UnitController::rttiHashUnitController))
                {
                    ((UnitController*)itC->mCtrl)->AddFriendlyFire(pPlayer);
                }
            }
        }
    }
}
}

// FUNCTION: SWBF2 0x00485f60
void ControllerManager::UpdatePilotSelf()
{
    for (PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext;
         UnitController* pCtrl = pNode->_pObject; pNode = pNode->_pNext)
    {
        Controllable* pOwner = pCtrl->mOwner;
        if (!pOwner)
        {
            continue;
        }
        if (pOwner->mPilotType != PILOT_SELF &&
            !(pOwner->mPilotType == PILOT_VEHICLESELF && (pOwner->mPilot == pOwner || !pOwner->mPilot)))
        {
            continue;
        }
        if (pOwner->mCharacter || pCtrl->GetCurCmdType() || pCtrl->IsNewCommand())
        {
            continue;
        }

        UnitCmdInfo cmd(CMD_DEATHMATCH, PblHandle<GameObject>(), PblVector3(0.0f, 0.0f, 0.0f),
                        PblHandle<GameObject>());
        pNode->_pObject->QueueCommand(cmd);
    }
}

// FUNCTION: SWBF2 0x00486040
void ControllerManager::UpdateCrowd(Controller* pCtrl)
{
    if (pCtrl->mCrowdTestTimer + 1.0f > GameLoop::GetMissionTime())
    {
        return;
    }

    Controllable* pOwner = pCtrl->mOwner;
    pCtrl->mCrowdEnemies = 0;
    pCtrl->mCrowdAllies = 0;
    if (!pOwner || !pOwner->GetGameObject())
    {
        return;
    }

    const Team* pTeam = Team::sTeams[GetTeam(pOwner->GetGameObject())];
    const PblVector3& vPos = *reinterpret_cast<const PblVector3*>(pOwner->GetMatrix().m[3]);

    for (PblList<PlayerController>::Node* pNode = sPlayerCtrls->_head._pNext; pNode->_pObject;
         pNode = pNode->_pNext)
    {
        PlayerController* pOther = pNode->_pObject;
        Controllable* pOtherOwner = pOther->mOwner;
        if (pOther == pCtrl || !pOtherOwner || !pOtherOwner->GetGameObject() ||
            !pOtherOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            continue;
        }
        if (DistanceSq(vPos, pOtherOwner->GetMatrix().m[3]) < 81.0f)
        {
            if (pTeam->GetAffiliation(GetTeam(pOtherOwner->GetGameObject())) < 0)
            {
                ++pCtrl->mCrowdEnemies;
            }
            else
            {
                ++pCtrl->mCrowdAllies;
            }
        }
    }

    for (PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext; pNode->_pObject;
         pNode = pNode->_pNext)
    {
        UnitController* pOther = pNode->_pObject;
        Controllable* pOtherOwner = pOther->mOwner;
        if (pOther == pCtrl || !pOtherOwner || !pOtherOwner->GetGameObject() ||
            !pOtherOwner->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier))
        {
            continue;
        }
        if (DistanceSq(vPos, pOtherOwner->GetMatrix().m[3]) < 81.0f)
        {
            if (pTeam->GetAffiliation(GetTeam(pOtherOwner->GetGameObject())) < 0)
            {
                ++pCtrl->mCrowdEnemies;
            }
            else
            {
                ++pCtrl->mCrowdAllies;
            }
        }
    }
}

namespace AIDifficulty { float SpaceAssault_GetFlyerKillForPlayerTeam(); }
namespace AIDifficulty { float SpaceAssault_GetCritSysKillForPlayerTeam(); }

namespace AIDifficulty {

// FUNCTION: SWBF2 0x00439620
static GameObject* GetRandomFlyer(const Team* pTeam, AI::AIAffil eAffil)
{
    GameObject* apObjects[40];
    const PblVector3 vOrigin(0.0f, 0.0f, 0.0f);
    const int iCount = ControllerManager::GetAIInRange(vOrigin, 3.402823466e38f,
                                                         apObjects, 40, pTeam, eAffil);
    for (int i = iCount - 1; i >= 0; --i)
    {
        GameObject* pObject = apObjects[i];
        if (!pObject || !pObject->GetControllable() ||
            !pObject->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            continue;
        }
        if (!CameraManager::Instance()->IsInsideAnyCamera(
                GetCollisionObject(pObject)->GetPosition(), 100.0f, 3000.0f) &&
            AIUtil::CountHumansInVehicle(pObject->GetControllable()) <= 0)
        {
            return pObject;
        }
    }
    return 0;
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x004396e0
static void SpaceAssault_RandomFlyerKill(const Team* pTeam)
{
    GameObject* pVictim = AIDifficulty::GetRandomFlyer(pTeam, AI::AFFIL_ALLY);
    if (!pVictim)
    {
        return;
    }
    GameObject* pKiller = AIDifficulty::GetRandomFlyer(pTeam, AI::AFFIL_ENEMY);
    if (!pKiller)
    {
        return;
    }
    Controllable* pCtrl = pKiller->GetControllable();
    DamageOwner owner(pCtrl->mCharacter, pKiller, pKiller->GetTeam(),
                      pKiller->mEntityClass, pCtrl->GetCurWpn()->mClass, 1.0f);
    pVictim->InstantDeath(owner, true);
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x00439770
static void SpaceAssault_RandomCritSysDamage(const Team* pTeam)
{
    GameObject* pFlyer = AIDifficulty::GetRandomFlyer(pTeam, AI::AFFIL_ENEMY);
    if (!pFlyer)
    {
        return;
    }
    GameObject* pTarget = SpaceAssault::mInstance->GetRandomCritSys(
        pTeam, AIDifficulty::mCritSysBias);
    if (!pTarget)
    {
        return;
    }

    Controllable* pCtrl = pFlyer->GetControllable();
    DamageOwner owner(pCtrl->mCharacter, pFlyer, pFlyer->GetTeam(),
                      pFlyer->mEntityClass, 0, 1.0f);
    WeaponClass* pBestWeapon = 0;
    float fBestDamage = 0.0f;
    for (int i = 0; i < pCtrl->GetWeaponCount(); ++i)
    {
        Weapon* pWeapon = pCtrl->GetWeaponPtr(i);
        OrdnanceClass* pOrdnance = pWeapon->mClass->mOrdnanceClass;
        float fDamage = CombatUtil::CalcDamagePerSecond(pCtrl, pWeapon->mClass, pTarget);
        owner.mWeapon = pWeapon->mClass;
        float fMinDamage = 0.0f;
        float fShieldDamage = 0.0f;
        CombatUtil::GetAverageDamageValues(owner, pOrdnance, pTarget,
                                           fMinDamage, fShieldDamage);
        if (fDamage < fMinDamage * 2.0f)
        {
            fDamage = fMinDamage * 2.0f;
        }
        if (fDamage > fBestDamage)
        {
            fBestDamage = fDamage;
            pBestWeapon = pWeapon->mClass;
        }
    }
    if (!pBestWeapon)
    {
        return;
    }

    if (pTarget->mCurHealth > 0.0f)
    {
        float fCap = fBestDamage * 5.0f;
        if (fCap > 15000.0f)
        {
            fCap = 15000.0f;
        }
        if (pTarget->mCurHealth < fCap)
        {
            fCap = pTarget->mCurHealth;
        }
        fBestDamage = fCap;
    }
    owner.mWeapon = pBestWeapon;
    DamageDesc damage(owner, fBestDamage);
    pTarget->ApplyDamage(damage, ~0u);
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x00439930
void SpaceAssault_CheatLikeABastard()
{
    if (GameLoop::GetMissionTime() < 30.0f)
    {
        return;
    }

    float fIntervalScale = 1.0f;
    if (AIDifficulty::mAutoBalancePlayerModifier < 0)
    {
        fIntervalScale = -1.0f / (AIDifficulty::mAutoBalancePlayerModifier - 1);
    }

    const float fTime = GameLoop::GetMissionTime();
    if (fTime > AIDifficulty::mNextFlyerKillTime)
    {
        AIDifficulty::mNextFlyerKillTime = fTime + AIDifficulty::SpaceAssault_GetRandomFlyerKillTime() * fIntervalScale;
        const bool bEnemy = AIUtil::IsPlayerTeam(1) ^
            (AIUtil::GetRandomInt(0, 100) > AIDifficulty::SpaceAssault_GetFlyerKillForPlayerTeam() * 100.0f);
        AIDifficulty::SpaceAssault_RandomFlyerKill(Team::GetPtr(bEnemy ? 2 : 1));
    }

    if (fTime > AIDifficulty::mNextCritSysDamageTime)
    {
        AIDifficulty::mNextCritSysDamageTime = fTime + AIDifficulty::SpaceAssault_GetRandomCritSysDamageTime() * fIntervalScale;
        const float fRoll = (float)AIUtil::GetRandomInt(0, 100);
        const bool bEnemy = AIUtil::IsPlayerTeam(1) ^ (fRoll > AIDifficulty::SpaceAssault_GetCritSysKillForPlayerTeam() * 100.0f);
        AIDifficulty::SpaceAssault_RandomCritSysDamage(Team::GetPtr(bEnemy ? 2 : 1));
    }
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x00439320
int GetAutoBalanceMode()
{
    if (FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_1FLAG ||
        FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_2FLAG)
    {
        return 0;
    }
    if (FlagItem::sFlagGameplayType == FlagItem::FLAG_GAMEPLAY_CAMPAIGN)
    {
        return 3;
    }
    if (SpaceAssault::mInstance && SpaceAssault::mInstance->mbEnabled)
    {
        return 1;
    }
    if (Team::GetPtr(1)->GetCurReinforcements() >= 1000 || Team::GetPtr(2)->GetCurReinforcements() >= 1000)
    {
        return 3;
    }
    return 2;
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x00439380
bool AutoBalanceActive()
{
    return AIDifficulty::gAutoBalanceEnabled && AIDifficulty::mAutoBalanceEnabled && AIDifficulty::GetAutoBalanceMode() != 3;
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x004393b0
void GetAutoBalanceScores(int& iPlayerScore, int& iEnemyScore, int& iThreshold, int& iDivisor)
{
    bool bPlayerTeam1 = AIUtil::IsPlayerTeam(1);
    Team* pPlayer = Team::GetPtr(bPlayerTeam1 ? 1 : 2);
    Team* pEnemy = Team::GetPtr(bPlayerTeam1 ? 2 : 1);
    switch (AIDifficulty::GetAutoBalanceMode())
    {
    case 0:
        iPlayerScore = pPlayer->GetPoints();
        iEnemyScore = pEnemy->GetPoints();
        iThreshold = 0;
        iDivisor = 1;
        break;
    case 1:
        iPlayerScore = pPlayer->GetPoints();
        iEnemyScore = pEnemy->GetPoints();
        iThreshold = 20;
        iDivisor = 10;
        break;
    case 2:
        iPlayerScore = pPlayer->GetCurReinforcements();
        iEnemyScore = pEnemy->GetCurReinforcements();
        iThreshold = 15;
        iDivisor = 10;
        break;
    }
}
}

namespace AIDifficulty {

// FUNCTION: SWBF2 0x00439470
void UpdateAutoBalance()
{
    int iDifficulty = NetGame::IsEnabled() ? PlayerPrefs::mAllUserPrefs[0].mDifficulty
                                           : PlayerProfile::GetProfile(0)->mGameOptions.mCurDiffLevel;
    if (iDifficulty != AIDifficulty::GetProfileDifficulty())
    {
        AIDifficulty::mProfileDifficulty = iDifficulty;
    }
    AIDifficulty::mAutoBalancePlayerModifier = 0;
    AIDifficulty::mAutoBalanceEnemyModifier = 0;

    if (!AIDifficulty::AutoBalanceActive())
    {
        return;
    }
    bool bPlayerTeam1 = AIUtil::IsPlayerTeam(1);
    if (bPlayerTeam1 == AIUtil::IsPlayerTeam(2))
    {
        return;
    }

    int iPlayerScore, iEnemyScore, iThreshold, iDivisor;
    AIDifficulty::GetAutoBalanceScores(iPlayerScore, iEnemyScore, iThreshold, iDivisor);

    Team* pEnemy = Team::GetPtr(bPlayerTeam1 ? 2 : 1);
    float fMax = (float)pEnemy->GetMaxReinforcements();
    float fCur = (float)pEnemy->GetCurReinforcements();
    int iHandicap = 0;
    if (fMax * 0.25f > fCur)
    {
        iHandicap = -20;
    }
    else if (fMax * 0.5f > fCur)
    {
        iHandicap = -10;
    }

    int iDiff = iPlayerScore - iHandicap - iEnemyScore;
    if (abs(iDiff) > iThreshold)
    {
        int iStep = (iDiff - (iDiff > 0 ? iThreshold : -iThreshold)) / iDivisor;
        int iPlayer = -iStep;
        AIDifficulty::mAutoBalancePlayerModifier = iPlayer < -4 ? -4 : (iPlayer > 4 ? 4 : iPlayer);
        AIDifficulty::mAutoBalanceEnemyModifier = iStep < -4 ? -4 : (iStep > 4 ? 4 : iStep);
    }

    if (AIDifficulty::GetAutoBalanceMode() == 1)
    {
        AIDifficulty::SpaceAssault_CheatLikeABastard();
    }
}
}

// FUNCTION: SWBF2 0x00486270
void ControllerManager::DoPeriodicUpdate(float fDeltaTime)
{
    UpdatePilotSelf();
    for (PblList<PlayerController>::Node* pNode = sPlayerCtrls->_head._pNext; pNode->_pObject;
         pNode = pNode->_pNext)
    {
        UpdateControllerAudibles(pNode->_pObject);
        pNode->_pObject->UpdateFreeSpace();
        UpdateCrowd(pNode->_pObject);
        Controllable* pOwner = pNode->_pObject->mOwner;
        pOwner->mCharacter->mAdrenaline.Update(fDeltaTime, pOwner->GetGameObject());
    }
    sUpdateFriendlyFire();
    AIDifficulty::UpdateAutoBalance();
}

// FUNCTION: SWBF2 0x00486370
void ControllerManager::Update(float fDeltaTime)
{
    for (PblList<PlayerController>::Node* pNode = sPlayerCtrls->_head._pNext; PlayerController* pCtrl = pNode->_pObject; )
    {
        pNode = pNode->_pNext;
        pCtrl->Update(fDeltaTime);
    }

    AI::VOHelper::Update();

    if (++sFrameCount > 15)
    {
        sFrameCount = 1;
        float fTime = GameLoop::GetMissionTime();
        DoPeriodicUpdate(fTime - sPeriodicUpdateTime);
        sPeriodicUpdateTime = fTime;
    }

    VehicleManager::Update();
    AIVisRegionManager::UpdateNextRegion();

    int iMaxUpdates = AIUtil::IsUberMode() ? 100 : 10;
    float fTime = GameLoop::GetMissionTime();
    for (int i = 0; i < iMaxUpdates; ++i)
    {
        UnitController* pCtrl = sAICtrls->_head._pNext->_pObject;
        if (!pCtrl || pCtrl->mNextUpdateTime > fTime)
        {
            break;
        }
        pCtrl->UpdateHighLevel();
        UpdateCrowd(pCtrl);
        PblOrderedList<UnitController, float>::Node* pNode = GetManagerNode(pCtrl);
        sAICtrls->Remove(pNode);
        sAICtrls->Add(pNode, pCtrl, pCtrl->mNextUpdateTime);
    }

    for (PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext; UnitController* pCtrl = pNode->_pObject; )
    {
        pNode = pNode->_pNext;
        pCtrl->UpdateLowLevel(fDeltaTime);
    }

    int iScale = 1;
    if (PathManager::IsDone())
    {
        iScale = 2;
    }
    else
    {
        PathManager::Update(AIUtil::IsUberMode() ? 10.0f : 1.0f);
    }
    if (!VisionManager::RayTests((AIUtil::IsUberMode() ? 50 : 10) * iScale))
    {
        PathManager::Update(1.0f);
    }
}

// FUNCTION: SWBF2 0x00486730
void ControllerManager::LogVODeath(int iVictimTeam, int iKillerTeam, const PblVector3& vPos)
{
    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); (PlayerController*)it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (pOwner && DistanceSq(vPos, pOwner->GetMatrix().m[3]) < 625.0f)
        {
            it->LogVODeath(iVictimTeam, iKillerTeam);
        }
    }
}

// FUNCTION: SWBF2 0x004867c0
int ControllerManager::GetPlayersInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                                         int iMax, const Team* pTeam, AI::AIAffil eAffil)
{
    if (iMax <= 0)
    {
        return 0;
    }

    float fRadiusSq = fRadius * fRadius;
    int iCount = 0;
    for (PblList<PlayerController>::Itor it(*sPlayerCtrls); (PlayerController*)it; ++it)
    {
        Controllable* pOwner = it->mOwner;
        if (!pOwner)
        {
            continue;
        }
        GameObject* pObject = pOwner->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        int iAffil = pTeam ? pTeam->GetAffiliation(GetPerceivedTeam(pObject)) : 0;
        if (!(eAffil & (1 << (iAffil + 1))))
        {
            continue;
        }
        const PblVector3& vObject = GetCollisionObject(pObject)->GetPosition();
        if (DistanceSq(vObject, &vPos.x) < fRadiusSq)
        {
            ppObjects[iCount++] = pObject;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x004868c0
int ControllerManager::GetAIInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                                    int iMax, const Team* pTeam, AI::AIAffil eAffil)
{
    if (iMax <= 0)
    {
        return 0;
    }

    float fRadiusSq = fRadius * fRadius;
    int iCount = 0;
    for (PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext;
         UnitController* pCtrl = pNode->_pObject; pNode = pNode->_pNext)
    {
        Controllable* pOwner = pCtrl->mOwner;
        if (!pOwner)
        {
            continue;
        }
        GameObject* pObject = pOwner->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        int iAffil = pTeam ? pTeam->GetAffiliation(GetPerceivedTeam(pObject)) : 0;
        if (!(eAffil & (1 << (iAffil + 1))))
        {
            continue;
        }
        const PblVector3& vObject = GetCollisionObject(pObject)->GetPosition();
        if (DistanceSq(vObject, &vPos.x) < fRadiusSq)
        {
            ppObjects[iCount++] = pObject;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x004869c0
int ControllerManager::GetUnitsInRange(const PblVector3& vPos, float fRadius, GameObject** const ppObjects,
                                       int iMax, const Team* pTeam, AI::AIAffil eAffil)
{
    int iPlayers = GetPlayersInRange(vPos, fRadius, ppObjects, iMax, pTeam, eAffil);
    return iPlayers + GetAIInRange(vPos, fRadius, ppObjects + iPlayers, iMax - iPlayers, pTeam, eAffil);
}

// FUNCTION: SWBF2 0x00486610
void ControllerManager::BroadcastEvent(AI::EventInfo* pInfo, const PblVector3& vPos, float fRadius,
                                      const UnitController* pSender, const Team* pTeam, AI::AIAffil eAffil)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }

    float fRadiusSq = fRadius * fRadius;
    PblOrderedList<UnitController, float>::Node* pNode = sAICtrls->_head._pNext;
    while (UnitController* pCtrl = pNode->_pObject)
    {
        pNode = pNode->_pNext;
        if (!pCtrl->mOwner || !pCtrl->mOwner->GetGameObject())
        {
            continue;
        }
        if (pTeam && !(eAffil & (1 << (pTeam->GetAffiliation(GetTeam(pCtrl->mOwner->GetGameObject())) + 1))))
        {
            continue;
        }
        if (!pCtrl->mOwner || pCtrl == pSender)
        {
            continue;
        }
        if (DistanceSq(vPos, pCtrl->mOwner->GetMatrix().m[3]) < fRadiusSq)
        {
            pCtrl->SendEvent(pInfo);
        }
    }
}

// FUNCTION: SWBF2 0x00486a10
template void PblOrderedList<UnitController, float>::Add(PblOrderedList<UnitController, float>::Node*, UnitController*, float);

namespace ControllerManager
{
template <class T> void UpdateControllerAudibles(T* pCtrl)
{
    AudibleManager::UpdateAudience(pCtrl);
}

// FUNCTION: SWBF2 0x00486a80
template void UpdateControllerAudibles<PlayerController>(PlayerController* pCtrl);
}

namespace AI { namespace VOHelper {

// FUNCTION: SWBF2 0x00672640
void Update()
{
    using namespace AI::VOHelper;

    const float fTime = GameLoop::GetMissionTime();
    for (int i = 0; i < 20; ++i)
    {
        QueuedSound& rSound = mSndQueue[i];
        if (rSound.mType == AI::VOT_NONE || !(fTime > rSound.mTime))
        {
            continue;
        }
        if (rSound.mUseTarget)
        {
            Play(rSound.mType, rSound.mSpeaker, rSound.mPlayer, rSound.mTarget, 0.0f);
        }
        else
        {
            Play(rSound.mType, rSound.mSpeaker, rSound.mPlayer, rSound.mPos, 0.0f);
        }
        rSound.mType = AI::VOT_NONE;
    }
}
} }
