#pragma once

class PblFileChunk;

#include <stddef.h>

#include "Factory.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"

class Entity;
class EntityClass;
class GameObject;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

struct EntityDesc
{
    // FUNCTION: SWBF2 0x00443f10
    EntityDesc()
        : mMatrix(*reinterpret_cast<const PblMatrix*>(&gMatrixIdentity)),
          mNameId(0), mPlayerId(-1), mPriority(0.001f) {}

    PblMatrix mMatrix;
    unsigned int mNameId;
    int mPlayerId;
    float mPriority;
};
 

// VTABLE: SWBF2 0x00797300
class Entity
{
public:
    Entity() {}

    // FUNCTION: SWBF2 0x00443ea0
    virtual bool IsRtti(unsigned int uRtti) const { return uRtti == rttiHashEntity; }
    // FUNCTION: SWBF2 0x00458a00
    virtual unsigned int GetDerivedRtti() const { return rttiHashEntity; }
    // FUNCTION: SWBF2 0x00458a10
    virtual const char* GetDerivedRttiName() const { return rttiNameEntity; }
    // FUNCTION: SWBF2 0x00458a20
    virtual ~Entity() {}
    // FUNCTION: SWBF2 0x00458a30
    virtual void SetProperty(unsigned int, const char*) {}
    // FUNCTION: SWBF2 0x00458a40
    virtual void Init() {}
    // FUNCTION: SWBF2 0x00458a50
    virtual Entity* GetEntity() { return this; }
    // FUNCTION: SWBF2 0x00458a60
    virtual const Entity* GetEntity() const { return this; }
    // FUNCTION: SWBF2 0x00458a70
    virtual GameObject* GetGameObject() { return 0; }
    // FUNCTION: SWBF2 0x00458a80
    virtual const GameObject* GetGameObject() const { return 0; }
    // FUNCTION: SWBF2 0x00458a90
    virtual EntityClass* GetEntityClass() { return 0; }
    // FUNCTION: SWBF2 0x00458aa0
    virtual const EntityClass* GetEntityClass() const { return 0; }

    static PblHash rttiHashEntity;

    static unsigned int GetRtti() { return rttiHashEntity; }
    static char rttiNameEntity[];
};
 

// SYNTHETIC: SWBF2 0x00458ab0
// Entity::`scalar deleting destructor'

// VTABLE: SWBF2 0x0079934c
class EntityEx : public Entity
{
public:
    EntityEx(unsigned int uId, EntityClass* pEntityClass);
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityEx();
    // FUNCTION: SWBF2 0x00443ef0
    virtual EntityClass* GetEntityClass() { return mEntityClass; }
    // FUNCTION: SWBF2 0x00443f00
    virtual const EntityClass* GetEntityClass() const { return mEntityClass; }

    unsigned int GetId() const { return mId; }

    unsigned int mId;
    EntityClass* mEntityClass;

    static PblHash rttiHashEntityEx;

    static unsigned int GetRtti() { return rttiHashEntityEx; }
    static char rttiNameEntityEx[];
    static int mIdMapNumEntries;
    static PblHashTable<EntityEx, 1024> mIdMap;

    static PblHashTable<EntityEx, 1024>& GetEntityMap() { return mIdMap; }

    static void InitFind();

    // FUNCTION: SWBF2 0x0046db30
    static EntityEx* Find(unsigned int uiHash) { return mIdMap.Find(uiHash); }

private:
    static void Store(unsigned int uId, EntityEx* pEntity);
    static void Remove(unsigned int uId);
};
 
 
 

class EntityClass : public Factory<Entity, EntityClass, EntityDesc>
{
public:

    static void Read(PblFileChunk& chunk);

    class MapParams
    {
    public:
        MapParams();

        float mScale;
        float mSpeedMin;
        float mSpeedMax;
        float mViewRangeMin;
        float mViewRangeMax;
        unsigned int mTexture;
        float mTargetFadeTime;
        float mRangeOverall;
        float mRangeShooting;
        float mRangeViewCone;
        float mViewConeCosine;
    };

    EntityClass(unsigned int uHash);
    EntityClass(EntityClass* pParent, unsigned int uHash);

    virtual ~EntityClass();
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual void PostReadSetup();

    static PblHash rttiHashEntityClass;
    static char rttiNameEntityClass[];

    unsigned short* mLabel;
    unsigned int mIconTexture;
    unsigned int mHealthTexture;
    unsigned int mHUDModelID;
    MapParams mMapParams;
    bool mRemoveFromMapWhenDead;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
