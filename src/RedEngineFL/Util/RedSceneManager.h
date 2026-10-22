#pragma once

#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblHeap.h"

class PblFileChunk;
class RedCamera;
class RedSceneObject;

class RedSceneAABox
{
public:

    RedSceneAABox() : mObjects(0), mChild(0), mSibling(0), mParent(0) {}

    void Add(RedSceneObject* pObject);
    void AddChild(RedSceneAABox* pBox);
    void AddSibling(RedSceneAABox* pBox);

    void Cleanup() { mChild = 0; mSibling = 0; mObjects = 0; }
    bool IsInside(RedSceneObject* pObject);
    void Move(RedSceneObject* pObject);
    void Remove(RedSceneObject* pObject);
    void Render(unsigned int uiFlags, int iClip);
    void RenderBox(int iDepth, bool bSiblings);
    bool SetupBox();
    float SetupRenderOrder(PblHeap<RedSceneAABox*, float>& heap);

    PblVector3 mMidPos;
    PblVector3 mSize;
    RedSceneObject* mObjects;
    RedSceneAABox* mChild;
    RedSceneAABox* mSibling;
    RedSceneAABox* mParent;
};

class RedScene
{
public:
    RedScene();
    ~RedScene();

    void Add(RedSceneObject* pObject);

    void Cleanup() { mObject = 0; mStaticWorldBox.Cleanup(); }
    void Remove(RedSceneObject* pObject);
    void Render(unsigned int uiFlags) const;
    void SetupStaticWorld();

    RedSceneObject* mObject;
    RedSceneAABox mStaticWorldBox;
};

class RedSceneCamera
{
public:
    RedSceneCamera() {}

    int ClassifyAABB(unsigned int uiFlags, const PblVector3& vPos,
                     const PblVector3& vSize, int iMask);
    int ClassifySphere(const PblVector3& vPos, float fRadius, int iMask);
    float GetDistance(const PblVector3& vPos, float fRadius);
    void Setup(const RedCamera* pCamera, float fNear, float fFar);

    PblVector3 mPosition;
    PblVector3 mPlane[4];
    int mMinIndex[4];
    int mMaxIndex[4];
    PblVector3 mFront;
    float mFarPlane;
    float mNearPlane;
    int mFrontMinIndex;
    int mFrontMaxIndex;
};

 
 
 

// VTABLE: SWBF2 0x007837b0
class RedSceneManager
{
public:
    RedSceneManager();
    virtual ~RedSceneManager();

    static RedSceneManager& GetInstance() { return *sInstance; }
    static void Init() { GetInstance()._Init(); }

    static void Cleanup() { GetInstance()._Cleanup(); }

    static void SetupStaticWorld() { GetInstance()._SetupStaticWorld(); }

    static void EnterClient() { sInstance->_EnterClient(); }
    static void LeaveClient() { sInstance->_LeaveClient(); }

    static void Read(PblFileChunk& chunk) { sInstance->Read_(chunk); }

    void Read_(PblFileChunk& chunk);

    static void Add(RedSceneObject* pObject) { sInstance->_Add(pObject); }
    static void Remove(RedSceneObject* pObject) { sInstance->_Remove(pObject); }
    static void Update(RedSceneObject* pObject) { sInstance->_Update(pObject); }

    static void AddTransparentObject(RedSceneObject* pObject);

    static bool InTransparentPhase() { return sInstance->mTransparentPhase; }

    static void AddTransparentObject(RedSceneObject* pObject, float fDist)
    {
        sInstance->_AddTransparentObject(pObject, fDist);
    }

    void Update(const RedCamera& camera);

    static void Render(unsigned int uiFlags, const RedCamera* pCamera)
    {
        sInstance->_Render(uiFlags, pCamera);
    }

    static RedSceneManager* sInstance;

protected:
    virtual void BeginSceneRender();
    virtual void EndSceneRender();

    void _Init();
    void _EnterClient();
    void _LeaveClient();
    void _Add(RedSceneObject* pObject);
    void _Remove(RedSceneObject* pObject);
    void _Update(RedSceneObject* pObject);
    void _AddTransparentObject(RedSceneObject* pObject, float fDist);
    void _Cleanup();
    void _Render(unsigned int uiFlags, const RedCamera* pCamera);
    void _RenderTransparentObjects(unsigned int uiFlags);
    void _SetupStaticWorld();

    int mState;
    RedScene mScene;
    bool mDoScene;
    bool mTransparentPhase;
    void* mTransparentObjects;
};

 
