#include <float.h>
#include <math.h>
#include <string.h>

#include "Character.h"
#include "AIGoalManager.h"
#include "AIUtil.h"
#include "BonusManager.h"
#include "BoundaryManager.h"
#include "CollisionObject.h"
#include "CollisionManager.h"
#include "CommandFlyer.h"
#include "Controllable.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntityFlyer.h"
#include "EntitySoldier.h"
#include "EventManager_Character.h"
#include "FlagItem.h"
#include "EntityPortableTurret.h"
#include "CameraManager.h"
#include "ControllerManager.h"
#include "UnitController.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "MedalsMgr.h"
#include "NetComm.h"
#include "NetGame.h"
#include "PauseMenu.h"
#include "PassengerSlot.h"
#include "PlayerProfile.h"
#include "PlayerStats.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "SpawnManager.h"
#include "Team.h"
#include "Tracker.h"
#include "VoiceOverManager.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblUnicode.h"

#include "RedEngineFL/Interface/RedTextElement.h"
#include "EventManager_Timer.h"
#include "HUDEventQueue.h"
#include "HUDObjectiveList.h"
#include "LuaCallbacks_Options.h"
#include "GameMusic.h"
#include "ShellLoop.h"
#include "TeamAssist.h"

// GLOBAL: SWBF2 0x01e30330
int Character::sCharacterLimit;
// GLOBAL: SWBF2 0x01e30334
Character* Character::sCharacters;
// GLOBAL: SWBF2 0x01e30338
PlayerStats* Character::sPlayerStats;

// GLOBAL: SWBF2 0x01f9c240
int sHostCharacterLimit;
// GLOBAL: SWBF2 0x01f9c244
int sClientCharacterCount;
// GLOBAL: SWBF2 0x01f9c248
Character* sHostCharacters;
// GLOBAL: SWBF2 0x01f9c24c
int sClientCharacterLimit;
// GLOBAL: SWBF2 0x01f9c250
Character* sClientCharacters;
// GLOBAL: SWBF2 0x01f9c254
int sHostCharacterCount;

extern int netCurMaxPlayers;
extern NetPlayer netPlayers[64];

namespace AIDebug { namespace TestSpawn {
// GLOBAL: SWBF2 0x01e2fd6c
Character* aic[4];
} }

namespace AIDebug { namespace TestSpawn {

// FUNCTION: SWBF2 0x00438750
PblMatrix* __fastcall GetTestSpawnMatrix(PblMatrix* pResult, Character* pCharacter)
{
    pResult->Reset();
    float* p = reinterpret_cast<float*>(pResult);
    if (pCharacter->mPlayerId < 0 &&
        pCharacter != AIDebug::TestSpawn::aic[0] &&
        pCharacter != AIDebug::TestSpawn::aic[1] &&
        pCharacter != AIDebug::TestSpawn::aic[2] &&
        pCharacter != AIDebug::TestSpawn::aic[3])
    {
        p[12] = (float)(pCharacter - Character::sCharacters) * 3.0f;
        p[13] = 100.0f;
        return pResult;
    }

    const float fAngle = PblRandom::_Global.GetFloat() * 6.2831855f;
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    const float fRadius = PblRandom::_Global.GetFloat() * 32.0f;
    const PblVector3 vStart(fRadius * fSin - 74.0f, 37.0f, fRadius * fCos + 20.0f);
    const PblVector3 vDirection(0.0f, -1.0f, 0.0f);
    CollisionObject* pHitObject = 0;
    const float fHit = CollisionManager::RayHit(vStart, vDirection, 50.0f,
                                                 pHitObject, 0, 0, 0, 0, false);
    p[0] = -fCos; p[1] = 0.0f; p[2] = fSin; p[3] = 0.0f;
    p[4] = 0.0f; p[5] = 1.0f; p[6] = 0.0f; p[7] = 0.0f;
    p[8] = -fSin; p[9] = 0.0f; p[10] = -fCos; p[11] = 0.0f;
    p[12] = vStart.x; p[13] = vStart.y - fHit * 50.0f;
    p[14] = vStart.z; p[15] = 1.0f;
    return pResult;
}
} }

namespace AIDebug {

// FUNCTION: SWBF2 0x00438a10
PblMatrix GetTestSpawnMatrix(Character* pCharacter)
{
    PblMatrix result;
    AIDebug::TestSpawn::GetTestSpawnMatrix(&result, pCharacter);
    return result;
}
}

// FUNCTION: SWBF2 0x00452310
float Character::TimeSinceLastVehicleOrRemoteChange()
{
    return GameLoop::GetMissionTime() - mLastVehicleOrRemoteChangeTime;
}

// FUNCTION: SWBF2 0x00453310
void Character::EnterClient()
{
    if (!NetGame::IsOneWorld())
    {
        sHostCharacterLimit = sCharacterLimit;
        sHostCharacterCount = sCharacterCount;
        sHostCharacters = sCharacters;
        sCharacterLimit = sClientCharacterLimit;
        sCharacterCount = sClientCharacterCount;
        sCharacters = sClientCharacters;
    }
}

// FUNCTION: SWBF2 0x00453360
void Character::LeaveClient()
{
    if (!NetGame::IsOneWorld())
    {
        sClientCharacterLimit = sCharacterLimit;
        sClientCharacterCount = sCharacterCount;
        sClientCharacters = sCharacters;
        sCharacterLimit = sHostCharacterLimit;
        sCharacterCount = sHostCharacterCount;
        sCharacters = sHostCharacters;
    }
}

// FUNCTION: SWBF2 0x00477fd0
bool CommandFlyer::IsLanded()
{
    return mState == LANDED;
}

// FUNCTION: SWBF2 0x00477f10
Controllable* CommandFlyer::GetPassengerSlot(int)
{
    EntityFlyerClass* pClass = EntityFlyer::mClass;
    if (!pClass->mPassengerSlotClass)
    {
        return 0;
    }

    Controllable* pLast = 0;
    const unsigned int uCount = pClass->mNumPassengerSlots;
    for (unsigned int i = 0; i < uCount; ++i)
    {
        PassengerSlot* pCraft = mPassengerSlots[i];
        if (!pCraft)
        {
            continue;
        }
        Controllable* pThis = static_cast<Controllable*>(this);
        Controllable* pCurrent = (pThis->mPilotType == PILOT_SELF ||
                                 (pThis->mPilotType == PILOT_VEHICLESELF && pThis->mPilot == pThis))
                                    ? 0 : pThis->mPilot;
        if (pCurrent == static_cast<Controllable*>(pCraft) ||
            static_cast<Controllable*>(pCraft)->mPlayerId >= 0)
        {
            continue;
        }
        pLast = static_cast<Controllable*>(pCraft);
        if (!static_cast<Trackable*>(pCraft)->GetControllable())
        {
            return pLast;
        }
    }
    return pLast;
}

