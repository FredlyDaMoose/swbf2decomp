#include <string.h>

#include "MissionLog.h"

// GLOBAL: SWBF2 0x01f9d020
int mLastPrintTime;
// GLOBAL: SWBF2 0x01f9d024
unsigned int mNumFrames;
float mFrameTimes[16];

namespace MissionLog
{

// FUNCTION: SWBF2 0x005a43e0
void Init()
{
    mLastPrintTime = 0;
    mNumFrames = 0;
    memset(mFrameTimes, 0, sizeof(mFrameTimes));
}

}
