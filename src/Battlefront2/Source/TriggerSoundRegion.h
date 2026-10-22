#pragma once

#include <stddef.h>
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "PebbleFL/Common/PblList.h"
#include "SoundFL/source/sndobjectlist.h"
#include "GameSound.h"
#include "Team.h"

class PblVector3;
class PblFileChunk;

// VTABLE: SWBF2 0x007af688
class TriggerSoundRegionFactory : public RedRegionFactory
{
public:

    TriggerSoundRegionFactory();

    // SYNTHETIC: SWBF2 0x0065dbb0
    // TriggerSoundRegionFactory::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
};
  

class TriggerSoundRegion : public RedRegion
{
public:

    class Properties
    {
    public:
        static const Properties* FindByID(unsigned int uID);

        unsigned int GetID() const;

        Properties();

        void Read(PblFileChunk& chunk);

        static void Load(PblFileChunk& chunk);

        static void* operator new(unsigned int)
        {
            if (!sMemoryPool.mCount)
            {
                sMemoryPool.Setup(32, sMemoryPool.GetSize(), sMemoryPool.GetGrow());
            }
            return sMemoryPool.Allocate(sizeof(Properties));
        }

        const GameSound* GetSound(Team::Side eSide, float fRatio) const;

        unsigned int mID;
        Snd::ObjectList mList;
        PblListSingle::Node mNode;

        static MemoryPool sMemoryPool;

        static PblListSingle sList;

        static void DestroyAll();

        ~Properties();

        static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }
    };

    class Group
    {
    public:

        Group();

        void Read(PblFileChunk& chunk);

        unsigned int GetID() const;

        static Group* FindByID(unsigned int uID);

        static void* operator new(unsigned int)
        {
            if (!sMemoryPool.mCount)
            {
                sMemoryPool.Setup(32, sMemoryPool.GetSize(), sMemoryPool.GetGrow());
            }
            return sMemoryPool.Allocate(sizeof(Group));
        }

        const GameSound* GetSound(Team::Side eSide) const;

        float GetReinforcementCount() const;

        bool IsEnabled() const { return mEnabled; }

        unsigned int mID;
        GameSound mSound[8];
        float mReinforcementCount;
        PblListSingle::Node mNode;
        bool mEnabled : 1;

        static MemoryPool sMemoryPool;

        static PblListSingle sList;

        static void DestroyAll();

        ~Group();

        static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }
    };

    static void Update(Team::Side eSide, float fRatio, const PblVector3* const pPos);

    static void DestroyAll();

    ~TriggerSoundRegion()
    {
        if (mNode.mNext != 0)
        {
            mNode.Remove();
        }
    }

    static PblListSingle sList;

    TriggerSoundRegion(RedRegionDesc& desc, const Properties* const pProperties);

    static void* operator new(unsigned int)
    {
        if (!sMemoryPool.mCount)
        {
            sMemoryPool.Setup(32, sMemoryPool.GetSize(), sMemoryPool.GetGrow());
        }
        return sMemoryPool.Allocate(sizeof(TriggerSoundRegion));
    }

    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    static MemoryPool sMemoryPool;

    Properties* mProperties;
    PblListSingle::Node mNode;
};
 
 
 
 
 
 
