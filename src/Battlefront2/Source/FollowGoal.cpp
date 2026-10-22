#include "FollowGoal.h"
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameObject.h"
#include "UnitCmdInfo.h"
#include "UnitController.h"

namespace
{
inline unsigned int GameObjectHandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

inline const GameObject* ReadHandle(PblHandle<const GameObject>& handle)
{
    if (handle.mObject && GameObjectHandleId(handle.mObject) != handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

inline PblHandle<GameObject> MakeHandle(const GameObject* pObject)
{
    PblHandle<GameObject> handle;
    handle.mObject = const_cast<GameObject*>(pObject);
    handle.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
    return handle;
}

inline bool IsAlive(const GameObject* pObject)
{
    return static_cast<const Damageable*>(pObject)->mIsAlive;
}
}

// GLOBAL: SWBF2 0x01ec0bd8
// SYNTHETIC: SWBF2 0x00403b50
// `dynamic initializer for 'AI::FollowGoal::rttiHashFollowGoal''
PblHash AI::FollowGoal::rttiHashFollowGoal("FollowGoal");
char AI::FollowGoal::rttiNameFollowGoal[] = "FollowGoal";

// FUNCTION: SWBF2 0x0052d1c0
bool AI::FollowGoal::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashFollowGoal || AIGoal::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0052d1f0
unsigned int AI::FollowGoal::GetDerivedRtti() const
{
    return rttiHashFollowGoal;
}

// FUNCTION: SWBF2 0x0052d200
const char* AI::FollowGoal::GetDerivedRttiName() const
{
    return rttiNameFollowGoal;
}

// SYNTHETIC: SWBF2 0x0052d210
// AI::FollowGoal::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052d240
AI::FollowGoal::FollowGoal(unsigned char uTeam, float fWeight, const GoalTarget& target)
    : AIGoal(AIGOAL_FOLLOW, uTeam, fWeight)
{
    mTarget = target;
}

// FUNCTION: SWBF2 0x0052d390
void AI::FollowGoal::Update()
{
    if (mEnabled)
    {
        if (mTarget.mType == GTT_GAMEOBJECT && ReadHandle(mTarget.mGameObj) &&
            !IsAlive(ReadHandle(mTarget.mGameObj)))
        {
            SetEnabled(false);
            return;
        }
        AIGoal::Update();
    }
    else if (mTarget.mType == GTT_GAMEOBJECT && ReadHandle(mTarget.mGameObj) &&
             IsAlive(ReadHandle(mTarget.mGameObj)))
    {
        SetEnabled(true);
    }
}

// FUNCTION: SWBF2 0x0052d2a0
void AI::FollowGoal::Assign(Character* pChr)
{
    if (!pChr || !pChr->GetControllable())
    {
        return;
    }

    UnitController* pCtrl = UnitControllerPtr(pChr->GetControllable()->mCtrl);
    if (!pCtrl)
    {
        return;
    }

    const GameObject* pObject = mTarget.GetGameObject();
    if (pObject)
    {
        pCtrl->ForceCommand(UnitFollowInfo(MakeHandle(pObject), PblHandle<GameObject>(), true));
    }
}
