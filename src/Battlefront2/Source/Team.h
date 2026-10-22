#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedColor.h"

class Character;
class Controllable;
class GameObjectClass;

// VTABLE: SWBF2 0x007af554
class Team
{
public:

    enum Side
    {
        side_Neutral = 0,
        side_Alliance = 1,
        side_Empire = 2,
        side_Republic = 3,
        side_CIS = 4,
        side_Locals = 5,
        side_HEROES = 6,
        side_VILLAINS = 7,
        side_MAX = 8
    };

    static const unsigned int MAX_SIDE_NAME_LOCAL_LEN = 11;

    Team(int iTeamNumber, int iMemberLimit, int iClassLimit);

    static void CreateAll();
    static void DestroyAll();

    static void OverrideMaxReinforcements(int iTeam, int iMax);
    static void CreateClient();
    static void DestroyClient();
    static void EnterClient();
    static void LeaveClient();
    static void InitAll();
    static void InitialUpdateAll();
    static void CopyFromHost();
    static RedColor GetColorByAffiliation(int iAffiliation);
    static Side TranslateTeamName(const char* pcName);

    static int FindSide(Side eSide);

    static int GetTeamMemberCount(int& iTeam1, int& iTeam2);
    static int GetAutoAssignTeamNumber();

    void SetName(const unsigned short* pName) { mName = (unsigned short*)pName; }

    // FUNCTION: SWBF2 0x00530e00
    const unsigned short* GetName() const { return mName; }
    void SetColor(int iTeam, RedColor color) { mColor[iTeam] = color; }
    unsigned int GetIcon() const { return mIcon; }
    void SetSide(Side side)
    {
        mSide = side;
        mSideNameLocal[0] = 0;
    }
    void SetCarrier(GameObjectClass* pCarrier) { mCarrier = pCarrier; }

    GameObjectClass* GetCarrierPtr() const { return mCarrier; }
    float GetAggressiveness() const { return mAggressiveness; }

    // FUNCTION: SWBF2 0x004293a0
    static Team* GetPtr(int iTeam) { return sTeams[iTeam]; }

    static int GetCount() { return 8; }
    // FUNCTION: SWBF2 0x00429370
    int GetSpecialClassCount() const { return mSpecialClassCount; }

    // FUNCTION: SWBF2 0x00429340
    Side GetSide() const { return mSide; }

    // FUNCTION: SWBF2 0x00429330
    int GetNumber() const { return mTeamNumber; }

    int GetMemberLimit() const { return mMemberLimit; }
    int GetMemberActive() const { return mMemberActive; }
    Character* GetMemberPtr(int iIndex) const
    {
        if (iIndex < mMemberCount)
        {
            return mMemberArray[iIndex];
        }
        return 0;
    }

    bool AddMember(Character* pCharacter)
    {
        if (mMemberCount < mMemberLimit)
        {
            mMemberArray[mMemberCount++] = pCharacter;
            return true;
        }
        return false;
    }

    // FUNCTION: SWBF2 0x00450320
    bool RemoveMember(Character* pCharacter)
    {
        for (int i = 0; i < mMemberCount; ++i)
        {
            if (mMemberArray[i] == pCharacter)
            {
                --mMemberCount;
                for (; i < mMemberCount; ++i)
                {
                    mMemberArray[i] = mMemberArray[i + 1];
                }
                mMemberArray[mMemberCount] = 0;
                return true;
            }
        }
        return false;
    }

    void SetSide(const char* pcName);
    void SetValues(int iCurReinforcements, int iMemberActive, int iPoints);
    void GetValues(int& iCurReinforcements, int& iMemberActive, int& iPoints);
    bool IsMemberAlive(int iIndex) const;
    int GetNumMembersAlive() const;
    Character* GetRandomLiveCharacter() const;
    bool AddClass(GameObjectClass* pClass, int iMinUnits, int iMaxUnits);
    bool AddClass(const char* pcName, int iMinUnits, int iMaxUnits);
    bool AddSpecialClass(GameObjectClass* pClass);
    bool AddSpecialClass(unsigned int uHash);
    GameObjectClass* FindClass(GameObjectClass* pClass);
    GameObjectClass* FindClass(const char* pcName);
    int GetClassIndex(GameObjectClass* pClass);
    bool SetClassUnitCounts(GameObjectClass* pClass, int iMinUnits, int iMaxUnits);
    void SetCarrier(const char* pcName);
    void SetBleedRate(float fRate);
    void SetPoints(int iPoints);
    void AddPoints(int iPoints);
    void SetCurReinforcements(int iCount);
    int PickRandomClassIndex(Character* pCharacter) const;
    int GetClassMinUnits(int iIndex) const;
    int GetClassMaxUnits(int iIndex) const;
    int GetClassCurrentUnits(int iIndex) const;
    void GenerateRandomName(unsigned short* const pName, int iTeam) const;
    void SetAggressiveness(float fAggressiveness);

protected:
    void _SetClassUnitCounts(int iIndex, int iMinUnits, int iMaxUnits);
    void _ClampClassCounts(int iIndex);

public:

