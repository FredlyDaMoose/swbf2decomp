#include <math.h>

#include "CollisionManager.h"
#include "CameraManager.h"
#include "CollisionModel.h"
#include "CollisionObject.h"
#include "EntityMine.h"
#include "EntityTerrain.h"
#include "GameObject.h"
#include "NetGame.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Util/RedStack.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "Restrictor.h"

#include <d3dx9math.h>
#include "TreeGridCache.h"

class EntityWater : public CollisionObject
{
public:
    virtual ~EntityWater();
    virtual float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, PblVector3* pNormal,
                         int* pBodyId, bool bOrdnance);
};

class Ordnance : public Thread
{
public:
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual const PblVector3& GetPosition() const;
    virtual const PblVector3& GetOldPosition() const;
    virtual const PblVector3& GetVelocity() const;
    virtual void KillOrdnance();
    virtual bool CheckCollision();

    char mOrdnanceFields[0x78 - 0x18];
    PblList<Ordnance>::Node mCollisionManagerNode;
};

namespace TeamManager
{
void AddObject(GameObject* pObject);
void RemoveObject(GameObject* pObject);
}

class CollisionObjectCallbackData : public TreeGridCallbackData
{
public:
    void SetObjectTypeFilter(int iFilter) { mData[0] = iFilter; }
    int GetObjectTypeFilter() const { return mData[0]; }
};

struct CollectData : public CollisionObjectCallbackData
{
    PblVector3 mMinPos;
    PblVector3 mMaxPos;
    PblVector3 mCenter;
    float mRadius;
    CollisionObject* mMe;
    CollisionObject* mArray[100];
    int mLength;
};
 

struct GetObjectsRectData : public CollisionObjectCallbackData
{
    GameObject** mArray;
    int mMaxLength;
    int mLength;
    PblVector3 mMinPos;
    PblVector3 mMaxPos;
    GameObject* mMe;
};
 

struct GetObjectsRangeData : public CollisionObjectCallbackData
{
    GameObject** mArray;
    int mMaxLength;
    int mLength;
    PblVector3 mCenter;
    float mRadius;
    GameObject* mMe;
};
 

struct GetObjectsRegionData : public CollisionObjectCallbackData
{
    GameObject** mArray;
    int mMaxLength;
    int mLength;
    RedRegion* mRegion;
    GameObject* mMe;
};
 

struct RayData : public CollisionObjectCallbackData
{
    CollisionObject* mFirstHit;
    GameObject** mIgnore;
    int mIgnoreCount;
    PblVector3 mStartPos;
    PblVector3 mDirection;
    float mLength;
    PblVector3* mNormal;
    bool mRayTestFlag;
    int* mBodyID;
    bool mOrdnanceCollision;
};
 

static inline float Max(float a, float b)
{
    return a > b ? a : b;
}

// FUNCTION: SWBF2 0x0045db70
PblVector3 Min(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(b.x > a.x ? a.x : b.x, b.y > a.y ? a.y : b.y, b.z > a.z ? a.z : b.z);
}

// FUNCTION: SWBF2 0x0045dbc0
PblVector3 Max(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y, a.z > b.z ? a.z : b.z);
}

static const int kDefaultRayFilter = COLL_SOFT | COLL_ASTEROID | COLL_RIGID | COLL_STATIC | COLL_ATTACHED |
                                     COLL_TERRAIN;

static const int kDefaultObjectFilter = COLL_SOFT | COLL_ASTEROID | COLL_RIGID | COLL_STATIC;

// GLOBAL: SWBF2 0x01e3036c
CollisionManager* CollisionManager::sInstance;
// GLOBAL: SWBF2 0x01e30364
CollisionManager* CollisionManager::sHostInstance;
// GLOBAL: SWBF2 0x01e30360
CollisionManager* CollisionManager::sClientInstance;

extern PblVector3 sMinPos;
extern PblVector3 sMaxPos;

// FUNCTION: SWBF2 0x0045dd00
bool CollisionManager::CreateClient()
{
    if (NetGame::IsOneWorld())
    {
        sClientInstance = 0;
        sHostInstance->DeactivateThread();
        sHostInstance->mOrdnanceUpdate.DeactivateThread();
        sHostInstance->ActivateThread(-10, PER_TURN);
        sHostInstance->mOrdnanceUpdate.ActivateThread(-60, PER_LOCAL_TURN);
    }
    else
    {
        sClientInstance = new CollisionManager(sMinPos, sMaxPos);
        sClientInstance->mTerrain = sHostInstance->mTerrain;
    }
    return true;
}

// FUNCTION: SWBF2 0x0045dde0
bool CollisionManager::DestroyClient()
{
    if (sClientInstance)
    {
        delete sClientInstance;
        sClientInstance = 0;
    }
    return true;
}

// FUNCTION: SWBF2 0x0045de00
void CollisionManager::EnterClient()
{
    if (!NetGame::IsOneWorld())
    {
        sInstance = sClientInstance;
    }
}

// FUNCTION: SWBF2 0x0045de20
void CollisionManager::LeaveClient()
{
    if (!NetGame::IsOneWorld())
    {
        sInstance = sHostInstance;
    }
}

// GLOBAL: SWBF2 0x01e3035c
static PblList<CollisionObject>::Itor* sSafetyIt;
// GLOBAL: SWBF2 0x01e30358
static CollisionObject** sSafetyObjPointer;
// GLOBAL: SWBF2 0x01e30368
static CollisionObject** sSafetyColliderPointer;

static inline PblList<CollisionObject>::Node* CollisionNode(CollisionObject* pObject)
{

    return (PblList<CollisionObject>::Node*)pObject->mCollisionObjectNode;
}

// FUNCTION: SWBF2 0x0045db30
void PerformSafeRemove(CollisionObject* pObject)
{
    if (sSafetyColliderPointer && *sSafetyColliderPointer == pObject)
    {
        *sSafetyColliderPointer = 0;
    }
    if (sSafetyObjPointer && *sSafetyObjPointer == pObject)
    {
        *sSafetyObjPointer = 0;
    }
    if (sSafetyIt && sSafetyIt->GetNode()->_pObject == pObject)
    {
        ++*sSafetyIt;
    }
}