// FUNCTION: SWBF2 0x005c4440
int NetGame::GetSpawnOrder(int iPlayer)
{
    if (iPlayer >= 0 && iPlayer < netCurMaxPlayers && NetGame::IsPlaying(iPlayer, false))
    {
        return netPlayers[iPlayer].mSpawnOrder;
    }
    return -1;
}

namespace EventManager
{
template <>
void Event<Character, char>::Callback::Trigger(Character& item, char& param);

template <>
void Event<Character, DamageOwner>::Callback::Trigger(Character& item, DamageOwner& param);
}

namespace AI
{
enum VOType
{
    VOT_Kill = 8
};
namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay);
}
}

extern int netHeroKillerID[3];
extern bool netHeroUnlocked[3];

static char sEventParam;

extern bool netHeroesEnabled;
extern int netHeroLastPlayerID[3];
extern bool gDoTakeoverSpawns;
extern bool gNoAIRespawn;
namespace GameLoop { extern bool mbHistoricalRules; }
extern int netNumBots;

// GLOBAL: SWBF2 0x01e2fde6
bool gCharacterSpawned;
extern int netHeroPlayerID[3];
extern int netHeroAIID[3];
extern int gDebugCount;

namespace AIDebug { bool GetTestSpawnMode(); }
namespace AIDebug { PblMatrix GetTestSpawnMatrix(Character* pCharacter); }

class AmmoCounter
{
public:
    void LoadClip();

    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
};

namespace
{

GameObject* GetPostObject(CommandPost* pPost)
{
    PblHandle<GameObject>& handle = pPost->mObject;
    if (handle.mObject &&
        handle.mObject->mHandleId !=
            handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

Damageable* GetDamageable(GameObject* pObject)
{
    return static_cast<Damageable*>(pObject);
}

const GameObjectClass* GetGameObjectClass(GameObject* pObject)
{
    return pObject->GetGameObjectClass();
}

const PblVector3& GetPosition(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject)->GetPosition();
}

int GetTeam(const GameObject* pObject)
{
    return pObject->GetTeam();
}

EntityFlyer* GetFlyer(Controllable* pControllable)
{
    return pControllable ? static_cast<EntityFlyer*>(pControllable) : 0;
}

bool GetFlyerAutoCorrectingToStayInBattlefield(const EntityFlyer* pFlyer)
{
    return pFlyer->mFlyerAutoCorrectingToStayInBattlefield;
}

const EntityFlyerClass* GetFlyerClass(const EntityFlyer* pFlyer)
{
    return pFlyer->mClass;
}

Controllable* GetSoldierControllable(Entity* pSoldier)
{
    return static_cast<Controllable*>(static_cast<EntitySoldier*>(pSoldier));
}

const GameObjectClass* GetSoldierClass(Entity* pSoldier)
{
    return static_cast<EntitySoldier*>(pSoldier)->mClass;
}

int GetWeaponChannelByWeaponIndex(Entity* pSoldier, int iIndex)
{
    return static_cast<EntitySoldier*>(pSoldier)->GetWeaponChannelByWeaponIndex(iIndex);
}

bool EnterControllable(Entity* pSoldier, Controllable* pControllable)
{
    return static_cast<EntitySoldier*>(pSoldier)->EnterControllable(pControllable);
}

const WeaponClass* GetWeaponClass(Weapon* pWeapon)
{
    return pWeapon->mClass;
}

AmmoCounter* GetAmmoCounter(Weapon* pWeapon)
{
    return pWeapon->m_pAmmoCounter;
}
}

// FUNCTION: SWBF2 0x00450450
void Character::CreateAll(int iCount)
{
    sCharacterLimit = 0;
    sCharacterCount = iCount;
    sCharacters = new Character[iCount];
    if (NetGame::IsEnabled() && NetGame::IsClient() && !NetGame::IsOneWorld())
    {
        return;
    }
    sPlayerStats = new PlayerStats[sCharacterCount];
    for (int i = 0; i < sCharacterCount; i++)
    {
        sPlayerStats[i].Init(i, sCharacterCount);
    }
}

// SYNTHETIC: SWBF2 0x004505d0
// Character::`vector deleting destructor'

// FUNCTION: SWBF2 0x004506b0
Character::Character()
    : mAssignedPost(0), mGoalNode(), mPlayerId(-1), mTeamNumber(0), mTeamPtr(0), mClassIndex(0),
      mClassPtr(0), mPost(0), mUnit(0), mVehicle(0), mRemote(0), mLastVehicleOrRemoteChangeTime(0.0f),
      mDelayTimer(0.0f), mSpawnCycle(0), mSpawnSlot(-1), mSpawnReady(true), mHeroFlag(false),
      mVehicleBlockTime(-1.0f), mOutOfBoundsTimer(-1.0f), mCarriedFlagPtr(0), mCPHold(0),
      mTakeoverChr(0)
{
    memset(mName, 0, sizeof(mName));
    memset(mRegularName, 0, sizeof(mRegularName));
    for (int i = 0; i < 3; ++i)
    {
        mObjectHitTimer[i] = 0.0f;
        mObjectHitMultiplier[i] = 1.0f;
    }
    mTakeoverFirstTime = true;
    mMedalsMgr = new MedalsMgr(this);
}

// FUNCTION: SWBF2 0x00450870
Character::~Character()
{
    if (mGoalNode.IsInList())
    {
        mGoalNode.GetList()->Remove(&mGoalNode);
    }
    delete mMedalsMgr;
}

// FUNCTION: SWBF2 0x004508c0
void Character::Init(int iTeamNumber, const unsigned short* pName, int iPlayerId)
{
    mTeamNumber = iTeamNumber;
    mPlayerId = iPlayerId;
    memset(mName, 0, sizeof(mName));
    memset(mRegularName, 0, sizeof(mRegularName));
    SetName(pName);
    PblUnicode::ustrncpy(mRegularName, pName, 64);
    if (mMedalsMgr)
    {
        mMedalsMgr->Init();
        mMedalsMgr->LoadMedals();
    }
}

// FUNCTION: SWBF2 0x00450940
void Character::SetName(const unsigned short* pName)
{
    PblUnicode::ustrncpy(mName, pName, 64);
}

// FUNCTION: SWBF2 0x00450960
void Character::GetHeroName(unsigned short* pName, int iSize) const
{
    if (IsHero() && Team::sTeams[mTeamNumber])
    {
        const GameObjectClass* pClass = Team::sTeams[mTeamNumber]->GetHeroClass();
        if (pClass)
        {
            PblUnicode::ustrlcpy(pName, pClass->mLabel, iSize);
        }
    }
}

// FUNCTION: SWBF2 0x004509a0
void Character::InitialUpdate()
{
    mTeamPtr = Team::sTeams[mTeamNumber];
    SelectClass(0);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        mDelayTimer = 60.0f;
        ActivateThread(0, PER_LOCAL_TURN);
    }
    else
    {
        Respawn();
        ActivateThread(0, PER_LOCAL_TURN);
    }
}

