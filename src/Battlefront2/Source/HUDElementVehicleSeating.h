#pragma once

#include <stddef.h>

#include "HUDElementGroupPlayer.h"
#include "HUDElementModel3D.h"
#include "HUDElementText.h"
#include "RedEngineFL/Interface/RedGroupElement.h"

class Character;
class GameObject;

namespace HUD {
class ElementVehicleSeating : public HUD::ElementGroupPlayer, public RedGroupElement
{
public:

    static PblListSingle sSeatingDisplays;

    static const char* const sMarkerNames[4];
    static const char* const sMarkerTextNames[4];

    // VTABLE: SWBF2 0x007a1c34

    class Factory : public HUD::ElementGroupPlayer::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0055ca40
        Factory();

        // SYNTHETIC: SWBF2 0x0055ca60
        // HUD::ElementVehicleSeating::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    // VTABLE: SWBF2 0x007a1c40
    class SeatingMarker : public HUD::ElementModel3D
    {
    public:

        SeatingMarker(const char* const pName = 0) : HUD::ElementModel3D(pName) {}

        // SYNTHETIC: SWBF2 0x0055cde0
        // HUD::ElementVehicleSeating::SeatingMarker::`default constructor closure'

        virtual ~SeatingMarker();
        virtual bool SetProperty(const unsigned int uHashID, const void* pValue);

        SeatingMarker& operator=(const SeatingMarker& other)
        {
            HUD::ElementModel3D::operator=(other);
            return *this;
        }

        ElementVehicleSeating* mSeatingDisplay;
    };

    // VTABLE: SWBF2 0x007a1d0c
    class SeatingText : public HUD::ElementText
    {
    public:

        SeatingText(const char* const pName = 0) : HUD::ElementText(pName) {}

        // SYNTHETIC: SWBF2 0x0055ce00
        // HUD::ElementVehicleSeating::SeatingText::`default constructor closure'

        virtual ~SeatingText();
        virtual bool SetProperty(const unsigned int uHashID, const void* pValue);

        ElementVehicleSeating* mSeatingDisplay;
    };

    enum SeatingState
    {
        seatingState_None   = 0,
        seatingState_Empty  = 1,
        seatingState_Self   = 2,
        seatingState_Player = 3,
        seatingState_AI     = 4,
        seatingState_MAX    = 5
    };

    ElementVehicleSeating(const char* const pName, Factory::ItemCallback pCallback,
                          void* const pUserData);

    virtual ~ElementVehicleSeating();
    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback,
                      void* const pUserData);
    virtual bool SetProperty(const unsigned int, const void*);
    virtual bool GetProperty(const unsigned int, void*);
    virtual bool ReadData(PblConfig&, PblConfig::Data&);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File&, unsigned int);
    virtual void Update(const float);

    virtual void* GetParent();

    void Clear();

    static const unsigned int sHPNames[16];

protected:

    void UpdateSeatingState(unsigned int uSeat, SeatingState eState, const unsigned short* const pText);
    unsigned int UpdateSeatingMarkers(const Character* const pCharacter, const GameObject* const pObject);

public:

    static ElementVehicleSeating* FindByPlayerIndex(int iPlayerIndex);

    bool GetSeatingDisplayChanged()
    {
        const bool bChanged = mChanged;
        mChanged = false;
        return bChanged;
    }

    SeatingMarker mMarkerTemplates[4];
    SeatingText mMarkerTextTemplate[4];
    SeatingMarker mSeatingMarkers[10];
    SeatingText mSeatingText[10];
    SeatingState mSeatingState[10];
    SeatingMarker mBackdrop;
    PblListSingle::Node mNode;
    bool mChanged : 1;
};
}

 
 
 
 
 
 
 
 
 
 
 
 
 
