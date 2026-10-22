#pragma once

#include <stddef.h>

#include "GameMusic.h"
#include "GameObject.h"
#include "GameSound.h"

class EntityHologramClass;
class Controllable;
class PblQuaternion;
class PblVector3;
class RedModel;

// VTABLE: SWBF2 0x00798604
class CommandPostClass
{
public:
    CommandPostClass(unsigned int uHash);

    CommandPostClass(CommandPostClass* pBase, unsigned int uHash);

    virtual ~CommandPostClass();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue,
                             const RedModel* pModel, const char* pszNode);

    float mNeutralizeTime;
    float mCaptureTime;
    RedModel* m_pHoloModel[8];
    EntityHologramClass* m_pHologramClass;
    float m_fHoloTurnOnTime;
    GameSound mSoundAmbience[8];
    GameSound mSoundDischarge;
    GameSound mSoundCharge;
    GameSound mSoundCaptured;
    GameSound mSoundLost;
    GameSound mSoundDispute;
    float mSoundPitchDev;
    GameMusic mMusicCapture[8];
    GameMusic mMusicLost[8];
    PblQuaternion* mSpawnRotation;
    PblVector3* mSpawnPosition;
    int mSpawnCount;
    int mBleedValue[8];
    int mStrategicAttackValue[8];
    int mStrategicDefendValue[8];
    int mStrategicBan;

    bool mDisplayHUDIndex : 1;
};

  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class CommandPost;

class CommandPostHandle
{
public:
    CommandPost* mPost;

    void operator=(CommandPost* pPost);

    CommandPostHandle() : mPost(0) {}
    explicit CommandPostHandle(CommandPost* pPost) : mPost(pPost) {}

    operator CommandPost*() const;
    CommandPost* operator->() const;
};

class ConstCommandPostHandle
{
public:
    ConstCommandPostHandle(const CommandPost* pPost) : mPost(const_cast<CommandPost*>(pPost)) {}

    void operator=(const CommandPost* pPost) { mPost = const_cast<CommandPost*>(pPost); }

    operator const CommandPost*() const;

    const CommandPost* operator->() const;

    CommandPost* mPost;
};

#include "GameSoundStream.h"
#include "Team.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class EntityHologram;
class GameObject;
class PblMatrix;
class RedPath;
class RedRegion;

// VTABLE: SWBF2 0x00798610
class CommandPost : public Thread
{
public:

    class VoiceOver
    {
    public:

        enum Stream
        {
            stream_Capture = 0,
            stream_Lost = 1,
            stream_Dispute = 2,
            stream_Saved = 3,
            stream_Info = 4,
            stream_MAX = 5
        };

        VoiceOver();
        ~VoiceOver();

        GameSoundStream mStream[Team::side_MAX][stream_MAX];
    };

    CommandPost(CommandPostClass* pClass, int iIndex);
    virtual ~CommandPost();

    virtual bool Update(float fDeltaTime);

    static int GetCount();
    static CommandPost* GetActive(int iIndex);
    static CommandPost* BuildPost(GameObject* pObject, CommandPostClass* pClass);
    static CommandPost* FindPostByName(const char* pszName);
    static CommandPost* FindPostByObject(GameObject* pObject);
    static void Create();

    static void Destroy();

    static void CreateClient();
    static void DestroyClient();
    static void EnterClient();
    static void LeaveClient();

    static void ForceIndex(int iIndex);

    static CommandPost** sPostArray;

    static int* sPostCount;

    // FUNCTION: SWBF2 0x0044c560
    static CommandPost* FindPostByIndex(int iIndex) { return sPostArray[iIndex]; }

    // FUNCTION: SWBF2 0x004503a0
    int GetIndex() const { return mPostIndex; }

    // FUNCTION: SWBF2 0x0044f910
    int GetHoldTeam() const { return mHoldTeam; }
    // FUNCTION: SWBF2 0x0044f920
    int GetHoldCount(int iTeam) const { return mHoldCount[iTeam]; }
    // FUNCTION: SWBF2 0x0044f930
    unsigned __int64 GetHoldPlayerMask() const { return mHoldPlayerMask; }
    // FUNCTION: SWBF2 0x0044f940
    int GetBiasTeam() const { return mBiasTeam; }

    // FUNCTION: SWBF2 0x00429350
    const unsigned short* GetLocalizeName() const { return mLocalizeName; }