// FUNCTION: SWBF2 0x00450a20
bool Character::Update(float fDeltaTime)
{
    if (mMedalsMgr)
    {
        mMedalsMgr->Update(fDeltaTime);
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        if (Controllable* pUnit = mUnit)
        {
            if (mClassPtr)
            {
                GetPlayerStats(this).AddTimePlayedAsClass(mClassPtr, fDeltaTime);
            }
            if (GameObject* pObject = pUnit->GetGameObject())
            {
                Damageable* pDamageable = GetDamageable(pObject);
                GetPlayerStats(this).UpdateStatHealth(pDamageable->mCurHealth / pDamageable->mMaxHealth);
            }
        }

        if (Controllable* pControllable = GetControllable())
        {
            GetPlayerStats(this).AddCampingTime(pControllable->GetMatrix().GetTranslation(), fDeltaTime);
            Weapon* pWeapon = pControllable->GetWeaponPtr(GetControllable()->GetWeaponIndex(0));
            if (pWeapon && GetWeaponClass(pWeapon))
            {
                GetPlayerStats(this).AddTimePlayedWithWeapon(GetWeaponClass(pWeapon), fDeltaTime);
            }
            if (pControllable != mUnit)
            {
                GetPlayerStats(this).AddTimePlayedInsideVehicle(pControllable->GetEntity(), fDeltaTime);
            }
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        float fTimer = mObjectHitTimer[i] - fDeltaTime;
        if (!(fTimer > 0.0f))
        {
            fTimer = 0.0f;
        }
        mObjectHitTimer[i] = fTimer;
        if (fTimer == 0.0f)
        {
            mObjectHitMultiplier[i] = 1.0f;
        }
    }

    if (mUnit)
    {

        if (mPlayerId >= 0)
        {
            Entity* pEntity = mUnit->GetEntity();
            if (pEntity && pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier) && netHeroesEnabled && IsHero())
            {
                Damageable* pDamageable = GetDamageable(reinterpret_cast<GameObject*>(pEntity));
                pDamageable->SetCurHealth(pDamageable->mCurHealth - fDeltaTime * 10.0f, true, 0);
                if (pDamageable->mCurHealth <= 0.0f && mVehicle)
                {
                    GetSoldierControllable(pEntity)->ExitVehicle(true, true);
                }
            }
        }
        SetSpawnReady(false);
        CheckOutOfBounds(fDeltaTime);
        return true;
    }

    if (gDoTakeoverSpawns && mTakeoverChr && mTakeoverChr->GetControllable())
    {
        if (GameLoop::GetMissionTime() >= mTakeoverTime)
        {
            PlayerTakeoverAI(mTakeoverChr);
            mTakeoverChr = 0;
        }
        return true;
    }

    if (mPlayerId < 0 && NetGame::IsEnabled() && NetGame::IsClient())
    {
        DeactivateThread();
        return true;
    }

    mDelayTimer -= fDeltaTime;
    if (mDelayTimer > 0.0f)
    {
        return true;
    }

    if (!mSpawnReady)
    {
        Respawn();
    }

    if (mCPHold && GetControllable())
    {
        if (!mCPHold->IsInsideCaptureRegion(GetControllable()->GetGameObject()))
        {
            mCPHold = 0;
            mCPHoldTimestamp = 0.0f;
        }
    }

    if (gNoAIRespawn && mPlayerId < 0)
    {
        return true;
    }

    if (mPlayerId < 0 && mTeamPtr->mCurReinforcements <= mTeamPtr->GetMemberActive())
    {
        return true;
    }
    if (mPlayerId < 0 && mTeamPtr->mMemberCount > mTeamPtr->GetMemberLimit())
    {
        SelectPost(0);
        SelectTeam(3 - mTeamNumber);
    }

    if (mPost)
    {
        GetPostObject(mPost);
    }

    if (!mSpawnReady || !mPost)
    {
        if (NetGame::IsEnabled() && !NetGame::IsPreGame())
        {
            mSpawnCycle = SpawnManager::Instance()->GetCycleIndex(mTeamNumber) + 1;
        }
        SelectPost(SpawnManager::Instance()->RequestSpawnPoint(mPost, this));
        return true;
    }

    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return true;
    }

    if (NetGame::IsEnabled() || mPlayerId < 0)
    {
        SpawnManager* pSpawnManager = SpawnManager::Instance();
        if (pSpawnManager->GetCycleIndex(mTeamNumber) < mSpawnCycle ||
            pSpawnManager->GetSlotTimer(mTeamNumber) > 0.0f)
        {
            return true;
        }
        if (!NetGame::IsEnabled())
        {
            pSpawnManager->ResetSlotTimer(mTeamNumber);
        }
    }

    Team* pTeam = mTeamPtr;
    Character* pFirstAI = 0;
    int iPlayersAlive = 0;
    int iPlayersDead = 0;
    int iAIAlive = 0;
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        Character* pMember = pTeam->GetMemberPtr(i);
        if (!pMember)
        {
            continue;
        }
        if (pMember->mUnit)
        {
            if (pMember->mPlayerId >= 0)
            {
                ++iPlayersAlive;
            }
            else
            {
                ++iAIAlive;
                if (!pFirstAI)
                {
                    pFirstAI = pMember;
                }
            }
        }
        else if (pMember->mPlayerId >= 0)
        {
            ++iPlayersDead;
        }
    }

    if (mPlayerId < 0 && !mHeroFlag && !GameLoop::mbHistoricalRules && !GameLoop::AreMetagameRulesOn())
    {
        if (!AIUtil::IsUberMode() && iAIAlive >= netNumBots)
        {
            mDelayTimer = 5.0f;
            return true;
        }
        if (mTeamNumber > 2 && iAIAlive >= SpawnManager::Instance()->GetAllyUnitCount())
        {
            return true;
        }
    }

    if (NetGame::IsEnabled())
    {

        if (iPlayersDead + iAIAlive + iPlayersAlive >= SpawnManager::Instance()->GetUnitCount(mTeamNumber))
        {
            if (mPlayerId < 0)
            {
                return true;
            }
            if (iAIAlive > 0)
            {
                if (pFirstAI && pFirstAI->mUnit)
                {
                    GameObject* pObject = pFirstAI->mUnit->GetGameObject();
                    if (pFirstAI->mRemote)
                    {
                        pFirstAI->mUnit->ExitRemote();
                    }
                    if (pFirstAI->mVehicle)
                    {
                        pFirstAI->mUnit->ExitVehicle(true, true);
                    }
                    Damageable* pDamageable = GetDamageable(pObject);
                    pDamageable->mIsAlive = false;
                    pDamageable->mVanishing = true;
                }
            }
            else if (iPlayersAlive + iAIAlive >= SpawnManager::Instance()->GetUnitCount(mTeamNumber))
            {
                return true;
            }
        }
        if (mPlayerId >= 0 && NetGame::GetSpawnOrder(mPlayerId))
        {
            return true;
        }
    }

    if (gDoTakeoverSpawns && mPlayerId >= 0 && !mTakeoverFirstTime)
    {
        Team* pOwnTeam = mTeamPtr;
        Character* pBest = 0;
        float fBest = 1000.0f;
        for (int i = 0; i < pOwnTeam->mMemberCount; ++i)
        {
            Character* pMember = pOwnTeam->GetMemberPtr(i);
            if (pMember && pMember != this && pMember->mClassIndex == mClassIndex &&
                pMember->GetControllable() && pMember->GetControllable()->GetGameObject())
            {
                GameObject* pObject = pMember->GetControllable()->GetGameObject();
                const PblVector3& vPost = mPost->GetMatrix().GetTranslation();
                const PblVector3& vPos = GetPosition(pObject);
                float fX = vPost.x - vPos.x;
                float fY = vPost.y - vPos.y;
                float fZ = vPost.z - vPos.z;
                float fDist = sqrtf(fX * fX + fY * fY + fZ * fZ);
                Damageable* pDamageable = GetDamageable(pObject);
                float fWeight = pDamageable->mMaxHealth > 0.0f
                                    ? 2.0f - pDamageable->mCurHealth / pDamageable->mMaxHealth
                                    : 0.0f;
                float fScore = fWeight * fDist;
                if (fBest > fScore)
                {
                    pBest = pMember;
                    fBest = fScore;
                }
            }
        }
        if (pBest)
        {
            Tracker::SetTrackable(pBest->GetControllable()->GetTrackable(), mPlayerId);
            if (!mTakeoverChr)
            {
                mTakeoverTime = GameLoop::GetMissionTime() + 1.0f;
            }
            mTakeoverChr = pBest;
            return true;
        }
    }

    mTakeoverFirstTime = false;
    Spawn(mPost->GetMatrixOnSpawnPath(mTeamNumber != GetTeam(GetPostObject(mPost))));
    return true;
}

