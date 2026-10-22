#pragma once

#include <stddef.h>

#include "HUDElementBar.h"
#include "HUDElementBitmap.h"
#include "HUDElementGroup.h"

namespace HUD {
class ElementBarSegmented : public HUD::ElementGroup, public HUD::ElementBar
{
public:

    // VTABLE: SWBF2 0x007a0b24

    class Factory : public HUD::ElementGroup::Factory, public HUD::ElementBar::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054baf0
        Factory();

        // SYNTHETIC: SWBF2 0x0054bc60
        // HUD::ElementBarSegmented::Factory::`scalar deleting destructor'

        // SYNTHETIC: SWBF2 0x0054bc90
        // HUD::ElementBarSegmented::Factory::~Factory -- the implicit chain:

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mAngleStart;
        Property mAngleEnd;
        Property mNumSegments;
        Property mCircular;
    };

    // VTABLE: SWBF2 0x007a0a58 (`vftable'), 0x007a0b30 (`vftable'_0, the

    class Segment : public HUD::ElementBitmap
    {
    public:
        // FUNCTION: SWBF2 0x0054b8f0
        Segment();

        // SYNTHETIC: SWBF2 0x0054c6e0
        // HUD::ElementBarSegmented::Segment::`scalar deleting destructor'
        virtual ~Segment();

        virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);

        void SetGroup(HUD::ElementBarSegmented* const pGroup) { mGroup = pGroup; }

        Segment& operator=(Segment& rhs)
        {
            HUD::ElementBitmap::operator=(rhs);
            mGroup = rhs.mGroup;
            return *this;
        }

        HUD::ElementBarSegmented* mGroup;
    };

    ElementBarSegmented(const char* const pName, Factory::ItemCallback pCallback,
                        void* const pUserData);

    virtual ~ElementBarSegmented();
    // SYNTHETIC: SWBF2 0x0054be90
    // HUD::ElementBarSegmented::`scalar deleting destructor'

    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);

    virtual bool GetProperty(const unsigned int uHashID, void* pValue);

    // FUNCTION: SWBF2 0x0054c130. DECLARED ONLY: every path ends in

    virtual float SetValue(const float fValue);

    // FUNCTION: SWBF2 0x0054c4d0. DECLARED ONLY: walks mSegments calling

    void RefreshFormatting(bool bAddToGroup);

    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData);

protected:

    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);

    virtual void PostReadSetup();

    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    // FUNCTION: SWBF2 0x0054c5a0. DECLARED ONLY: reallocates mSegments

    void SetupSegments(unsigned int uNumSegments);

public:

    Segment mSegmentAll;
    Segment* mSegments;
    unsigned int mNumSegments;
    float mAngleStart;
    float mAngleEnd;
    bool mCircular : 1;
};
}

 
 
 

 
 

 
 
 
 
 
 
 
