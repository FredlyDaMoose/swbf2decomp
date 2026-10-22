#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class Character;
class Controllable;
class RedCamera;
class GameSound;
class GameObject;

class LockOnManager
{
public:

    void CalculateScreenCoordinates(Controllable* const pCtrl, RedCamera* const pCamera, const PblVector3* pPos,
                                    float* pScreenX, float* pScreenY, bool* pOffScreen, bool bAimRay);
    class TargetInfo;

    struct Target
    {

        // FUNCTION: SWBF2 0x0057abd0
        Target()
        {
            obj.mObject = 0;
            obj.mSavedHandleId = 0;
        }

        // FUNCTION: SWBF2 0x0057abe0
        ~Target() {}

        void SetObj(GameObject* const pObject, TargetInfo* const pInfo);

        void InitPlayer(GameObject* const initObj, unsigned int initTeamMask, TargetInfo& targetInfo,
                        bool isFriendly);

        void InitCommon(GameObject* const initObj, unsigned int initTeamMask);

        PblHandle<GameObject> obj;
        unsigned int teamMask;
        float maxDistTrack;
        float maxDist;
        float minDist;
        float timer;
    };

    class TargetHistory
    {
    public:
        TargetHistory()
        {

            for (int i = 0; i < 64; i++)
            {
                mHistory[i].mObject = 0;
                mHistory[i].mSavedHandleId = 0;
            }
            Clear();
        }
        ~TargetHistory() { Clear(); }

        void Clear()
        {
            mNextIndexToFill = 0;
            mCount = 0;
        }

        void Add(GameObject* const pObject);
        bool IsEmpty() const { return mCount == 0; }

        bool Check(GameObject* const pObject) const;

        PblHandle<GameObject> mHistory[64];
        unsigned int mNextIndexToFill;
        unsigned int mCount;
    };

    class TargetInfo
    {
    public:
        TargetInfo()
        {
            mVisibilityChecked = false;
            mVisible = false;
            mAttacker = false;
            mPositionValid = false;
            mFriendly = false;
            mFlagCarrier = false;
        }

        void Clean()
        {
            mFlagCarrier = false;
            mVisibilityChecked = false;
            mOnScreen = false;
            mVisible = false;
            mAttacker = false;
            mLocked = false;
            mHintTarget = false;
            mPositionValid = false;
            mFriendly = false;
        }

        void SetLocked(bool bLocked) { mLocked = bLocked; }
        void SetHint(bool bHint) { mHintTarget = bHint; }
        void SetAttacker(bool bAttacker) { mAttacker = bAttacker; }
        void SetOnScreen(bool bOnScreen) { mOnScreen = bOnScreen; }
        void SetFriendly(bool bFriendly) { mFriendly = bFriendly; }
        void SetFlagCarrier(bool bFlagCarrier) { mFlagCarrier = bFlagCarrier; }
        void ClearPositionValid() { mPositionValid = false; }
        bool GetVisible() const { return mVisible; }
        bool GetFlagCarrier() const { return mFlagCarrier; }

        PblVector3 mPosition;
        bool mVisibilityChecked : 1;
        bool mOnScreen : 1;
        bool mVisible : 1;
        bool mAttacker : 1;
        bool mLocked : 1;
        bool mHintTarget : 1;
        bool mPositionValid : 1;
        bool mFriendly : 1;
        bool mFlagCarrier : 1;
    };

    LockOnManager(int iPlayerIndex);

    GameObject* GetLockedTarget() const;
    void ForceNewTargetLock(GameObject* pObject);
    unsigned int GetTargetLockedChangedTurn() const;
    static LockOnManager* GetByPlayerIndex(unsigned int uIndex);
    void RemoveTargetLock();

    bool IsGameObjectTargeted(const GameObject* const pObject);
    static bool IsGameObjectTargetedAll(const GameObject* const pObject);

protected:

    static Target* FindTargetByGameObject(Target* const pTargets, unsigned int uNumTargets,
                                          const GameObject* const pObject);

public:

