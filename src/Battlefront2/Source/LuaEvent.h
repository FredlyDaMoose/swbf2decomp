#pragma once

#include "Thread.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

struct lua_State;

class LuaEvent : public Thread
{
public:

    enum EVENT_TYPE
    {
        EVENT_TYPE_RELATIVE_TIMER = 0,
        EVENT_TYPE_UNDEFINED = 1
    };

    class List : public PblList<LuaEvent>
    {
    public:
        void Add(LuaEvent* pEvent) { PblList<LuaEvent>::Add(&pEvent->_Node, pEvent); }
        void Remove(LuaEvent* pEvent) { PblList<LuaEvent>::Remove(&pEvent->_Node); }
    };

    LuaEvent();
    virtual ~LuaEvent();
    virtual bool Update(float fDeltaTime);

    void Deinit();
    void Reset();

private:
    void SendEvent();

public:
    PblList<LuaEvent>::Node _Node;
    EVENT_TYPE _eEventType;
    char* _pcCallbackFunctionName;
    char* _pcFlagName;
    float _fRelativeTimer;
    float _fProximitySq;
    PblVector3 _vProximityPos;
    int _iHitPoints;
    int _iEventId;

    static lua_State* _pLuaState;
    static int _iEventCount;
};
 
 
 
 

class LuaEventManager
{
    friend class LuaEvent;

private:
    static void DestroyEvent(LuaEvent* pEvent);

    LuaEventManager();

public:
    ~LuaEventManager() {}
    static LuaEventManager _Instance;

public:
    static LuaEvent::List _ActiveEventList;
    static LuaEvent::List _InactiveEventList;
    static lua_State* _pLuaState;
    static LuaEvent _EventInstances[100];

    static void SetLuaState(lua_State* pState) { _pLuaState = pState; }

    static void Reset() { _ActiveEventList.RemoveAll(); _InactiveEventList.RemoveAll(); }
};
