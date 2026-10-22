#include <stdio.h>

#include "TriggerSoundRegion.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndsoundstream.h"
#include "SoundFL/source/win/sndstream.h"

void TriggerSoundRegion::Group::DestroyAll()
{
    while (!sList.IsEmpty())
    {
        Group* const pGroup = (Group*)((char*)sList.mHead.mNext - offsetof(Group, mNode));
        delete pGroup;
    }
}

// GLOBAL: SWBF2 0x007ebe78
// SYNTHETIC: SWBF2 0x00406be0
// `dynamic initializer for 'TriggerSoundRegion::sList''
// SYNTHETIC: SWBF2 0x007696a0
// `dynamic atexit destructor for 'TriggerSoundRegion::sList''
PblListSingle TriggerSoundRegion::sList;

// FUNCTION: SWBF2 0x0065d6d0
unsigned int TriggerSoundRegion::Properties::GetID() const
{
    return mID;
}

// FUNCTION: SWBF2 0x0065d9c0
const TriggerSoundRegion::Properties* TriggerSoundRegion::Properties::FindByID(unsigned int uID)
{
    PblListSingle::Node* pNode = sList.mHead.mNext;
    while (pNode != &sList.mHead)
    {
        const Properties* pProperties = reinterpret_cast<const Properties*>(reinterpret_cast<const char*>(pNode) - offsetof(Properties, mNode));
        if (pProperties->GetID() == uID)
        {
            return pProperties;
        }
        pNode = pNode->mNext;
    }
    return 0;
}

// GLOBAL: SWBF2 0x01f9a108
// SYNTHETIC: SWBF2 0x00406b60
// `dynamic initializer for 'TriggerSoundRegion::Group::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769650
// `dynamic atexit destructor for 'TriggerSoundRegion::Group::sMemoryPool''
MemoryPool TriggerSoundRegion::Group::sMemoryPool("TriggerSoundRegion::Group", 0x50);

// GLOBAL: SWBF2 0x01f9a0b8
// SYNTHETIC: SWBF2 0x00406b90
// `dynamic initializer for 'TriggerSoundRegion::Properties::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769670
// `dynamic atexit destructor for 'TriggerSoundRegion::Properties::sMemoryPool''
MemoryPool TriggerSoundRegion::Properties::sMemoryPool("TriggerSoundRegion::Properties", 0xc);

// GLOBAL: SWBF2 0x01f9a158
// SYNTHETIC: SWBF2 0x00406bc0
// `dynamic initializer for 'TriggerSoundRegion::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769690
// `dynamic atexit destructor for 'TriggerSoundRegion::sMemoryPool''
MemoryPool TriggerSoundRegion::sMemoryPool("TriggerSoundRegion", 0x70);

