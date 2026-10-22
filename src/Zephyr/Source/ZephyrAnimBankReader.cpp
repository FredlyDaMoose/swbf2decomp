#include "ZephyrAnimBankReader.h"
#include "PebbleFL/Common/PblChunk.h"
#include "Battlefront2/Source/GameMemory.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include <string.h>
#include "ZephyrAnimBank.h"
#include "ZephyrCommon.h"

// FUNCTION: SWBF2 0x0072f0b0
bool ZephyrAnimBankReader::ReadFile(PblMemChunk& kChunk)
{
    const char* pcData = kChunk.GetData();
    int iSize = kChunk._iSize;
    ZephyrAnimFileHeaderD kHeader = *(const ZephyrAnimFileHeaderD*)pcData;
    pcData += sizeof(ZephyrAnimFileHeaderD);

    if (kHeader.kHeader.uiID != ZEPHYR_HEADER_ID('A', 'N', 'M', 'S'))
    {
        return false;
    }
    if (kHeader.u16MajorVersion != ZEPHYR_FILE_MAJOR_VERSION || kHeader.u16MinorVersion > ZEPHYR_FILE_MINOR_VERSION)
    {
        return false;
    }

    const unsigned int uBankSize = ZEPHYR_ALIGN(sizeof(ZephyrAnimBank), 8);
    unsigned int uDescSize, uRuntimeSize;
    unsigned int uMemSize = ZEPHYR_ALIGN(uBankSize + ZephyrAnimBank::GetRequiredMemSize(&kHeader, uDescSize, uRuntimeSize), 8);
    unsigned int uDataSize = iSize - sizeof(ZephyrAnimFileHeaderD) - uDescSize;

    m_pBank = (ZephyrAnimBank*)RedAllocFromHeap(GameMemory::RunTimeHeap, ZEPHYR_ALIGN(uBankSize + uRuntimeSize + uDataSize, 8), 0);
    if (!m_pBank)
    {
        return false;
    }

    int iTempHeap = GameMemory::TempLoadHeap;
    if (iTempHeap <= 0)
    {
        iTempHeap = GameMemory::RunTimeHeap;
    }
    char* pcDesc = (char*)RedAllocFromHeap(iTempHeap, uDescSize, 0);
    if (!pcDesc)
    {
        return false;
    }
    memcpy(pcDesc, pcData, uDescSize);

    char* pcMem = (char*)m_pBank + uBankSize;
    memcpy(pcMem + uRuntimeSize, pcData + uDescSize, iSize - uDescSize - sizeof(ZephyrAnimFileHeaderD));
    if (!m_pBank->Open(pcMem, uMemSize - uBankSize, pcDesc, &kHeader))
    {
        return false;
    }

    RedFreeToHeap(iTempHeap, pcDesc, 0);
    return true;
}
