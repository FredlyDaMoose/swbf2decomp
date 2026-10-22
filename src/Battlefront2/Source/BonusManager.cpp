#include <string.h>

#include "BonusManager.h"

#include "Character.h"
#include "CommandPost.h"
#include "Controllable.h"
#include "Damageable.h"
#include "Entity.h"
#include "EntityDefenseGridTurret.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "GameState.h"
#include "NetGame.h"
#include "Team.h"
#include "Trackable.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/RedPath.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netHeroesEnabled;
extern int netHeroRuleTeam;
extern int netHeroRuleUnlock;
extern int netHeroRuleUnlockVal;
extern int netHeroRulePlayer;
extern int netHeroRuleRespawn;
extern int netHeroRuleRespawnVal;
extern bool netHeroUnlocked[3];
extern int netHeroPlayerID[3];
extern int netHeroAIID[3];
extern bool netHeroFirstUnlocked[3];

static Trackable* GetTrackable(Controllable* pControllable)
{
    return static_cast<Trackable*>(pControllable);
}

static Damageable* GetDamageable(GameObject* pObject)
{
    return static_cast<Damageable*>(pObject);
}

static float& CurHealth(Damageable* pDamageable)
{
    return pDamageable->mCurHealth;
}

static float& MaxHealth(Damageable* pDamageable)
{
    return pDamageable->mMaxHealth;
}

static bool IsAlive(const GameObject* pGameObject)
{
    return pGameObject->Damageable::IsAlive();
}

static int GetTeam(const GameObject* pGameObject)
{
    return pGameObject->GetTeam();
}

static void SetPerceivedTeam(GameObject* pGameObject, int iTeam)
{
    pGameObject->SetPerceivedTeam(iTeam);
}

static void SetTeam(GameObject* pGameObject, int iTeam)
{
    pGameObject->SetTeam(iTeam);
}

static Entity* AsEntity(EntityDefenseGridTurret* pTurret)
{
    return reinterpret_cast<Entity*>(pTurret);
}

static GameObject* GetGameObject(CommandPost* pPost)
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

static int GetSide(const Team* pTeam)
{
    return pTeam->mSide;
}

const float BonusManager::sTeamBonusLeaderUnlockTime = 120.0f;
const float BonusManager::sTeamBonusGarrisonThreshold = 0.2f;
const float BonusManager::sTeamBonusGarrisonExtraReinforcements = 0.2f;

// GLOBAL: SWBF2 0x01eb0630
// SYNTHETIC: SWBF2 0x00402350
// `dynamic initializer for 'BonusManager::sInstance''
// SYNTHETIC: SWBF2 0x00766db0
// `dynamic atexit destructor for 'BonusManager::sInstance''
BonusManager BonusManager::sInstance;

// FUNCTION: SWBF2 0x0044c5b0
BonusManager::BonusManager()
{
    mNumCPs = 0;
    mbIsBonusAutoturretsInitialized = false;
    mpCommandPostData = 0;
    ClearTeamBonuses();
    ClearIndividualBonuses();
}

// SYNTHETIC: SWBF2 0x0044c630
// BonusManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0044c660
BonusManager::~BonusManager()
{
    if (IsThreadActive())
    {
        DeactivateThread();
    }
}

// FUNCTION: SWBF2 0x0044c5a0
void BonusManager::Reset()
{
    sInstance.ClearTeamBonuses();
    sInstance.ClearIndividualBonuses();
}

// FUNCTION: SWBF2 0x0044c580
bool BonusManager::Destroy()
{
    Reset();
    sInstance.DeactivateThread();
    return false;
}

// FUNCTION: SWBF2 0x0044c6c0
void BonusManager::InitialUpdate()
{
    ActivateThread(-10, PER_TURN);
    InitialUpdateTeamBonusAutoturrets();
}

// FUNCTION: SWBF2 0x0044c6e0
bool BonusManager::Update(float fDeltaTime)
{
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        UpdateTeamBonuses(fDeltaTime);
        UpdateIndividualBonuses(fDeltaTime);
    }
    return true;
}

void BonusManager::UpdateIndividualBonuses(float)
{
}

// FUNCTION: SWBF2 0x0044c720
void BonusManager::ClearTeamBonuses()
{
    mVOStartTimer = 5.0f;
    for (int i = 0; i < 2; ++i)
    {
        mTeamBonuses[i] = 0;
        mUnlockHeroTimer[i] = 0.0f;
        mbDidUseGarrison[i] = false;
    }
    CleanupTeamBonusAutoturrets();
}