    void AddReinforcements(int iCount);

    bool IsBleeding() const { return mBleedRate > 0.0f; }

    // FUNCTION: SWBF2 0x0058aac0
    float GetBleedRate() { return mBleedRate; }

    int GetAffiliation(int iTeam) const { return mAffiliation[iTeam]; }
    bool GetAIMembersIgnoreVehicles() { return mbAIMembersIgnoreVehicles; }

    static Team** sTeams;

    static Team* sHostTeams[8];
    static Team* sClientTeams[8];
    static int sTeamMaxReinforcementsOverride[2];

    // FUNCTION: SWBF2 0x00490b30
    static unsigned int GetWinningTeams() { return sWinningTeams; }
    static unsigned int sWinningTeams;

    // FUNCTION: SWBF2 0x0058aad0
    static void SetWinningTeams(unsigned int uTeams) { sWinningTeams = uTeams; }

    // FUNCTION: SWBF2 0x00429360
    int GetClassCount() const { return mClassCount; }

    // FUNCTION: SWBF2 0x00429380
    GameObjectClass* GetClassPtr(int iIndex) const
    {
        if (iIndex >= 0 && iIndex < mClassCount)
        {
            return mClassArray[iIndex];
        }
        return 0;
    }

    void SetIcon(unsigned int uIcon)
    {
        mIcon = uIcon;
        if (!mConquestIcon)
        {
            mConquestIcon = uIcon;
        }
        if (!mCTFIcon)
        {
            mCTFIcon = uIcon;
        }
    }
    void SetConquestIcon(unsigned int uIcon) { mConquestIcon = uIcon; }
    void SetCTFIcon(unsigned int uIcon) { mCTFIcon = uIcon; }
    void SetAffiliation(int iTeam, int iAffiliation) { mAffiliation[iTeam] = (char)iAffiliation; }
    int GetCurReinforcements() const { return mCurReinforcements; }

    // FUNCTION: SWBF2 0x00438ae0
    int GetMaxReinforcements() const { return mMaxReinforcements; }

    // FUNCTION: SWBF2 0x00438af0
    int GetPoints() const { return mPoints; }

    const GameObjectClass* GetHeroClass() const;

    RedColor GetColor(int iTeam) const;

    virtual ~Team();
    virtual void Init();
    virtual void InitialUpdate();
    virtual void Update(float);
    virtual Character * FindByUnit(Controllable *);
    virtual void CreateCharacter(const unsigned short *, int);

    mutable PblRandom mNameRand;
    int mTeamNumber;
    unsigned short* mName;
    char mSideNameLocal[12];
    unsigned int mIcon;
    unsigned int mConquestIcon;
    unsigned int mCTFIcon;
    int mCurReinforcements;
    int mMaxReinforcements;
    float mBleedRate;
    int mMemberLimit;
    int mMemberCount;
    int mMemberActive;
    Character** mMemberArray;
    int mClassLimit;
    int mClassCount;
    int mSpecialClassCount;
    GameObjectClass** mClassArray;
    int* mClassMinUnits;
    int* mClassMaxUnits;
    GameObjectClass* mCarrier;
    int mPoints;
    Side mSide;
    RedColor mColor[8];
    char mAffiliation[8];
    int mCaptureTeam;
    float mAggressiveness;
    unsigned short* mFirstNames[64];
    int mFirstCount;
    unsigned short* mLastNames[64];
    int mLastCount;
    bool mbAIMembersIgnoreVehicles;
    bool mbAllowAISpawn;
    bool mbInitialUpdateDone;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
