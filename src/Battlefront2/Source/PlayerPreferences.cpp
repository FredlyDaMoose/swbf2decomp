#include "PlayerProfile.h"

namespace PlayerPrefs {

// FUNCTION: SWBF2 0x0061c4b0
int __fastcall FindByViewport(int iValue)
{
    for (int i = 0; i < 1; ++i)
    {
        if (PlayerPrefs::mAllUserPrefs[i].mViewport == iValue)
        {
            return i;
        }
    }
    return -1;
}

}
