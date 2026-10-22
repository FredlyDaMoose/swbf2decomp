#pragma once

#include "EventManager_Timer.h"
#include "Damageable.h"
#include "Entity.h"

namespace EventManager
{

template <>
void Event<GameObject, DamageOwner>::Trigger(GameObject& object, DamageOwner& owner);

template <>
bool LuaPushItem<GameObject>(GameObject& object);

template <>
bool LuaPushItem<DamageOwner>(DamageOwner& owner);

template <>
bool LuaPushItem<int>(int& item);
template <>
bool LuaPushItem<float>(float& item);

inline const EntityEx& ObjectEntity(GameObject& object)
{
    return *reinterpret_cast<const EntityEx*>(&object);
}

template <class PARAM>
class ObjectMatches
{
public:

    static const char* GetName();

    bool operator()(GameObject& object, PARAM&) const
    {
        return &ObjectEntity(object) == mObject;
    }

    bool operator==(const ObjectMatches& other) const
    {
        return mObject == other.mObject;
    }

    EntityEx* mObject;
};

template <class PARAM>
class ObjectMatchesName
{
public:
    static const char* GetName();

    bool operator()(GameObject& object, PARAM&) const
    {
        return ObjectEntity(object).mId == mName;
    }

    bool operator==(const ObjectMatchesName& other) const
    {
        return mName == other.mName;
    }

    unsigned int mName;
};

template <class PARAM>
class ObjectMatchesTeam
{
public:

    ObjectMatchesTeam(int iTeam) : mTeam(iTeam) {}

    static const char* GetName();

    bool operator()(GameObject& object, PARAM&) const
    {
        return (*(const int*)((const char*)&object + 0x234) << 28 >> 28) == mTeam;
    }

    bool operator==(const ObjectMatchesTeam& other) const
    {
        return mTeam == other.mTeam;
    }

    int mTeam;
};

template <class PARAM>
class ObjectMatchesClass
{
public:
    static const char* GetName();

    bool operator()(GameObject& object, PARAM&) const
    {
        return ObjectEntity(object).GetEntityClass() == mClass;
    }

    bool operator==(const ObjectMatchesClass& other) const
    {
        return mClass == other.mClass;
    }

    EntityClass* mClass;
};

 
 

namespace ObjectEvent
{
extern Event<GameObject, float> mHealthChange;
extern Event<GameObject, float> mShieldChange;
extern Event<GameObject, DamageOwner> mDamage;

extern Event<GameObject, char> mInit;
extern Event<GameObject, char> mRespawn;
extern Event<GameObject, DamageOwner> mKill;

extern Event<GameObject, DamageOwner> mHeadshot;

extern Event<GameObject, char> mCreate;

extern Event<GameObject, char> mDelete;
}

}
