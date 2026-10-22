#pragma once

#include "PebbleFL/Common/PblList.h"
#include "Adrenaline.h"
#include "CommandPost.h"
#include "Thread.h"

class CommandPost;
class Controllable;
class GameObject;
class MedalsMgr;
class FlagItem;
class EntityClass;
class Team;

class PlayerStats;
class NetPktGroup;
class PblMatrix;
namespace PlayerProfile { struct tProfile; }

// VTABLE: SWBF2 0x00797134
class Character : public Thread
{
public:
    Character();
    virtual ~Character();
    virtual bool Update(float fDeltaTime);
    virtual void InitialUpdate();
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    void SetName(const unsigned short* pName);
    void GetHeroName(unsigned short* pName, int iSize) const;
    void SetSpawnReady(bool bReady);
    void ChangeClass(int iClassIndex);
    PlayerProfile::tProfile* GetProfile() const;
    static PlayerStats& GetPlayerStats(Character* pCharacter);

    void SetUnit(Controllable* pUnit);
    void MakePlayer(int iTeam, const unsigned short* pName, int iPlayerId);
    void SetVehicle(Controllable* pVehicle);
    void SetRemote(Controllable* pRemote);
    void Died();
    void SelectTeam(int iTeam);
    float TimeSinceLastVehicleOrRemoteChange();
private:
    bool ChangeTeam(int iTeam);
public:
    void SelectClass(int iClassIndex);
    void SelectPost(CommandPost* pPost);
    void SelectSpawn(int iTeam, int iClass, CommandPost* pPost);
    void Spawn(const PblMatrix& mMatrix);
    void Respawn();

private:
    void PlayerTakeoverAI(Character* pCharacter);
    void CheckOutOfBounds(float fDeltaTime);

public:

    PlayerStats& GetPlayerStats();

    // FUNCTION: SWBF2 0x00429440
    float GetDelayTimer() const { return mDelayTimer; }

    int GetSpawnCycle() const { return mSpawnCycle; }

    // FUNCTION: SWBF2 0x004293e0
    int GetPlayerId() const { return mPlayerId; }
    // FUNCTION: SWBF2 0x00450440
    Controllable* GetRemote() { return mRemote; }
    // FUNCTION: SWBF2 0x00429460
    MedalsMgr* GetMedalsManager() const { return mMedalsMgr; }

    bool IsHero() const;

    // FUNCTION: SWBF2 0x00429470
    static Character* FindPlayer(int iPlayerId)
    {
        if (sCharacters && iPlayerId >= 0 && iPlayerId < sCharacterLimit)
        {
            Character* pCharacter = &sCharacters[iPlayerId];
            if (pCharacter->mPlayerId == iPlayerId)
            {
                return pCharacter;
            }
        }
        return 0;
    }

    // FUNCTION: SWBF2 0x00437510
    static Character* GetPtr(int iIndex)
    {
        if (sCharacters && iIndex >= 0 && iIndex < sCharacterLimit)
        {
            return &sCharacters[iIndex];
        }
        return 0;
    }

    // FUNCTION: SWBF2 0x00429420
    Controllable* GetControllable()
    {
        if (mRemote)
        {
            return mRemote;
        }
        if (mVehicle)
        {
            return mVehicle;
        }
        return mUnit;
    }

    const CommandPost* GetAssignedCP() const;

    void SetAssignedCP(const CommandPost* pPost);

    // FUNCTION: SWBF2 0x0055ca30
    const unsigned short* GetName() const { return mName; }

    // FUNCTION: SWBF2 0x004293f0
    int GetTeamNumber() const { return mTeamNumber; }
    // FUNCTION: SWBF2 0x00429400
    Team* GetTeamPtr() const { return mTeamPtr; }
    // FUNCTION: SWBF2 0x004385c0
    bool IsPlayer() const { return mPlayerId >= 0; }
    // FUNCTION: SWBF2 0x00429410
    Controllable* GetUnit() { return mUnit; }

    // FUNCTION: SWBF2 0x0043aac0
    Controllable* GetVehicle() { return mVehicle; }

    // FUNCTION: SWBF2 0x00439b20
    const Controllable* GetControllable() const
    {
        if (mRemote)
        {
            return mRemote;
        }
        if (mVehicle)
        {
            return mVehicle;
        }
        return mUnit;
    }

    const unsigned short* GetRegularName() const { return mRegularName; }

    static Character* CreateCharacter() { return &sCharacters[sCharacterLimit++]; }

    static void EnterClient();
    static void LeaveClient();

    void Init(int iTeamNumber, const unsigned short* pName, int iPlayerId);

    bool IsOutOfBounds() const { return mOutOfBoundsTimer >= 0.0f; }

    PblList<Character>::Node* GetGoalNode() { return &mGoalNode; }

    Adrenaline& GetAdrenaline() { return mAdrenaline; }

    // FUNCTION: SWBF2 0x0047a6f0
    float GetSpawnTime() const { return mSpawnTime; }

    // FUNCTION: SWBF2 0x00429450
    void SetSpawnTime(float fTime) { mSpawnTime = fTime; }

    CommandPost* GetCPHold() { return mCPHold; }
    float GetCPHoldTimestamp() { return mCPHoldTimestamp; }
    void SetCPHold(CommandPost* pPost) { mCPHold = pPost; }
    void SetCPHoldTimestamp(float fTimestamp) { mCPHoldTimestamp = fTimestamp; }
    GameObject* GetDamagedVehicle() const { return mDamagedVehicle; }
    void SetDamagedVehicle(GameObject* pVehicle)
    {
        mDamagedVehicle = pVehicle;
        mTotalVehicleDamage = 0.0f;
    }
    void AddTotalVehicleDamage(float fDamage) { mTotalVehicleDamage += fDamage; }

    ConstCommandPostHandle mAssignedPost;
    Adrenaline mAdrenaline;
    PblList<Character>::Node mGoalNode;
    unsigned short mName[64];
    unsigned short mRegularName[64];
    int mPlayerId;
    int mTeamNumber;
    Team* mTeamPtr;
    int mClassIndex;
    EntityClass* mClassPtr;
    CommandPostHandle mPost;
    Controllable* mUnit;
    Controllable* mVehicle;
    Controllable* mRemote;
    float mLastVehicleOrRemoteChangeTime;
    float mDelayTimer;
    int mSpawnCycle;
    int mSpawnSlot;
    bool mSpawnReady;
    bool mHeroFlag;
    float mVehicleBlockTime;
    float mObjectHitTimer[3];
    float mObjectHitMultiplier[3];
    float mOutOfBoundsTimer;
    float mSpawnTime;

    FlagItem* mCarriedFlagPtr;
    GameObject* mDamagedVehicle;
    float mTotalVehicleDamage;
    CommandPostHandle mCPHold;
    float mCPHoldTimestamp;
    MedalsMgr* mMedalsMgr;
    Character* mTakeoverChr;
    float mTakeoverTime;
    bool mTakeoverFirstTime : 1;

    static void CreateAll(int iCount);

    static void InitAll();
    static void AddPlayer(int iPlayerId);
    static void RemovePlayer(int iPlayerId);
    void MakeAI();

    static void DestroyAll();

    static int sCharacterLimit;
    static Character* sCharacters;
    static PlayerStats* sPlayerStats;
    static int sCharacterCount;

    void SetVehicleBlockTime(float fTime);

    float GetVehicleBlockTime() const;

    float GetObjectHitValue(int iAffiliation) const;

    void RegisterHit(int iTeam, float fMultiplier);

    float GetObjectHitMultiplier(int iAffiliation) const;
};
 

 
 
 
 
 
 
