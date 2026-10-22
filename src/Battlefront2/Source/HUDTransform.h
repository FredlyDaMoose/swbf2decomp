#pragma once

#include <stddef.h>
#include "HUDItem.h"
#include "HUDEvent.h"

namespace HUD {
// VTABLE: SWBF2 0x007a3590
class Transform : public HUD::Item
{
public:
    Transform(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);
    virtual ~Transform();

    virtual bool SetProperty(const unsigned int uHashID, const void* pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* pValue);
    virtual const char* PropertyTranslateEnum(const unsigned int uHashID, const unsigned int uValue);

protected:
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    bool ReadData(PblConfig& config, PblConfig::Data& data, EventClass::Type eOutputType);

public:
    static PblListSingle sList;

    static void DestroyAll();

    EventHandler mEventInput;
    EventClass* mEventClassOutput;
    PblListSingle::Node mTransformNode;
};
}
 
 
 
 
