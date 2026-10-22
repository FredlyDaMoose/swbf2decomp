#include "SpawnManager.h"

#include <float.h>

#include "AIGoalManager.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "Team.h"
#include "ToolTips.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class EntitySoldier { public: static MemoryPool sMemoryPool; };

// GLOBAL: SWBF2 0x01eafb34
SpawnManager* SpawnManager::sInstance;
// GLOBAL: SWBF2 0x01eafb38
SpawnManager* SpawnManager::sHostInstance;
// GLOBAL: SWBF2 0x01eafb3c
SpawnManager* SpawnManager::sClientInstance;

// FUNCTION: SWBF2 0x0064e070
void SpawnManager::CreateClient()
{
    if (NetGame::IsOneWorld())
    {
        sHostInstance->DeactivateThread();
        sHostInstance->ActivateThread(0, PER_LOCAL_TURN);
    }
    else
    {
        sClientInstance = new SpawnManager;
    }
}

// FUNCTION: SWBF2 0x0064e110
void SpawnManager::DestroyClient()
{
    if (!NetGame::IsOneWorld())
    {
        Character::DestroyAll();
    }
}

// FUNCTION: SWBF2 0x0064e160
void SpawnManager::EnterClient()
{
    if (!netIsOneWorld)
    {
        sInstance = sClientInstance;
    }
}

// FUNCTION: SWBF2 0x0064e180
void SpawnManager::LeaveClient()
{
    if (!netIsOneWorld)
    {
        sInstance = sHostInstance;
    }
}

extern int netPlayerLimit;
extern int netNumBots;
extern bool gAttractMode;

namespace PlayerPrefs { int __fastcall FindByPlayerId(int iPlayer); }

namespace FriendBuddyList { bool __fastcall IsFriendBuddy(int iLocalIndex, int iPlayerId); }

namespace PblUnicode { unsigned int ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount); }

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

static bool IsAlive(const GameObject* pObject)
{
    return pObject->IsAlive();
}

static int GetTeam(const GameObject* pObject)
{
    return pObject->GetTeam();
}

static int GetAllyCount(const CommandPost* pPost)
{
    return pPost->mAllyCount;
}