// FUNCTION: SWBF2 0x00451520
void Character::PlayerTakeoverAI(Character* pCharacter)
{
    if (pCharacter->mUnit)
    {
        SetUnit(pCharacter->mUnit);
        pCharacter->SetUnit(0);
        mUnit->ClearControllable();
        mUnit->ReleaseController();
        mUnit->SetCharacter(this);
    }
    if (pCharacter->mVehicle)
    {
        SetVehicle(pCharacter->mVehicle);
        pCharacter->SetVehicle(0);
        mVehicle->ClearControllable();
        mVehicle->ReleaseController();
        mVehicle->SetCharacter(this);
    }
    if (pCharacter->mRemote)
    {
        SetRemote(pCharacter->mRemote);
        pCharacter->SetRemote(0);
        mRemote->ClearControllable();
        mRemote->ReleaseController();
        mRemote->SetCharacter(this);
    }
    GetControllable()->SetupController();
}

// FUNCTION: SWBF2 0x00451610
void Character::SetSpawnReady(bool bReady)
{
    mSpawnReady = bReady;
    if (NetGame::IsEnabled() && !NetGame::IsClient() && mPlayerId >= 0)
    {
        if (bReady)
        {
            NetGame::AddToSpawnQueue(mPlayerId);
        }
        else
        {
            NetGame::RemoveFromSpawnQueue(mPlayerId);
        }
    }
}

// FUNCTION: SWBF2 0x00451680
bool Character::IsHero() const
{
    if (mHeroFlag)
    {
        return true;
    }
    if (mUnit)
    {
        return GetGameObjectClass(mUnit->GetGameObject())->mHeroFlag;
    }
    return netHeroLastPlayerID[mTeamNumber] == mPlayerId && mPlayerId != -1;
}

