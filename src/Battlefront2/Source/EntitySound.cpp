#include <stdio.h>
#include <string.h>

#include "EntitySound.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "SoundFL/source/sndspace.h"

class SoundSpaceRegion : public Snd::Space::Region
{
public:

    SoundSpaceRegion(const RedRegion& rRegion) : Region(rRegion) {}

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }

    ~SoundSpaceRegion() {}

    static void operator delete(void* p) { sMemoryPool.Free(p); }

    static void DestroyAll();

    static MemoryPool sMemoryPool;
};
 

// GLOBAL: SWBF2 0x01ebc1a0
// SYNTHETIC: SWBF2 0x00402dd0
// `dynamic initializer for 'EntitySound::rttiHashEntitySound''
PblHash EntitySound::rttiHashEntitySound("EntitySound");
char EntitySound::rttiNameEntitySound[] = "EntitySound";

// GLOBAL: SWBF2 0x01ebc0fc
// SYNTHETIC: SWBF2 0x00402df0
// `dynamic initializer for 'EntitySoundClass::rttiHashEntitySoundClass''
PblHash EntitySoundClass::rttiHashEntitySoundClass("EntitySoundClass");
char EntitySoundClass::rttiNameEntitySoundClass[] = "EntitySoundClass";

// GLOBAL: SWBF2 0x01ebc150
// SYNTHETIC: SWBF2 0x00402e00
// `dynamic initializer for 'SoundSpaceRegion::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766fe0
// `dynamic atexit destructor for 'SoundSpaceRegion::sMemoryPool''
MemoryPool SoundSpaceRegion::sMemoryPool("SoundSpaceRegion", 0x70);

// GLOBAL: SWBF2 0x007eb884
// SYNTHETIC: SWBF2 0x00402de0
// `dynamic initializer for 'EntitySound::sEntitySoundList''
// SYNTHETIC: SWBF2 0x00766fd0
// `dynamic atexit destructor for 'EntitySound::sEntitySoundList''
PblListSingle EntitySound::sEntitySoundList;

// FUNCTION: SWBF2 0x004fac70
bool EntitySound::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntitySound || Entity::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004faca0
unsigned int EntitySound::GetDerivedRtti() const
{
    return rttiHashEntitySound;
}

// FUNCTION: SWBF2 0x004facb0
const char* EntitySound::GetDerivedRttiName() const
{
    return rttiNameEntitySound;
}

// FUNCTION: SWBF2 0x004facc0
bool EntitySoundClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntitySoundClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004facf0
unsigned int EntitySoundClass::GetDerivedRtti()
{
    return rttiHashEntitySoundClass;
}

// FUNCTION: SWBF2 0x004fad00
const char* EntitySoundClass::GetDerivedRttiName()
{
    return rttiNameEntitySoundClass;
}

