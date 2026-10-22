#pragma once

#include <stddef.h>

#include "SoundFL/source/sndobjectlist.h"
#include "PebbleFL/Common/PblHashTable.h"

class FoleyFX;
class PblFileChunk;

// VTABLE: SWBF2 0x0079f218
class FoleyFX
{
public:
    FoleyFX();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~FoleyFX();
    virtual bool Read(PblFileChunk& chunk, unsigned int uChunkID);

    unsigned int GetID() const;
    unsigned int GetClassID() const;
    void ReadChunk(PblFileChunk& chunk);
    static FoleyFX* FindByID(unsigned int uID);
    static void DestroyAll();
    static void Load(PblFileChunk& chunk);

    unsigned int mID;
    unsigned int mClass;
    PblListSingle::Node mNode;

    static PblHash rttiHashFoleyFX;
    static char rttiNameFoleyFX[];
    static PblListSingle smList;
};
 

class FoleyFXGroup
{
public:
    static FoleyFXGroup* FindByID(unsigned int uID);

    FoleyFXGroup();
    void Read(PblFileChunk& chunk);

    ~FoleyFXGroup();
    unsigned int GetID() const;
    static void DestroyAll();
    static FoleyFXGroup* GetWaterFX();
    static FoleyFXGroup* GetTerrainFX();

    FoleyFX* FindFoleyFX(unsigned int uClassID) const;

    unsigned int mID;
    Snd::ObjectList mList;
    PblListSingle::Node mNode;

    static PblListSingle smList;
    static FoleyFXGroup* smWater;
    static FoleyFXGroup* smTerrain;
};
 

class FoleyFXCollideeClass
{
public:
    FoleyFXCollideeClass();

    FoleyFXCollideeClass(const FoleyFXCollideeClass* pClass);

    bool SetProperty(unsigned int uProperty, const char* pszValue);

    FoleyFXGroup* GetFoleyFXGroup() const;

    FoleyFXGroup* mGroup;
};
 
 

class FoleyFXColliderClass
{
public:
    FoleyFXColliderClass();
    FoleyFXColliderClass(const FoleyFXColliderClass* pClass);
    unsigned int GetFoleyFXClassID() const;
    bool SetProperty(unsigned int uProperty, const char* pszValue);

    unsigned int mClassID;
};
 
 