// FUNCTION: SWBF2 0x0065d400
TriggerSoundRegion::Group::Group()
    : mID(0), mReinforcementCount(0.0f), mEnabled(true)
{
    sList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0065d460
TriggerSoundRegion::Group::~Group()
{
    if (mNode.mNext != 0)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0065d4a0
void TriggerSoundRegion::Group::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk data = chunk.ReadNextChild();
        const unsigned int uHash = data._pFile->Read32();
        data._pFile->Read8();

        switch (uHash)
        {
        case 0x8d39bde6:
            mID = data._pFile->Read32();
            break;
        case 0x581a579b:
            mReinforcementCount = data._pFile->ReadFloat();
            break;
        case 0x1d083cc0:
            mSound[Team::side_Alliance].SetID(data._pFile->Read32(), false, false);
            break;
        case 0x2c5f5065:
            mSound[Team::side_Empire].SetID(data._pFile->Read32(), false, false);
            break;
        case 0xc7836fe1:
            mSound[Team::side_Republic].SetID(data._pFile->Read32(), false, false);
            break;
        case 0xf387c5d6:
            mSound[Team::side_CIS].SetID(data._pFile->Read32(), false, false);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0065d5b0
unsigned int TriggerSoundRegion::Group::GetID() const
{
    return mID;
}

// FUNCTION: SWBF2 0x0065d5c0
const GameSound* TriggerSoundRegion::Group::GetSound(Team::Side eSide) const
{
    return &mSound[eSide];
}

// FUNCTION: SWBF2 0x0065d5e0
float TriggerSoundRegion::Group::GetReinforcementCount() const
{
    return mReinforcementCount;
}

// FUNCTION: SWBF2 0x0065d5f0
TriggerSoundRegion::Group* TriggerSoundRegion::Group::FindByID(unsigned int uID)
{
    for (PblListSingle::Node* pNode = sList.mHead.mNext; pNode != &sList.mHead;
         pNode = pNode->mNext)
    {
        Group* pGroup = (Group*)((char*)pNode - offsetof(Group, mNode));
        if (pGroup->GetID() == uID)
        {
            return pGroup;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0065d660
TriggerSoundRegion::Properties::Properties()
    : mID(0)
{
    sList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0065d6a0
TriggerSoundRegion::Properties::~Properties()
{
    if (mNode.mNext != 0)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0065d6e0
const GameSound* TriggerSoundRegion::Properties::GetSound(Team::Side eSide, float fRatio) const
{
    for (PblListSingle::Itor it(mList); !it.IsDone(); ++it)
    {
        const Group* pGroup = (const Group*)((Snd::ObjectList::Node*)it.GetNode())->mObject;
        if (fRatio >= pGroup->GetReinforcementCount() && pGroup->IsEnabled())
        {
            return pGroup->GetSound(eSide);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0065d720
void TriggerSoundRegion::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk data = chunk.ReadNextChild();
        const unsigned int uHash = data._pFile->Read32();
        data._pFile->Read8();

        if (uHash == 0x5fb91e8c)
        {
            Group* const pGroup = Group::FindByID(data._pFile->Read32());
            if (!pGroup)
            {
                continue;
            }
            PblListSingle::Itor it(mList);
            for (; !it.IsDone(); ++it)
            {
                const Group* pOther = (const Group*)((Snd::ObjectList::Node*)it.GetNode())->mObject;
                if (pGroup->GetReinforcementCount() > pOther->GetReinforcementCount())
                {
                    break;
                }
            }
            if (it.IsDone())
            {
                mList.Add(pGroup);
            }
            else
            {
                mList.AddBefore((Snd::ObjectList::Node*)it.GetNode(), pGroup);
            }
        }
        else if (uHash == 0x8d39bde6)
        {
            mID = data._pFile->Read32();
        }
    }
}

// FUNCTION: SWBF2 0x0065d840
void TriggerSoundRegion::Properties::Load(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        const unsigned int uHash = child._pFile->Read32();
        child._pFile->Read8();
        if (uHash == 0x031afe7d)
        {
            Properties* pProperties = new Properties;
            child = chunk.ReadNextChild();
            pProperties->Read(child);
        }
        else if (uHash == 0x5fb91e8c)
        {
            Group* pGroup = new Group;
            child = chunk.ReadNextChild();
            pGroup->Read(child);
        }
    }
}

// FUNCTION: SWBF2 0x0065da70
void TriggerSoundRegion::Update(Team::Side eSide, float fRatio, const PblVector3* const pPos)
{
    for (PblListSingle::Node* pLink = sList.mHead.mNext; pLink != &sList.mHead;
         pLink = pLink->mNext)
    {
        TriggerSoundRegion* pRegion =
            (TriggerSoundRegion*)((char*)pLink - offsetof(TriggerSoundRegion, mNode));
        const PblVector3 vPos = *pPos;
        if (!pRegion->IsPointInside(vPos))
        {
            continue;
        }

        const GameSound* pSound = pRegion->mProperties->GetSound(eSide, fRatio);
        if (!pSound)
        {
            continue;
        }

        if (pSound->GetPropertiesType() == GameSound::type_soundStream)
        {
            const Snd::SoundStream::Properties* pProps =
                (const Snd::SoundStream::Properties*)pSound->GetProperties();
            Snd::Stream* pStream = Snd::EngineBase::GetOpenStream(pProps->mStreamID);
            if (!pStream || pStream->GetPlayState() == Snd::SourceTransport::playState_playing)
            {
                continue;
            }
        }

        pSound->Play(&pRegion->mMatrix.GetTranslation(), 0, false, true);
    }
}

// FUNCTION: SWBF2 0x0065da30
TriggerSoundRegion::TriggerSoundRegion(RedRegionDesc& desc, const Properties* const pProperties)
    : RedRegion(desc), mProperties(const_cast<Properties*>(pProperties))
{
    sList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0065db90
TriggerSoundRegionFactory::TriggerSoundRegionFactory()
    : RedRegionFactory("soundtrigger")
{
}

// FUNCTION: SWBF2 0x0065dbe0
void* TriggerSoundRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    char acPropertiesName[512];
    char acFormat[512];

    acPropertiesName[0] = 0;
    strlcpy(acFormat, GetPrefix(), sizeof(acFormat));
    strlcat(acFormat, " %s", sizeof(acFormat));
    sscanf(pcName, acFormat, acPropertiesName);

    const TriggerSoundRegion::Properties* const pProperties =
        TriggerSoundRegion::Properties::FindByID(PblHash(acPropertiesName));
    if (!pProperties)
    {
        return 0;
    }
    return new TriggerSoundRegion(desc, pProperties);
}

// FUNCTION: SWBF2 0x0065d9f0
void TriggerSoundRegion::Properties::DestroyAll()
{
    while (!sList.IsEmpty())
    {
        Properties* const pProperties =
            (Properties*)((char*)sList.mHead.mNext - offsetof(Properties, mNode));
        delete pProperties;
    }
}

// FUNCTION: SWBF2 0x0065db30
void TriggerSoundRegion::DestroyAll()
{
    while (!sList.IsEmpty())
    {
        TriggerSoundRegion* const pRegion =
            (TriggerSoundRegion*)((char*)sList.mHead.mNext - offsetof(TriggerSoundRegion, mNode));
        delete pRegion;
    }
    Properties::DestroyAll();
    Group::DestroyAll();
}

// GLOBAL: SWBF2 0x007ebe98
// SYNTHETIC: SWBF2 0x00406b80
// `dynamic initializer for 'TriggerSoundRegion::Group::sList''
// SYNTHETIC: SWBF2 0x00769660
// `dynamic atexit destructor for 'TriggerSoundRegion::Group::sList''
PblListSingle TriggerSoundRegion::Group::sList;

// GLOBAL: SWBF2 0x007ebe7c
// SYNTHETIC: SWBF2 0x00406bb0
// `dynamic initializer for 'TriggerSoundRegion::Properties::sList''
// SYNTHETIC: SWBF2 0x00769680
// `dynamic atexit destructor for 'TriggerSoundRegion::Properties::sList''
PblListSingle TriggerSoundRegion::Properties::sList;
