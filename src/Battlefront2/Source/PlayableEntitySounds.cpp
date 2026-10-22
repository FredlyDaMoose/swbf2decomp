#include "PlayableEntitySounds.h"

// FUNCTION: SWBF2 0x00619b60
PlayableEntitySounds::PlayableEntitySounds(PlayableEntitySoundsClass* pClass)
    : mPESClass(pClass)
{
}

// FUNCTION: SWBF2 0x00619b90
PlayableEntitySoundsClass::PlayableEntitySoundsClass()
{
}

// FUNCTION: SWBF2 0x00619c10
PlayableEntitySoundsClass::PlayableEntitySoundsClass(const PlayableEntitySoundsClass* pBase)
{
    for (int i = 0; i < command_count; ++i)
    {
        mAICommands[i] = pBase->mAICommands[i];
        mPlayerCommands[i] = pBase->mPlayerCommands[i];
    }
    mSCResponseYessirSound = pBase->mSCResponseYessirSound;
    mSCResponseNosirSound = pBase->mSCResponseNosirSound;
}

// FUNCTION: SWBF2 0x00619d00
bool PlayableEntitySoundsClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    GameSound* pSound;
    switch (uProperty)
    {
    case 0x150a38e9: pSound = &mAICommands[command_fieldMoveOut]; break;
    case 0x8018b09d: pSound = &mAICommands[command_fieldHold]; break;
    case 0x1d988641: pSound = &mAICommands[command_fieldFollow]; break;
    case 0xe0999151: pSound = &mAICommands[command_driverGetIn]; break;
    case 0x3b62b992: pSound = &mAICommands[command_driverGetOut]; break;
    case 0x2512bb6d: pSound = &mAICommands[command_passengerMoveOut]; break;
    case 0xa473e5ea: pSound = &mAICommands[command_passengerStop]; break;
    case 0xf9d92705: pSound = &mAICommands[command_passengerGetIn]; break;
    case 0x45912fae: pSound = &mAICommands[command_passengerGetOut]; break;
    case 0x22987695: pSound = &mAICommands[command_gunnerAllClear]; break;
    case 0x1aec5f13: pSound = &mAICommands[command_gunnerSteady]; break;
    case 0x7ef094d0: pSound = &mAICommands[command_gunnerGetIn]; break;
    case 0xdf526f61: pSound = &mAICommands[command_gunnerGetOut]; break;

    case 0xe0e7aa4f: pSound = &mPlayerCommands[command_fieldMoveOut]; break;
    case 0xb2a1a19f: pSound = &mPlayerCommands[command_fieldHold]; break;
    case 0x491b3917: pSound = &mPlayerCommands[command_fieldFollow]; break;
    case 0x47041baf: pSound = &mPlayerCommands[command_driverGetIn]; break;
    case 0x4efe8d94: pSound = &mPlayerCommands[command_driverGetOut]; break;
    case 0x5b7c85cb: pSound = &mPlayerCommands[command_passengerMoveOut]; break;
    case 0x5d42f4c8: pSound = &mPlayerCommands[command_passengerStop]; break;
    case 0x6c89065f: pSound = &mPlayerCommands[command_passengerGetIn]; break;
    case 0xf260f2a4: pSound = &mPlayerCommands[command_passengerGetOut]; break;
    case 0xa3d45943: pSound = &mPlayerCommands[command_gunnerAllClear]; break;
    case 0x981f3d5d: pSound = &mPlayerCommands[command_gunnerSteady]; break;
    case 0xd1f9b472: pSound = &mPlayerCommands[command_gunnerGetIn]; break;
    case 0xc27c316f: pSound = &mPlayerCommands[command_gunnerGetOut]; break;

    case 0x8e131444: pSound = &mSCResponseYessirSound; break;
    case 0x60681218: pSound = &mSCResponseNosirSound; break;

    default:
        return false;
    }
    pSound->SetID(pszValue, false, false);
    return true;
}
