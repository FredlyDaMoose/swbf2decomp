#include "TeamManager.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Team.h"
#include "AIUtil.h"
#include <math.h>
#include "NetGame.h"
#include "TeamManager.h"

// GLOBAL: SWBF2 0x01f7d0e0
// SYNTHETIC: SWBF2 0x004069a0
// `dynamic initializer for 'sTeamList''
// SYNTHETIC: SWBF2 0x00769570
// `dynamic atexit destructor for 'sTeamList''
static TeamManager::ObjCacheData sTeamList[8][600];

// GLOBAL: SWBF2 0x01faa430
static int sMemberCount[8];

// GLOBAL: SWBF2 0x007ec554
int sUnmatchedRemovalCount = 1;

// GLOBAL: SWBF2 0x01faa214
static bool sDirty[8];

namespace TeamManager { void Init(); }

// FUNCTION: SWBF2 0x00654fd0
void TeamManager::Init()
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < 8; ++i)
    {
        sDirty[i] = true;
        sMemberCount[i] = 0;
    }
}

// FUNCTION: SWBF2 0x00655200
void TeamManager::UpdatePositions()
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    for (int i = 0; i < 8; ++i)
    {
        sDirty[i] = true;
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

namespace TeamManager
{

// FUNCTION: SWBF2 0x00655240
static void UpdatePositionsInternal(int iTeam)
{
    for (int i = 0; i < sMemberCount[iTeam]; ++i)
    {
        ObjCacheData& rData = sTeamList[iTeam][i];
        if (GameObject* pObject = rData.mObj)
        {
            rData.mSphere = pObject->GetCollisionObject()->GetCollisionSphere();
        }
    }
    sDirty[iTeam] = false;
}

// FUNCTION: SWBF2 0x006552d0
int sGetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
                       const Team* pTeam, int iAffilMask, const GameObject* pIgnore)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }

    int iCount = 0;
    for (int iTeam = 0; iTeam < 8; ++iTeam)
    {
        const int iAffil = pTeam ? pTeam->GetAffiliation(iTeam) : 0;
        if (!(iAffilMask & (1 << (iAffil + 1))))
        {
            continue;
        }
        if (sDirty[iTeam])
        {
            UpdatePositionsInternal(iTeam);
        }
        for (int i = 0; i < sMemberCount[iTeam]; ++i)
        {
            ObjCacheData& rData = sTeamList[iTeam][i];
            GameObject* pObject = rData.mObj;
            if (!pObject || pObject == pIgnore)
            {
                continue;
            }
            const int iObjAffil = pTeam ? pTeam->GetAffiliation(pObject->mPerceivedTeam) : 0;
            if (iObjAffil > iAffil && !(iAffilMask & (1 << (iObjAffil + 1))))
            {
                continue;
            }
            const float fRadius = rData.mSphere.GetRadius() + fRange;
            const float fDX = vPos.x - rData.mSphere.x;
            const float fDY = vPos.y - rData.mSphere.y;
            const float fDZ = vPos.z - rData.mSphere.z;
            if (fRadius * fRadius > fDX * fDX + fDY * fDY + fDZ * fDZ)
            {
                ppObjects[iCount++] = pObject;
                if (iCount == iMax)
                {
                    return iCount;
                }
            }
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x00655480
int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
                      const GameObject* pObject, AI::AIAffil eAffil)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }
    return sGetObjectsInRange(vPos, fRange, ppObjects, iMax, Team::GetPtr(pObject->GetTeam()), eAffil,
                              pObject);
}

// FUNCTION: SWBF2 0x00655550
int GetApproxObjectsAlongLine(const PblVector3& vStart, const PblVector3& vEnd, float fRadius,
                              GameObject** const ppObjects, int iMax, const GameObject* pIgnore,
                              AI::AIAffil eAffil)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }

    const PblVector3 vCenter((vEnd.x + vStart.x) * 0.5f, (vEnd.y + vStart.y) * 0.5f,
                             (vEnd.z + vStart.z) * 0.5f);
    const PblVector3 vDelta(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    const PblVector3 vDir = AIUtil::GetSafeNormal(vDelta);
    const float fHalfLength = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z) * 0.5f;

    GameObject* apObjects[1200];
    const int iCount = GetObjectsInRange(vCenter, fHalfLength, apObjects, iMax, pIgnore, eAffil);
    const float fRadius2 = fRadius * fRadius;
    int iFound = 0;
    for (int i = 0; i < iCount; ++i)
    {
        GameObject* pObject = apObjects[i];
        const PblVector3 vPos = pObject->GetMatrix().GetTranslation();
        const float fX = vPos.x - vStart.x;
        const float fY = vPos.y - vStart.y;
        const float fZ = vPos.z - vStart.z;
        const float fCX = fY * vDir.z - fZ * vDir.y;
        const float fCY = fZ * vDir.x - fX * vDir.z;
        const float fCZ = fX * vDir.y - fY * vDir.x;
        if (fRadius2 > fCX * fCX + fCY * fCY + fCZ * fCZ)
        {
            ppObjects[iFound++] = pObject;
        }
    }
    return iFound;
}

// FUNCTION: SWBF2 0x00655050
void AddObject(GameObject* pObject)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (!pObject->AddToTeamManager())
    {
        return;
    }

    const int iTeam = pObject->mTeam;
    ObjCacheData& rData = sTeamList[iTeam][sMemberCount[iTeam]];
    rData.mObj = pObject;
    rData.mSphere = pObject->GetCollisionObject()->GetCollisionSphere();
    ++sMemberCount[iTeam];
}

// FUNCTION: SWBF2 0x00655100
void RemoveObject(GameObject* pObject)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }
    if (!pObject || !pObject->AddToTeamManager())
    {
        return;
    }

    const int iTeam = pObject->mTeam;
    for (int i = 0; i < sMemberCount[iTeam]; ++i)
    {
        if (sTeamList[iTeam][i].mObj == pObject)
        {
            const int iLast = --sMemberCount[iTeam];
            sTeamList[iTeam][i] = sTeamList[iTeam][iLast];
            return;
        }
    }
    ++sUnmatchedRemovalCount;
}

}

namespace TeamManager
{

// FUNCTION: SWBF2 0x00655030
void EnterClient() {}

// FUNCTION: SWBF2 0x00655040
void LeaveClient() {}
}

namespace TeamManager {

// FUNCTION: SWBF2 0x006554f0
int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
               const Team* pTeam, int iAffilMask)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }
    return TeamManager::sGetObjectsInRange(vPos, fRange, ppObjects, iMax, pTeam, iAffilMask, 0);
}

}
