#pragma once

#include <stddef.h>

#include "GameSound.h"

class PlayableEntitySoundsClass
{
public:

    enum Command
    {
        command_fieldMoveOut = 0,
        command_fieldHold = 1,
        command_fieldFollow = 2,
        command_driverGetIn = 3,
        command_driverGetOut = 4,
        command_passengerMoveOut = 5,
        command_passengerStop = 6,
        command_passengerGetIn = 7,
        command_passengerGetOut = 8,
        command_gunnerAllClear = 9,
        command_gunnerSteady = 10,
        command_gunnerGetIn = 11,
        command_gunnerGetOut = 12,
        command_count = 13
    };

    PlayableEntitySoundsClass();

    bool SetProperty(unsigned int uProperty, const char* pszValue);

    PlayableEntitySoundsClass(const PlayableEntitySoundsClass* pBase);
    ~PlayableEntitySoundsClass();

    GameSound mAICommands[13];
    GameSound mPlayerCommands[13];
    GameSound mSCResponseYessirSound;
    GameSound mSCResponseNosirSound;
};
 
 
 
 
 

class PlayableEntitySounds
{
public:

    PlayableEntitySounds(PlayableEntitySoundsClass* pClass);

    // SYNTHETIC: SWBF2 0x00619b30
    // PlayableEntitySounds::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x00619b80
    virtual ~PlayableEntitySounds() {}

    // FUNCTION: SWBF2 0x0049e2f0
    virtual const PlayableEntitySoundsClass* GetPlayableEntitySoundsClass() const { return mPESClass; }

    // FUNCTION: SWBF2 0x0049e300
    virtual PlayableEntitySoundsClass* GetPlayableEntitySoundsClass() { return mPESClass; }

    PlayableEntitySoundsClass* mPESClass;
};
 
