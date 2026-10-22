#pragma once

#include <stddef.h>

#include <stddef.h>

#include "HUDItem.h"

class RedScreenGroupElement;

class PblFileChunk;

namespace HUD {

class Editor;

namespace Manager {

// VTABLE: SWBF2 0x007a329c
class ConfigFile : public HUD::Item
{
public:

    ConfigFile(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData)
        : HUD::Item(pName, pCallback, pUserData)
    {
        mWidescreen = false;
        mWidescreenSet = false;
        mNumSupportedViewPorts = 0;
        mNumSupportedSplitModes = 0;
    }

    // SYNTHETIC: SWBF2 0x00565030
    // HUD::Manager::ConfigFile::`scalar deleting destructor'

    virtual bool SetProperty(const unsigned int uHashID, const void* pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* pValue);
    virtual const char* PropertyTranslateEnum(const unsigned int uHashID, const unsigned int uValue);

protected:
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    bool ReadData(PblConfig& config, PblConfig::Data& data);

public:
    unsigned int mNumSupportedViewPorts;
    unsigned int mSupportedViewPorts[4];
    unsigned int mNumSupportedSplitModes;
    unsigned int mSupportedSplitModes[2];
    bool mWidescreen : 1;
    bool mWidescreenSet : 1;

    bool SplitSupported() const
    {
        for (unsigned int i = 0; i < mNumSupportedSplitModes; ++i)
        {
            if (mSupportedSplitModes[i] == 1)
            {
                return true;
            }
        }
        return mNumSupportedSplitModes == 0;
    }

    bool ViewportsSupported() const;
    bool Widescreen() const { return !mWidescreenSet || mWidescreen; }

    // VTABLE: SWBF2 0x007a3284

    class Factory : public HUD::Item::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x00565000
        // HUD::Manager::ConfigFile::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };
};

// FUNCTION: SWBF2 0x00565140
void Open();

void Close();

bool Write();

void Load(PblFileChunk& chunk);

}

extern Manager::ConfigFile* gConfigFiles[256];

extern int gEditHeap;
extern RedScreenGroupElement* gScreenGroup;
extern RedScreenGroupElement* gScreenGroupEdit;
extern bool gScreenGroupEditEnabled;

}

extern HUD::Editor* gEditor;

 
 
 
 
 
