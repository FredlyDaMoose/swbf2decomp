#pragma once

#include "ZephyrSkeletonShared.h"

class PblMemChunk;

class ZephyrJointSet
{
public:
    unsigned char m_puJoints[32];
    unsigned char m_puSkinSets[16];
    unsigned char m_uNumJoints;
    unsigned char m_uNumSkinSets;
};

 

class ZephyrSkinShared
{
public:

    class DataOffsets
    {
    public:
        DataOffsets();
        ~DataOffsets();

        ZephyrSkeletonShared::DataOffsets m_kSkeletonOffsets;
        unsigned int m_uSkeleton;
        unsigned int m_uJointSets;
        unsigned int m_uSkeletonSize;
    };

    struct SizeHeader
    {
        int iNumMaterials;
        int iNumJointSets;
        int iNumSkinSets;
        int iNumJoints;
        int iNumProxiesTotal;
        int iNumProxiesHigh;
        int iNumProxiesHighLow;
    };

    static unsigned int GetRequiredMemSize(PblMemChunk& kChunk, DataOffsets& kOffsets);
    bool Open(PblMemChunk& kChunk, const DataOffsets& kOffsets, unsigned int uMemSize);

    ZephyrSkeletonShared* GetSkeleton() { return m_pSkeleton; }

private:

     

    bool ReadJointSet(PblMemChunk& kChunk, ZephyrJointSet& kJointSet);
    static bool ReadSizeChunk(PblMemChunk& kChunk, SizeHeader& kHeader);

public:
    ZephyrSkeletonShared* m_pSkeleton;
    ZephyrJointSet* m_pJointSets;
    unsigned char m_uNumJointSets;
};

 
 
 
