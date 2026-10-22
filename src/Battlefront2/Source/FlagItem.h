#pragma once

#include <stddef.h>

#include "EntityItem.h"
#include "CollisionModel.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class GameModel;
class MemoryPool;
template <class TYPE> class PblList;
class FlagItem;
class GameObject;

// VTABLE: SWBF2 0x0079ec8c
class FlagItemClass : public EntityItemClass
{
public:
    FlagItemClass(unsigned int uHash);
    FlagItemClass(FlagItemClass* pBase, unsigned int uHash);
    virtual ~FlagItemClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    GameModel* mCarriedModel;
    GameModel* mDroppedInLandingZoneModel;
    PblVector3 mCarriedOffset;
    bool mTeamColorize[2];
    GameSound mPickupSound;
    GameSound mDropSound;
    GameSound mAmbientSound;
    unsigned int mCarriedSoldierBoneHash;
    unsigned int mCarriedDroidekaBoneHash;

    static PblHash rttiHashFlagItemClass;
    static char rttiNameFlagItemClass[];
};
  
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079ecd8
// VTABLE: SWBF2 0x0079ec38 CollisionObject
// VTABLE: SWBF2 0x0079ee2c RedSceneObject
// VTABLE: SWBF2 0x0079ee00 Damageable
// VTABLE: SWBF2 0x0079edf8 PblHandled
// VTABLE: SWBF2 0x0079ec20 Thread
class FlagItem : public EntityItem
{
public:

    FlagItem(FlagItemClass* pClass, const EntityDesc& desc);

    virtual ~FlagItem();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void SetModel(const GameModel* pModel);
    virtual EntityType GetEntityType() const;
    virtual bool AddToTeamManager() const;
    virtual void WriteCreate(NetPktGroup& group);
    virtual void ReadCreate(NetPktGroup& group);
    virtual void WriteObject(NetPktGroup& group, NetObjState* pState);
    virtual void ReadObject(NetPktGroup& group, NetObjState* pState);

    virtual bool CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor);

    virtual void Render(unsigned char ucLod, float fAlpha, unsigned int uiFlags);

    virtual void Kill();
    virtual void InstantDeath(const DamageOwner& owner, bool bCheckProtection);

    virtual bool Update(float fDeltaTime);

    void SetupCollisionGeometry();

    GameObject* GetCarrier();
    bool Drop();
    void ResetFlag();
    void ClearCarrierFlagItem();
    bool IsInLandingZone() const;
    bool PickUp(GameObject* pObject);
    void BroadcastMessage(unsigned int uHash);
    void CheckFloatingInSpace();

    bool IsASpaceMap();

    enum FlagGameplayType
    {
        FLAG_GAMEPLAY_NONE = 0,
        FLAG_GAMEPLAY_CAMPAIGN = 1,
        FLAG_GAMEPLAY_1FLAG = 2,
        FLAG_GAMEPLAY_2FLAG = 3
    };

    static void SetGameplayType(FlagGameplayType eType) { sFlagGameplayType = eType; }

    // FUNCTION: SWBF2 0x00438b90
    static FlagGameplayType GetGameplayType() { return sFlagGameplayType; }

    class Character* GetLastCarrierCharacter() const { return mLastCarrier; }
    unsigned int GetCarriedSoldierBoneHash() const { return mCarriedSoldierBoneHash; }
    bool IsMoved() const { return mMoved; }
    unsigned int GetCarriedDroidekaBoneHash() const { return mCarriedDroidekaBoneHash; }
    const class RedRegion* GetHomeRegion() const { return mHomeRegion; }
    bool GetAllowAIPickup() const { return mAllowAIPickup; }

    void RenderHelper(unsigned char ucLod, float fAlpha, unsigned int uiFlags);

    static void Create();
    static void Destroy();
    static void CreateClient();
    static void DestroyClient();
    static void EnterClient();
    static void LeaveClient();
    static void Reset();

    PblList<FlagItem>::Node mNode;
    PblQuaternion mStartRotation;
    PblVector3 mStartPosition;
    GameSoundControllable mAmbientSound;
    GameModel* mDroppedModel;
    GameModel* mCarriedModel;
    GameModel* mDroppedInLandingZoneModel;
    class Character* mLastCarrier;
    float mCarriedTimer;
    CollisionModel mCollisionModel;
    unsigned int mHomeRegionId;
    class RedRegion* mHomeRegion;
    bool mAllowTeamPickup;
    bool mAllowAIPickup;
    bool mMoved;
    class EntityCloth* mClothPiece[8];
    unsigned int mCarriedSoldierBoneHash;
    unsigned int mCarriedDroidekaBoneHash;

    static MemoryPool sMemoryPool;
    static PblList<FlagItem> sHostList;
    static PblList<FlagItem> sClientList;
    static PblList<FlagItem>* sList;
    static FlagGameplayType sFlagGameplayType;
    static PblHash rttiHashFlagItem;
    static unsigned int GetRtti();
    static char rttiNameFlagItem[];
};
 
 
 
 
 
 
 
 
 
 
 
