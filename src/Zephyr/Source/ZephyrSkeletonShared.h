#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"

class ZephyrJointShared
{
public:
    PblMatrix m_kBaseInverse;
    char m_kBaseTransform[28];
    unsigned int m_kCrc;
    char m_iParent;
    char m_iChild;
    char m_iSibling;
    unsigned char mTransformFlags;
    float mScale;
};
 
 

class PblMemChunk;

class ZephyrSkeletonShared
{
public:

    struct DataOffsets
    {
        unsigned int m_uHashTable;
        unsigned int m_uJoints;
    };

    bool Open(PblMemChunk& kChunk, int iNumJoints, const DataOffsets& kOffsets, unsigned int uMemSize);
    static unsigned int GetRequiredMemSize(int iNumJoints, DataOffsets& kOffsets);

    // FUNCTION: SWBF2 0x00470930
    int GetJointIdxByCrc(unsigned int uiCrc) const { return (int)m_kHashCRC2Joint.Find(uiCrc) - 1; }

    // FUNCTION: SWBF2 0x0072ee20
    void SetJointTransformAnimationFlag(int iJoint, bool bFlag);

    void SetJointScale(int iJoint, float fScale);

    const ZephyrJointShared* GetJointByCrc(unsigned int uiCrc) const
    {
        int iJoint = (int)m_kHashCRC2Joint.Find(uiCrc) - 1;
        return iJoint < 0 ? 0 : &m_pJoints[iJoint];
    }

private:

    struct JointRelatives
    {
        unsigned int m_kParent;
        unsigned int m_kChild;
        unsigned int m_kSibling;
    };

    static unsigned int GetJointHashTableSize(int iNumJoints) { return iNumJoints * 4; }

    bool InitJointHierarchy(unsigned int kCrc, char& iDest);
    void CalcBasePoseInverses(char iJointIdx);

public:
    ZephyrJointShared* m_pJoints;
    unsigned int m_uNumJoints;

    int m_kHashCRC2JointNumEntries;
    PblHashTable<unsigned int, 64> m_kHashCRC2Joint;
};
 
 