// FUNCTION: SWBF2 0x0044c750
int BonusManager::GetTeamIndex(int iTeam)
{
    if (GameState::GetState() == 0x8FF60339 && GameLoop::AreMetagameRulesOn() &&
        GetSide(Team::sTeams[1]) > GetSide(Team::sTeams[2]))
    {
        return 2 - iTeam;
    }
    return iTeam - 1;
}

// FUNCTION: SWBF2 0x0044c790
void BonusManager::SetTeamBonusActive(int iTeam, unsigned int uBonus, bool bActive)
{
    int iIndex = GetTeamIndex(iTeam);
    if (bActive)
    {
        mTeamBonuses[iIndex] = uBonus;
        if (GameState::GetState() == 0x8FF60339 && mVOStartTimer < 0.0f)
        {
            PlayTeamBonusVO(uBonus, iTeam);
        }
    }
    else
    {
        mTeamBonuses[iIndex] = 0;
    }
}

// FUNCTION: SWBF2 0x0044c7e0
bool BonusManager::IsTeamBonusActive(int iTeam, unsigned int uBonus)
{
    if (iTeam < 1 || iTeam > 2)
    {
        return false;
    }
    return mTeamBonuses[GetTeamIndex(iTeam)] == uBonus;
}

// FUNCTION: SWBF2 0x0044c810
void BonusManager::ClearIndividualBonuses()
{
    memset(mIndividualBonuses, 0, sizeof(mIndividualBonuses));
}

