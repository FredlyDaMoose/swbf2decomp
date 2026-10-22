#include "TeamAssist.h"
#include "TargetManager.h"
#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"

namespace AI
{

enum VOType
{
    VOT_TA_First = 40
};
namespace VOHelper
{
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const PblVector3& vPos, float fDelay);
}
}

// GLOBAL: SWBF2 0x01eafcc4
unsigned int TeamAssist::sColorHash;

static const unsigned int sMarkerTexture[TeamAssistManager::Max] = {
    0x1e9ab20a,
    0x16d1df7b,
    0x407c4980,
    0x11171965,
    0x6533cbcf,
    0x971f3155,
    0xb3804d5f,
    0x4cd87d13,
};

// GLOBAL: SWBF2 0x01eafcc8
TeamAssistManager* TeamAssistManager::sInstance;

float TeamAssistManager::sDuration = 5.0f;

// GLOBAL: SWBF2 0x01f7ce88
// SYNTHETIC: SWBF2 0x00406920
// `dynamic initializer for 'TeamAssist::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769560
// `dynamic atexit destructor for 'TeamAssist::sMemoryPool''
MemoryPool TeamAssist::sMemoryPool("TeamAssist", sizeof(TeamAssist));

// FUNCTION: SWBF2 0x00654bd0
void TeamAssistManager::Create()
{
    sInstance = new TeamAssistManager;
}

// FUNCTION: SWBF2 0x00654c60
TeamAssistManager::TeamAssistManager()
{
    mList = 0;
    ActivateThread(0, PER_LOCAL_TURN);
}

// SYNTHETIC: SWBF2 0x00654cc0
// TeamAssistManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00654cf0
TeamAssistManager::~TeamAssistManager()
{
    DeactivateThread();
    DeleteRest(mList);
}

// FUNCTION: SWBF2 0x00654d50
void TeamAssistManager::DeleteRest(TeamAssist* pAssist)
{
    while (pAssist)
    {
        TeamAssist* pNext = pAssist->mNext;
        delete pAssist;
        pAssist = pNext;
    }
}

// FUNCTION: SWBF2 0x00654d80
void TeamAssistManager::AddRequest(Code code, Character* pCharacter)
{
    sInstance->mList = new TeamAssist(sInstance->mList, code, pCharacter, sDuration);
}

// FUNCTION: SWBF2 0x00654e40
bool TeamAssistManager::Update(float fDeltaTime)
{
    if (mList)
    {
        if (!mList->Update(fDeltaTime))
        {
            DeleteRest(mList);
            mList = 0;
            return true;
        }
        TeamAssist* pPrev = mList;
        for (TeamAssist* pAssist = pPrev->mNext; pAssist; pAssist = pAssist->mNext)
        {
            if (!pAssist->Update(fDeltaTime))
            {
                DeleteRest(pAssist);
                pPrev->mNext = 0;
                break;
            }
            pPrev = pAssist;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00654eb0
TeamAssist::TeamAssist(TeamAssist* pNext, int type, Character* pRequestor, float fTimer)
{
    mNext = pNext;
    mType = type;
    mRequestor = 0;
    mTimer = fTimer;
    Controllable* pControllable = pRequestor->mRemote;
    if (!pControllable)
    {
        pControllable = pRequestor->mVehicle;
    }
    if (!pControllable)
    {
        pControllable = pRequestor->mUnit;
    }
    if (pControllable)
    {
        mRequestor = static_cast<Trackable*>(pControllable)->GetGameObject();
        mTeamMask = 1 << pRequestor->mTeamNumber;
        TargetManager::AddMarker(mRequestor, 0, sMarkerTexture[type], 5.0f, mTeamMask, sColorHash, false, true, true, 0.0f);
        AI::VOHelper::Play((AI::VOType)(AI::VOT_TA_First + type), mRequestor, 0,
                           static_cast<CollisionObject*>(mRequestor)->GetPosition(), 0.0f);
    }
}

// FUNCTION: SWBF2 0x00654f70
TeamAssist::~TeamAssist()
{
    if (mRequestor)
    {
        TargetManager::RemoveMarker(mRequestor, 0, mTeamMask);
    }
}

// FUNCTION: SWBF2 0x00654f90
bool TeamAssist::Update(float fDeltaTime)
{
    mTimer -= fDeltaTime;
    return mTimer > 0.0f;
}

// FUNCTION: SWBF2 0x00654c40
void TeamAssistManager::Destroy()
{
    delete sInstance;
    sInstance = 0;
}
