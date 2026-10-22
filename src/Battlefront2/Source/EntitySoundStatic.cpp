#include "EntitySoundStatic.h"
#include "SoundFL/source/sndvoicevirtual.h"

// GLOBAL: SWBF2 0x01ebc1a4
// SYNTHETIC: SWBF2 0x00402e50
// `dynamic initializer for 'EntitySoundStaticClass::rttiHashEntitySoundStaticClass''
PblHash EntitySoundStaticClass::rttiHashEntitySoundStaticClass("EntitySoundStaticClass");
char EntitySoundStaticClass::rttiNameEntitySoundStaticClass[] = "EntitySoundStaticClass";

// FUNCTION: SWBF2 0x004fb6b0
EntitySoundStaticClass::EntitySoundStaticClass(unsigned int uHash)
    : EntitySoundClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fb710
EntitySoundStaticClass::EntitySoundStaticClass(EntitySoundStaticClass* pBase,
                                               unsigned int uHash)
    : EntitySoundClass(pBase, uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }
}

// FUNCTION: SWBF2 0x004fb750
EntitySoundStaticClass::~EntitySoundStaticClass()
{
}

// SYNTHETIC: SWBF2 0x004fb6e0
// EntitySoundStaticClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004fb760
EntityClass* EntitySoundStaticClass::Derive(unsigned int uHash)
{
    return new EntitySoundStaticClass(this, uHash);
}

// FUNCTION: SWBF2 0x004fb480
bool EntitySoundStaticClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntitySoundStaticClass || EntitySoundClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004fb7d0
Entity* EntitySoundStaticClass::Build(const EntityDesc& desc)
{
    return new EntitySoundStatic(this, desc);
}

// FUNCTION: SWBF2 0x004fb4b0
unsigned int EntitySoundStaticClass::GetDerivedRtti()
{
    return rttiHashEntitySoundStaticClass;
}

// FUNCTION: SWBF2 0x004fb4c0
const char* EntitySoundStaticClass::GetDerivedRttiName()
{
    return rttiNameEntitySoundStaticClass;
}

// GLOBAL: SWBF2 0x01ebc1a8
// SYNTHETIC: SWBF2 0x00402e20
// `dynamic initializer for 'EntitySoundStatic::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766ff0
// `dynamic atexit destructor for 'EntitySoundStatic::sMemoryPool''
MemoryPool EntitySoundStatic::sMemoryPool("EntitySoundStatic", 0x4c);

// GLOBAL: SWBF2 0x01ebc1f8
// SYNTHETIC: SWBF2 0x00402e40
// `dynamic initializer for 'EntitySoundStatic::rttiHashEntitySoundStatic''
PblHash EntitySoundStatic::rttiHashEntitySoundStatic("EntitySoundStatic");
char EntitySoundStatic::rttiNameEntitySoundStatic[] = "EntitySoundStatic";

// FUNCTION: SWBF2 0x004fb420
bool EntitySoundStatic::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntitySoundStatic || Entity::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004fb450
unsigned int EntitySoundStatic::GetDerivedRtti() const
{
    return rttiHashEntitySoundStatic;
}

// FUNCTION: SWBF2 0x004fb460
const char* EntitySoundStatic::GetDerivedRttiName() const
{
    return rttiNameEntitySoundStatic;
}

void* EntitySoundStatic::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

// FUNCTION: SWBF2 0x004fb500
EntitySoundStatic::EntitySoundStatic(EntitySoundStaticClass* pClass, const EntityDesc& desc)
    : EntitySound(pClass, desc)
{
    ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x004fb4d0
// EntitySoundStatic::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x004fb835
// [thunk]:EntitySoundStatic::`vector deleting destructor'

// FUNCTION: SWBF2 0x004fb580
EntitySoundStatic::~EntitySoundStatic()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x004fb5f0
void EntitySoundStatic::Play()
{
    if (mSoundId)
    {
        mSoundProperties.SetID(mSoundId, false, false);
        mSound.Play(&mSoundProperties, &mPosition, 0, true);
    }
}

// FUNCTION: SWBF2 0x004fb620
bool EntitySoundStatic::Update(float fDeltaTime)
{
    mSound.Update(&mSoundProperties, &mPosition, 0);
    Snd::VoiceVirtual* pVoice = mSound.GetVoiceVirtual();
    if (pVoice)
    {
        if (mFlags & 1)
        {
            pVoice->SetMinDistance(mMinDistance);
        }
        if (mFlags & 4)
        {
            pVoice->SetMuteDistance(mMuteDistance);
        }
        if (mFlags & 2)
        {
            pVoice->SetMaxDistance(mMaxDistance);
        }
    }
    if (!(mSound.mFlags & GameSoundControllable::flags_playing))
    {
        mSound.Play(&mSoundProperties, &mPosition, 0, true);
    }
    return mSoundProperties.GetProperties() != 0;
}
