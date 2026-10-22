#pragma once

#include <stddef.h>

#include "HUDElementGroup.h"
#include "HUDEvent.h"

namespace HUD {
class ElementGroupPlayer : public HUD::ElementGroupBase
{
public:

    // VTABLE: SWBF2 0x007a0e3c

    class Factory : public HUD::ElementGroupBase::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054f640
        Factory(unsigned int uHashID);

        // SYNTHETIC: SWBF2 0x0054f6c0
        // HUD::ElementGroupPlayer::Factory::`scalar deleting destructor'

        Property mEventPlayerIndex;
    };

    ElementGroupPlayer(RedGroupElement& group, const char* const pName,
                       Factory::ItemCallback pCallback, void* const pUserData)
        : HUD::ElementGroupBase(group, pName, pCallback, pUserData), mPlayerIndex(-1)
    {
        mEventPlayerIndex.Init(EventPlayerIndex, this);
    }

    // FUNCTION: SWBF2 0x0054f830
    virtual ~ElementGroupPlayer() { RemoveAllElements(); }

    bool SetProperty(const unsigned int, const void*);
    bool GetProperty(const unsigned int, void*);

    bool ReadData(PblConfig&, PblConfig::Data&);
    void WriteData(HUD::File&, unsigned int);

protected:

    static void EventPlayerIndex(const HUD::Event& event, void* const pData);

    virtual void PlayerIndexChanged();

public:

    int mPlayerIndex;
    EventHandler mEventPlayerIndex;
};
}

 
 
 