// FUNCTION: SWBF2 0x004516e0
void Character::ChangeClass(int iClassIndex)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (!mUnit)
    {
        return;
    }

    delete mRemote;
    SetRemote(0);

    PblMatrix mMatrix = mUnit->GetMatrix();
    MedalsMgr* pMedals = mMedalsMgr;
    float fHealth = 1.0f;
    float fAmmo[2] = { 1.0f, 1.0f };
    int iAmmoCount[2] = { 0, 0 };

    Entity* pEntity = mUnit->GetEntity();
    if (pEntity && pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        Damageable* pDamageable = GetDamageable(reinterpret_cast<GameObject*>(pEntity));
        if (pDamageable->mMaxHealth > 0.0001f)
        {
            fHealth = pDamageable->mCurHealth / pDamageable->mMaxHealth;
        }

        Controllable* pControllable = GetSoldierControllable(pEntity);
        int iCount = pControllable->GetWeaponCount();
        for (int i = 0; i < iCount; ++i)
        {
            Weapon* pWeapon = GetSoldierControllable(pEntity)->GetWeaponPtr(i);
            const WeaponClass* pWeaponClass = GetWeaponClass(pWeapon);
            bool bAvailable;
            if (pMedals)
            {
                bAvailable = pWeaponClass->mMedalsTypeToUnlock == -1 ||
                             pMedals->IsAwardAvailable(pWeaponClass->mMedalsTypeToUnlock);
                if (bAvailable && pWeaponClass->mMedalsTypeToLock != -1 &&
                    pMedals->IsAwardAvailable(pWeaponClass->mMedalsTypeToLock))
                {
                    bAvailable = false;
                }
            }
            else
            {
                bAvailable = pWeaponClass->mMedalsTypeToUnlock == -1;
            }

            AmmoCounter* pAmmo = GetAmmoCounter(pWeapon);
            if (pAmmo && pAmmo->mMaxClips != FLT_MAX && bAvailable && pAmmo->mMaxClips > 0.0001f)
            {
                float fNumClips = pAmmo->mNumClips;
                float fCurClip = pAmmo->mCurClip;
                int iChannel = GetWeaponChannelByWeaponIndex(pEntity, i);
                float fFraction = (fNumClips + fCurClip) / pAmmo->mMaxClips;
                int iOld = iAmmoCount[iChannel]++;
                fAmmo[iChannel] = (fFraction + (float)iOld * fAmmo[iChannel]) / (float)iAmmoCount[iChannel];
            }
        }
    }

    delete mUnit;
    mUnit = 0;
    SelectClass(iClassIndex);
    Spawn(mMatrix);

    if (!mUnit)
    {
        return;
    }
    pEntity = mUnit->GetEntity();
    if (!pEntity || !pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier))
    {
        return;
    }

    if (GetSoldierClass(pEntity)->mHeroFlag)
    {
        fHealth = 1.0f;
        fAmmo[0] = 1.0f;
        fAmmo[1] = 1.0f;
    }
    Damageable* pDamageable = GetDamageable(reinterpret_cast<GameObject*>(pEntity));
    if (pDamageable->mMaxHealth > 0.0001f)
    {
        pDamageable->SetCurHealth(pDamageable->mMaxHealth * fHealth, false, 0);
    }

    Controllable* pControllable = GetSoldierControllable(pEntity);
    int iCount = pControllable->GetWeaponCount();
    for (int i = 0; i < iCount; ++i)
    {
        Weapon* pWeapon = pControllable->GetWeaponPtr(i);
        const WeaponClass* pWeaponClass = GetWeaponClass(pWeapon);
        bool bAvailable;
        if (pMedals)
        {
            bAvailable = pWeaponClass->mMedalsTypeToUnlock == -1 ||
                         pMedals->IsAwardAvailable(pWeaponClass->mMedalsTypeToUnlock);
            if (bAvailable && pWeaponClass->mMedalsTypeToLock != -1 &&
                pMedals->IsAwardAvailable(pWeaponClass->mMedalsTypeToLock))
            {
                bAvailable = false;
            }
        }
        else
        {
            bAvailable = pWeaponClass->mMedalsTypeToUnlock == -1;
        }

        AmmoCounter* pAmmo = GetAmmoCounter(pWeapon);
        if (pAmmo && pAmmo->mMaxClips != FLT_MAX && bAvailable && pAmmo->mMaxClips > 0.0001f)
        {
            int iChannel = GetWeaponChannelByWeaponIndex(pEntity, i);
            float fTotal = fAmmo[iChannel] * pAmmo->mMaxClips;
            float fClips = (float)floor(fTotal);
            pAmmo->mNumClips = fClips;
            pAmmo->mCurClip = fTotal - fClips;
            pAmmo->LoadClip();
        }
    }
}

