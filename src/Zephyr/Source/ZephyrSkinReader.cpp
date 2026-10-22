#include "ZephyrSkinReader.h"
#include "PebbleFL/Common/PblChunk.h"
#include "ZephyrSkinShared.h"

// FUNCTION: SWBF2 0x0072efe0
bool ZephyrSkinReader::ReadFile(PblMemChunk& kChunk)
{
    const char* pcData = kChunk.GetData();
    PblMemChunk kZafChunk(pcData);
    ZephyrSkinShared::DataOffsets kOffsets;

    unsigned int uMemSize = ZephyrSkinShared::GetRequiredMemSize(kZafChunk, kOffsets);
    bool bResult = false;
    if (uMemSize)
    {
        m_pSkin = (ZephyrSkinShared*)new char[uMemSize];
        kZafChunk = PblMemChunk(pcData);
        if (m_pSkin->Open(kZafChunk, kOffsets, uMemSize))
        {
            bResult = true;
        }
    }
    return bResult;
}
