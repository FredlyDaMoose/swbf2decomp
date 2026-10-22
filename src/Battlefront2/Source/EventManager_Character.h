#pragma once

#include "EventManager_LuaUser.h"
#include "EventInfo.h"
#include "Damageable.h"

class Character;
class GameObject;
class EntityClass;

namespace EventManager
{

int DummyLuaCallback(lua_State* L);

template <>
bool LuaPushItem<Character>(Character& character);

template <>
bool LuaPushItem<DamageOwner>(DamageOwner& owner);
template <>
bool LuaPushItem<GameObject>(GameObject& object);

template <>
bool LuaPushItem<AI::EventType>(AI::EventType& eType);

template <class PARAM>
class CharacterMatchesName
{
public:

    CharacterMatchesName(const char* pcName);

    static const char* GetName();

    bool operator()(const Character& character, const PARAM& param);

    bool operator==(const CharacterMatchesName& other) const
    {
        return mId == other.mId;
    }

    unsigned int mId;
};

template <class PARAM>
class CharacterMatchesTeam
{
public:

    CharacterMatchesTeam(int iTeam) : mTeam(iTeam) {}

    static const char* GetName();

    bool operator()(const Character& character, const PARAM& param) const;

    bool operator==(const CharacterMatchesTeam& other) const
    {
        return mTeam == other.mTeam;
    }

    int mTeam;
};

template <class PARAM>
class CharacterMatchesClass
{
public:

    CharacterMatchesClass(const char* pcName);

    static const char* GetName();

    bool operator()(const Character& character, const PARAM& param);

    bool operator==(const CharacterMatchesClass& other) const
    {
        return mClass == other.mClass;
    }

    EntityClass* mClass;
};

 
 

namespace CharacterEvent
{
extern Event<Character, DamageOwner> mDeath;
extern Event<Character, char> mSpawn;
extern Event<Character, char> mChangeClass;
extern Event<Character, GameObject> mDispensePowerup;
extern Event<Character, GameObject> mDispenseControllable;
extern Event<Character, GameObject> mLandedFlyer;
extern Event<Character, GameObject> mEnterVehicle;
extern Event<Character, AI::EventType> mIssueAICommand;

extern Event<Character, int> mKillsChange;
extern Event<Character, int> mDeathsChange;
extern Event<Character, int> mScoreChange;

void Init();
void Cleanup();
}

template <>
void Event<Character, char>::Trigger(Character& item, char& param);

template <>
void Event<Character, GameObject>::Trigger(Character& item, GameObject& param);

}