// FUNCTION: SWBF2 0x00451b50
PlayerProfile::tProfile* Character::GetProfile() const
{
    int iLocal = NetGame::GetJoystickIndex(mPlayerId);
    if (iLocal >= 0 && iLocal < 1 && playerJoystickNumbers[iLocal * 11] >= 0)
    {
        return PlayerProfile::GetProfile(playerJoystickNumbers[iLocal * 11]);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00451b80
PlayerStats& Character::GetPlayerStats(Character* pCharacter)
{
    return pCharacter->GetPlayerStats();
}

// FUNCTION: SWBF2 0x00451b90
void Character::Spawn(const PblMatrix& mMatrix)
{
    if (mUnit)
    {
        return;
    }
    if (mPost && mTeamPtr->GetAffiliation(GetTeam(mPost->GetGameObject())) <= 0)
    {
        SelectPost(0);
        Respawn();
        return;
    }

    int iPlayerId = mPlayerId;
    if (iPlayerId < 0 && !mTeamPtr->mbAllowAISpawn)
    {
        return;
    }

    gCharacterSpawned = true;
    mSpawnTime = NetComm::GetTime();
    if (iPlayerId >= 0 && NetGame::IsEnabled())
    {
        NetGame::SetSpawning(iPlayerId, false);
        if (!NetGame::IsClient())
        {
            NetGame::SetSpectator(iPlayerId, -1);
        }
    }
    if (mPlayerId >= 0)
    {
        int iJoystick = NetGame::GetJoystickIndex(mPlayerId);
        if (iJoystick >= 0)
        {
            HUD::Event event;
            event.mClass = HUD::GameEvents::gPlayerEvents[iJoystick].spawn;
            event.mData.boolValue = true;
            event.Send();
            if (!PauseMenu::GetPaused(iJoystick))
            {
                PauseMenu::SetPaused(iJoystick, false);
            }
        }
    }

    EntityDesc desc;
    desc.mMatrix = mMatrix;
    desc.mPlayerId = mPlayerId;
    if (AIDebug::GetTestSpawnMode())
    {
        desc.mMatrix = AIDebug::GetTestSpawnMatrix(this);
    }

    if ((NetGame::IsPreGame() || gNoAIRespawn) && mPlayerId < 0)
    {
        return;
    }
    if (!mClassPtr)
    {
        return;
    }

    if (mPlayerId >= 0 && mClassIndex)
    {
        if (!GetPlayerStats(this).IsClassSelectable(mClassIndex, 0))
        {
            SelectClass(0);
        }
        else if (NetGame::IsEnabled() && mTeamPtr->GetClassPtr(mClassIndex)->mHeroFlag &&
                 mPlayerId != netHeroPlayerID[mTeamNumber])
        {
            SelectClass(0);
        }
    }

    Entity* pEntity = mClassPtr->Build(desc);
    if (!pEntity)
    {
        if (!mClassIndex)
        {
            return;
        }
        SelectClass(0);
        pEntity = mClassPtr->Build(desc);
        if (!pEntity)
        {
            return;
        }
    }

    GameObject* pObject = pEntity->GetGameObject();
    mUnit = pObject->GetControllable();
    pObject->SetTeam(mTeamNumber);
    pObject->mPerceivedTeam = mTeamNumber;
    mUnit->SetCharacter(this);
    pEntity->Init();
    mHeroFlag = pObject->GetGameObjectClass()->mHeroFlag;
    if (BonusManager::Instance()->IsTeamBonusActive(mTeamNumber, 0xDDB212E1))
    {
        pObject->SetCurHealth(pObject->mCurHealth + pObject->mMaxHealth * 0.25f, true, 0);
    }

    if (!mHeroFlag)
    {
        ++mTeamPtr->mMemberActive;
        if (mPlayerId != -1)
        {
            if (netHeroLastPlayerID[mTeamNumber] == mPlayerId)
            {
                NetGame::ClearLastHero(mTeamNumber);
            }
            NetGame::DeclineHero(mTeamNumber, mPlayerId);
        }
    }
    else if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        if (mPlayerId != -1)
        {
            NetGame::AcceptHero(mTeamNumber, mPlayerId);
            NetGame::HeroSpawned(mTeamNumber);
        }
        else
        {
            int iHeroId = netHeroAIID[mTeamNumber];
            if (iHeroId != -1 && GetPtr(iHeroId) == this)
            {
                NetGame::AcceptHero(mTeamNumber, iHeroId);
                NetGame::HeroSpawned(mTeamNumber);
            }
        }
    }

    if (mPost && mSpawnSlot >= 0)
    {
        mPost->PopSpawnQueue(mPlayerId >= 0, mTeamNumber == GetTeam(mPost->GetGameObject()));
        mSpawnSlot = -1;
    }

    if (mPlayerId >= 0 &&
        ((GameLoop::GetNumCameras() == 1 && mPlayerId == NetGame::GetLocalIndex(0)) ||
         GameLoop::GetNumCameras() > 1))
    {
        VoiceOverManager::sInstance->Enable(true);
    }

    GameObject* pPostObject;
    Controllable* pCraft;
    if (mPost && mPost->GetGameObject() &&
        mPost->GetGameObject()->IsRtti(CommandFlyer::rttiHashCommandFlyer) &&
        (pPostObject = mPost->GetGameObject(), (pCraft = ((CommandFlyer*)pPostObject)->GetPassengerSlot(0)) != 0) &&
        pEntity->IsRtti(EntitySoldier::rttiHashEntitySoldier) && !((CommandFlyer*)pPostObject)->IsLanded())
    {
        EnterControllable(pEntity, pCraft);
    }
    else if (FLEffectClass* pEffectClass = FLEffect::Get(0x24D98596))
    {

        if (FLEffectObject* pEffect = pEffectClass->BuildEffect())
        {
            pEffect->AttachEffectToObject(pObject, true);
        }
    }

    EventManager::CharacterEvent::mSpawn.Trigger(*this, sEventParam);

    if (mPlayerId >= 0 && NetGame::IsEnabled() && !NetGame::IsClient())
    {
        mSpawnTime = NetComm::GetTime();
        NetGame::CreateEvent_UpdateInvincibility(this);
    }
    SetSpawnReady(false);
}

// FUNCTION: SWBF2 0x00452280
void Character::SetVehicle(Controllable* pVehicle)
{
    if (this - GetPtr(0) == 5)
    {
        ++gDebugCount;
    }
    mVehicle = pVehicle;
    mLastVehicleOrRemoteChangeTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x004522e0
void Character::SetRemote(Controllable* pRemote)
{
    mRemote = pRemote;
    mLastVehicleOrRemoteChangeTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x00452330
bool Character::ChangeTeam(int iTeam)
{
    if (mHeroFlag || mTeamNumber == iTeam)
    {
        return false;
    }
    Team* pTeam = Team::sTeams[iTeam];
    if (pTeam->mMemberCount >= pTeam->GetMemberLimit())
    {
        return false;
    }
    mTeamPtr->RemoveMember(this);
    mTeamNumber = iTeam;
    mTeamPtr = pTeam;
    pTeam->AddMember(this);
    return true;
}

// FUNCTION: SWBF2 0x004523a0
void Character::SelectTeam(int iTeam)
{
    if (!ChangeTeam(iTeam))
    {
        return;
    }
    if (mPlayerId >= 0)
    {
        int iClass = mClassIndex;
        int iLast = mTeamPtr->GetClassCount() - 1;
        if (iClass < 0)
        {
            iClass = 0;
        }
        else if (iClass > iLast)
        {
            iClass = iLast;
        }
        SelectClass(iClass);
    }
    else
    {
        int iClass = mTeamPtr->PickRandomClassIndex(this);
        if (iClass >= 0)
        {
            SelectClass(iClass);
        }
        else
        {
            SelectClass(-1);
            SetSpawnReady(false);
        }
    }
    SelectPost(SpawnManager::Instance()->RequestSpawnPoint(mPost, this));
}

// FUNCTION: SWBF2 0x00452460
void Character::SelectClass(int iClassIndex)
{
    if (iClassIndex >= 0 && iClassIndex < mTeamPtr->GetClassCount())
    {
        mClassIndex = iClassIndex;
        mClassPtr = mTeamPtr->GetClassPtr(iClassIndex);
        if (mTeamPtr->GetClassMaxUnits(iClassIndex) <= 1 && mClassPtr->mLabel && mPlayerId < 0)
        {
            SetName(mClassPtr->mLabel);
        }
        else
        {
            SetName(mRegularName);
        }
    }
    else
    {
        mClassIndex = -1;
        mClassPtr = 0;
    }
}

// FUNCTION: SWBF2 0x004524f0
void Character::SelectPost(CommandPost* pPost)
{
    if (mPost == pPost)
    {
        return;
    }
    if (mPost)
    {
        mPost->PopSpawnQueue(mPlayerId >= 0, mTeamNumber == GetTeam(mPost->GetGameObject()));
    }
    mPost = pPost;
    mSpawnSlot = -1;
    if (mPost)
    {
        mSpawnSlot = mPost->PushSpawnQueue(mPlayerId >= 0, mTeamNumber == GetTeam(mPost->GetGameObject()));
    }
}

// FUNCTION: SWBF2 0x00452700
void Character::SelectSpawn(int iTeam, int iClass, CommandPost* pPost)
{
    SelectTeam(iTeam);
    SelectClass(iClass);
    SelectPost(pPost);
    if (mPlayerId >= 0 && NetGame::IsEnabled())
    {
        NetGame::SetSpawning(mPlayerId, true);
        if (!NetGame::IsClient())
        {
            NetGame::SetPlayerToSpectator(mPlayerId);
        }
    }
}

// FUNCTION: SWBF2 0x00452780
void Character::Died()
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (mCarriedFlagPtr)
    {
        mCarriedFlagPtr->Drop();
    }
    if (mMedalsMgr)
    {
        mMedalsMgr->ClearAllMedalPoints();
    }

    UnitControllerPtr(GetControllable());
    DamageOwner& rOwner = mUnit->GetGameObject()->mDamageOwner;
    mDelayTimer = 4.0f;
    if (gDoTakeoverSpawns && mPlayerId < 0)
    {
        mDelayTimer = FLT_MAX;
    }
    if (mPlayerId < 0)
    {
        mPost = 0;
    }
    if (GetControllable() && GetControllable()->GetGameObject())
    {
        mAdrenaline.AddDied(GetControllable()->GetGameObject());
    }
    if (rOwner.mCharacter)
    {
        if (rOwner.mCharacter->mPlayerId >= 0)
        {
            AI::VOHelper::Play(AI::VOT_Kill, 0, 0, mUnit->GetGameObject(), 0.0f);
        }
        if (rOwner.mCharacter && GetControllable())
        {
            ControllerManager::LogVODeath(mTeamNumber, rOwner.mTeam,
                                          GetControllable()->GetMatrix().GetTranslation());
        }
    }
    if (GetControllable())
    {
        HUD::GameEvents::Die(GetControllable());
    }

    int iTeam = mTeamNumber;
    if (netHeroesEnabled && netHeroUnlocked[iTeam] &&
        (netHeroPlayerID[iTeam] != -1 || netHeroAIID[iTeam] != -1) &&
        (netHeroPlayerID[iTeam] != -1 ? mPlayerId == netHeroPlayerID[iTeam]
                                    : this == GetPtr(netHeroAIID[iTeam])) &&
        mUnit && mHeroFlag)
    {
        if (rOwner.mCharacter && rOwner.mCharacter != this)
        {
            netHeroKillerID[rOwner.mCharacter->mTeamNumber] = rOwner.mCharacter->mPlayerId;
        }
        NetGame::HeroKilled(iTeam);
    }

    GameObject* pDeadObject = mUnit->GetGameObject();
    mUnit = 0;
    SetVehicle(0);
    if (!mHeroFlag)
    {
        --mTeamPtr->mMemberActive;
    }
    if (pDeadObject->mVanishing)
    {
        mDelayTimer = 0.0001f;
        return;
    }

    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::CharacterEvent::mDeath.Trigger(*this, rOwner);
    }
    GetPlayerStats(this).AddDeath(rOwner.mCharacter);

    Team* pEnemyTeam = Team::GetPtr(3 - mTeamNumber);
    bool bPlayer = mPlayerId >= 0;
    bool bTurret = rOwner.mGameObject &&
                   rOwner.mGameObject->GetEntity()->IsRtti(EntityPortableTurret::rttiHashEntityPortableTurret);
    Character* pKiller = rOwner.mCharacter;
    if (!pKiller && bTurret)
    {
        pKiller = reinterpret_cast<EntityPortableTurret*>(rOwner.mGameObject->GetEntity())->mOwnerChar;
    }

    if (pKiller)
    {
        int iAffiliation = pKiller->mTeamPtr->GetAffiliation(mTeamNumber);
        if (iAffiliation < 0)
        {
            GetPlayerStats(pKiller).AddKill(this, pDeadObject);
        }
        else if (iAffiliation > 0)
        {
            GetPlayerStats(pKiller).AddAllyKill(this, pDeadObject);
        }
        pKiller->mAdrenaline.AddKill();
        bool bMessage = bPlayer;
        if (!NetGame::IsEnabled() && CameraManager::Instance()->GetNumCameras() == 1)
        {
            bMessage = bPlayer || pKiller->mPlayerId >= 0 || pKiller->mHeroFlag;
        }
        NetGame::MessageType eType;
        if (pKiller == this)
        {
            eType = NetGame::MSG_SUICIDE;
        }
        else
        {
            eType = pKiller->mTeamNumber != mTeamNumber ? NetGame::MSG_KILLED : NetGame::MSG_KILLED_TEAMMATE;
        }
        if (bMessage)
        {
            NetGame::CreateEvent_Message(eType, this, pKiller, rOwner.mClass, rOwner.mWeapon, pEnemyTeam, 0);
        }
        return;
    }

    int iKillerTeam = rOwner.mTeam;
    if (!iKillerTeam && rOwner.mGameObject)
    {
        iKillerTeam = rOwner.mGameObject->GetTeam();
    }
    if (iKillerTeam)
    {
        int iAffiliation = Team::GetPtr(iKillerTeam)->GetAffiliation(mTeamNumber);
        if (iAffiliation < 0)
        {
            PlayerStats::PointsProcessTeam(iKillerTeam, IsHero() ? 20 : (mPlayerId >= 0 ? 1 : 0));
        }
        else if (iAffiliation > 0)
        {
            PlayerStats::PointsProcessTeam(iKillerTeam, 4);
        }
    }
    else
    {
        GetPlayerStats(this).PointsProcess(3, 0);
    }
    if (bPlayer)
    {
        NetGame::CreateEvent_Message(NetGame::MSG_DIED, this, 0, rOwner.mClass, rOwner.mWeapon, pEnemyTeam, 0);
    }
}

// FUNCTION: SWBF2 0x00452cc0
void Character::Respawn()
{
    mUnit = 0;
    SetVehicle(0);
    mCarriedFlagPtr = 0;
    if (mPlayerId >= 0)
    {
        SetSpawnReady(false);
        return;
    }
    AI::AIGoalManager::Instance()->ReassignIfNeeded(this);
    int iClassIndex = mTeamPtr->PickRandomClassIndex(this);
    SelectClass(iClassIndex);
    SetSpawnReady(iClassIndex >= 0);
    SelectPost(SpawnManager::Instance()->RequestSpawnPoint(mPost, this));

    if (mPost)
    {
        mPost->GetGameObject();
    }
    mSpawnCycle = SpawnManager::Instance()->GetCycleIndex(mTeamNumber) + 1;
}

// FUNCTION: SWBF2 0x00453090
float Character::GetVehicleBlockTime() const
{
    return mVehicleBlockTime;
}

// FUNCTION: SWBF2 0x004530a0
void Character::SetVehicleBlockTime(float fTime)
{
    mVehicleBlockTime = fTime;
}

// FUNCTION: SWBF2 0x00453150
const CommandPost* Character::GetAssignedCP() const
{
    return mAssignedPost;
}

// FUNCTION: SWBF2 0x00453180
void Character::SetAssignedCP(const CommandPost* pPost)
{
    mAssignedPost = pPost;
}

// FUNCTION: SWBF2 0x00453190
void Character::CheckOutOfBounds(float fDeltaTime)
{
    Controllable* pUnits[3] = { mUnit, mVehicle, mRemote };
    bool bGuidedMissile = false;
    for (int i = 0; i < 3; ++i)
    {
        Controllable* pUnit = pUnits[i];
        if (!pUnit)
        {
            continue;
        }

        EntityFlyer* pFlyer = 0;
        if (pUnit->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            pFlyer = GetFlyer(pUnit);
            if (pFlyer && GetFlyerClass(pFlyer)->mGuidedMissile)
            {
                bGuidedMissile = true;
            }
        }

        if (!BoundaryManager::Instance()->IsOutOfBounds(pUnit->GetMatrix().GetTranslation()) &&
            !(pFlyer && GetFlyerAutoCorrectingToStayInBattlefield(pFlyer)))
        {
            pUnits[i] = 0;
        }
    }

    if (pUnits[0] || pUnits[1] || pUnits[2])
    {
        if (mOutOfBoundsTimer < 0.0f)
        {
            mOutOfBoundsTimer = 6.5f;
            if (bGuidedMissile)
            {
                mOutOfBoundsTimer = 2.1666667f;
            }
        }
        mOutOfBoundsTimer -= fDeltaTime;
        if (mOutOfBoundsTimer < 0.0f)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (pUnits[i])
                {
                    GetDamageable(pUnits[i]->GetGameObject())->InstantDeath(DamageOwner(0, 0, 0, 0, 0, 1.0f), true);
                }
            }
        }
    }
    else
    {
        mOutOfBoundsTimer = -1.0f;
    }
}

