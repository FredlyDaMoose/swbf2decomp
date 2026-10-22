#include "EntitySoundStream.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameLoop.h"
#include "GameState.h"

// GLOBAL: SWBF2 0x01ebc1fc
// SYNTHETIC: SWBF2 0x00402e90
// `dynamic initializer for 'EntitySoundStreamClass::rttiHashEntitySoundStreamClass''
PblHash EntitySoundStreamClass::rttiHashEntitySoundStreamClass("EntitySoundStreamClass");
char EntitySoundStreamClass::rttiNameEntitySoundStreamClass[] = "EntitySoundStreamClass";

// FUNCTION: SWBF2 0x004fbc80
EntitySoundStreamClass::EntitySoundStreamClass(unsigned int uHash)
    : EntitySoundClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fbce0
EntitySoundStreamClass::EntitySoundStreamClass(EntitySoundStreamClass* pBase,
                                               unsigned int uHash)
    : EntitySoundClass(pBase, uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fbd20
EntitySoundStreamClass::~EntitySoundStreamClass()
{
}

// SYNTHETIC: SWBF2 0x004fbcb0
// EntitySoundStreamClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fbd30
EntityClass* EntitySoundStreamClass::Derive(unsigned int uHash)
{
    return new EntitySoundStreamClass(this, uHash);
}

// FUNCTION: SWBF2 0x004fba10
bool EntitySoundStreamClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntitySoundStreamClass || EntitySoundClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004fbda0
Entity* EntitySoundStreamClass::Build(const EntityDesc& desc)
{
    return new EntitySoundStream(this, desc);
}

// FUNCTION: SWBF2 0x004fba40
unsigned int EntitySoundStreamClass::GetDerivedRtti()
{
    return rttiHashEntitySoundStreamClass;
}

// FUNCTION: SWBF2 0x004fba50
const char* EntitySoundStreamClass::GetDerivedRttiName()
{
    return rttiNameEntitySoundStreamClass;
}

// GLOBAL: SWBF2 0x01ebc250
// SYNTHETIC: SWBF2 0x00402e80
// `dynamic initializer for 'EntitySoundStream::rttiHashEntitySoundStream''
PblHash EntitySoundStream::rttiHashEntitySoundStream("EntitySoundStream");
char EntitySoundStream::rttiNameEntitySoundStream[] = "EntitySoundStream";

// GLOBAL: SWBF2 0x01ebc200
// SYNTHETIC: SWBF2 0x00402e60
// `dynamic initializer for 'EntitySoundStream::sMemoryPool''
// SYNTHETIC: SWBF2 0x00767000
// `dynamic atexit destructor for 'EntitySoundStream::sMemoryPool''
MemoryPool EntitySoundStream::sMemoryPool("EntitySoundStream", 0x10c);

// FUNCTION: SWBF2 0x004fb9b0
bool EntitySoundStream::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntitySoundStream || Entity::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004fb9e0
unsigned int EntitySoundStream::GetDerivedRtti() const
{
    return rttiHashEntitySoundStream;
}

// FUNCTION: SWBF2 0x004fb9f0
const char* EntitySoundStream::GetDerivedRttiName() const
{
    return rttiNameEntitySoundStream;
}

// FUNCTION: SWBF2 0x004fb890
Snd::SoundStream::SoundStream()
    : VoiceVirtualBus(0, 0, true),
      mProperties(0),
      mInfo(0)
{
    smList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x004fb920
Snd::SoundStream::~SoundStream()
{
    mInfo = 0;
    mProperties = 0;
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x004fba90
EntitySoundStream::EntitySoundStream(EntitySoundStreamClass* pClass, const EntityDesc& desc)
    : EntitySound(pClass, desc)
{
    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x004fbb10
EntitySoundStream::~EntitySoundStream()
{
    DeactivateThread();
}

// SYNTHETIC: SWBF2 0x004fbe08
// [thunk]:EntitySoundStream::`scalar deleting destructor' adjustor{40}
// SYNTHETIC: SWBF2 0x004fba60
// EntitySoundStream::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fbb80
void EntitySoundStream::Play()
{
    if (GameState::GetState() != 0x11E1FC01 && GameLoop::IsGameOver())
    {
        return;
    }
    if (!mSoundId)
    {
        return;
    }
    const Snd::SoundStream::Properties* pProps =
        Snd::SoundStream::Properties::FindByHashID(mSoundId);
    if (!pProps)
    {
        return;
    }

    mSoundStream.SetProperties(pProps);
    mSoundStream.Set3DMode(true);
    mSoundStream.SetPosition(&mPosition);
    if (mFlags & 1)
    {
        mSoundStream.SetMinDistance(mMinDistance);
    }
    if (mFlags & 4)
    {
        mSoundStream.SetMuteDistance(mMuteDistance);
    }
    if (mFlags & 2)
    {
        mSoundStream.SetMaxDistance(mMaxDistance);
    }
    mSoundStream.Play(true);
}

// FUNCTION: SWBF2 0x004fbc50
bool EntitySoundStream::Update(float fDeltaTime)
{
    if (mSoundStream.GetPlayState() == Snd::SourceTransport::playState_stopped &&
        mSoundStream.mProperties)
    {
        mSoundStream.Play(true);
    }
    return true;
}

// SYNTHETIC: SWBF2 0x004fb840
// Snd::VoiceVirtualBus::~VoiceVirtualBus
