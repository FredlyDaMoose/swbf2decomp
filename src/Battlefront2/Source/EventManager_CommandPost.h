#pragma once

#include "EventManager_Timer.h"
#include "CommandPost.h"
#include "Damageable.h"
#include "GameObject.h"

namespace EventManager
{

template <>
bool LuaPushItem<CommandPost>(CommandPost& post);

template <>
bool LuaPushItem<unsigned __int64>(unsigned __int64& mask);

template <>
bool LuaPushItem<DamageOwner>(DamageOwner& owner);

template <class PARAM>
class PostMatchesName
{
public:

    static const char* GetName();

    bool operator()(CommandPost& post, PARAM&) const
    {
        return post.mPostIndex == mIndex;
    }

    bool operator==(const PostMatchesName& other) const
    {
        return mIndex == other.mIndex;
    }

    int mIndex;
};

inline GameObject* PostObject(CommandPost& post)
{
    PblHandle<GameObject>& h = post.mObject;
    if (h.mObject && h.mObject->mHandleId != h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

template <class PARAM>
class PostMatchesTeam
{
public:

    PostMatchesTeam(int iTeam) : mTeam(iTeam) {}

    static const char* GetName();

    bool operator()(CommandPost& post, PARAM&) const
    {
        if (!PostObject(post))
        {
            return false;
        }
        return (*(const int*)((const char*)PostObject(post) + 0x234) << 28 >> 28) == mTeam;
    }

    bool operator==(const PostMatchesTeam& other) const
    {
        return mTeam == other.mTeam;
    }

    int mTeam;
};

namespace CommandPostEvent
{
extern Event<CommandPost, DamageOwner> mKill;
extern Event<CommandPost, char> mRespawn;
}

 
 

}