// FUNCTION: SWBF2 0x0061c490
int __fastcall PlayerPrefs::FindByPlayerId(int iPlayer)
{
    for (int i = 0; i < 1; ++i)
    {
        if ((&PlayerPrefs::mAllUserPrefs[0].mPlayerId)[i * 11] == iPlayer)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0064e000
void SpawnManager::Create()
{
    sHostInstance = new SpawnManager;
    sClientInstance = 0;
    sInstance = sHostInstance;
}

// FUNCTION: SWBF2 0x0064e1a0
SpawnManager::SpawnManager()
{
    for (int i = 0; i < 8; ++i)
    {
        mMaxUnitCount[i] = 0;
        mCurrentUnitCount[i] = 0;
    }
    mAllyUnitCount = 0;
    for (int iTeam = 0; iTeam < 8; ++iTeam)
    {
        if (iTeam == 1 || iTeam == 2)
        {
            SetSpawnDelay(15.0f, 0.5f, iTeam);
        }
        else
        {
            SetSpawnDelay(0.0f, 0.0f, iTeam);
        }
    }
    mHistorical = false;
    ActivateThread(0, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x0064e280
SpawnManager::~SpawnManager()
{
}

// FUNCTION: SWBF2 0x0064e290
void SpawnManager::SetSpawnDelay(float fCycleDelay, float fSlotDelay, int iTeam)
{
    if (iTeam >= 0 && iTeam < 8)
    {
        mCycleDelay[iTeam] = fCycleDelay;
        mSlotDelay[iTeam] = fSlotDelay;
    }
}

// FUNCTION: SWBF2 0x0064e2b0
void SpawnManager::AddPlayerCharacter(int iTeam, int iPlayer)
{
    unsigned short name[32];
    if (NetGame::IsEnabled())
    {
        PblUnicode::ustrncnv(name, (const unsigned char*)NetGame::GetPlayerName(iPlayer), 32);
    }
    else
    {
        PlayerProfile::tProfile* pProfile = 0;
        int iSlot = PlayerPrefs::FindByPlayerId(iPlayer);
        if (iSlot >= 0)
        {
            pProfile = PlayerProfile::GetProfile(playerJoystickNumbers[11 * iSlot]);
        }
        const unsigned short* pName = pProfile->GetName();
        if (pProfile && *pName)
        {
            PblUnicode::ustrlcpy(name, pName, 32);
        }
        else
        {
            PblUnicode::usnprintf(name, 32, "Player %d", iPlayer + 1);
        }
    }
    Team::sTeams[iTeam]->CreateCharacter(name, iPlayer);
    ++mCurrentUnitCount[iTeam];

    if (NetGame::IsEnabled() && ToolTipsDisplay::sInstance->ShouldTestForToolTipState(TTT_FriendsListColor))
    {
        Team* pTeam = Team::sTeams[iTeam];
        int iCount = pTeam->mMemberCount;
        for (int i = 0; i < iCount; ++i)
        {
            if (pTeam->GetMemberPtr(i)->GetControllable() &&
                pTeam->GetMemberPtr(i)->GetControllable()->GetGameObject())
            {
                int iPlayerId = pTeam->GetMemberPtr(i)->mPlayerId;
                if (FriendBuddyList::IsFriendBuddy(NetGame::GetLocalIndex(iPlayer), iPlayerId))
                {
                    ToolTipsDisplay::sInstance->ShowTip(TTT_FriendsListColor, 0);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0064e460
int SpawnManager::GetTotalCharacterCount()
{
    int iCount = mMaxUnitCount[1] + mMaxUnitCount[2];
    if (NetGame::IsEnabled() && netPlayerLimit + 2 * netNumBots > iCount)
    {
        iCount = netPlayerLimit + 2 * netNumBots;
    }
    iCount += mMaxUnitCount[0] + mMaxUnitCount[3] + mMaxUnitCount[4] + mMaxUnitCount[5] +
              mMaxUnitCount[6] + mMaxUnitCount[7];
    if (iCount > 600)
    {
        iCount = 600;
    }
    return iCount;
}

// FUNCTION: SWBF2 0x0064e4c0
void SpawnManager::SetupSoldierMemoryPool()
{
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EntitySoldier::sMemoryPool.Setup(GetTotalCharacterCount(), EntitySoldier::sMemoryPool.mSize,
                                         EntitySoldier::sMemoryPool.mGrow);
    }
}

// FUNCTION: SWBF2 0x005b41d0
int Team::FindSide(Side eSide)
{
    for (int i = 0; i < 8; i++)
    {
        if (sTeams[i]->GetSide() == eSide)
        {
            return i;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0064e510
void SpawnManager::InitialUpdate()
{
    if (NetGame::IsEnabled() && NetGame::IsClient() && !NetGame::IsOneWorld())
    {
        SpawnManager* pHost = sHostInstance;
        for (int i = 0; i < 8; ++i)
        {
            mMaxUnitCount[i] = pHost->mMaxUnitCount[i];
            mCycleDelay[i] = pHost->mCycleDelay[i];
            mSlotDelay[i] = pHost->mSlotDelay[i];
        }
    }

    Character::CreateAll(GetTotalCharacterCount());

    for (int i = 0; i < 8; ++i)
    {
        mCycleTimer[i] = mCycleDelay[i];
        if (gAttractMode || !NetGame::IsEnabled())
        {
            mCycleTimer[i] = 0.1f;
        }
        mSlotTimer[i] = mSlotDelay[i];
        mCycleIndex[i] = 0;
    }

    if (NetGame::IsEnabled())
    {
        for (int i = 0; i < netPlayerLimit; ++i)
        {
            int iTeam = NetGame::GetStartTeam(i);
            if (mCurrentUnitCount[iTeam] >= mMaxUnitCount[iTeam])
            {
                iTeam = 3 - iTeam;
                NetGame::SetStartTeam(i, iTeam);
            }

            unsigned short name[64];
            unsigned short otherName[64];
            Team::sTeams[iTeam]->GenerateRandomName(name, iTeam);
            Team::sTeams[3 - iTeam]->GenerateRandomName(otherName, 3 - iTeam);

            if (NetGame::IsPlaying(i, false))
            {
                AddPlayerCharacter(iTeam, i);
            }
            else
            {
                int iBotTeam = mCurrentUnitCount[1] < mCurrentUnitCount[2] ? 1 : 2;
                if (mCurrentUnitCount[iBotTeam] >= mMaxUnitCount[iBotTeam])
                {
                    iBotTeam = 3 - iBotTeam;
                }
                Team::sTeams[iBotTeam]->CreateCharacter(iBotTeam == NetGame::GetStartTeam(i) ? name : otherName, -1);
                ++mCurrentUnitCount[iBotTeam];
            }
        }
    }
    else if (GameLoop::AreMetagameRulesOn())
    {
        for (unsigned int i = 0; i < GameLoop::mNumCameras; ++i)
        {
            int iTeam = 1;
            int iSlot = PlayerPrefs::FindByPlayerId(i);
            if (iSlot >= 0)
            {
                iTeam = Team::FindSide(static_cast<Team::Side>(GameLoop::GetMetagameSide(iSlot)));
            }
            AddPlayerCharacter(iTeam, i);
        }
    }
    else if (!gAttractMode)
    {
        for (unsigned int i = 0; i < GameLoop::mNumCameras; ++i)
        {
            int iTeam = 1;
            int iSlot = PlayerPrefs::FindByPlayerId(i);
            if (iSlot >= 0)
            {
                iTeam = playerTeamNumbers[11 * iSlot];
            }
            AddPlayerCharacter(iTeam, i);
        }
    }

    for (int iTeam = 1; iTeam < 8; ++iTeam)
    {
        for (int i = mCurrentUnitCount[iTeam]; i < mMaxUnitCount[iTeam]; ++i)
        {
            if (Character::sCharacterLimit < Character::sCharacterCount)
            {
                unsigned short name[64];
                Team::sTeams[iTeam]->GenerateRandomName(name, iTeam);
                Team::sTeams[iTeam]->CreateCharacter(name, -1);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0064ea40
bool SpawnManager::Update(float fDeltaTime)
{
    mAllyUnitCount = 0;
    for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
    {
        CommandPost* pPost = NetGame::ConvertCP(*itor);
        if (pPost && Team::sTeams[GetTeam(GetGameObject(pPost))]->mAffiliation[3] > 0)
        {
            mAllyUnitCount += GetAllyCount(pPost);
        }
    }
    if (mAllyUnitCount > mMaxUnitCount[3])
    {
        mAllyUnitCount = mMaxUnitCount[3];
    }

    bool bPreGame = NetGame::IsPreGame();
    for (int i = 0; i < 8; ++i)
    {
        mCycleTimer[i] -= fDeltaTime;
        if (mCycleTimer[i] <= 0.0f)
        {
            if (bPreGame)
            {
                mCycleTimer[i] += 0.5f;
            }
            else
            {
                mCycleTimer[i] += mCycleDelay[i];
                if (gAttractMode && mCycleIndex[i] < 3)
                {
                    mCycleTimer[i] = 1.0f;
                }
            }
            ++mCycleIndex[i];
            mSlotTimer[i] = mSlotDelay[i];
        }
        mSlotTimer[i] -= fDeltaTime;
    }

    for (int iTeam = 1; iTeam < 8; ++iTeam)
    {
        Team::sTeams[iTeam]->Update(fDeltaTime);
    }
    return true;
}

// FUNCTION: SWBF2 0x0064eba0
CommandPost* SpawnManager::RequestSpawnPoint(CommandPost* pPost, Character* pChr)
{
    if (pChr->mPlayerId >= 0)
    {
        return Request(pPost, pChr->mTeamNumber);
    }
    if (pPost && IsAlive(GetGameObject(pPost)) && pPost->CanTeamSpawnHere(pChr->mTeamNumber))
    {
        return pPost;
    }
    return AI::AIGoalManager::Instance()->GetSpawnLocation(pChr);
}

// FUNCTION: SWBF2 0x0064ec30
CommandPost* SpawnManager::Request(CommandPost* pPost, int iTeam)
{
    if (!pPost || !GetGameObject(pPost))
    {
        return NetGame::ConvertCP(CommandPostManager::Instance()->FindPostClosestToEnemy(iTeam));
    }

    if (IsAlive(GetGameObject(pPost)) && pPost->CanTeamSpawnHere(iTeam))
    {
        return pPost;
    }

    float fBestDistSq = FLT_MAX;
    const float* pPos = pPost->GetMatrix().m[3];
    CommandPost* pBest = 0;
    for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
    {
        CommandPost* pOther = NetGame::ConvertCP(*itor);
        if (pOther && pOther->CanTeamSpawnHere(iTeam))
        {
            const float* pOtherPos = pOther->GetMatrix().m[3];
            float fX = pOtherPos[0] - pPos[0];
            float fY = pOtherPos[1] - pPos[1];
            float fZ = pOtherPos[2] - pPos[2];
            float fDistSq = fX * fX + fY * fY + fZ * fZ;
            if (fDistSq < fBestDistSq)
            {
                fBestDistSq = fDistSq;
                pBest = pOther;
            }
        }
    }
    return pBest;
}

// FUNCTION: SWBF2 0x0064ed80
CommandPost* SpawnManager::GetRandomSpawnPoint(int iTeam)
{
    CommandPost* posts[64];
    int iCount = 0;
    for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
    {
        CommandPost* pPost = NetGame::ConvertCP(*itor);
        if (pPost && pPost->CanTeamSpawnHere(iTeam))
        {
            posts[iCount++] = pPost;
        }
    }
    if (iCount <= 0)
    {
        return 0;
    }
    return posts[PblRandom::GetSim().GetInt() % iCount];
}

// FUNCTION: SWBF2 0x005b4210
void Team::OverrideMaxReinforcements(int iTeam, int iMax)
{
    sTeamMaxReinforcementsOverride[iTeam] = iMax;
}

// FUNCTION: SWBF2 0x0064e120
void SpawnManager::Destroy()
{
    sInstance = 0;
    sHostInstance = 0;
    sClientInstance = 0;
    for (int i = 0; i < 2; ++i)
    {
        Team::OverrideMaxReinforcements(i, -1);
    }
    Character::DestroyAll();
}
