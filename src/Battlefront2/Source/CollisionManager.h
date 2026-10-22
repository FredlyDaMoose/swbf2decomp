#pragma once

#include <stddef.h>

#include "Thread.h"
#include "TreeGrid.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class CollisionObject;
class EntityTerrain;
class EntityWater;
class GameObject;
class Ordnance;
class RedRegion;

class CollisionManager : public Thread
{
public:

    class OrdnanceUpdate : public Thread
    {
    public:
        OrdnanceUpdate(CollisionManager* pOwner);
        virtual ~OrdnanceUpdate();
        virtual bool Update(float fDeltaTime);

        CollisionManager* mOwner;
    };

    static bool Create(const PblVector3& vMin, const PblVector3& vMax);
    static bool CreateClient();
    static bool Destroy();
    static bool DestroyClient();
    static void EnterClient();
    static void LeaveClient();

    static bool SetTerrain(EntityTerrain* pTerrain);
    static bool SetWater(EntityWater* pWater);
    static void SetMaxCollisionDistance(float fDistance);

    static bool AddItemBody(CollisionObject* pObject);
    static bool AddSoftBody(CollisionObject* pObject);
    static bool AddRigidBody(CollisionObject* pObject);
    static bool AddAsteroidBody(CollisionObject* pObject);
    static bool AddStaticBody(CollisionObject* pObject);
    static bool AddAttachedBody(CollisionObject* pObject);
    static bool AddLeafPatchBody(CollisionObject* pObject);
    static bool AddShieldBody(CollisionObject* pObject);
    static bool RemoveBody(CollisionObject* pObject);
    static bool DeactivateItem(CollisionObject* pObject);
    static bool AddOrdnance(Ordnance* pOrdnance);
    static bool RemoveOrdnance(Ordnance* pOrdnance);

    static TreeGrid* GetTreeGrid();

    static int GetLastRayHitBodyId();

    static void BeginList(PblList<CollisionObject>::Itor& itor);
    static void ListObjects(PblList<CollisionObject>::Itor& itor, CollisionObject** const pObjects,
                            int& iCount);

    static int GetObjectsInRect(const PblVector3& vMin, const PblVector3& vMax, GameObject** const pObjects,
                                int iMaxCount, const GameObject* pIgnore, int iFilter);
    static int GetObjectsInRange(const PblVector3& vCenter, float fRadius, GameObject** const pObjects,
                                 int iMaxCount, const GameObject* pIgnore, int iFilter);
    static int GetObjectsInFatRay(const PblVector3& vFrom, const PblVector3& vTo, float fRadius,
                                  GameObject** const pObjects, int iMaxCount, const GameObject* pIgnore,
                                  int iFilter);
    static int GetObjectsInRegion(const RedRegion& rRegion, GameObject** const pObjects, int iMaxCount,
                                  const GameObject* pIgnore, int iFilter);
    static int GetCollidablesInRegion(const RedRegion& rRegion, CollisionObject** const pObjects,
                                      int iMaxCount, const CollisionObject* pIgnore, int iFilter);

    static float RayHit(const PblVector3& vStart, const PblVector3& vEnd, CollisionObject*& pHitObject,
                        PblVector3* pNormal, const GameObject** const pIgnore, int iIgnoreCount, int iFilter,
                        bool bOrdnance);
    static float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        CollisionObject*& pHitObject, PblVector3* pNormal, const GameObject** const pIgnore,
                        int iIgnoreCount, int iFilter, bool bOrdnance);
    static bool RayTest(const PblVector3& vStart, const PblVector3& vEnd, const GameObject** const pIgnore,
                        int iIgnoreCount, int iFilter);
    static bool RayTest(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                        const GameObject** const pIgnore, int iIgnoreCount, int iFilter);

    virtual bool Update(float fDeltaTime);

    CollisionManager(const PblVector3& vMin, const PblVector3& vMax);
    virtual ~CollisionManager();

    static bool CollectDataCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData, float& fDist);
    static bool GetObjectsRectCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                       float& fDist);
    static bool GetObjectsRayCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                      float& fDist);
    static bool GetObjectsRangeCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                        float& fDist);
    static bool GetObjectsRegionCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                         float& fDist);
    static bool GetCollidablesRegionCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData,
                                             float& fDist);
    static bool RayCallback(const TreeGridObject* pTreeObject, TreeGridCallbackData* pData, float& fDist);

    bool I_AddBody(CollisionObject* pObject, PblList<CollisionObject>& list);
    bool I_RemoveBody(CollisionObject* pObject);
    bool I_DeactivateItem(CollisionObject* pObject);
    bool I_AddOrdnance(Ordnance* pOrdnance);
    bool I_RemoveOrdnance(Ordnance* pOrdnance);
    bool I_GetObjectsFromList(const PblVector3& vMin, const PblVector3& vMax, TreeGridCallback pCallback,
                              TreeGridCallbackData* pData, PblList<CollisionObject>& list);
    void I_GetObjectsFromLists(const PblVector3& vMin, const PblVector3& vMax, TreeGridCallback pCallback,
                               int iFilter, TreeGridCallbackData* pData);
    int I_GetObjectsInRect(const PblVector3& vMin, const PblVector3& vMax, GameObject** const pObjects,
                           int iMaxCount, const GameObject* pIgnore, int iFilter);
    int I_GetObjectsInRange(const PblVector3& vCenter, float fRadius, GameObject** const pObjects,
                            int iMaxCount, const GameObject* pIgnore, int iFilter);
    int I_GetObjectsInFatRay(const PblVector3& vFrom, const PblVector3& vTo, float fRadius,
                             GameObject** const pObjects, int iMaxCount, const GameObject* pIgnore, int iFilter);
    int I_GetObjectsInRegion(const RedRegion& rRegion, GameObject** const pObjects, int iMaxCount,
                             const GameObject* pIgnore, int iFilter);
    int I_GetCollidablesInRegion(const RedRegion& rRegion, CollisionObject** const pObjects, int iMaxCount,
                                 const CollisionObject* pIgnore, int iFilter);
    float I_RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, CollisionObject*& pHitObject,
                   PblVector3* pNormal, const GameObject** const pIgnore, int iIgnoreCount, int iFilter,
                   int* pBodyId, bool bOrdnance);
    bool I_RayTest(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                   const GameObject** const pIgnore, int iIgnoreCount, int iFilter);
    void UpdateSimple(PblList<CollisionObject>& list, int iFilter);
    void UpdateRestricted(PblList<CollisionObject>& list, int iFilter);
    void UpdateTreeGrid(PblList<CollisionObject>& list);

    PblList<CollisionObject> mItemBodies;
    PblList<CollisionObject> mSoftBodies;
    PblList<CollisionObject> mRigidBodies;
    PblList<CollisionObject> mAsteroidBodies;
    PblList<CollisionObject> mStaticBodies;
    PblList<CollisionObject> mAttachedBodies;
    PblList<CollisionObject> mLeafPatchBodies;
    PblList<CollisionObject> mShieldBodies;
    PblList<Ordnance> mOrdnance;
    TreeGrid* mTreeGrid;
    EntityTerrain* mTerrain;
    EntityWater* mWater;
    float mMaxCollisionDistanceSq;
    unsigned char mUpdateRestrictedCounter;
    unsigned char mMaxUpdateRestrictedGroups;
    int mLastRayHitBodyId;
    OrdnanceUpdate mOrdnanceUpdate;

    static CollisionManager* sInstance;
    static CollisionManager* sHostInstance;
    static CollisionManager* sClientInstance;
};
 
 
 
 
 
 
 
