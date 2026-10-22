#pragma once

#include <stddef.h>

#include "Thread.h"

class Character;
class CommandPost;

// VTABLE: SWBF2 0x007af2ac
class SpawnManager : public Thread
{
public:
    static void Create();
    static void CreateClient();
    static void Destroy();
    static void DestroyClient();
    static void EnterClient();
    static void LeaveClient();

    // FUNCTION: SWBF2 0x004292e0
    static SpawnManager* Instance() { return sInstance; }

    SpawnManager();
    virtual ~SpawnManager();

    // SYNTHETIC: SWBF2 0x0064e250
    // SpawnManager::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    void InitialUpdate();
    void SetSpawnDelay(float fCycleDelay, float fSlotDelay, int iTeam);
    void SetupSoldierMemoryPool();

    CommandPost* GetRandomSpawnPoint(int iTeam);
    CommandPost* Request(CommandPost* pPost, int iTeam);
    CommandPost* RequestSpawnPoint(CommandPost* pPost, Character* pChr);

    int GetAllyUnitCount() const { return mAllyUnitCount; }
    bool IsHistorical() const { return mHistorical; }

    int GetCycleIndex(int iTeam) const { return mCycleIndex[iTeam]; }
    float GetSlotTimer(int iTeam) const { return mSlotTimer[iTeam]; }
    void ResetSlotTimer(int iTeam) { mSlotTimer[iTeam] = mSlotDelay[iTeam]; }

    float GetCycleDelay(int iTeam) const { return mCycleDelay[iTeam]; }
    float GetCycleTimer(int iTeam) const { return mCycleTimer[iTeam]; }

    // FUNCTION: SWBF2 0x0064ee40
    int GetUnitCount(int iTeam) const { return mMaxUnitCount[iTeam]; }

    void SetUnitCount(int iTeam, int iCount) { mMaxUnitCount[iTeam] = iCount; }
    void SetHistorical(bool bHistorical) { mHistorical = bHistorical; }

private:
    void AddPlayerCharacter(int iTeam, int iPlayer);
    int GetTotalCharacterCount();

public:
    int mMaxUnitCount[8];
    int mCurrentUnitCount[8];
    int mAllyUnitCount;
    float mCycleDelay[8];
    float mSlotDelay[8];
    float mCycleTimer[8];
    int mCycleIndex[8];
    float mSlotTimer[8];
    bool mHistorical;

    static SpawnManager* sInstance;
    static SpawnManager* sHostInstance;
    static SpawnManager* sClientInstance;
};
 
 
 
 
 
 
