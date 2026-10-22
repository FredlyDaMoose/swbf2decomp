#include "ChangeClassDisplay.h"

// GLOBAL: SWBF2 0x01e3032c
ChangeClassDisplay* ChangeClassDisplay::sInstance[1];

// FUNCTION: SWBF2 0x00450270
void ChangeClassDisplay::Create()
{
    sInstance[0] = new ChangeClassDisplay;
}

// FUNCTION: SWBF2 0x004502a0
void ChangeClassDisplay::Destroy()
{
    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
}

// FUNCTION: SWBF2 0x004502c0
ChangeClassDisplay::ChangeClassDisplay()
{
}

// FUNCTION: SWBF2 0x00450300
ChangeClassDisplay::~ChangeClassDisplay()
{
}

// FUNCTION: SWBF2 0x00450310
bool ChangeClassDisplay::Update(float fDeltaTime)
{
    return true;
}
