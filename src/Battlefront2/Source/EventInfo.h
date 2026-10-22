#pragma once

#include <stddef.h>

#include "GameObject.h"
#include "UnitCmdInfo.h"

class AIPath;
class Character;

namespace AI
{

enum EventType
{
    EVT_None = 0,
    EVT_EmptyVehicle = 1,
    EVT_Grenade = 2,
    EVT_Damaged = 3,
    EVT_Vehicle_WaitForPickUp = 4,
    EVT_Vision_CanSeeEnemy = 5,
    EVT_Audible_CanHearEnemy = 6,
    EVT_Path_PathFollowerStuck = 7,
    EVT_Path_NextNode = 8,
    EVT_Path_PathResult = 9,
    EVT_Path_DynamicCongraphChange = 10,
    EVT_Nav_NavigatorFinished = 11,
    EVT_CTF_FlagPickup = 12,
    EVT_CTF_FlagDrop = 13,
    EVT_CTF_FlagKill = 14,
    EVQ_SC_IsFollowingChr = 15,
    EVT_SC_FollowChr = 16,
    EVT_SC_FollowChrLoose = 17,
    EVT_SC_StopFollowChr = 18,
    EVT_SC_GetIntoVehicle = 19,
    EVT_SC_GetOutOfVehicle = 20,
    EVQ_GetSkillModifier = 21,
    EVT_EndInvisibility = 22
};

enum EVTResult
{
    EVT_Ignored = -1,
    EVT_NotHandled = 0,
    EVT_Handled = 1
};

class EventInfo
{
public:

    // FUNCTION: SWBF2 0x00438b00
    EventInfo(EventType eType)
        : mNextCmd(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                   PblHandle<GameObject>()),
          mType(eType),
          mGameObject(),
          mPos(0, 0, 0),
          mFloat(0.0f),
          mInt(0),
          mVoidPtr(0)
    {
    }

    // FUNCTION: SWBF2 0x005ad280
    EventInfo(EventType eType, void* pVoidPtr)
        : mNextCmd(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                   PblHandle<GameObject>()),
          mType(eType),
          mGameObject(),
          mPos(0, 0, 0),
          mFloat(0.0f),
          mInt(0),
          mVoidPtr(pVoidPtr)
    {
    }

    // FUNCTION: SWBF2 0x0043ee90
    EventInfo(EventType eType, const PblVector3& vPos)
        : mNextCmd(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                   PblHandle<GameObject>()),
          mType(eType),
          mGameObject(),
          mPos(vPos),
          mFloat(0.0f),
          mInt(0),
          mVoidPtr(0)
    {
    }

    EventInfo(EventType eType, float fFloat, int iInt);

    // FUNCTION: SWBF2 0x0043edf0
    EventInfo(EventType eType, GameObject* pObject)
        : mNextCmd(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                   PblHandle<GameObject>()),
          mType(eType),
          mGameObject(pObject),
          mPos(0, 0, 0),
          mFloat(0.0f),
          mInt(0),
          mVoidPtr(0)
    {
    }

    // FUNCTION: SWBF2 0x00670160
    EventInfo(EventType eType, GameObject* pObject, const PblVector3& vPos)
        : mNextCmd(CMD_NONE, PblHandle<GameObject>(), PblVector3(0, 0, 0),
                   PblHandle<GameObject>()),
          mType(eType),
          mGameObject(pObject),
          mPos(vPos),
          mFloat(0.0f),
          mInt(0),
          mVoidPtr(0)
    {
    }

    UnitCmdInfo mNextCmd;
    EventType mType;
    PblHandle<GameObject> mGameObject;
    PblVector3 mPos;
    float mFloat;
    int mInt;
    void* mVoidPtr;
};

class EventInfo_Damaged : public EventInfo
{
public:
    EventInfo_Damaged(Character* pDamageOwner, bool bIsShot, const PblVector3& vPos,
                      float fAmountLost, bool bIsPropagated)
        : EventInfo(EVT_Damaged, vPos),
          mDamageOwner(pDamageOwner),
          mIsShot(bIsShot),
          mAmountLost(fAmountLost),
          mIsPropagated(bIsPropagated),
          mIsCriticalHit(false)
    {
    }

    Character* mDamageOwner;
    bool mIsShot;
    float mAmountLost;
    bool mIsPropagated;
    bool mIsCriticalHit;
};

class EventInfo_PathResult : public EventInfo
{
public:
    EventInfo_PathResult(AIPath* pPath) : EventInfo(EVT_Path_PathResult, pPath) {}
    ~EventInfo_PathResult() {}
};

}

 
 
 
 
 