// FUNCTION: SWBF2 0x004605b0
bool CollisionManager::I_AddBody(CollisionObject* pObject, PblList<CollisionObject>& list)
{
    PblList<CollisionObject>::Node* pNode = CollisionNode(pObject);
    if (pNode->IsInList())
    {
        return false;
    }

    list.Add(pNode, pObject);
    if (mTreeGrid)
    {
        mTreeGrid->Add(pObject, pObject->GetCollisionCenter(), pObject->GetCollisionRadius() * 2.0f,
                       pObject->m2dRadius * 2.0f);
    }

    GameObject* pGameObject = pObject->GetGameObject();
    if (pGameObject)
    {
        TeamManager::AddObject(pGameObject);
    }
    return true;
}

// FUNCTION: SWBF2 0x00460670
bool CollisionManager::I_AddOrdnance(Ordnance* pOrdnance)
{
    if (pOrdnance->mCollisionManagerNode.IsInList())
    {
        return false;
    }
    mOrdnance.Add(&pOrdnance->mCollisionManagerNode, pOrdnance);
    return true;
}

// FUNCTION: SWBF2 0x00460850
bool CollisionManager::I_RemoveOrdnance(Ordnance* pOrdnance)
{
    if (!pOrdnance->mCollisionManagerNode.IsInList())
    {
        return false;
    }
    mOrdnance.Remove(&pOrdnance->mCollisionManagerNode);
    return true;
}

// FUNCTION: SWBF2 0x004606c0
bool CollisionManager::I_RemoveBody(CollisionObject* pObject)
{
    PerformSafeRemove(pObject);

    PblList<CollisionObject>::Node* pNode = CollisionNode(pObject);
    if (!pNode->IsInList())
    {
        if (pObject->GetCollisionType() == COLL_ITEM)
        {
            if (mTreeGrid && pObject->mTreeGridCellIndex >= 0)
            {
                mTreeGrid->Remove(pObject);
            }
            GameObject* pGameObject = pObject->GetGameObject();
            if (pGameObject)
            {
                TeamManager::RemoveObject(pGameObject);
            }
        }
        return false;
    }

    if (mTreeGrid)
    {
        mTreeGrid->Remove(pObject);
    }
    GameObject* pGameObject = pObject->GetGameObject();
    if (pGameObject)
    {
        TeamManager::RemoveObject(pGameObject);
    }

    switch (pObject->GetCollisionType())
    {
    case COLL_UNKNOWN:
        return false;
    case COLL_ITEM:
        mItemBodies.Remove(pNode);
        break;
    case COLL_SOFT:
        mSoftBodies.Remove(pNode);
        break;
    case COLL_ASTEROID:
        mAsteroidBodies.Remove(pNode);
        break;
    case COLL_RIGID:
        mRigidBodies.Remove(pNode);
        break;
    case COLL_STATIC:
        mStaticBodies.Remove(pNode);
        break;
    case COLL_ATTACHED:
        mAttachedBodies.Remove(pNode);
        break;
    case COLL_LEAFPATCH:
        mLeafPatchBodies.Remove(pNode);
        break;
    case COLL_SHIELD:
        mShieldBodies.Remove(pNode);
        break;
    }
    return true;
}

// FUNCTION: SWBF2 0x0045dcc0
bool CollisionManager::Destroy()
{
    if (!sInstance)
    {
        return false;
    }
    delete sHostInstance;
    sHostInstance = 0;
    sInstance = 0;
    return true;
}