// FUNCTION: SWBF2 0x00453530
PlayerStats& Character::GetPlayerStats()
{
    return sPlayerStats[this - sCharacters];
}

namespace EventManager
{

// FUNCTION: SWBF2 0x00453560
template <>
void Event<Character, DamageOwner>::Callback::Trigger(Character& item, DamageOwner& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

namespace EventManager
{

// FUNCTION: SWBF2 0x0042cd60
template <>
void Event<Character, char>::Trigger(Character& item, char& param)
{
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        mRoot.Trigger(item, param);
    }
}

// FUNCTION: SWBF2 0x0042cdb0
template <>
void Event<Character, char>::Callback::Trigger(Character& item, char& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

// FUNCTION: SWBF2 0x00453100
float Character::GetObjectHitValue(int iAffiliation) const
{
    float fValue = mObjectHitTimer[iAffiliation + 1] * 10.0f;
    return (fValue < 1.0f) ? fValue : 1.0f;
}

// FUNCTION: SWBF2 0x00453130
float Character::GetObjectHitMultiplier(int iAffiliation) const
{
    return mObjectHitMultiplier[iAffiliation + 1];
}

// FUNCTION: SWBF2 0x00452230
void Character::SetUnit(Controllable* pUnit)
{
    Character* pBase = sCharacters && sCharacterLimit > 0 ? sCharacters : 0;
    if ((reinterpret_cast<char*>(this) - reinterpret_cast<char*>(pBase)) / 432 == -1)
    {
        ++gDebugCount;
    }
    mUnit = pUnit;
}

// FUNCTION: SWBF2 0x00453080
void Character::InitAll()
{
    return;
}

namespace
{
void RemoveCharacterUnit(Character* pCharacter)
{
    Controllable* pUnit = pCharacter->mUnit;
    if (!pUnit)
    {
        return;
    }
    if (netEnabledNext && netClientActive)
    {
        GameObject* pObject = pUnit->GetGameObject();
        if (pObject)
        {
            NetGame::Unlink(pObject);
        }
        if (pCharacter->mUnit)
        {
            pCharacter->mUnit->SetCharacter(0);
        }
    }
    else
    {
        pCharacter->SetUnit(0);
        pUnit->SetPilot(0);
        delete pUnit;
    }
    pCharacter->mUnit = 0;
    --pCharacter->mTeamPtr->mMemberActive;
}

void RemoveCharacterPilots(Character* pCharacter)
{
    if (pCharacter->mVehicle)
    {
        pCharacter->mVehicle->SetPilot(0);
        pCharacter->SetVehicle(0);
    }
    if (pCharacter->mRemote)
    {
        pCharacter->mRemote->SetPilot(0);
        pCharacter->SetRemote(0);
    }
    RemoveCharacterUnit(pCharacter);
}
}

// FUNCTION: SWBF2 0x00452dd0
void Character::MakePlayer(int iTeam, const unsigned short* pName, int iPlayerId)
{
    RemoveCharacterPilots(this);
    SetName(pName);
    PblUnicode::ustrncpy(mRegularName, pName, 64);
    mPlayerId = iPlayerId;
    ChangeTeam(iTeam);
    mTeamPtr = Team::sTeams[mTeamNumber];
    SelectClass(0);
    SelectPost(0);
    mDelayTimer = 0.0f;
    GetPlayerStats().Reset();
    Respawn();
}

// FUNCTION: SWBF2 0x00452f10
void Character::MakeAI()
{
    RemoveCharacterPilots(this);
    Character* pBase = Character::sCharacters && Character::sCharacterLimit > 0
                     ? Character::sCharacters : 0;
    const int iIndex = (reinterpret_cast<char*>(this) - reinterpret_cast<char*>(pBase)) / 432;
    mPlayerId = -1;
    ChangeTeam(2 - (iIndex & 1));
    mTeamPtr = Team::sTeams[mTeamNumber];
    unsigned short name[64];
    mTeamPtr->GenerateRandomName(name, mTeamNumber);
    SetName(name);
    PblUnicode::ustrncpy(mRegularName, name, 64);
    SelectClass(0);
    SelectPost(0);
    mDelayTimer = 0.0f;
    Respawn();
}

// FUNCTION: SWBF2 0x004533b0
void Character::AddPlayer(int iPlayerId)
{
    const int iTeam = NetGame::GetStartTeam(iPlayerId);
    Character* pCharacter = Character::GetPtr(iPlayerId);
    if (!(NetGame::IsEnabled() && netEnabledNext && netClientActive))
    {
        pCharacter->GetPlayerStats().Reset();
    }
    unsigned short name[64];
    if (NetGame::IsEnabled())
    {
        PblUnicode::ustrncnv(name, NetGame::GetPlayerName(iPlayerId), 64);
    }
    else
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
        const unsigned short* pName = pProfile ? pProfile->GetName() : 0;
        if (pName && *pName)
        {
            PblUnicode::ustrlcpy(name, pName, 64);
        }
        else
        {
            PblUnicode::usnprintf(name, 64, "Player %d", iPlayerId + 1);
        }
    }
    pCharacter->MakePlayer(iTeam, name, iPlayerId);
}

// FUNCTION: SWBF2 0x004534c0
void Character::RemovePlayer(int iPlayerId)
{
    Character* pCharacter = Character::GetPtr(iPlayerId);
    if (!(NetGame::IsEnabled() && netEnabledNext && netClientActive))
    {
        pCharacter->GetPlayerStats().Reset();
    }
    pCharacter->MakeAI();
}

// FUNCTION: SWBF2 0x004530b0
void Character::RegisterHit(int iTeam, float fMultiplier)
{
    int i = mTeamPtr->GetAffiliation(iTeam) + 1;
    if (mObjectHitTimer[i] == 0.0f || fMultiplier > mObjectHitMultiplier[i])
    {
        mObjectHitTimer[i] = 0.5f;
        mObjectHitMultiplier[i] = fMultiplier;
    }
}

// GLOBAL: SWBF2 0x01e3033c
int Character::sCharacterCount;

void Character::DestroyAll()
{
    sCharacterCount = 0;
    sCharacterLimit = 0;
    delete[] sCharacters;
    sCharacters = 0;
    delete[] sPlayerStats;
    sPlayerStats = 0;
}

// GLOBAL: SWBF2 0x01e2fde7
bool gDoTakeoverSpawns = false;
