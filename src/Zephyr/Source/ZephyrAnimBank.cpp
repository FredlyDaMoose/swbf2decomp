#include "ZephyrAnimBank.h"
#include "ZephyrAnimJoint.h"

// FUNCTION: SWBF2 0x0072bac0
bool ZephyrAnimBank::Open(char* pcMem, unsigned int uiMemSize, const char* pcData, const ZephyrAnimFileHeaderD* pkHeader)
{
    m_u16NumAnims = pkHeader->u16NumAnims;

    const ZephyrAnimBlockHeader* pkBlock = (const ZephyrAnimBlockHeader*)pcData;
    if (pkBlock->uiID != 'ANIM')
    {
        return false;
    }
    const ZephyrAnimD* pkAnimD = (const ZephyrAnimD*)(pkBlock + 1);

    pkBlock = (const ZephyrAnimBlockHeader*)(pkAnimD + m_u16NumAnims);
    if (pkBlock->uiID != 'AJNT')
    {
        return false;
    }
    const ZephyrAnimJointD* pkJointD = (const ZephyrAnimJointD*)(pkBlock + 1);

    pkBlock = (const ZephyrAnimBlockHeader*)((const char*)pkJointD + 48 * pkHeader->u32NumTotalJoints);
    if (pkBlock->uiID != 'ADAT')
    {
        return false;
    }

    m_pkAnims = (ZephyrAnim*)pcMem;
    char* pcJoints = (char*)(m_pkAnims + m_u16NumAnims);
    char* pcJointData = pcJoints + 56 * pkHeader->u32NumTotalJoints;

    m_kHashCRC2Anim.Clear();
    m_kHashCRC2AnimNumEntries = 0;

    ZephyrAnim* pkAnim = m_pkAnims;
    for (int i = 0; i < m_u16NumAnims; ++i)
    {
        if (!pkAnim->Open(pkAnimD, pkJointD, (ZephyrAnimJoint*)pcJoints, pcJointData))
        {
            return false;
        }
        if (m_kHashCRC2Anim.Store(pkAnim->GetNameCRC(), pkAnim))
        {
            ++m_kHashCRC2AnimNumEntries;
        }

        unsigned short u16NumJoints = pkAnimD->u16NumJoints;
        ++pkAnim;
        ++pkAnimD;
        pkJointD = (const ZephyrAnimJointD*)((const char*)pkJointD + 48 * u16NumJoints);
        pcJoints += 56 * u16NumJoints;
    }
    return true;
}

// FUNCTION: SWBF2 0x0072ba40
const ZephyrAnim* ZephyrAnimBank::Find(unsigned int uiCrc) const
{
    return m_kHashCRC2Anim.Find(uiCrc);
}

// FUNCTION: SWBF2 0x0072ba60
unsigned int ZephyrAnimBank::GetRequiredMemSize(const ZephyrAnimFileHeaderD* pkHeader, unsigned int& uDescSize,
                                                unsigned int& uRuntimeSize)
{
    uRuntimeSize = pkHeader->u32NumTotalJoints * sizeof(ZephyrAnimJoint) + pkHeader->u16NumAnims * sizeof(ZephyrAnim);
    unsigned int uSize = uRuntimeSize + pkHeader->u32DatChunkSize;

    uDescSize = sizeof(ZephyrAnimBlockHeader);
    uDescSize += pkHeader->u16NumAnims * sizeof(ZephyrAnimD) + sizeof(ZephyrAnimBlockHeader);
    uDescSize += pkHeader->u32NumTotalJoints * sizeof(ZephyrAnimJointD) + sizeof(ZephyrAnimBlockHeader);
    return uSize;
}
