#pragma once

#include <stddef.h>

#include "Thread.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

class CollisionObject;
class PAHierarchy;
class PblFileChunk;
class NetPktGroup;

enum eTransitionType
{
    PA_POP = 0,
    PA_LINEAR = 1,
    PA_SPLINE = 2
};

class PABaseKey
{
public:
    float mTime;
    PblVector3 mVal;
    eTransitionType mTransition;
    PblVector3 mSlope0;
    PblVector3 mSlope1;
};
 
 
 

class PAAnimation
{
public:
    PAAnimation();
    virtual ~PAAnimation();

    static void Read(PblFileChunk& chunk);

    void SetAnimationInfo(bool bLoop, bool bLocalTranslation, float fTotalTime);
    bool AddPositionKey(const PABaseKey& key);
    bool AddRotationKey(const PABaseKey& key);
    float GetTotalTime() { return mTotalTime; }

    void CalculateNextMatrix(float fDeltaTime, float& fTime, const PblMatrix& start, PblMatrix& result) const;
    void Update(float fDeltaTime, float& fTime, CollisionObject* pObject, const PblMatrix& start) const;

    bool GetKeys(float fTime, PABaseKey& key0, PABaseKey& key1, bool bRotation, unsigned char,
                 unsigned char* pKeyIndex) const;
    void InterpolateKeys(PABaseKey key0, PABaseKey key1, float fTime, PABaseKey& result) const;

    static int _HashTableNumEntries;
    static PblHashTable<PAAnimation, 32> _HashTable;

    unsigned char mNumPosKeys;
    unsigned char mStartPosKey;
    unsigned char mNumRotKeys;
    unsigned char mStartRotKey;
    bool mLoop;
    bool mLocalTranslation;
    float mTotalTime;
};
 
 
 
 

struct PAAnimationData
{

    // FUNCTION: SWBF2 0x00626bf0
    PAAnimationData() {}

    unsigned int mAnimHash;
    unsigned int mObjHash;
    float mClientTimer;
    float mHostTimer;
    float* mTimer;
    CollisionObject** mGeometry;
    CollisionObject* mHostGeometry;
    CollisionObject* mClientGeometry;
    PblMatrix mInitialMatrix;
};
 
 

class PAHierarchyNode
{
public:
    unsigned int mObjHash;
    CollisionObject** mGeometry;
    CollisionObject* mHostGeometry;
    CollisionObject* mClientGeometry;
    PblMatrix mLocalMatrix;
};
 
 

class PAHierarchy
{
public:
    PAHierarchy();

    PAHierarchyNode mRoot;
    int mNumObjs;
    PAHierarchyNode* mObjs;
};
 
 
 

class PAAnimationGroup
{
public:

    void ReadObject(NetPktGroup& group);
    void WriteObject(NetPktGroup& group);

    bool* EnterClient();
    bool* LeaveClient();

    static void __cdecl ReadHierarchy(PblFileChunk& chunk);
    PAAnimationGroup();
    virtual ~PAAnimationGroup();

    static void Read(PblFileChunk& chunk);

    static void GeometryDeleted(CollisionObject* pObject);

    static void GeometryCreated(unsigned int uObjHash, CollisionObject* pObject, const PblMatrix& matrix);

    static void ClientGeometryCreated(CollisionObject* pHost, CollisionObject* pClient);

    bool AddAnimation(unsigned int uiAnimHash, unsigned int uiObjHash);
    void Play();
    void Pause();
    void SetCurrentTime(float fTime);
    void SetEndTime(float fTime);
    void UpdateStartPositions();

    static void UpdateTreeGrid(CollisionObject* pObject);

    void UpdateAnimations(float fDeltaTime);

    static int _HashTableNumEntries;
    static PblHashTable<PAAnimationGroup, 32> _HashTable;

    static int mNumHierarchies;
    static PAHierarchy* mHierarchies[32];

    bool* mActive;
    bool mHostActive;
    bool mClientActive;
    float mEndTime;
    unsigned char mStartAnim;
    unsigned char mNumAnims;
    bool mStopWhenControlled : 1;
    bool mDisableHierarchies : 1;
};
 
 
 
 
 
 
 

class PAManagerInterpolator : public Thread
{
public:
    PAManagerInterpolator() {}
    virtual ~PAManagerInterpolator() {}
    virtual bool Update(float fDeltaTime);

    void Create();

    void Destroy();

    float mTotalTurnTime;
    float mTotalFrameTime;
    float mNextTotalTurnTime;
    bool mIsFrameUpdate;
};
 
 

// VTABLE: SWBF2 0x007ade08
class PAManager : public Thread
{
public:
    PAManager();
    virtual ~PAManager();
    virtual bool Update(float fDeltaTime);

    static void Create();
    static void Destroy();

    static void CreateClient();
    static void DestroyClient();
    static void EnterClient();
    static void LeaveClient();

    static void ReadObjects(NetPktGroup& group);

    static void WriteObjects(NetPktGroup& group);

    static PAManager* sHostInstance;
    static PAManager* sClientInstance;
    static PAManager* sInstance;
};
 

extern PAManagerInterpolator pamanagerInterpolator;
extern unsigned char sNextAvailableKey;
extern unsigned char sNextAvailableAnim;
extern PABaseKey sPAKeys[255];
extern PAAnimationData sPAAnims[255];
