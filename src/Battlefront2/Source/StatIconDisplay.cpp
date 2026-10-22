#include "StatIconDisplay.h"

// FUNCTION: SWBF2 0x00650810
void StatIconDisplay::Create()
{
    sInstance[0] = new StatIconDisplay;
}

// FUNCTION: SWBF2 0x00650860
StatIconDisplay::StatIconDisplay()
{
}

// FUNCTION: SWBF2 0x006508a0
StatIconDisplay::~StatIconDisplay()
{
}

// FUNCTION: SWBF2 0x006508b0
bool StatIconDisplay::Update(float fDeltaTime)
{
    return true;
}

// FUNCTION: SWBF2 0x00650840
void StatIconDisplay::Destroy()
{
    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
}

// GLOBAL: SWBF2 0x01eafb48
StatIconDisplay* StatIconDisplay::sInstance[1];