// FUNCTION: SWBF2 0x0045de40
bool CollisionManager::AddItemBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mItemBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_ITEM);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045dea0
bool CollisionManager::AddSoftBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mSoftBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_SOFT);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045df00
bool CollisionManager::AddRigidBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mRigidBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_RIGID);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045df60
bool CollisionManager::AddAsteroidBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mAsteroidBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_ASTEROID);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045dfc0
bool CollisionManager::AddStaticBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mStaticBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_STATIC);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e020
bool CollisionManager::AddAttachedBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mAttachedBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_ATTACHED);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e080
bool CollisionManager::AddLeafPatchBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mLeafPatchBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_LEAFPATCH);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e0e0
bool CollisionManager::AddShieldBody(CollisionObject* pObject)
{
    if (sInstance && sInstance->I_AddBody(pObject, sInstance->mShieldBodies))
    {
        pObject->mLastPosition = pObject->GetPosition();
        pObject->SetCollisionType(COLL_SHIELD);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e140
bool CollisionManager::AddOrdnance(Ordnance* pOrdnance)
{
    if (sInstance)
    {
        return sInstance->I_AddOrdnance(pOrdnance);
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e160
bool CollisionManager::RemoveBody(CollisionObject* pObject)
{
    if (sInstance)
    {
        return sInstance->I_RemoveBody(pObject);
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e180
bool CollisionManager::RemoveOrdnance(Ordnance* pOrdnance)
{
    if (sInstance)
    {
        return sInstance->I_RemoveOrdnance(pOrdnance);
    }
    return false;
}

// FUNCTION: SWBF2 0x0045e1a0
bool CollisionManager::SetTerrain(EntityTerrain* pTerrain)
{
    if (!sInstance)
    {
        return false;
    }
    sInstance->mTerrain = pTerrain;
    return true;
}

// FUNCTION: SWBF2 0x0045e1c0
bool CollisionManager::SetWater(EntityWater* pWater)
{
    if (!sInstance)
    {
        return false;
    }
    sInstance->mWater = pWater;
    return true;
}

// FUNCTION: SWBF2 0x0045e1e0
int CollisionManager::GetObjectsInRect(const PblVector3& vMin, const PblVector3& vMax, GameObject** const pObjects,
                                       int iMaxCount, const GameObject* pIgnore, int iFilter)
{
    if (sInstance)
    {
        return sInstance->I_GetObjectsInRect(vMin, vMax, pObjects, iMaxCount, pIgnore, iFilter);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0045e210
int CollisionManager::GetObjectsInRange(const PblVector3& vCenter, float fRadius, GameObject** const pObjects,
                                        int iMaxCount, const GameObject* pIgnore, int iFilter)
{
    if (sInstance)
    {
        return sInstance->I_GetObjectsInRange(vCenter, fRadius, pObjects, iMaxCount, pIgnore,
                                              iFilter ? iFilter : kDefaultObjectFilter);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0045e250
int CollisionManager::GetObjectsInFatRay(const PblVector3& vFrom, const PblVector3& vTo, float fRadius,
                                         GameObject** const pObjects, int iMaxCount, const GameObject* pIgnore,
                                         int iFilter)
{
    if (sInstance)
    {
        return sInstance->I_GetObjectsInFatRay(vFrom, vTo, fRadius, pObjects, iMaxCount, pIgnore, iFilter);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0045e280
int CollisionManager::GetObjectsInRegion(const RedRegion& rRegion, GameObject** const pObjects, int iMaxCount,
                                         const GameObject* pIgnore, int iFilter)
{
    if (sInstance)
    {
        return sInstance->I_GetObjectsInRegion(rRegion, pObjects, iMaxCount, pIgnore,
                                               iFilter ? iFilter : kDefaultObjectFilter);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0045e2b0
float CollisionManager::RayHit(const PblVector3& vStart, const PblVector3& vEnd, CollisionObject*& pHitObject,
                               PblVector3* pNormal, const GameObject** const pIgnore, int iIgnoreCount,
                               int iFilter, bool bOrdnance)
{
    if (sInstance)
    {
        PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
        float fLength = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
        if (fLength != 0.0f)
        {
            float fInvLength = 1.0f / fLength;
            vDir.x *= fInvLength;
            vDir.y *= fInvLength;
            vDir.z *= fInvLength;
            return RayHit(vStart, vDir, fLength, pHitObject, pNormal, pIgnore, iIgnoreCount, iFilter, bOrdnance);
        }
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x0045e3a0
float CollisionManager::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                               CollisionObject*& pHitObject, PblVector3* pNormal, const GameObject** const pIgnore,
                               int iIgnoreCount, int iFilter, bool bOrdnance)
{
    pHitObject = NULL;
    if (fLength != 0.0f && sInstance)
    {
        return sInstance->I_RayHit(vStart, vDir, fLength, pHitObject, pNormal, pIgnore, iIgnoreCount,
                                   iFilter ? iFilter : kDefaultRayFilter, &sInstance->mLastRayHitBodyId,
                                   bOrdnance);
    }
    return 1.0f;
}

// FUNCTION: SWBF2 0x0045e400
bool CollisionManager::RayTest(const PblVector3& vStart, const PblVector3& vEnd, const GameObject** const pIgnore,
                               int iIgnoreCount, int iFilter)
{
    if (!sInstance)
    {
        return false;
    }

    PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    float fLength = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
    if (fLength == 0.0f)
    {
        return false;
    }

    float fInvLength = 1.0f / fLength;
    vDir.x *= fInvLength;
    vDir.y *= fInvLength;
    vDir.z *= fInvLength;
    return sInstance->I_RayTest(vStart, vDir, fLength, pIgnore, iIgnoreCount,
                                iFilter ? iFilter : kDefaultRayFilter);
}

// FUNCTION: SWBF2 0x0045e4e0
bool CollisionManager::RayTest(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                               const GameObject** const pIgnore, int iIgnoreCount, int iFilter)
{
    if (!sInstance)
    {
        return false;
    }
    return sInstance->I_RayTest(vStart, vDir, fLength, pIgnore, iIgnoreCount,
                                iFilter ? iFilter : kDefaultRayFilter);
}

// FUNCTION: SWBF2 0x0045e520
CollisionManager::CollisionManager(const PblVector3& vMin, const PblVector3& vMax)
    : mOrdnanceUpdate(this)
{
    float fSize = Max(vMax.x - vMin.x, vMax.z - vMin.z);
    mTreeGrid = new TreeGrid(vMin, vMax, fSize / 64.0f * 0.999f);
    ActivateThread(-10, PER_TURN);
    mOrdnanceUpdate.ActivateThread(-60, PER_LOCAL_TURN);
    mUpdateRestrictedCounter = 0;
    mWater = NULL;
    mMaxCollisionDistanceSq = 3600.0f;
}

// SYNTHETIC: SWBF2 0x0045e710
// CollisionManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0045e740
CollisionManager::OrdnanceUpdate::~OrdnanceUpdate()
{
}

// TEMPLATE: SWBF2 0x00460960
// PblList<CollisionObject>::RemoveAll

// TEMPLATE: SWBF2 0x004609a0
// PblList<Ordnance>::RemoveAll

// FUNCTION: SWBF2 0x0045e750
CollisionManager::~CollisionManager()
{
    mOrdnanceUpdate.DeactivateThread();
    DeactivateThread();
    delete mTreeGrid;
    mOrdnance.RemoveAll();
    mItemBodies.RemoveAll();
    mSoftBodies.RemoveAll();
    mRigidBodies.RemoveAll();
    mAsteroidBodies.RemoveAll();
    mStaticBodies.RemoveAll();
    mAttachedBodies.RemoveAll();
    mLeafPatchBodies.RemoveAll();
    mShieldBodies.RemoveAll();
}

// FUNCTION: SWBF2 0x0045e830
void CollisionManager::BeginList(PblList<CollisionObject>::Itor& itor)
{
    itor = PblList<CollisionObject>::Itor(sInstance->mStaticBodies);
}

// FUNCTION: SWBF2 0x0045e840
void CollisionManager::ListObjects(PblList<CollisionObject>::Itor& itor, CollisionObject** const pObjects,
                                   int& iCount)
{
    int iMaxCount = iCount;
    iCount = 0;
    while (iCount < iMaxCount)
    {
        CollisionObject* pObject = itor;
        if (pObject)
        {
            pObjects[iCount++] = pObject;
            ++itor;
        }
        else
        {
            PblList<CollisionObject>* pList = itor.GetNode()->GetList();
            if (pList == &sInstance->mStaticBodies)
            {
                itor = PblList<CollisionObject>::Itor(sInstance->mRigidBodies);
            }
            else if (pList == &sInstance->mRigidBodies)
            {
                itor = PblList<CollisionObject>::Itor(sInstance->mAsteroidBodies);
            }
            else
            {
                return;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0045e8b0
bool CollisionManager::CollectDataCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                           float& fDist)
{
    CollisionObject* pObject = (CollisionObject*)pTreeObject;
    CollectData* pCollect = (CollectData*)pData;

    if (pObject == pCollect->mMe || pCollect->mMinPos.x > pObject->mAABB._vMax.x ||
        pCollect->mMinPos.z > pObject->mAABB._vMax.z || pObject->mAABB._vMin.x > pCollect->mMaxPos.x ||
        pObject->mAABB._vMin.z > pCollect->mMaxPos.z || pCollect->mMinPos.y > pObject->mAABB._vMax.y ||
        pObject->mAABB._vMin.y > pCollect->mMaxPos.y)
    {
        return true;
    }

    pCollect->mArray[pCollect->mLength++] = pObject;
    return pCollect->mLength != 100;
}

// FUNCTION: SWBF2 0x0045e930
bool CollisionManager::GetObjectsRectCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                              float& fDist)
{
    CollisionObject* pObject = (CollisionObject*)pTreeObject;
    if (!pObject)
    {
        return true;
    }

    GetObjectsRectData* pRect = (GetObjectsRectData*)pData;
    GameObject* pGameObject = pObject->GetGameObject();
    if (pRect->GetObjectTypeFilter() != COLL_LEAFPATCH && (!pGameObject || pGameObject == pRect->mMe))
    {
        return true;
    }

    pRect->mArray[pRect->mLength++] = pGameObject;
    return pRect->mLength != pRect->mMaxLength;
}

// FUNCTION: SWBF2 0x0045e980
bool CollisionManager::GetObjectsRayCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                             float& fDist)
{
    CollisionObject* pObject = (CollisionObject*)pTreeObject;
    if (!pObject)
    {
        return true;
    }

    GetObjectsRectData* pRay = (GetObjectsRectData*)pData;
    GameObject* pGameObject = pObject->GetGameObject();
    if (pRay->GetObjectTypeFilter() != COLL_LEAFPATCH && (!pGameObject || pGameObject == pRay->mMe))
    {
        return true;
    }

    pRay->mArray[pRay->mLength++] = pGameObject;
    return pRay->mLength != pRay->mMaxLength;
}

// FUNCTION: SWBF2 0x0045e9d0
bool CollisionManager::I_GetObjectsFromList(const PblVector3& vMin, const PblVector3& vMax,
                                            TreeGridCallback pCallback, TreeGridCallbackData* pData,
                                            PblList<CollisionObject>& list)
{
    float fDist = 1.0f;
    PblList<CollisionObject>::Itor it(list);
    while (CollisionObject* pObject = it)
    {
        ++it;
        PblVector3 vCenter = pObject->GetCollisionCenter();
        float fRadius = pObject->GetCollisionRadius();

        int i;
        for (i = 0; i < 3; ++i)
        {
            if ((&vCenter.x)[i] - fRadius > (&vMax.x)[i] || (&vMin.x)[i] > (&vCenter.x)[i] + fRadius)
            {
                break;
            }
        }
        if (i >= 3 && !pCallback(pObject, pData, fDist))
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0045eab0
void CollisionManager::I_GetObjectsFromLists(const PblVector3& vMin, const PblVector3& vMax,
                                             TreeGridCallback pCallback, int iFilter, TreeGridCallbackData* pData)
{
    if ((iFilter & COLL_ITEM) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mItemBodies))
    {
        return;
    }
    if ((iFilter & COLL_SOFT) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mSoftBodies))
    {
        return;
    }
    if ((iFilter & COLL_RIGID) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mRigidBodies))
    {
        return;
    }
    if ((iFilter & COLL_ASTEROID) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mAsteroidBodies))
    {
        return;
    }
    if ((iFilter & COLL_STATIC) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mStaticBodies))
    {
        return;
    }
    if ((iFilter & COLL_ATTACHED) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mAttachedBodies))
    {
        return;
    }
    if ((iFilter & COLL_LEAFPATCH) && !I_GetObjectsFromList(vMin, vMax, pCallback, pData, mLeafPatchBodies))
    {
        return;
    }
    if (iFilter & COLL_SHIELD)
    {
        I_GetObjectsFromList(vMin, vMax, pCallback, pData, mShieldBodies);
    }
}

// FUNCTION: SWBF2 0x0045ebc0
int CollisionManager::I_GetObjectsInRect(const PblVector3& vMin, const PblVector3& vMax, GameObject** const pObjects,
                                         int iMaxCount, const GameObject* pIgnore, int iFilter)
{
    GetObjectsRectData data;
    data.SetObjectTypeFilter(iFilter);
    data.mArray = pObjects;
    data.mMaxLength = iMaxCount;
    data.mLength = 0;
    data.mMinPos = vMin;
    data.mMaxPos = vMax;
    data.mMe = (GameObject*)pIgnore;

    if (mTreeGrid)
    {
        mTreeGrid->CallbackOnRect(vMin, vMax, GetObjectsRectCallback, &data);
    }
    else
    {
        I_GetObjectsFromLists(vMin, vMax, GetObjectsRectCallback, iFilter, &data);
    }
    return data.mLength;
}

// FUNCTION: SWBF2 0x0045ec60
bool CollisionManager::GetObjectsRangeCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                               float& fDist)
{
    CollisionObject* pObject = (CollisionObject*)pTreeObject;
    if (!pObject)
    {
        return true;
    }

    GetObjectsRangeData* pRange = (GetObjectsRangeData*)pData;
    GameObject* pGameObject = pObject->GetGameObject();
    if (pRange->GetObjectTypeFilter() != COLL_LEAFPATCH && (!pGameObject || pGameObject == pRange->mMe))
    {
        return true;
    }

    PblVector3 vCenter = pObject->GetCollisionCenter();
    float fRadius = pObject->GetCollisionRadius();
    float fDX = pRange->mCenter.x - vCenter.x;
    float fDZ = pRange->mCenter.z - vCenter.z;
    float fReach = pRange->mRadius + fRadius;
    if (fDX * fDX + fDZ * fDZ > fReach * fReach)
    {
        return true;
    }

    pRange->mArray[pRange->mLength++] = pGameObject;
    return pRange->mLength != pRange->mMaxLength;
}

// FUNCTION: SWBF2 0x0045ed40
int CollisionManager::I_GetObjectsInRange(const PblVector3& vCenter, float fRadius, GameObject** const pObjects,
                                          int iMaxCount, const GameObject* pIgnore, int iFilter)
{
    GetObjectsRangeData data;
    data.SetObjectTypeFilter(iFilter);
    data.mArray = pObjects;
    data.mMaxLength = iMaxCount;
    data.mLength = 0;
    data.mCenter = vCenter;
    data.mRadius = fRadius;
    data.mMe = (GameObject*)pIgnore;

    PblVector3 vMin(vCenter.x - fRadius, vCenter.y - fRadius, vCenter.z - fRadius);
    PblVector3 vMax(vCenter.x + fRadius, vCenter.y + fRadius, vCenter.z + fRadius);
    if (mTreeGrid)
    {
        mTreeGrid->CallbackOnRect(vMin, vMax, GetObjectsRangeCallback, &data);
    }
    else
    {
        I_GetObjectsFromLists(vMin, vMax, GetObjectsRangeCallback, iFilter, &data);
    }
    return data.mLength;
}

// FUNCTION: SWBF2 0x0045ee60
bool CollisionManager::GetObjectsRegionCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                                float& fDist)
{
    CollisionObject* pObject = (CollisionObject*)pTreeObject;
    if (!pObject)
    {
        return true;
    }

    GetObjectsRegionData* pRegion = (GetObjectsRegionData*)pData;
    GameObject* pGameObject = pObject->GetGameObject();
    if (pRegion->GetObjectTypeFilter() != COLL_LEAFPATCH && (!pGameObject || pGameObject == pRegion->mMe))
    {
        return true;
    }

    PblVector3 vCenter = pObject->GetCollisionCenter();
    if (!pRegion->mRegion->DoesSphereIntersect(vCenter, pObject->GetCollisionRadius()))
    {
        return true;
    }

    pRegion->mArray[pRegion->mLength++] = pGameObject;
    return pRegion->mLength != pRegion->mMaxLength;
}

// FUNCTION: SWBF2 0x0045ef20
int CollisionManager::I_GetObjectsInRegion(const RedRegion& rRegion, GameObject** const pObjects, int iMaxCount,
                                           const GameObject* pIgnore, int iFilter)
{
    GetObjectsRegionData data;
    data.SetObjectTypeFilter(iFilter);
    data.mArray = pObjects;
    data.mMaxLength = iMaxCount;
    data.mLength = 0;
    data.mRegion = (RedRegion*)&rRegion;
    data.mMe = (GameObject*)pIgnore;

    const PblVector3& vPos = *(const PblVector3*)rRegion.mMatrix.m[3];
    float fRadius = rRegion.mSize.w;
    PblVector3 vMin(vPos.x - fRadius, vPos.y - fRadius, vPos.z - fRadius);
    PblVector3 vMax(vPos.x + fRadius, vPos.y + fRadius, vPos.z + fRadius);
    if (mTreeGrid)
    {
        mTreeGrid->CallbackOnRect(vMin, vMax, GetObjectsRegionCallback, &data);
    }
    else
    {
        I_GetObjectsFromLists(vMin, vMax, GetObjectsRectCallback, iFilter, &data);
    }
    return data.mLength;
}

// FUNCTION: SWBF2 0x0045f010
int CollisionManager::I_GetObjectsInFatRay(const PblVector3& vFrom, const PblVector3& vTo, float fRadius,
                                           GameObject** const pObjects, int iMaxCount, const GameObject* pIgnore,
                                           int iFilter)
{
    GetObjectsRectData data;
    data.SetObjectTypeFilter(iFilter);
    data.mArray = pObjects;
    data.mMaxLength = iMaxCount;
    data.mLength = 0;
    data.mMe = (GameObject*)pIgnore;

    if (!mTreeGrid)
    {
        return 0;
    }
    mTreeGrid->CallbackOnFatRay(vFrom, vTo, fRadius, GetObjectsRayCallback, &data);
    return data.mLength;
}

// FUNCTION: SWBF2 0x0045f070
bool CollisionManager::RayCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData, float& fDist)
{
    CollisionObject* pObject = (CollisionObject*)pTreeObject;
    if (!pObject)
    {
        return true;
    }

    RayData* pRay = (RayData*)pData;
    GameObject* pGameObject = pObject->GetGameObject();
    for (int i = 0; i < pRay->mIgnoreCount; ++i)
    {
        if (pGameObject && pGameObject == pRay->mIgnore[i])
        {
            return true;
        }
    }

    float fHit = pObject->RayHit(pRay->mStartPos, pRay->mDirection, pRay->mLength, pRay->mNormal, pRay->mBodyID,
                                 pRay->mOrdnanceCollision, 0.0f);
    if (fHit < 1.0f)
    {
        if (pRay->mRayTestFlag)
        {
            pRay->mLength = 0.0f;
            return false;
        }
        pRay->mFirstHit = pObject;
        pRay->mLength *= fHit;
        fDist = pRay->mLength;
    }
    return true;
}

// FUNCTION: SWBF2 0x0045f130
float CollisionManager::I_RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                 CollisionObject*& pHitObject, PblVector3* pNormal, const GameObject** const pIgnore,
                                 int iIgnoreCount, int iFilter, int* pBodyId, bool bOrdnance)
{
    RayData data;
    data.SetObjectTypeFilter(iFilter);
    data.mFirstHit = NULL;
    data.mIgnore = (GameObject**)pIgnore;
    data.mIgnoreCount = iIgnoreCount;
    data.mStartPos = vStart;
    data.mDirection = vDir;
    data.mLength = fLength;
    data.mNormal = pNormal;
    data.mRayTestFlag = false;
    data.mBodyID = pBodyId;
    data.mOrdnanceCollision = bOrdnance;

    if ((iFilter & COLL_TERRAIN) && mTerrain)
    {
        float fHit = mTerrain->RayHit(vStart, vDir, fLength, pNormal, pBodyId, false);
        if (fHit < 1.0f)
        {
            data.mLength = fHit * data.mLength;
            data.mFirstHit = mTerrain;
        }
    }
    if ((iFilter & COLL_WATER) && mWater)
    {
        float fHit = mWater->RayHit(vStart, vDir, data.mLength, pNormal, pBodyId, false);
        if (fHit < 1.0f)
        {
            data.mLength = fHit * data.mLength;
            data.mFirstHit = mWater;
        }
    }

    PblVector3 vEnd(vStart.x + vDir.x * data.mLength, vStart.y + vDir.y * data.mLength,
                    vStart.z + vDir.z * data.mLength);
    PblVector3 vMin = Min(vStart, vEnd);
    PblVector3 vMax = Max(vStart, vEnd);
    if (mTreeGrid)
    {
        mTreeGrid->CallbackOnLazyRay(vStart, vEnd, RayCallback, &data);
    }
    else
    {
        I_GetObjectsFromLists(vMin, vMax, RayCallback, iFilter, &data);
    }

    pHitObject = data.mFirstHit;
    return data.mLength / fLength;
}

// FUNCTION: SWBF2 0x0045f310
bool CollisionManager::I_RayTest(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                                 const GameObject** const pIgnore, int iIgnoreCount, int iFilter)
{
    if ((iFilter & COLL_TERRAIN) && mTerrain->RayTest(vStart, vDir, fLength))
    {
        return true;
    }

    RayData data;
    data.SetObjectTypeFilter(iFilter);
    data.mFirstHit = NULL;
    data.mIgnore = (GameObject**)pIgnore;
    data.mIgnoreCount = iIgnoreCount;
    data.mStartPos = vStart;
    data.mDirection = vDir;
    data.mLength = fLength;
    data.mNormal = NULL;
    data.mRayTestFlag = true;
    data.mBodyID = NULL;
    data.mOrdnanceCollision = true;

    PblVector3 vEnd(vStart.x + vDir.x * fLength, vStart.y + vDir.y * fLength, vStart.z + vDir.z * fLength);
    PblVector3 vMin = Min(vStart, vEnd);
    PblVector3 vMax = Max(vStart, vEnd);
    if (mTreeGrid)
    {
        mTreeGrid->CallbackOnRect(vMin, vMax, RayCallback, &data);
    }
    else
    {
        I_GetObjectsFromLists(vMin, vMax, RayCallback, iFilter, &data);
    }
    return data.mLength == 0.0f;
}

// FUNCTION: SWBF2 0x0045f600
void CollisionManager::UpdateSimple(PblList<CollisionObject>& list, int iFilter)
{
    PblVector3 vCameraPos[1];
    CollectData data;
    data.mLength = 0;

    int iNumCameras = CameraManager::Instance()->GetNumCameras();
    for (int i = 0; i < iNumCameras; ++i)
    {
        vCameraPos[i] = CameraManager::Instance()->GetCamera(i)->GetMatrix().GetTranslation();
    }

    bool bDistanceGated = !NetGame::IsEnabled() || NetGame::IsClient();
    bool bClient = NetGame::IsEnabled() && NetGame::IsClient();

    PblList<CollisionObject>::Itor it(list);
    while (CollisionObject* pObject = it)
    {
        bool bMine = pObject->GetGameObject() &&
                     pObject->GetGameObject()->IsRtti(EntityMine::rttiHashEntityMine);
        ++it;

        bool bNear = false;
        data.mMe = pObject;
        PblVector3 vCenter = pObject->GetCollisionCenter();
        for (int i = 0; i < iNumCameras && !bNear; ++i)
        {
            float fDx = vCameraPos[i].x - vCenter.x;
            float fDy = vCameraPos[i].y - vCenter.y;
            float fDz = vCameraPos[i].z - vCenter.z;
            bNear = mMaxCollisionDistanceSq > fDy * fDy + fDx * fDx + fDz * fDz;
        }

        if (bClient && bNear)
        {
            float fDeltaTime = 0.05f;
            if (NetGame::IsSkipped(pObject->GetGameObject(), fDeltaTime))
            {
                continue;
            }
        }
        else if (bDistanceGated && !bNear)
        {
            continue;
        }

        int iTypes = iFilter;
        if ((iFilter & COLL_SOFT) && pObject->mCollisionModel->GetCollisionMask(COLL_SOFT).IsEmpty())
        {
            iTypes &= ~COLL_SOFT;
        }
        if ((iFilter & COLL_RIGID) && pObject->mCollisionModel->GetCollisionMask(COLL_RIGID).IsEmpty())
        {
            iTypes &= ~COLL_RIGID;
        }
        if ((iFilter & COLL_ASTEROID) && pObject->mCollisionModel->GetCollisionMask(COLL_ASTEROID).IsEmpty())
        {
            iTypes &= ~COLL_ASTEROID;
        }
        if ((iFilter & COLL_STATIC) && pObject->mCollisionModel->GetCollisionMask(COLL_STATIC).IsEmpty())
        {
            iTypes &= ~COLL_STATIC;
        }

        data.SetObjectTypeFilter(iTypes);
        data.mCenter = vCenter;
        data.mRadius = pObject->GetCollisionRadius();
        data.mMinPos = pObject->mAABB._vMin;
        data.mMaxPos = pObject->mAABB._vMax;

        if (mTreeGrid)
        {
            mTreeGrid->CallbackOnRect(data.mMinPos, data.mMaxPos, CollectDataCallback, &data);
        }
        else
        {
            I_GetObjectsFromLists(data.mMinPos, data.mMaxPos, CollectDataCallback, iFilter, &data);
        }

        while (data.mLength > 0)
        {
            CollisionObject* pOther = data.mArray[--data.mLength];
            if (bMine || (int)pOther > (int)pObject)
            {
                pObject->CheckForCollision(pOther, NULL, NULL);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0065c810
void TreeGridCache::CallbackOnRect(TreeGrid* pTreeGrid, const PblVector3& rMin, const PblVector3& rMax, TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    pTreeGrid->CallbackOnRect(rMin, rMax, pCallback, pData);
}

// FUNCTION: SWBF2 0x0065c830
void TreeGridCache::CallbackOnLazyRay(TreeGrid* pTreeGrid, const PblVector3& rFrom, const PblVector3& rTo, TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    pTreeGrid->CallbackOnLazyRay(rFrom, rTo, pCallback, pData);
}

// FUNCTION: SWBF2 0x0065c850
void TreeGridCache::CallbackOnSphere(TreeGrid* pTreeGrid, const PblVector3& rCenter, float fRadius, TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    pTreeGrid->CallbackOnSphere(rCenter, fRadius, pCallback, pData);
}

struct MatrixStruct
{
    MatrixStruct() : objPtr(NULL), matrices(NULL) {}

    CollisionObject* objPtr;
    PblMatrix* matrices;
};
 

static inline const PblMatrix operator*(const PblMatrix& a, const PblMatrix& b)
{
    PblMatrix result;
    D3DXMatrixMultiply((D3DXMATRIX*)&result, (const D3DXMATRIX*)&a, (const D3DXMATRIX*)&b);
    return result;
}

static const float EPSILON = 0.0001f;

// FUNCTION: SWBF2 0x0045f970
void CollisionManager::UpdateRestricted(PblList<CollisionObject>& aList, int aTypeMask)
{
    const int MAX_OBJECTS_STORED = 300;
    const int maxStoredMatrices = 0;
    PblMatrix* const storedMatrices = NULL;

    MatrixStruct structInfo[MAX_OBJECTS_STORED];
    int structMap[100];
    int testFrame[100];
    PblVector3 camPos[1];

    CollectData data;
    data.SetObjectTypeFilter(aTypeMask);
    data.mLength = 0;
    Restrictor restrictor(10);

    int numStoredMatrices = 0;
    int numStoredObjects = 0;
    int numCams = CameraManager::Instance()->GetNumCameras();
    for (int i = 0; i < numCams; ++i)
    {
        camPos[i] = CameraManager::Instance()->GetCamera(i)->GetMatrix().GetTranslation();
    }

    bool bClient = !NetGame::IsEnabled() || NetGame::IsClient();
    bool bNetClient = NetGame::IsEnabled() && NetGame::IsClient();

    PblList<CollisionObject>::Itor it(aList);
    sSafetyIt = &it;
    unsigned char ctr = 0;
    while (CollisionObject* obj = it)
    {
        ++it;
        restrictor.Clear();
        sSafetyObjPointer = &obj;

        const PblVector3& vPosition = obj->GetPosition();
        const PblVector3& vLastPosition = obj->mLastPosition;
        PblVector3 diff(vPosition.x - vLastPosition.x, vPosition.y - vLastPosition.y,
                        vPosition.z - vLastPosition.z);
        float totalDist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        if (totalDist > 100.0f)
        {
            obj->mLastPosition = obj->GetPosition();
            continue;
        }

        float fInvDist = 1.0f / (totalDist + EPSILON);
        PblVector3 deltaMove(diff.x * fInvDist, diff.y * fInvDist, diff.z * fInvDist);
        float fRadius = obj->GetCollisionRadius();
        float fHalfDim = obj->GetSmallestCollisionDim() * 0.5f;
        float fStepSize = fRadius > fHalfDim ? fHalfDim : fRadius;
        int maxTests = (int)(totalDist / fStepSize + 1.0f);
        float fStepLength = totalDist / (float)maxTests;
        deltaMove.x *= fStepLength;
        deltaMove.y *= fStepLength;
        deltaMove.z *= fStepLength;

        bool bNear = false;
        PblVector3 pos = obj->GetCollisionCenter();
        for (int i = 0; i < numCams && !bNear; ++i)
        {
            float fDx = camPos[i].x - pos.x;
            float fDy = camPos[i].y - pos.y;
            float fDz = camPos[i].z - pos.z;
            bNear = mMaxCollisionDistanceSq > fDy * fDy + fDx * fDx + fDz * fDz;
        }

        bool bTest = false;
        if (maxTests > 1 || !bClient || ctr == mUpdateRestrictedCounter || obj->mLastCollidedWithNonTerrain || bNear)
        {
            bTest = true;
            if (bNetClient)
            {
                float aDt = 0.05f;
                if (NetGame::IsSkipped(obj->GetGameObject(), aDt))
                {
                    bTest = false;
                }
            }
        }
        obj->mLastCollidedWithNonTerrain = false;

        int currTest = 0;
        data.mLength = 0;
        CollisionOptimizationInfo optimizationInfo;
        RedStack stack;
        CollisionMask** ignoreMasks = NULL;
        if (bTest)
        {
            data.mMe = obj;
            const PblVector3& vCenter = obj->GetCollisionCenter();
            data.mCenter = PblVector3(vCenter.x - diff.x * 0.5f, vCenter.y - diff.y * 0.5f, vCenter.z - diff.z * 0.5f);
            data.mRadius = totalDist * 0.5f + obj->GetCollisionRadius();
            data.mMinPos = PblVector3(data.mCenter.x - data.mRadius, data.mCenter.y - data.mRadius,
                                      data.mCenter.z - data.mRadius);
            data.mMaxPos = PblVector3(data.mCenter.x + data.mRadius, data.mCenter.y + data.mRadius,
                                      data.mCenter.z + data.mRadius);
            if (mTreeGrid)
            {
                mTreeGrid->CallbackOnSphere(data.mCenter, data.mRadius, CollectDataCallback, &data);
            }
            else
            {
                I_GetObjectsFromLists(data.mMinPos, data.mMaxPos, CollectDataCallback, aTypeMask, &data);
            }

            for (int i = 0; i < 3; ++i)
            {
                if (i >= data.mLength)
                {
                    break;
                }
                for (int j = 0; j < data.mLength; ++j)
                {
                    if (data.mArray[j] == obj->mLastCollidedObjs[i] && j != i)
                    {
                        CollisionObject* pSwap = data.mArray[i];
                        data.mArray[i] = data.mArray[j];
                        data.mArray[j] = pSwap;
                        break;
                    }
                }
            }

            ignoreMasks = (CollisionMask**)stack.Alloc(data.mLength * sizeof(CollisionMask*));
            optimizationInfo.bodyMatrices =
                (PblMatrix*)stack.Alloc(obj->mCollisionModel->GetNumBodies() * sizeof(PblMatrix));

            for (int i = 0; i < data.mLength; ++i)
            {
                CollisionObject* pOther = data.mArray[i];
                testFrame[i] = 0;
                if (ignoreMasks)
                {
                    if (pOther && pOther->mCollisionModel && pOther->mCollisionModel->GetNumBodies() > 0)
                    {
                        ignoreMasks[i] = (CollisionMask*)stack.Alloc(pOther->mCollisionModel->GetNumBodies() *
                                                                     sizeof(CollisionMask));
                        for (int t = 0; t < pOther->mCollisionModel->GetNumBodies(); ++t)
                        {
                            ignoreMasks[i][t] = CollisionMask(0);
                        }
                    }
                    else
                    {
                        ignoreMasks[i] = NULL;
                    }
                }

                int mat = -1;
                for (int k = 0; k < numStoredObjects && mat == -1; ++k)
                {
                    if (structInfo[k].objPtr == pOther)
                    {
                        mat = k;
                    }
                }
                if (mat != -1)
                {
                    structMap[i] = mat;
                }
                else if (numStoredObjects < MAX_OBJECTS_STORED - 1 &&
                         numStoredMatrices + pOther->mCollisionModel->GetNumBodies() < maxStoredMatrices)
                {
                    CollisionModel* model = pOther->mCollisionModel;
                    structMap[i] = numStoredObjects;
                    structInfo[numStoredObjects].objPtr = pOther;
                    structInfo[numStoredObjects].matrices = storedMatrices + numStoredMatrices;
                    PblMatrix obj2Mat = pOther->GetMatrix();
                    for (int t = 0; t < model->GetNumBodies(); ++t)
                    {
                        structInfo[numStoredObjects].matrices[t] = model->GetBodyMatrix(t) * obj2Mat;
                    }
                    numStoredMatrices += pOther->mCollisionModel->GetNumBodies();
                    ++numStoredObjects;
                }
                else
                {
                    structMap[i] = -1;
                }
            }
        }

        bool collision = false;
        while (currTest < maxTests && obj)
        {
            ++currTest;
            const PblVector3& vLast = obj->mLastPosition;
            PblVector3 vStep(deltaMove.x + vLast.x, vLast.y + deltaMove.y, vLast.z + deltaMove.z);
            obj->SetPosition(vStep);
            for (int b = 0; b < obj->mCollisionModel->GetNumBodies(); ++b)
            {
                optimizationInfo.refreshMatrices[b] = true;
            }
            if (mTerrain->CheckForCollision(obj, &restrictor))
            {
                collision = true;
            }

            for (int i = 0; i < data.mLength && obj;)
            {
                CollisionObject* pOther = data.mArray[i];
                optimizationInfo.ignoreMasks = NULL;
                sSafetyColliderPointer = &data.mArray[i];
                if (ignoreMasks)
                {
                    optimizationInfo.ignoreMasks = ignoreMasks[i];
                }
                optimizationInfo.otherModelMatrices = NULL;
                int mat = structMap[i];
                if (mat > -1 && structInfo[mat].objPtr == pOther)
                {
                    optimizationInfo.otherModelMatrices = structInfo[mat].matrices;
                }

                if (pOther && testFrame[i] != currTest &&
                    obj->CheckForCollision(pOther, &restrictor, &optimizationInfo))
                {
                    testFrame[i] = currTest;
                    i = 0;
                    collision = true;
                    if (obj)
                    {
                        obj->mLastCollidedWithNonTerrain = true;
                    }
                }
                else
                {
                    ++i;
                }
            }

            if (obj)
            {
                obj->mLastPosition = obj->GetPosition();
            }
            if (collision)
            {
                restrictor.RestrictMovement(deltaMove);
            }
        }

        if (++ctr >= mMaxUpdateRestrictedGroups)
        {
            ctr = 0;
        }
    }

    sSafetyColliderPointer = NULL;
    sSafetyObjPointer = NULL;
    sSafetyIt = NULL;
}

// FUNCTION: SWBF2 0x00460500
void CollisionManager::UpdateTreeGrid(PblList<CollisionObject>& aList)
{
    PblList<CollisionObject>::Itor it(aList);
    while (CollisionObject* obj = it)
    {
        ++it;
        if (mTreeGrid)
        {
            mTreeGrid->Update(obj, obj->GetCollisionCenter(), obj->GetCollisionRadius() * 2.0f,
                              obj->m2dRadius * 2.0f);
        }
    }
}

// FUNCTION: SWBF2 0x0045f470
bool CollisionManager::Update(float fDeltaTime)
{
    mMaxUpdateRestrictedGroups = 2;
    if (!RedTerrain::_bEnable)
    {
        mMaxUpdateRestrictedGroups = 1;
    }
    if (++mUpdateRestrictedCounter >= mMaxUpdateRestrictedGroups)
    {
        mUpdateRestrictedCounter = 0;
    }

    UpdateSimple(mRigidBodies, COLL_ITEM | COLL_ASTEROID | COLL_RIGID);
    UpdateRestricted(mRigidBodies, COLL_STATIC);
    UpdateRestricted(mAsteroidBodies, COLL_STATIC);
    UpdateTreeGrid(mRigidBodies);
    UpdateTreeGrid(mAsteroidBodies);

    UpdateSimple(mSoftBodies, COLL_ITEM | COLL_SOFT | COLL_ATTACHED);
    UpdateRestricted(mSoftBodies, COLL_ITEM | COLL_ASTEROID | COLL_RIGID | COLL_STATIC);
    UpdateTreeGrid(mSoftBodies);

    UpdateSimple(mItemBodies, COLL_SOFT | COLL_RIGID);
    UpdateRestricted(mItemBodies, COLL_STATIC);
    UpdateTreeGrid(mItemBodies);

    UpdateTreeGrid(mAttachedBodies);
    UpdateTreeGrid(mShieldBodies);
    return true;
}

// FUNCTION: SWBF2 0x0045f550
CollisionManager::OrdnanceUpdate::OrdnanceUpdate(CollisionManager* pOwner)
    : mOwner(pOwner)
{
}

// SYNTHETIC: SWBF2 0x0045f570
// CollisionManager::OrdnanceUpdate::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0045f5a0
bool CollisionManager::OrdnanceUpdate::Update(float fDeltaTime)
{
    PblList<Ordnance>::Itor it(mOwner->mOrdnance);
    while (Ordnance* pOrdnance = it)
    {
        ++it;
        if (!pOrdnance->CheckCollision())
        {
            pOrdnance->KillOrdnance();
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0045dc10
bool CollisionManager::Create(const PblVector3& vMin, const PblVector3& vMax)
{
    if (CollisionManager::sInstance)
    {
        return false;
    }
    CollisionManager::sInstance = new CollisionManager(vMin, vMax);
    CollisionManager::sHostInstance = CollisionManager::sInstance;
    sMinPos = vMin;
    sMaxPos = vMax;
    return true;
}

// GLOBAL: SWBF2 0x01eb135c
PblVector3 sMinPos;

// GLOBAL: SWBF2 0x01eb1324
PblVector3 sMaxPos;
