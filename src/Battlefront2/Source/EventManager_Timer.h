#pragma once

#include "Battlefront2/Source/EventManager.h"
#include "PebbleFL/Common/PblHandle.h"
#include "Timer.h"

namespace EventManager
{

template <class ITEM, class PARAM>
class Event
{
public:

    class Callback : public Node
    {
    public:

        virtual ~Callback() {}
        virtual bool operator()(ITEM& item, PARAM& param);

        void Trigger(ITEM& item, PARAM& param);
    };

    template <class PREDICATE>
    class Filter : public Callback
    {
    public:

        Filter(PREDICATE& predicate);

        virtual const char* GetName();
        virtual bool operator()(ITEM& item, PARAM& param);

        PREDICATE mPredicate;
    };

    void Unregister(Callback& callback);
    template <class PREDICATE>
    Filter<PREDICATE>* GetFilter(PREDICATE& predicate, Callback* pParent = 0);

    Event(char* pcName) : mName(pcName) {}

    void Init() {}
    void Cleanup() { mRoot.Clear(); }

    void Trigger(ITEM& item, PARAM& param) { mRoot.Trigger(item, param); }

    char* mName;
    Callback mRoot;
};

template <class ITEM, class PARAM>
class LuaCallback : public Event<ITEM, PARAM>::Callback
{
public:

    LuaCallback(int iRef);
    virtual ~LuaCallback();
    virtual bool operator()(ITEM& item, PARAM& param);

    int mFunction;
};

template <class ITEM>
bool LuaPushItem(ITEM& item);

template <>
bool LuaPushItem<char>(char& item);

class TimerMatches
{
public:

    TimerMatches(const Timer* pTimer) : mTimer(pTimer) {}
    TimerMatches(const char* pcName);

    static const char* GetName();

    bool operator==(const TimerMatches& other) const
    {
        return mTimer == other.mTimer;
    }

    bool operator()(Timer& timer, char&) const
    {
        return mTimer == &timer;
    }

    PblHandle<Timer const> mTimer;
};

namespace TimerEvent
{
extern Event<Timer, char> mElapsed;
void Cleanup();
}

}
