#include "PatrolPath.h"

#include "GameLoop.h"
#include "GameObject.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblCatmullRom.h"
#include "PathManager.h"
#include <d3dx9math.h>

// FUNCTION: SWBF2 0x00615750
AI::PatrolPath::PatrolPath()
    : mNodeProps(0), mReservingTime(0.0f)
{
    *(unsigned int*)&mReservingObject[0] = 0;
    *(unsigned int*)&mReservingObject[4] = 0;
    sList.Add((PblList<PatrolPath>::Node*)&mNode, this);
}

// FUNCTION: SWBF2 0x006156b0
bool AI::PatrolPath::NodeProperties::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                                       RedPath::PathObject object,
                                                       RedPath::PathObjectType type, void* pUserData)
{
    PatrolPath* pPath = (PatrolPath*)pUserData;
    unsigned int uIndex =
        (unsigned int)((char*)object.node - (char*)&pPath->mNodeArray.GetNode(0)) / sizeof(RedPath::Node);
    NodeProperties* pProps = pPath->mNodeProps;
    switch (data.GetId())
    {
    case 0x88BE386Eu:
    {
        float fMax = data.GetFloatArg(0);
        pProps[uIndex].mWaitMax = fMax;
        pProps[uIndex].mWaitMin = fMax > pProps[uIndex].mWaitMin ? pProps[uIndex].mWaitMin : fMax;
        return true;
    }
    case 0x9AD1DC7Cu:
    {
        float fMin = data.GetFloatArg(0);
        pProps[uIndex].mWaitMin = fMin;
        pProps[uIndex].mWaitMax = fMin > pProps[uIndex].mWaitMax ? fMin : pProps[uIndex].mWaitMax;
        return true;
    }
    }
    return RedPath::Node::ReadPropertyScope(config, data, object, type, pUserData);
}

// FUNCTION: SWBF2 0x00615d70
bool AI::PatrolPath::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                       RedPath::PathObject object,
                                       RedPath::PathObjectType type, void* pUserData)
{
    return RedPath::ReadPropertyScope(config, data, object, type, pUserData);
}

// FUNCTION: SWBF2 0x00615a40
void AI::PatrolPath::SetReservingObject(const GameObject* pObj)
{
    *(const GameObject**)&mReservingObject[0] = pObj;
    *(unsigned int*)&mReservingObject[4] = pObj ? pObj->mHandleId : 0;
    mReservingTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x00615a80
const GameObject* AI::PatrolPath::GetReservingObject()
{
    const GameObject* pObject = *(const GameObject* const*)&mReservingObject[0];
    if (pObject &&
        (pObject->mHandleId != *(unsigned int*)&mReservingObject[4]
         || GameLoop::GetMissionTime() - 8.0f > mReservingTime))
    {
        *(unsigned int*)&mReservingObject[4] = 0;
        *(const GameObject**)&mReservingObject[0] = 0;
    }

    pObject = *(const GameObject* const*)&mReservingObject[0];
    if (pObject && pObject->mHandleId != *(unsigned int*)&mReservingObject[4])
    {
        *(const GameObject**)&mReservingObject[0] = 0;
        *(unsigned int*)&mReservingObject[4] = 0;
        return 0;
    }
    return pObject;
}

// FUNCTION: SWBF2 0x00615ae0
AIPath* AI::PatrolPath::GetAIPath()
{
    AIPath* pPath = new AIPath();
    if (!pPath)
    {
        return 0;
    }

    const int iNumPoints = GetNumPoints();
    if (GetSplineType() == RedPath::CatmullRom)
    {
        for (int i = 0; i < iNumPoints - 1; ++i)
        {
            const PblVector3& p1 = *GetPointPosition(i);
            const PblVector3& p2 = *GetPointPosition(i + 1);
            PblVector3 p0, p3;
            if (i > 0)
            {
                p0 = *GetPointPosition(i - 1);
            }
            else
            {
                PblVector3 delta(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
                PblVector3 direction;
                D3DXVec3Normalize((D3DXVECTOR3*)&direction,
                                  (const D3DXVECTOR3*)&delta);
                p0 = PblVector3(p1.x - direction.x, p1.y - direction.y, p1.z - direction.z);
            }
            if (i < iNumPoints - 2)
            {
                p3 = *GetPointPosition(i + 2);
            }
            else
            {
                PblVector3 delta(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z);
                PblVector3 direction;
                D3DXVec3Normalize((D3DXVECTOR3*)&direction,
                                  (const D3DXVECTOR3*)&delta);
                p3 = PblVector3(p2.x - direction.x, p2.y - direction.y, p2.z - direction.z);
            }
            for (float t = 0.0f; t < 1.0f; t += 0.125f)
            {
                PblVector3 pos;
                PblCatmullRom::CalculatePoint(t, p0, p1, p2, p3, pos);
                pPath->Append(pos, 0);
            }
        }
        pPath->Append(*GetPointPosition(iNumPoints - 1), 0);
    }
    else
    {
        for (int i = 0; i < iNumPoints; ++i)
        {
            pPath->Append(*GetPointPosition(i), 0);
        }
    }
    return pPath;
}

// GLOBAL: SWBF2 0x007ebc0c
PblList<AI::PatrolPath> AI::PatrolPath::sList;

// FUNCTION: SWBF2 0x00615a00
const PblVector3* AI::PatrolPath::GetPointPosition(int iIndex) const
{
    if (iIndex < 0 || iIndex >= GetNumPoints())
    {
        return 0;
    }
    return &RedPath::GetPointPosition(iIndex);
}

// FUNCTION: SWBF2 0x00615a30
const AI::PatrolPath::NodeProperties& AI::PatrolPath::GetNodeProperties(unsigned int uIndex) const
{
    return mNodeProps[uIndex];
}

// FUNCTION: SWBF2 0x006157e0
AI::PatrolPath::~PatrolPath()
{
    if (mNodeProps)
    {
        delete[] mNodeProps;
        mNodeProps = 0;
    }
    sList.Remove((PblList<PatrolPath>::Node*)&mNode);
}

// FUNCTION: SWBF2 0x00615870
bool AI::PatrolPath::Read(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0x84874D36u)
        {
            if (data.GetNumArgs())
            {
                mName = PblHash(data.GetStringArg(0));
            }
            else
            {
                mName = 0;
            }

            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data child = scope.ReadNextData();
                if (child.GetId() == 0x514E905Au)
                {
                    mNodeProps = new NodeProperties[child.GetUint32Arg(0)];
                }
                ReadScope(scope, child, ReadPropertyScope, 0,
                          NodeProperties::ReadPropertyScope, this);
            }
            break;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00615dc0
int AI::PatrolPath::RequestPatrolPaths(const PblVector3& pos, float fRadius, int iMax, PatrolPath** ppPaths)
{
    int iCount = 0;
    for (PblList<PatrolPath>::Itor it = sList.Begin(); it != sList.End(); ++it)
    {
        const PblVector3* pPoint = it->GetPointPosition(0);
        if (!pPoint)
        {
            continue;
        }
        float fX = pos.x - pPoint->x;
        float fY = pos.y - pPoint->y;
        float fZ = pos.z - pPoint->z;
        if (fRadius * fRadius > fX * fX + fY * fY + fZ * fZ)
        {
            ppPaths[iCount++] = it;
            if (iCount >= iMax)
            {
                break;
            }
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x00615d90
void AI::PatrolPath::DestroyAll()
{
    PblList<PatrolPath>::Itor it(sList);
    while ((PatrolPath*)it)
    {
        PatrolPath* pPath = it;
        ++it;
        delete pPath;
    }
}
