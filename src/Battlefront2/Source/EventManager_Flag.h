#pragma once

#include "EventManager_Timer.h"
#include "Character.h"

class FlagItem;
class RedRegion;
class EntityClass;

namespace EventManager
{

template <>
bool LuaPushItem<FlagItem>(FlagItem& flag);

template <>
bool LuaPushItem<Character>(Character& character);

class CharacterMatchesName
{
public:

    CharacterMatchesName(const char* pcName);

    static const char* GetName();

    bool operator()(const FlagItem& flag, const Character& character);

    bool operator()(const RedRegion& region, const Character& character);

    bool operator==(const CharacterMatchesName& other) const
    {
        return mId == other.mId;
    }

    unsigned int mId;
};

class CharacterMatchesTeam
{
public:

    CharacterMatchesTeam(int iTeam) : mTeam(iTeam) {}

    static const char* GetName();

    bool operator()(const FlagItem&, const Character& character) const
    {
        return character.mTeamNumber == mTeam;
    }

    bool operator()(const RedRegion&, const Character& character) const
    {
        return character.mTeamNumber == mTeam;
    }

    bool operator==(const CharacterMatchesTeam& other) const
    {
        return mTeam == other.mTeam;
    }

    int mTeam;
};

class CharacterMatchesClass
{
public:

    CharacterMatchesClass(const char* pcName);

    static const char* GetName();

    bool operator()(const FlagItem& flag, const Character& character);

    bool operator()(const RedRegion& region, const Character& character);

    bool operator==(const CharacterMatchesClass& other) const
    {
        return mClass == other.mClass;
    }

    EntityClass* mClass;
};

 
 
 

}