// SYNTHETIC: SWBF2 0x004fad20
// EntitySound::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fad50
EntitySound::EntitySound(EntitySoundClass* pClass, const EntityDesc& desc)
{
    mSoundId = pClass->mSoundId;
    if (pClass->mFlags & 1)
    {
        SetMinDistance(pClass->mMinDistance);
    }
    if (pClass->mFlags & 4)
    {
        SetMuteDistance(pClass->mMuteDistance);
    }
    if (pClass->mFlags & 2)
    {
        SetMaxDistance(pClass->mMaxDistance);
    }
    mPosition = *(const PblVector3*)desc.mMatrix.m[3];
    sEntitySoundList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x004fadf0
EntitySound::~EntitySound()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x004fae20
void EntitySound::SetProperty(unsigned int uProperty, const char* pcValue)
{
    float fValue;
    switch (uProperty)
    {
    case 0x0E0D9594:
    case 0x2E93EF4C:
        mSoundId = PblHash(pcValue);
        break;
    case 0xB835CF50:
        sscanf(pcValue, "%f", &fValue);
        SetMinDistance(fValue);
        break;
    case 0xE6915CF6:
        sscanf(pcValue, "%f", &fValue);
        SetMuteDistance(fValue);
        SetMaxDistance(fValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004faec0
void EntitySound::PlayAll()
{
    for (PblListSingle::Node* pNode = sEntitySoundList.mHead.mNext;
         pNode != &sEntitySoundList.mHead; pNode = pNode->mNext)
    {
        ((EntitySound*)((char*)pNode - offsetof(EntitySound, mNode)))->Play();
    }
}

// FUNCTION: SWBF2 0x004faef0
void EntitySound::DestroyAll()
{
    while (sEntitySoundList.mHead.mNext != &sEntitySoundList.mHead)
    {
        delete (EntitySound*)((char*)sEntitySoundList.mHead.mNext - offsetof(EntitySound, mNode));
    }
}

// FUNCTION: SWBF2 0x004faf20
EntitySoundClass::EntitySoundClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mSoundId = 0;
}

// SYNTHETIC: SWBF2 0x004faf60
// EntitySoundClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004faf90
EntitySoundClass::EntitySoundClass(EntitySoundClass* pBase, unsigned int uHash)
    : EntityClass(uHash)
{
    mSoundId = pBase->mSoundId;
}

// FUNCTION: SWBF2 0x004fafd0
EntitySoundClass::~EntitySoundClass()
{
}

// FUNCTION: SWBF2 0x004fafe0
void EntitySoundClass::SetProperty(unsigned int uProperty, const char* pcValue)
{
    float fValue;
    switch (uProperty)
    {
    case 0x0E0D9594:
    case 0x2E93EF4C:
        mSoundId = PblHash(pcValue);
        break;
    case 0xB835CF50:
        sscanf(pcValue, "%f", &fValue);
        SetMinDistance(fValue);
        break;
    case 0xE6915CF6:
        sscanf(pcValue, "%f", &fValue);
        SetMuteDistance(fValue);
        SetMaxDistance(fValue);
        break;
    default:
        EntityClass::SetProperty(uProperty, pcValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004fb090
EntitySound* EntitySoundClass::CreateSoundFromRegion(RedRegionDesc& desc, const char* pcName,
                                                     const char* pcPrefix, EntitySoundClass& soundClass)
{
    EntityDesc entityDesc;
    float fScale = 0.0f;
    char acSoundName[512];
    char acFormat[512];

    acSoundName[0] = 0;
    entityDesc.mMatrix = desc.mMatrix;
    entityDesc.mNameId = PblHash(pcName);
    entityDesc.mPlayerId = 0;

    strncpy(acFormat, pcPrefix, sizeof(acFormat) - 1);
    acFormat[sizeof(acFormat) - 1] = 0;
    strcat(acFormat, " %s %f");
    sscanf(pcName, acFormat, acSoundName, &fScale);

    EntitySound* pSound = (EntitySound*)soundClass.Build(entityDesc);
    if (pSound)
    {
        pSound->SetSoundId(PblHash(acSoundName));
        if (fScale > 0.0f)
        {
            pSound->SetMinDistance(desc.mSize.w / fScale);
        }
        pSound->SetMuteDistance(desc.mSize.w);
        pSound->SetMaxDistance(desc.mSize.w);
    }
    return pSound;
}

unsigned int Snd::Space::GetCount()
{
    unsigned int uCount = 0;
    for (PblListSingle::Itor it(smSpaceList); !it.IsDone(); ++it)
    {
        ++uCount;
    }
    return uCount;
}

// FUNCTION: SWBF2 0x004fac30
unsigned int Snd::Space::GetRegionCount() const
{
    unsigned int uCount = 0;
    for (PblListSingle::Itor it(mList); !it.IsDone(); ++it)
    {
        ++uCount;
    }
    return uCount;
}

// FUNCTION: SWBF2 0x004fac50
Snd::Space::Region* Snd::Space::FindRegionByIndex(unsigned int uIndex)
{
    PblListSingle::Node* pNode = 0;
    if (!mList.IsEmpty())
    {
        pNode = mList.mHead.mNext;
        for (; uIndex; --uIndex)
        {
            pNode = pNode->mNext;
        }
    }
    if (!pNode || pNode == &mList.mHead)
    {
        return 0;
    }
    return reinterpret_cast<Region*>(reinterpret_cast<char*>(pNode) - offsetof(Region, mSpaceNode));
}

Snd::Space* Snd::Space::FindByIndex(unsigned int uIndex)
{
    PblListSingle::Node* pNode = 0;
    if (!smSpaceList.IsEmpty())
    {
        pNode = smSpaceList.mHead.mNext;
        for (; uIndex; --uIndex)
        {
            pNode = pNode->mNext;
        }
    }
    if (pNode == &smSpaceList.mHead)
    {
        return 0;
    }
    return reinterpret_cast<Space*>(reinterpret_cast<char*>(pNode) - offsetof(Space, mNode));
}

// FUNCTION: SWBF2 0x004fb1e0
void SoundSpaceRegion::DestroyAll()
{
    unsigned int uCount = Snd::Space::GetCount();
    for (unsigned int i = 0; i < uCount; ++i)
    {
        Snd::Space* pSpace = Snd::Space::FindByIndex(i);
        while (pSpace->GetRegionCount())
        {
            SoundSpaceRegion* pRegion =
                static_cast<SoundSpaceRegion*>(pSpace->FindRegionByIndex(0));
            delete pRegion;
        }
    }
}

// FUNCTION: SWBF2 0x004fb2c0
void* SoundSpaceRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    char acFormat[512];
    char acSpaceName[512];

    strncpy(acFormat, GetPrefix(), sizeof(acFormat) - 1);
    acFormat[sizeof(acFormat) - 1] = 0;
    strcat(acFormat, " %s");
    sscanf(pcName, acFormat, acSpaceName);

    Snd::Space* const pSpace = Snd::Space::Find(PblHash(acSpaceName));
    if (!pSpace)
    {
        return 0;
    }

    RedRegion region(desc);
    SoundSpaceRegion* const pRegion = new SoundSpaceRegion(region);
    if (!pRegion)
    {
        return 0;
    }
    pSpace->mList.AddLast(&pRegion->mSpaceNode);
    return pRegion;
}
