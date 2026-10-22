#include "HUDTransform.h"

// GLOBAL: SWBF2 0x007ebab8
// SYNTHETIC: SWBF2 0x004040b0
// `dynamic initializer for 'HUD::Transform::sList''
// SYNTHETIC: SWBF2 0x00768c20
// `dynamic atexit destructor for 'HUD::Transform::sList''
PblListSingle HUD::Transform::sList;

static const unsigned int kEventInputHash = 0xcce718ed;
static const unsigned int kEventOutputHash = 0x6903ed32;

// FUNCTION: SWBF2 0x00567200
bool HUD::Transform::SetProperty(const unsigned int, const void* const)
{
    return false;
}

// FUNCTION: SWBF2 0x00567210
bool HUD::Transform::GetProperty(const unsigned int, void* const)
{
    return false;
}

// FUNCTION: SWBF2 0x00567220
const char* HUD::Transform::PropertyTranslateEnum(const unsigned int, const unsigned int)
{
    return 0;
}

// FUNCTION: SWBF2 0x00567230
HUD::Transform::Transform(const char* const pName, Factory::ItemCallback pCallback,
                          void* const pUserData)
    : HUD::Item(pName, pCallback, pUserData)
{
    sList.AddFirst(&mTransformNode);
}

// SYNTHETIC: SWBF2 0x00567270
// HUD::Transform::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005672a0
HUD::Transform::~Transform()
{
    if (mTransformNode.mNext)
    {
        mTransformNode.Remove();
    }
}

// FUNCTION: SWBF2 0x005672e0
bool HUD::Transform::ReadData(PblConfig&, PblConfig::Data& data, EventClass::Type eOutputType)
{
    switch (data.GetId())
    {
    case kEventOutputHash:
        mEventClassOutput = Item::CreateEventA(data.GetStringArg(0), eOutputType);
        return true;
    case kEventInputHash:
        Item::ReadEvent(data, mEventInput);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00567330
void HUD::Transform::PostReadSetup()
{
}

// FUNCTION: SWBF2 0x00567340
void HUD::Transform::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("EventInput", uTabs, mEventInput.GetRegisteredClassName(true), "");
    if (mEventClassOutput)
    {
        file.WriteProperty("EventOutput", uTabs, mEventClassOutput->GetName(), "");
    }
}

// FUNCTION: SWBF2 0x00567390
void HUD::Transform::DestroyAll()
{
    while (!sList.IsEmpty())
    {
        delete (Transform*)((char*)sList.mHead.mNext - offsetof(Transform, mTransformNode));
    }
}
