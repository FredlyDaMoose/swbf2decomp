#pragma once

#include "EventManager_Flag.h"
#include "EventManager_LuaUser.h"
#include "RedEngineFL/Util/RedRegion.h"

namespace EventManager
{

int DummyLuaCallback(lua_State* L);

template <>
bool LuaPushItem<RedRegion>(RedRegion& region);

class RegionMatches
{
public:

    RegionMatches(RedRegion* pRegion) : mRegion(pRegion) {}

    RegionMatches(const char* pcName);

    static const char* GetName();

    bool operator()(RedRegion& region, Character&) const
    {
        return &region == mRegion;
    }

    bool operator==(const RegionMatches& other) const
    {
        return mRegion == other.mRegion;
    }

    RedRegion* mRegion;
};

template <class ITEM, class PARAM, class P1, class P2>
int RegisterLuaCallback(lua_State* L);
template <class ITEM, class PARAM, class P1, class P2>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient);

namespace RegionEvent
{
extern Event<RedRegion, Character> mEnterRegion;
extern Event<RedRegion, Character> mLeaveRegion;

void Init();
void Cleanup();
}

 
 
 

}