// FUNCTION: SWBF2 0x0044c830
void BonusManager::UpdateTeamBonuses(float fDeltaTime)
{
    if (mVOStartTimer >= 0.0f)
    {
        mVOStartTimer -= fDeltaTime;
        if (mVOStartTimer < 0.0f)
        {
            for (int iTeam = 1; iTeam <= 2; ++iTeam)
            {
                unsigned int uBonus = mTeamBonuses[GetTeamIndex(iTeam)];
                if (uBonus)
                {
                    PlayTeamBonusVO(uBonus, iTeam);
                }
            }
        }
    }

    for (int iTeam = 1; iTeam < 3; ++iTeam)
    {
        UpdateTeamBonusBactaTanks(iTeam, fDeltaTime);
        UpdateTeamBonusAutoturrets(iTeam, fDeltaTime);
        UpdateTeamBonusLeader(iTeam, fDeltaTime);
        UpdateTeamBonusGarrison(iTeam, fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x0044c9c0
void BonusManager::UpdateTeamBonusBactaTanks(int iTeam, float fDeltaTime)
{
    if (!IsTeamBonusActive(iTeam, 0xc10b9cdc))
    {
        return;
    }

    Team* pTeam = Team::sTeams[iTeam];
    for (int i = 0; i < pTeam->mMemberCount; ++i)
    {
        Character* pCharacter = pTeam->mMemberArray[i];
        if (!pCharacter || !pCharacter->mUnit || !GetTrackable(pCharacter->mUnit)->GetGameObject())
        {
            continue;
        }

        Damageable* pDamageable = GetDamageable(GetTrackable(pCharacter->mUnit)->GetGameObject());
        float fHealth = CurHealth(pDamageable);
        if (fHealth > 0.0f)
        {
            float fMaxHealth = MaxHealth(pDamageable);
            float fNewHealth = fHealth + fMaxHealth * 0.01f * fDeltaTime;
            if (!(fMaxHealth > fNewHealth))
            {
                fNewHealth = fMaxHealth;
            }
            pDamageable->SetCurHealth(fNewHealth, true, 0);
        }
    }
}

// FUNCTION: SWBF2 0x0044c8d0
void BonusManager::UpdateTeamBonusLeader(int iTeam, float fDeltaTime)
{
    if (!IsTeamBonusActive(iTeam, 0xeac1e9d4))
    {
        return;
    }

    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        netHeroesEnabled = true;
    }

    netHeroRuleUnlock = 7;
    netHeroRuleUnlockVal = 5;
    netHeroRulePlayer = 2;
    netHeroRuleTeam = 3;
    netHeroRuleRespawn = 2;
    netHeroRuleRespawnVal = 120;

    if (netHeroPlayerID[iTeam] == -1 && netHeroAIID[iTeam] == -1 && !netHeroFirstUnlocked[iTeam])
    {
        mUnlockHeroTimer[iTeam - 1] -= fDeltaTime;
        if (mUnlockHeroTimer[iTeam - 1] < 0.0f)
        {
            mUnlockHeroTimer[iTeam - 1] = sTeamBonusLeaderUnlockTime;
            netHeroUnlocked[iTeam] = true;
            netHeroFirstUnlocked[iTeam] = true;
        }
    }
}

// FUNCTION: SWBF2 0x0044cfe0
void BonusManager::UpdateTeamBonusGarrison(int iTeam, float fDeltaTime)
{
    if (!IsTeamBonusActive(iTeam, 0x96f36950))
    {
        return;
    }

    Team* pTeam = Team::sTeams[iTeam];
    if (pTeam->mCurReinforcements <= (int)(pTeam->mMaxReinforcements * sTeamBonusGarrisonThreshold) &&
        !mbDidUseGarrison[iTeam - 1])
    {
        pTeam->AddReinforcements((int)(pTeam->mMaxReinforcements * sTeamBonusGarrisonExtraReinforcements));
        mbDidUseGarrison[iTeam - 1] = true;
        PlayTeamBonusVO(0xd6a6e644, iTeam);
    }
}

// FUNCTION: SWBF2 0x0044ca80
void BonusManager::InitialUpdateTeamBonusAutoturrets()
{
    mNumCPs = CommandPost::GetCount();
    if (mNumCPs <= 0)
    {
        return;
    }

    if (EntityDefenseGridTurret::sMemoryPool.GetCount() < (unsigned int)mNumCPs)
    {
        EntityDefenseGridTurret::sMemoryPool.SetCount(mNumCPs);
    }

    mpCommandPostData = new CommandPostData[mNumCPs];
    for (int i = 0; i < mNumCPs; ++i)
    {
        CommandPostData& data = mpCommandPostData[i];
        data.pCommandPost = CommandPost::FindPostByIndex(i);
        const RedPath* pPath = data.pCommandPost->GetTurretPath();
        if (pPath)
        {
            int iNumPoints = pPath->GetNumPoints();
            data.ppTurrets = new EntityDefenseGridTurret*[iNumPoints];
            for (int j = 0; j < iNumPoints; ++j)
            {
                data.ppTurrets[j] = 0;
            }
        }
        else
        {
            data.ppTurrets = new EntityDefenseGridTurret*[1];
            data.ppTurrets[0] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0044cb80
void BonusManager::CleanupTeamBonusAutoturrets()
{
    if (mpCommandPostData)
    {
        for (int i = 0; i < mNumCPs; ++i)
        {
            CommandPostData& data = mpCommandPostData[i];
            if (data.pCommandPost && data.ppTurrets)
            {
                RemoveTurretsFromCP(data);
            }
            if (data.ppTurrets)
            {
                delete[] data.ppTurrets;
                data.ppTurrets = 0;
            }
        }
        delete[] mpCommandPostData;
        mpCommandPostData = 0;
    }
    mbIsBonusAutoturretsInitialized = false;
    mNumCPs = 0;
}

// FUNCTION: SWBF2 0x0044cc10
void BonusManager::UpdateTeamBonusAutoturrets(int iTeam, float fDeltaTime)
{
    bool bActive = IsTeamBonusActive(iTeam, 0xde68cb69);
    for (int i = 0; i < mNumCPs; ++i)
    {
        CommandPostData& data = mpCommandPostData[i];
        CommandPost* pPost = data.pCommandPost;
        if (bActive)
        {
            if (IsAlive(GetGameObject(pPost)) && GetTeam(GetGameObject(pPost)) == iTeam)
            {
                AddTurretsToCP(data, iTeam);
            }
        }
        else
        {
            if (GetGameObject(pPost) && GetTeam(GetGameObject(pPost)) == iTeam)
            {
                RemoveTurretsFromCP(data);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0044cd40
void BonusManager::AddTurretsToCP(CommandPostData& data, int iTeam)
{
    const RedPath* pPath = data.pCommandPost->GetTurretPath();
    if (pPath)
    {
        for (int i = 0; i < pPath->GetNumPoints(); ++i)
        {
            EntityDefenseGridTurret* pTurret = data.ppTurrets[i];
            if (!pTurret)
            {
                const PblVector3& vPosition = pPath->GetPointPosition(i);
                PblQuaternion qRotation;
                PblMatrix matrix;
                D3DXMatrixRotationQuaternion(&matrix,
                                             D3DXQuaternionNormalize(&qRotation, &pPath->GetPointRotation(i)));
                matrix.m[3][0] = vPosition.x;
                matrix.m[3][1] = vPosition.y;
                matrix.m[3][2] = vPosition.z;
                matrix.m[3][3] = 1.0f;
                data.ppTurrets[i] = MakeNewAutoturret(matrix, iTeam);
            }
            else if (GetTeam(pTurret) != iTeam)
            {
                delete AsEntity(pTurret);
                data.ppTurrets[i] = 0;
            }
        }
    }
    else
    {
        EntityDefenseGridTurret* pTurret = data.ppTurrets[0];
        if (!pTurret)
        {
            data.ppTurrets[0] = MakeNewAutoturret(data.pCommandPost->GetMatrix(), iTeam);
        }
        else if (GetTeam(pTurret) != iTeam)
        {
            delete AsEntity(pTurret);
            data.ppTurrets[0] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0044ce90
void BonusManager::RemoveTurretsFromCP(CommandPostData& data)
{
    const RedPath* pPath = data.pCommandPost->GetTurretPath();
    if (pPath)
    {
        for (int i = 0; i < pPath->GetNumPoints(); ++i)
        {
            if (data.ppTurrets[i])
            {
                delete AsEntity(data.ppTurrets[i]);
                data.ppTurrets[i] = 0;
            }
        }
    }
    else if (data.ppTurrets[0])
    {
        delete AsEntity(data.ppTurrets[0]);
        data.ppTurrets[0] = 0;
    }
}

// FUNCTION: SWBF2 0x0044cf00
EntityDefenseGridTurret* BonusManager::MakeNewAutoturret(const PblMatrix& matrix, int iTeam)
{
    EntityClass* pClass = EntityClass::Find(0x07673219);
    if (!pClass || !pClass->IsRtti(EntityDefenseGridTurretClass::GetRtti()))
    {
        return 0;
    }

    EntityDesc desc;
    desc.mMatrix = matrix;
    Entity* pEntity = pClass->Build(desc);
    if (pEntity)
    {
        GameObject* pObject = static_cast<EntityDefenseGridTurret*>(pEntity);
        SetTeam(pObject, iTeam);
        SetPerceivedTeam(pObject, iTeam);
        pEntity->Init();
    }
    return reinterpret_cast<EntityDefenseGridTurret*>(pEntity);
}

// FUNCTION: SWBF2 0x0044d050
void BonusManager::PlayTeamBonusVO(unsigned int uBonus, int iTeam)
{
    char acName[128] = "team_bonus_";

    const char* pcBonus;
    switch (uBonus)
    {
    case 0x85ad826d: pcBonus = "energy_boost"; break;
    case 0x56e2ce71: pcBonus = "sensor_array"; break;
    case 0x96f36950: pcBonus = "garrison"; break;
    case 0xbb03442e: pcBonus = "advanced_blasters"; break;
    case 0xc10b9cdc: pcBonus = "bacta_tanks"; break;
    case 0xc81956e6: pcBonus = "supplies"; break;
    case 0xd265e905: pcBonus = "sabotage"; break;
    case 0xd6a6e644: pcBonus = "garrison_used_by"; break;
    case 0xddb212e1: pcBonus = "combat_shielding"; break;
    case 0xde68cb69: pcBonus = "autoturrets"; break;
    case 0xeac1e9d4: pcBonus = "leader"; break;
    default: return;
    }
    strlcat(acName, pcBonus, sizeof(acName));

    for (int i = 1; i <= 2; ++i)
    {
        Team* pTeam = Team::sTeams[i];
        if (!Team::sTeams[iTeam] || !pTeam)
        {
            continue;
        }

        strlcat(acName, i == iTeam ? "_us_" : "_them_", sizeof(acName));
        switch (GetSide(pTeam))
        {
        case 2: strlcat(acName, "imp", sizeof(acName)); break;
        case 3: strlcat(acName, "rep", sizeof(acName)); break;
        case 4: strlcat(acName, "cis", sizeof(acName)); break;
        default: strlcat(acName, "all", sizeof(acName)); break;
        }

        NetGame::CreateEvent_PlayVoiceOver(PblHash(acName), 1 << i);

        if (i == iTeam)
        {
            acName[strlen(acName) - 7] = '\0';
        }
        else
        {
            acName[strlen(acName) - 9] = '\0';
        }
    }
}