    void Kill();
    void Respawn();

    void SetProperty(unsigned int uProperty, const char* pszValue);

    void SetTeam(int iOldTeam, int iNewTeam);

    void TurnHologramOff();

    const RedPath* GetTurretPath() const { return mTurretPath; }

    const PblMatrix& GetMatrix() const;

    bool CanTeamSpawnHere(int iTeam) const;

    void PlayVoiceOver(Team::Side eSide, VoiceOver::Stream eStream, bool bPlayMusic);

    int GetStrategicAttackValue(Team::Side eSide) const;

    int GetObjectsInCaptureRegion(GameObject** const ppObjects, int iMaxObjects) const
    {
        return GetObjectsInRegion(mCaptureRegion, ppObjects, iMaxObjects);
    }

    int GetBleedValue(int iTeam) const;

    int PushSpawnQueue(bool bPlayer, bool bTeam);
    int PopSpawnQueue(bool bPlayer, bool bTeam);

    // FUNCTION: SWBF2 0x004293b0
    GameObject* GetGameObject() { return mObject; }

    // FUNCTION: SWBF2 0x0043ed40
    const GameObject* GetGameObject() const
    {
        return const_cast<CommandPost*>(this)->GetGameObject();
    }

    int GetStrategicDefendValue(Team::Side eSide) const;

    int GetStrategicBan() const;

    PblMatrix GetMatrixOnSpawnPath(bool bAlly) const;

    bool IsInsideCaptureRegion(const GameObject* pObject) const
    {
        return IsInsideRegion(mCaptureRegion, pObject);
    }

    bool IsInsideControlRegion(const GameObject* pObject) const
    {
        return IsInsideRegion(mControlRegion, pObject);
    }

    float GetCaptureRatio() const;
    float GetNeutralizeRatio() const;
    void CopyFrom(const CommandPost* pOther);
    void SetAICanCapture(unsigned int iTeam, bool bCanCapture);

protected:
    void Attach(GameObject* pObject);

    bool IsInsideRegion(const RedRegion* pRegion, const GameObject* pObject) const;

    int GetObjectsInRegion(const RedRegion* pRegion, GameObject** const ppObjects, int iMaxObjects) const;

    float GetCaptureTime() const;
    float GetNeutralizeTime() const;

public:
    int mPostIndex;
    int mHUDPostIndex;
    char* mName;
    char* mLabel;
    unsigned short* mLocalizeName;
    PblHandle<GameObject> mObject;
    PblVector3 mPosition;
    int mHoldTeam;
    int mHoldCount[8];
    unsigned __int64 mHoldPlayerMask;
    unsigned __int64 mSkippedPlayerMask;
    int mBiasTeam;
    unsigned int mAICanCaptureTeamMask;
    RedRegion* mCaptureRegion;
    RedRegion* mControlRegion;
    RedRegion* mKillRegion;
    RedPath* mSpawnPath;
    RedPath* mAllyPath;
    RedPath* mTurretPath;
    RedRegion* mSpawnRegion;
    int mAllyCount;
    float mNeutralizeTimer;
    float mCaptureTimer;
    int mSpawnQueue[2][2];
    int mBleedValue[8];
    int mStrategicAttackValue[8];
    int mStrategicDefendValue[8];
    int mStrategicBan;

    float mAISpawnWeight;
    VoiceOver mVoiceOver[8];
    EntityHologram* m_pHologram;
    GameSoundControllable mSoundAmbience;
    GameSound mSoundAmbienceProps;
    GameSoundControllable mSoundTransition;
    GameSound mSoundTransitionProps;
    CommandPostClass* mClass;
    bool mIsAISpawnPoint : 1;
    bool mDisplayHUDIndex : 1;
    bool mbIsEnabled : 1;
public:
    static bool sEnableVO;
};

// FUNCTION: SWBF2 0x004503b0
inline CommandPostHandle::operator CommandPost*() const
{
    if (mPost)
    {
        if (!mPost->GetGameObject())
        {
            return 0;
        }
    }
    return mPost;
}

// FUNCTION: SWBF2 0x004503e0
inline CommandPost* CommandPostHandle::operator->() const
{
    return *this;
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

unsigned int __fastcall GetDoorEntityId(const unsigned int* pRecord);
