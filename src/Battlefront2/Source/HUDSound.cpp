#include "HUDSound.h"

HUD::Sound::Factory::Factory()
    : HUD::Item::Factory(0x0E0D9594u)
{
    mName = "Sound";
}

// GLOBAL: SWBF2 0x007eba9c
// SYNTHETIC: SWBF2 0x004040a0
// `dynamic initializer for 'HUD::Sound::sList''
// SYNTHETIC: SWBF2 0x00768c10
// `dynamic atexit destructor for 'HUD::Sound::sList''
PblListSingle HUD::Sound::sList;

static const unsigned int kSoundHash = 0x0e0d9594;
static const unsigned int kEventTriggerHash = 0x4cb6be21;
static const unsigned int kEventStopHash = 0xc061c1e7;

// FUNCTION: SWBF2 0x00566f60
bool HUD::Sound::SetProperty(const unsigned int, const void* const)
{
    return false;
}

// FUNCTION: SWBF2 0x00566f70
bool HUD::Sound::GetProperty(const unsigned int, void* const)
{
    return false;
}

// FUNCTION: SWBF2 0x00566f80
const char* HUD::Sound::PropertyTranslateEnum(const unsigned int, const unsigned int)
{
    return 0;
}

// FUNCTION: SWBF2 0x00566f90
HUD::Sound::Sound(const char* const, Factory::ItemCallback, void* const)
    : HUD::Item(0, 0, 0)
    , mSoundID(0)
{
    sList.AddLast(&mSoundNode);
}

// SYNTHETIC: SWBF2 0x00567000
// HUD::Sound::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00567030
HUD::Sound::~Sound()
{
    HUD::FreeName(&mSoundID);
    if (mSoundNode.mNext)
    {
        mSoundNode.Remove();
    }
}

// FUNCTION: SWBF2 0x005670c0
bool HUD::Sound::ReadData(PblConfig&, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kSoundHash:
        HUD::FreeName(&mSoundID);
        HUD::CopyName(&mSoundID, data.GetStringArg(0), 0);
        mSound.SetID(data.GetStringArg(0), false, true);
        return true;
    case kEventTriggerHash:
        Item::ReadEvent(data, mEventTrigger);
        return true;
    case kEventStopHash:
        Item::ReadEvent(data, mEventStop);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00567150
void HUD::Sound::PostReadSetup()
{
}

// FUNCTION: SWBF2 0x00567160
void HUD::Sound::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("EventTrigger", uTabs, mEventTrigger.GetRegisteredClassName(true), "");
    file.WriteProperty("EventStop", uTabs, mEventStop.GetRegisteredClassName(true), "");
    file.WriteProperty("Sound", uTabs, mSoundID, "");
}

// FUNCTION: SWBF2 0x005671d0
void HUD::Sound::DestroyAll()
{
    while (!sList.IsEmpty())
    {
        delete (Sound*)((char*)sList.mHead.mNext - offsetof(Sound, mSoundNode));
    }
}
