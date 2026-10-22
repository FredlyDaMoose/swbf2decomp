#include "CommandPostManager.h"

#include "CommandPost.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "Team.h"

#include <float.h>

// GLOBAL: SWBF2 0x01e30898
CommandPostManager* CommandPostManager::sInstance;

// FUNCTION: SWBF2 0x0047f5b0
CommandPostManager::CommandPostManager()
{
}

// FUNCTION: SWBF2 0x0047f580
void CommandPostManager::Create()
{
    sInstance = new CommandPostManager();
}

void CommandPostManager::Destroy()
{
    CommandPost::Destroy();
}

// FUNCTION: SWBF2 0x0047f5c0
CommandPost* CommandPostManager::FindPostFromID(unsigned int uID) const
{
    for (CommandPostItor itor = GetItor(); itor; ++itor)
    {
        CommandPost* pPost = *itor;
        GameObject* pObject = pPost->mObject;
        if (pObject->GetId() == uID)
        {
            return pPost;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0047f630
CommandPost* CommandPostManager::FindPost(const char* pName) const
{
    return FindPostFromID(PblHash(pName));
}

// FUNCTION: SWBF2 0x0047f650
CommandPost* CommandPostManager::FindPostClosestToEnemy(int iTeam) const
{
    CommandPost* pClosest = 0;
    float fClosest = FLT_MAX;
    Team* pTeam = Team::sTeams[iTeam];

    for (CommandPostItor itor = GetItor(); itor; ++itor)
    {
        CommandPost* pPost = *itor;
        if (!pPost->CanTeamSpawnHere(iTeam))
        {
            continue;
        }

        const PblVector3& vPost = pPost->GetMatrix().GetTranslation();
        for (CommandPostItor enemy = GetItor(); enemy; ++enemy)
        {
            CommandPost* pEnemy = *enemy;
            GameObject* pObject = pEnemy->mObject;
            if (pTeam->GetAffiliation(pObject->mTeam) < 0)
            {
                const PblVector3& vEnemy = pEnemy->GetMatrix().GetTranslation();
                float fDX = vPost.x - vEnemy.x;
                float fDY = vPost.y - vEnemy.y;
                float fDZ = vPost.z - vEnemy.z;
                float fDist2 = fDX * fDX + fDY * fDY + fDZ * fDZ;
                if (fClosest > fDist2)
                {
                    fClosest = fDist2;
                    pClosest = pPost;
                }
            }
        }
    }
    return pClosest;
}

// FUNCTION: SWBF2 0x0047f840
CommandPost* CommandPostManager::FindPostFromIndex(int iIndex) const
{
    int i = 0;
    for (CommandPostItor itor = GetItor(); itor; ++itor)
    {
        CommandPost* pPost = *itor;
        if (i == iIndex)
        {
            return pPost;
        }
        ++i;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0047f890
int CommandPostManager::GetCount() const
{
    return CommandPost::GetCount();
}

// FUNCTION: SWBF2 0x0047f8a0
CommandPost* CommandPostItor::operator*()
{
    return CommandPost::GetActive(mIndex);
}

// FUNCTION: SWBF2 0x0047f8b0
CommandPostItor::operator bool()
{
    while (mIndex < 16)
    {
        if (CommandPost::GetActive(mIndex))
        {
            return true;
        }
        ++mIndex;
    }
    return false;
}

// FUNCTION: SWBF2 0x0047f8e0
const CommandPostItor& CommandPostItor::operator++()
{
    ++mIndex;
    return *this;
}

// FUNCTION: SWBF2 0x0047f8f0
CommandPostItor CommandPostManager::GetItor() const
{
    CommandPostItor itor;
    itor.mIndex = 0;
    return itor;
}
