#pragma once

#include <stddef.h>

#include "HUDItem.h"
#include "HUDEvent.h"
#include "GameSound.h"

namespace HUD {
// VTABLE: SWBF2 0x007a3548
class Sound : public HUD::Item
{
public:
    Sound(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);
    virtual ~Sound();

    virtual bool SetProperty(const unsigned int uHashID, const void* pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* pValue);
    virtual const char* PropertyTranslateEnum(const unsigned int uHashID, const unsigned int uValue);

protected:
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    bool ReadData(PblConfig& config, PblConfig::Data& data);

public:
    static PblListSingle sList;

    static void DestroyAll();

    EventHandler mEventTrigger;
    EventHandler mEventStop;
    PblListSingle::Node mSoundNode;
    GameSoundControllable mSoundControl;
    GameSound mSound;
    char* mSoundID;

    // VTABLE: SWBF2 0x007a3278

    class Factory : public HUD::Item::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x00564cc0
        // HUD::Sound::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };
};
}
 
 
 
 
 
 
 
