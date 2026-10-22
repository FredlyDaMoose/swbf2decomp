#include "sndsoundproperties.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074b750
bool SoundProperties::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    switch (uiPropertyID)
    {
    case 0x5FB91E8Cu:
        pFile->Read8();
        mGroupID = pFile->Read32();
        return true;
    case 0x5FD56DF9u:
    {
        pFile->Read8();
        Bus* const pBus = Bus::FindByHashID(pFile->Read32());
        if (pBus)
        {
            mBus = pBus;
        }
        return true;
    }
    case 0x8D39BDE6u:
        pFile->Read8();
        mHashID = pFile->Read32();
        return true;
    }
    return false;
}

}
