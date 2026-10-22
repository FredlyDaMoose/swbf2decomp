#pragma once

#include <stddef.h>
#include "RedEngineFL/Util/RedRegion.h"
#include "PebbleFL/Common/PblTimeBase.h"

// VTABLE: SWBF2 0x0079fbf0
class SoundStaticRegionFactory : public RedRegionFactory
{
public:
    SoundStaticRegionFactory() : RedRegionFactory("soundstatic") {}

    // SYNTHETIC: SWBF2 0x005393a0
    // SoundStaticRegionFactory::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
};
  

// VTABLE: SWBF2 0x0079fc28
class SoundStreamRegionFactory : public RedRegionFactory
{
public:
    SoundStreamRegionFactory() : RedRegionFactory("soundstream") {}

    // SYNTHETIC: SWBF2 0x00539440
    // SoundStreamRegionFactory::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
};
  

namespace GameState
{

// VTABLE: SWBF2 0x0079fbdc
class State
{
public:
    // SYNTHETIC: SWBF2 0x00539540
    // GameState::State::`scalar deleting destructor'
    virtual ~State() {}
    virtual void Enter();
    virtual void Update(float, float);
    virtual void Exit();
};

// VTABLE: SWBF2 0x0079fc00
class MissionState : public State
{
public:

    // VTABLE: SWBF2 0x0079fc14
    class State
    {
    public:
        // SYNTHETIC: SWBF2 0x00539600
        // GameState::MissionState::State::`scalar deleting destructor'
        virtual ~State() {}
        virtual void Enter();
        virtual void Update(float, float);
        virtual void Exit();
    };

    class PlayState : public State
    {
    public:
        // VTABLE: SWBF2 0x0079fbac
        class State
        {
        public:
            // SYNTHETIC: SWBF2 0x00539630
            // GameState::MissionState::PlayState::State::`scalar deleting destructor'
            virtual ~State() {}
            virtual void Enter();
            virtual void Update(float, float);
            virtual void Exit();
        };

        class NormalState : public State
        {
        public:
            // SYNTHETIC: SWBF2 0x0053bda0
            // GameState::MissionState::PlayState::NormalState::`scalar deleting destructor'
            virtual ~NormalState() {}
            virtual void Enter();
            virtual void Update(float, float);
            virtual void Exit();
        };
        class PausedState : public State
        {
        public:
            // SYNTHETIC: SWBF2 0x0053bdd0
            // GameState::MissionState::PlayState::PausedState::`scalar deleting destructor'
            virtual ~PausedState() {}
            virtual void Enter();
            virtual void Update(float, float);
            virtual void Exit();
        };

        // SYNTHETIC: SWBF2 0x0053bd70
        // GameState::MissionState::PlayState::`scalar deleting destructor'
        virtual ~PlayState() {}
        virtual void Enter();
        virtual void Update(float, float);
        virtual void Exit();

        static NormalState _NormalState;
        static PausedState _PausedState;
    };

    static PlayState _PlayState;

    // SYNTHETIC: SWBF2 0x00539660
    // GameState::MissionState::`scalar deleting destructor'
    virtual ~MissionState() {}
    virtual void Enter();
    virtual void Update(float, float);
    virtual void Exit();

    static State* _pCurrentState;

    static void SetState(unsigned int uiState);

private:

    static void _SetState(State* pState);
};

// VTABLE: SWBF2 0x0079fb80
class ShellState : public State
{
public:
    // SYNTHETIC: SWBF2 0x00539570
    // GameState::ShellState::`scalar deleting destructor'
    virtual ~ShellState() {}
    virtual void Enter();
    virtual void Update(float, float);
    virtual void Exit();
};

void SetState(unsigned int uiState);
unsigned int GetState();
bool Update();

void _SetState(State* pState);

void UpdateTime();

void PostStateCleanup(bool bMission);

void PreStateInit(bool bMission);

void OneTimeInit();

void JournalStartTimes();

void SetIgnoreNextUpdate(bool bIgnore);

bool GetKioskMode();

void OneTimeCleanup();

void CreateBaseExplosionClasses();
void CreateBaseOrdnanceClasses();
void CreateBaseWeaponClasses();
void CreateBaseEntityClasses();

extern PblTimeBase _TimeBase;
extern PblTimeBase _TotalTimeBase;
extern PblTimeBase _SystemTimeBase;

// FUNCTION: SWBF2 0x00539190
inline PblTimeBase* GetSystemTimeBase() { return &_SystemTimeBase; }
}

extern bool gMemoryPoolErrors;