    void SetAllowedTypes(int iTypes) { mTargetsAllowedFilter = iTypes; }
    void SetAutoLose(float fTime, bool bVisibilityCheck, float fDotProd, float fDist)
    {
        mAutoLoseTime = fTime;
        mAutoLoseVisibilityCheck = bVisibilityCheck;
        mAutoLoseDotProd = fDotProd;
        mAutoLoseDistSquare = fDist * fDist;
    }
    void SetAutoLoseLocked(float fTime, bool bVisibilityCheck, float fDotProd, float fDist)
    {
        mAutoLoseLockedTime = fTime;
        mAutoLoseLockedVisibilityCheck = bVisibilityCheck;
        mAutoLoseLockedDotProd = fDotProd;
        mAutoLoseLockedDistSquare = fDist * fDist;
    }
    void SetTarget(float fDotProd, float fDotProdLocked, float fDist, bool bVisible)
    {
        mLockDotProdFilter = fDotProd;
        mLockDotProdFilterLocked = fDotProdLocked;
        mLockDistFilter = fDist;
        mLockDistSquareFilter = fDist * fDist;
        mLockVisibleFilter = bVisible;
    }
    void SetAuto(bool bAutoTarget, bool bAutoLock)
    {
        mAutoTarget = bAutoTarget;
        mAutoLock = bAutoLock;
    }
    void SetNumNearestTargets(unsigned int uCount) { mNewNumNearestTargets = uCount; }
    void SetNumAttackerTargets(unsigned int uCount) { mNewNumAttackerTargets = uCount; }

    static bool IsTargetObjectTypeAllowed(GameObject* gameObject, GameObject* me, bool noLockon,
                                          unsigned int targetsAllowedFilter, unsigned int newNumNearestTargets,
                                          bool displayIconsOnEverything);
    ~LockOnManager();

    static void CreateAll();
    static void DestroyAll();

    static void UpdateAll(float fDeltaTime);

    void Update(float fDeltaTime);
    void SetDisplayIconsOnEverything(bool bEverything);

    bool GetDisplayIconsEverything() const;

    static void ClearVisibilityChecked(TargetInfo* const pInfo, unsigned int uCount);
    static void ClearPositionValid(TargetInfo* const pInfo, unsigned int uCount);

    static void AddTargetCommon(GameObject* pActor, unsigned int uTeamMask);

    static void RemoveTargetCommon(GameObject* pActor, unsigned int uTeamMask);
protected:
    void SetLockedTarget(GameObject* const pObject, TargetInfo* const pInfo);
    void SetupNearestTargets(Controllable* const playerControllable, GameObject* const playerObject,
                             unsigned int arrayStartIndex, unsigned int arrayEndIndex, float deltaTime,
                             unsigned int teamMask);
    void UpdateTargetVisibility(Controllable* const pCtrl, RedCamera* const pCamera, PblVector3& vEye,
                                Target& rTarget, TargetInfo& rInfo, bool bCommon, unsigned int uTeamMask,
                                bool& bArg8, unsigned int& uArg9);
public:

    int mPlayerIndex;
    Character* mPlayerCharacter;
    Target mTargetPlayer[64];
    TargetHistory mTargetHistory;
    PblHandle<GameObject> mNewAttacker;
    unsigned int mNumNearestTargets;
    unsigned int mNumAttackerTargets;
    unsigned int mNewNumNearestTargets;
    unsigned int mNewNumAttackerTargets;
    PblHandle<GameObject> mLockedObj;
    unsigned int mLockedObjChangedTurn;
    PblHandle<GameObject> mForceNewLockTarget;
    float mTimeSinceLastForceLock;
    int mForceNewLock;
    float mAutoLoseTime;
    float mAutoLoseDotProd;
    float mAutoLoseDistSquare;
    float mAutoLoseLockedTime;
    float mAutoLoseLockedDotProd;
    float mAutoLoseLockedDistSquare;
    float mLockDotProdFilter;
    float mLockDotProdFilterLocked;
    float mLockDistFilter;
    float mLockDistSquareFilter;
    float mSafeZone;
    TargetInfo mTargetPlayerInfo[64];
    TargetInfo mTargetCommonInfo[32];
    int mTargetsAllowedFilter;
    bool mAutoLoseVisibilityCheck : 1;
    bool mAutoLoseLockedVisibilityCheck : 1;
    bool mLockVisibleFilter : 1;
    bool mAutoTarget : 1;
    bool mAutoLock : 1;
    bool mTargetVisibilityUpdate : 1;
    bool mDisplayIconsOnEverything : 1;

    const Target* GetPlayerTargets() const { return mTargetPlayer; }
    const TargetInfo* GetPlayerTargetInfos() const { return mTargetPlayerInfo; }
    const TargetInfo* GetCommonTargetInfo() const { return mTargetCommonInfo; }
    static const Target* GetCommonTargets() { return sTargetCommon; }

    static Target sTargetCommon[32];

    static LockOnManager* sLockOnManagers[4];
    static unsigned int sNumLockOnManagers;

    static GameSound sTargetLockSound;
};

 
 
 
 
 
 
 
 
