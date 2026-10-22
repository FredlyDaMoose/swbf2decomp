#include "PblHandle.h"

// GLOBAL: SWBF2 0x007e2c78
unsigned int sNextHandleId = 1;

// FUNCTION: SWBF2 0x00727d20
PblHandled::PblHandled()
{
    mHandleId = sNextHandleId++;
    if (sNextHandleId == 0)
    {
        sNextHandleId = 1;
    }
}

// FUNCTION: SWBF2 0x00727d40
unsigned int PblHandled::SetNextHandleId(unsigned int uId)
{
    unsigned int uOld = sNextHandleId;
    sNextHandleId = uId;
    return uOld;
}
