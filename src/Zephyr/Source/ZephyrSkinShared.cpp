#include "PebbleFL/Common/PblChunk.h"
#include "ZephyrCommon.h"
#include "ZephyrFile.h"
#include "ZephyrSkinShared.h"

// FUNCTION: SWBF2 0x0072f610
bool CheckChunkId(PblMemChunk& kChunk, const char* pcID)
{
    return kChunk.GetId() == ZEPHYR_HEADER_ID(pcID[0], pcID[1], pcID[2], pcID[3]);
}

// FUNCTION: SWBF2 0x0072f640
ZephyrSkinShared::DataOffsets::DataOffsets()
{
}

// FUNCTION: SWBF2 0x0072f650
ZephyrSkinShared::DataOffsets::~DataOffsets()
{
}

// FUNCTION: SWBF2 0x0072fab0
bool ZephyrSkinShared::ReadSizeChunk(PblMemChunk& kChunk, SizeHeader& kHeader)
{
    if (!CheckChunkId(kChunk, "SIZE"))
    {
        return false;
    }

    kHeader.iNumMaterials = kChunk.Read32();
    kHeader.iNumJointSets = kChunk.Read32();
    kHeader.iNumSkinSets = kChunk.Read32();
    kHeader.iNumJoints = kChunk.Read32();
    kHeader.iNumProxiesTotal = kChunk.Read32();
    kHeader.iNumProxiesHigh = kChunk.Read32();
    kHeader.iNumProxiesHighLow = kChunk.Read32();
    return kHeader.iNumJoints > 0;
}

// FUNCTION: SWBF2 0x0072f660
unsigned int ZephyrSkinShared::GetRequiredMemSize(PblMemChunk& kChunk, DataOffsets& kOffsets)
{
    if (!CheckChunkId(kChunk, "ZAFF"))
    {
        return 0;
    }

    PblMemChunk kVersion = kChunk.ReadNextChild();
    if (!CheckChunkId(kVersion, "VERS"))
    {
        return 0;
    }
    unsigned short u16Major = kVersion.Read16();
    unsigned short u16Minor = kVersion.Read16();
    if (u16Major != ZEPHYR_FILE_MAJOR_VERSION || u16Minor > ZEPHYR_FILE_MINOR_VERSION)
    {
        return 0;
    }

    PblMemChunk kSize = kChunk.ReadNextChild();
    SizeHeader kHeader;
    if (!ReadSizeChunk(kSize, kHeader))
    {
        return 0;
    }

    kChunk.ReadNextChild();
    kChunk.ReadNextChild();

    kOffsets.m_uSkeleton = ZEPHYR_ALIGN(sizeof(ZephyrSkinShared), 16);
    kOffsets.m_uSkeletonSize = ZephyrSkeletonShared::GetRequiredMemSize(kHeader.iNumJoints, kOffsets.m_kSkeletonOffsets);
    if (!kOffsets.m_uSkeletonSize)
    {
        return 0;
    }
    kOffsets.m_uJointSets = ZEPHYR_ALIGN(kOffsets.m_uSkeleton + kOffsets.m_uSkeletonSize, 4);
    unsigned int uSize = ZEPHYR_ALIGN(kOffsets.m_uJointSets + kHeader.iNumJointSets * sizeof(ZephyrJointSet), 4)
                         + kHeader.iNumSkinSets * 4;

    PblMemChunk kSkins = kChunk.ReadNextChild();
    if (!CheckChunkId(kSkins, "SKNS"))
    {
        return 0;
    }
    for (int i = 0; i < kHeader.iNumSkinSets; i++)
    {
        kSkins.ReadNextChild();
    }
    return uSize;
}

// FUNCTION: SWBF2 0x0072f7f0
bool ZephyrSkinShared::Open(PblMemChunk& kChunk, const DataOffsets& kOffsets, unsigned int uMemSize)
{
    if (!CheckChunkId(kChunk, "ZAFF"))
    {
        return false;
    }

    PblMemChunk kVersion = kChunk.ReadNextChild();
    if (!CheckChunkId(kVersion, "VERS"))
    {
        return false;
    }
    unsigned short u16Major = kVersion.Read16();
    unsigned short u16Minor = kVersion.Read16();
    if (u16Major != ZEPHYR_FILE_MAJOR_VERSION || u16Minor > ZEPHYR_FILE_MINOR_VERSION)
    {
        return false;
    }

    PblMemChunk kSize = kChunk.ReadNextChild();
    SizeHeader kHeader;
    if (!ReadSizeChunk(kSize, kHeader))
    {
        return false;
    }

    PblMemChunk kMaterials = kChunk.ReadNextChild();
    if (!CheckChunkId(kMaterials, "MTLS"))
    {
        return false;
    }
    for (int i = 0; i < kHeader.iNumMaterials; i++)
    {
        kMaterials.ReadNextChild();
    }

    m_pJointSets = (ZephyrJointSet*)((char*)this + kOffsets.m_uJointSets);
    m_uNumJointSets = (unsigned char)kHeader.iNumJointSets;

    PblMemChunk kJointSets = kChunk.ReadNextChild();
    if (!CheckChunkId(kJointSets, "JSTS"))
    {
        return false;
    }
    for (int i = 0; i < kHeader.iNumJointSets; i++)
    {
        PblMemChunk kJointSet = kJointSets.ReadNextChild();
        if (!ReadJointSet(kJointSet, m_pJointSets[i]))
        {
            return false;
        }
    }

    PblMemChunk kSkins = kChunk.ReadNextChild();
    if (!CheckChunkId(kSkins, "SKNS"))
    {
        return false;
    }
    for (int i = 0; i < kHeader.iNumSkinSets; i++)
    {
        kSkins.ReadNextChild();
    }

    m_pSkeleton = (ZephyrSkeletonShared*)((char*)this + kOffsets.m_uSkeleton);
    PblMemChunk kSkeleton = kChunk.ReadNextChild();
    return m_pSkeleton->Open(kSkeleton, kHeader.iNumJoints, kOffsets.m_kSkeletonOffsets, kOffsets.m_uSkeletonSize);
}

// FUNCTION: SWBF2 0x0072fa30
bool ZephyrSkinShared::ReadJointSet(PblMemChunk& kChunk, ZephyrJointSet& kJointSet)
{
    if (!CheckChunkId(kChunk, "JSET"))
    {
        return false;
    }

    kJointSet.m_uNumJoints = (unsigned char)kChunk.Read32();
    for (int i = 0; i < kJointSet.m_uNumJoints; i++)
    {
        kJointSet.m_puJoints[i] = (unsigned char)kChunk.Read32();
    }

    kJointSet.m_uNumSkinSets = (unsigned char)kChunk.Read32();
    for (int i = 0; i < kJointSet.m_uNumSkinSets; i++)
    {
        kJointSet.m_puSkinSets[i] = (unsigned char)kChunk.Read32();
    }

    return true;
}
