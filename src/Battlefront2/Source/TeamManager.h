#pragma once
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class GameObject;
class PblVector3;
class Team;

namespace TeamManager
{
void EnterClient();
void LeaveClient();

void AddObject(GameObject* pObject);
void RemoveObject(GameObject* pObject);
int sGetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
                       const Team* pTeam, int iAffilMask, const GameObject* pIgnore);
}
namespace TeamManager { void UpdatePositions(); }

class GameObject;

namespace TeamManager
{

struct ObjCacheData
{

    // FUNCTION: SWBF2 0x00654fb0
    ObjCacheData() {}

    // FUNCTION: SWBF2 0x00654fc0
    ~ObjCacheData() {}

    PblHandle<GameObject> mObj;
    PblSphere mSphere;
};

}
