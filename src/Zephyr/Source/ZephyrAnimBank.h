#pragma once

#include "PebbleFL/Common/PblHashTable.h"
#include "ZephyrAnim.h"

class ZephyrAnimBank
{
public:
    bool Open(char* pcMem, unsigned int uiMemSize, const char* pcData, const ZephyrAnimFileHeaderD* pkHeader);
    const ZephyrAnim* Find(unsigned int uiCrc) const;

    static unsigned int GetRequiredMemSize(const ZephyrAnimFileHeaderD* pkHeader, unsigned int& uDescSize,
                                           unsigned int& uRuntimeSize);

    int m_kHashCRC2AnimNumEntries;
    PblHashTable<const ZephyrAnim, 256> m_kHashCRC2Anim;
    ZephyrAnim* m_pkAnims;
    unsigned short m_u16NumAnims;
};
 
