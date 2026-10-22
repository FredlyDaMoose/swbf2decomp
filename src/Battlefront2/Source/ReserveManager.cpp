#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "ListPool.h"
#include "PlayerController.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"

enum FormationType
{
    FT_RANDOM = 0,
    FT_SINGLEFILE = 1,
    FT_DOUBLEFILE = 2,
    FT_DOUBLEFILE_RAND = 3,
    FT_CIRCLE = 4,
    FT_CONVOY = 5,
    FT_FLYINGV = 6
};

namespace ReserveManager
{

struct ReserveStruct
{
    ~ReserveStruct();

    PblHandle<const GameObject> mObj;
    float mReserveTime;
    PblHandle<const GameObject> mReserver;
    ReserveType mType;
};

// SYNTHETIC: SWBF2 0x0062f990
// ReserveManager::ReserveStruct::ReserveStruct

extern ListPool<ReserveStruct, int>* sList;

// FUNCTION: SWBF2 0x0062f550
void Init()
{
    sList = new ListPool<ReserveStruct, int>(60);
}

// FUNCTION: SWBF2 0x0062f5d0
bool sValidateTimeStamp(ReserveStruct& rEntry, float fTimeout)
{
    const float fNow = GameLoop::GetMissionTime();
    if (fTimeout < 0.0f)
    {
        fTimeout = 10.0f;
    }
    bool bValid = rEntry.mReserveTime + fTimeout > fNow;
    if (!rEntry.mObj || !rEntry.mReserver)
    {
        bValid = false;
    }
    if (bValid)
    {
        return true;
    }

    for (int i = 0; i < sList->GetLength(); ++i)
    {
        if (&(*sList)[i] == &rEntry)
        {
            sList->Remove(i);
            break;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x00630030
const GameObject* __fastcall GetFirstReserver(const GameObject* pObj)
{
    for (int i = 0; i < sList->GetLength();)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (!sValidateTimeStamp(rEntry, -1.0f))
        {
            continue;
        }
        if (rEntry.mObj == pObj && rEntry.mType == RT_BOARD)
        {
            return rEntry.mReserver;
        }
        ++i;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0062f690
static ReserveStruct* GetReserveStruct(const GameObject* pObj, const GameObject* pReserver, ReserveType eType)
{
    for (int i = 0; i < sList->GetLength();)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (!sValidateTimeStamp(rEntry, -1.0f))
        {
            continue;
        }
        if (rEntry.mObj == pObj && rEntry.mReserver == pReserver && rEntry.mType == eType)
        {
            return &rEntry;
        }
        ++i;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0062f740
bool IsReservedBy(const GameObject* pObj, const GameObject* pReserver, ReserveType eType)
{
    return GetReserveStruct(pObj, pReserver, eType) != 0;
}

// FUNCTION: SWBF2 0x0062f760
int GetReservationCountForObject(const GameObject* pObj, int iTeam, ReserveType eType)
{
    int iCount = 0;
    for (int i = 0; i < sList->GetLength();)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (!sValidateTimeStamp(rEntry, -1.0f))
        {
            continue;
        }
        if (rEntry.mObj == pObj && rEntry.mReserver->GetTeam() == iTeam && rEntry.mType == eType)
        {
            ++iCount;
        }
        ++i;
    }
    return iCount;
}

// FUNCTION: SWBF2 0x0062f830
int GetPlayerOccupiedSlots(const GameObject* pObj)
{
    int iCount = 0;
    for (PblList<Controllable>::Itor it(pObj->GetControllableList()); it; ++it)
    {
        if (it->mCtrl && it->mCtrl->IsRtti(PlayerController::rttiHashPlayerController))
        {
            ++iCount;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x0062f870
int GetOpenReservableSlots(const GameObject* pObj, int iTeam, ReserveType eType)
{
    int iSlots = pObj->GetReservableSlots(eType);
    const int iReserved = GetReservationCountForObject(pObj, iTeam, eType);
    int iPlayers = 0;
    if (eType == RT_BOARD)
    {
        iPlayers = GetPlayerOccupiedSlots(pObj);
        if (iPlayers + iReserved <= iSlots)
        {
            ++iSlots;
        }
    }
    if (iPlayers + iReserved > iSlots)
    {
        return 0;
    }
    return iSlots - iPlayers - iReserved;
}

// FUNCTION: SWBF2 0x0062f8d0
void Reserve(const GameObject* pObj, const GameObject* pReserver, ReserveType eType)
{
    if (IsReservedBy(pObj, pReserver, eType))
    {
        GetReserveStruct(pObj, pReserver, eType)->mReserveTime = GameLoop::GetMissionTime();
        return;
    }

    ReserveStruct newEntry;
    newEntry.mObj = pObj;
    newEntry.mReserveTime = GameLoop::GetMissionTime();
    newEntry.mReserver = pReserver;
    newEntry.mType = eType;
    sList->Append(newEntry);
}

// FUNCTION: SWBF2 0x0062f9c0
int GetReserveOrder(const GameObject* pObj, const GameObject* pReserver, ReserveType eType)
{
    int iOrder = 0;
    for (int i = 0; i < sList->GetLength();)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (!sValidateTimeStamp(rEntry, -1.0f))
        {
            continue;
        }
        if (rEntry.mObj == pObj && rEntry.mType == eType)
        {
            if (rEntry.mReserver == pReserver)
            {
                return iOrder;
            }
            ++iOrder;
        }
        ++i;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0062fa70
PblVector3 ReserveOffset(const GameObject* pTarget, const GameObject* pSelf, FormationType eFormation)
{
    Reserve(pTarget, pSelf, RT_FORMATION);

    int iOrder = 0;
    int iCount = 0;
    for (int i = 0; i < sList->GetLength();)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (!sValidateTimeStamp(rEntry, -1.0f))
        {
            continue;
        }
        if (rEntry.mObj == pTarget && rEntry.mType == RT_FORMATION)
        {
            if (rEntry.mReserver == pSelf)
            {
                iOrder = iCount;
            }
            ++iCount;
        }
        ++i;
    }

    switch (eFormation)
    {
    case FT_RANDOM:
    {
        PblRandom random = { ((unsigned int)pTarget - (int)(GameLoop::GetMissionTime() * -0.125f)) ^ 0xC71C2830u };
        for (int i = iOrder; i > 0; --i)
        {
            random.GetVector3();
        }
        const PblVector3 vRand = random.GetVector3();
        return PblVector3(vRand.x * 1.5f, 0.0f, vRand.z * 1.5f);
    }

    case FT_SINGLEFILE:
    {
        const float fRow = (float)iOrder;
        return PblVector3(fRow * 0.0f, fRow * 0.0f, fRow * -1.0f);
    }

    case FT_DOUBLEFILE:
    {
        const float fRow = (float)(iOrder / 2) + 0.25f;
        const float fSide = (float)((iOrder % 2 == 0) ? 1 : -1);
        return PblVector3(fSide * 0.75f + fRow * 0.0f, fSide * 0.0f + fRow * 0.0f, fSide * 0.0f + fRow * -1.0f);
    }

    case FT_CONVOY:
    {
        const float fRow = (float)(iOrder / 2) - 0.25f;
        const float fSide = (float)((iOrder % 2 == 0) ? 1 : -1);
        return PblVector3(fSide * 0.75f + fRow * 0.0f, fSide * 0.0f + fRow * 0.0f, fSide * 0.0f + fRow);
    }

    case FT_DOUBLEFILE_RAND:
    {
        const float fBits = (float)(((unsigned int)pSelf >> 5) & 7);
        PblRandom random = { ((unsigned int)pTarget - (int)((GameLoop::GetMissionTime() + fBits) * -0.125f)) ^ 0xC71C2830u };
        for (int i = iOrder; i > 0; --i)
        {
            random.GetVector3();
        }
        const PblVector3 vRand = random.GetVector3();
        const float fRow = (float)(iOrder / 2) + 0.25f;
        const float fSide = (float)((iOrder % 2 == 0) ? 1 : -1);
        return PblVector3(fRow * 0.0f + (fSide * 0.75f + vRand.x * 0.2f),
                          fRow * 0.0f + (fSide * 0.0f + vRand.y * 0.2f),
                          fRow * -1.0f + (fSide * 0.0f + vRand.z * 0.2f));
    }

    case FT_CIRCLE:
    {
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, (float)iOrder / (float)iCount * PblMath::fTwoPi);
        return PblVector3(fSin, 0.0f, -fCos);
    }

    case FT_FLYINGV:
    {
        const float fRow = (float)(iOrder / 2) + 0.25f;
        const float fSide = (float)((iOrder % 2 == 0) ? 1 : -1);
        return PblVector3(fSide * 0.75f + fSide * fRow, fSide * 0.0f + fRow * 0.0f, fSide * 0.0f + fRow * -1.0f);
    }

    default:
        return PblVector3(0.0f, 0.0f, 0.0f);
    }
}

// FUNCTION: SWBF2 0x0062fed0
void UnreserveReserver(const GameObject* pReserver, ReserveType eType)
{
    for (int i = 0; i < sList->GetLength(); ++i)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (rEntry.mReserver == pReserver && rEntry.mType == eType)
        {
            sList->Remove(i);
        }
    }
}

// FUNCTION: SWBF2 0x0062ff80
void UnreserveObject(const GameObject* pObj, ReserveType eType)
{
    for (int i = 0; i < sList->GetLength(); ++i)
    {
        ReserveStruct& rEntry = (*sList)[i];
        if (rEntry.mObj == pObj && rEntry.mType == eType)
        {
            sList->Remove(i);
            --i;
        }
    }
}

}

 
 

// FUNCTION: SWBF2 0x006300e0
template ListPool<ReserveManager::ReserveStruct, int>::ListPool(int);

// FUNCTION: SWBF2 0x00630190
template void ListPool<ReserveManager::ReserveStruct, int>::Remove(int);

namespace ReserveManager {

// FUNCTION: SWBF2 0x0062f5c0
void Reset()
{
    sList = 0;
}
}

// FUNCTION: SWBF2 0x0062f9b0
ReserveManager::ReserveStruct::~ReserveStruct()
{
}
