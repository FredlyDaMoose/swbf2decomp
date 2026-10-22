#include "ZephyrSkeletonShared.h"
#include "ZephyrCommon.h"
#include "ZephyrFile.h"
#include "PebbleFL/Common/PblChunk.h"
#include <d3dx9math.h>

// FUNCTION: SWBF2 0x0072ee20
void ZephyrSkeletonShared::SetJointTransformAnimationFlag(int iJoint, bool bFlag)
{
    ZephyrJointShared* pJoint = &m_pJoints[iJoint];
    if (pJoint)
    {
        if (bFlag)
        {
            pJoint->mTransformFlags |= 1;
        }
        else
        {
            pJoint->mTransformFlags &= ~1;
        }
    }
}

// FUNCTION: SWBF2 0x0072ede0
void ZephyrSkeletonShared::SetJointScale(int iJoint, float fScale)
{
    ZephyrJointShared* pJoint = &m_pJoints[iJoint];
    if (pJoint)
    {
        if (fScale != 1.0f)
        {
            pJoint->mTransformFlags |= 2;
        }
        else
        {
            pJoint->mTransformFlags &= ~2;
        }
        pJoint->mScale = fScale;
    }
}

// FUNCTION: SWBF2 0x0072e780
void ZephyrSkeletonShared::CalcBasePoseInverses(char iJointIdx)
{
    ZephyrJointShared& kJoint = m_pJoints[iJointIdx];
    const ZephyrTransform& kBase = *(const ZephyrTransform*)kJoint.m_kBaseTransform;
    PblMatrix& mInverse = kJoint.m_kBaseInverse;

    D3DXQUATERNION qUnit;
    PblMatrix mLocal;
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mLocal,
                                 D3DXQuaternionNormalize(&qUnit, (const D3DXQUATERNION*)&kBase.kQuat));
    mLocal.m[3][0] = kBase.kTrans.x;
    mLocal.m[3][1] = kBase.kTrans.y;
    mLocal.m[3][2] = kBase.kTrans.z;
    mLocal.m[3][3] = 1.0f;
    mInverse = mLocal;

    if (kJoint.m_iParent >= 0)
    {
        PblMatrix mWorld;
        D3DXMatrixMultiply((D3DXMATRIX*)&mWorld, (const D3DXMATRIX*)&mInverse,
                           (const D3DXMATRIX*)&m_pJoints[kJoint.m_iParent].m_kBaseInverse);
        mInverse = mWorld;
    }

    if (kJoint.m_iSibling >= 0)
    {
        CalcBasePoseInverses(kJoint.m_iSibling);
    }

    if (kJoint.m_iChild >= 0)
    {
        CalcBasePoseInverses(kJoint.m_iChild);
    }

    if (kJoint.m_iParent < 0)
    {
        for (unsigned int i = 0; i < m_uNumJoints; i++)
        {
            m_pJoints[i].m_kBaseInverse = m_pJoints[i].m_kBaseInverse.GetInverseFast();
        }
    }
}

// FUNCTION: SWBF2 0x0072e990
unsigned int ZephyrSkeletonShared::GetRequiredMemSize(int iNumJoints, DataOffsets& kOffsets)
{
    unsigned int uSize = sizeof(ZephyrSkeletonShared);

    uSize = ZEPHYR_ALIGN(uSize, 4);
    kOffsets.m_uHashTable = uSize;
    uSize += GetJointHashTableSize(iNumJoints) * sizeof(unsigned int);

    uSize = ZEPHYR_ALIGN(uSize, 16);
    kOffsets.m_uJoints = uSize;
    uSize += iNumJoints * sizeof(ZephyrJointShared);

    return uSize;
}

// FUNCTION: SWBF2 0x0072e9c0
bool ZephyrSkeletonShared::Open(PblMemChunk& kChunk, int iNumJoints, const DataOffsets& kOffsets, unsigned int uMemSize)
{
    unsigned int uNumRoots = 0;

    if (!CheckChunkId(kChunk, "SKEL"))
    {
        return false;
    }

    m_pJoints = (ZephyrJointShared*)((char*)this + kOffsets.m_uJoints);
    m_uNumJoints = (unsigned int)iNumJoints;

    m_kHashCRC2Joint.Clear();
    m_kHashCRC2JointNumEntries = 0;

    JointRelatives* pRel = new JointRelatives[iNumJoints];

    int i;
    for (i = 0; i < iNumJoints; i++)
    {
        PblMemChunk kChunkJoint = kChunk.ReadNextChild();
        if (!CheckChunkId(kChunkJoint, "JONT"))
        {
            return false;
        }

        ZephyrJointShared& kJoint = m_pJoints[i];
        ZephyrTransform& kBase = *(ZephyrTransform*)kJoint.m_kBaseTransform;

        kJoint.m_kCrc = (unsigned int)kChunkJoint.Read32();

        float pRot[4];
        for (int j = 0; j < 4; j++)
        {
            pRot[j] = kChunkJoint.ReadFloat();
        }
        kBase.kQuat = *(const PblQuaternion*)pRot;

        float pTrans[3];
        for (int j = 0; j < 3; j++)
        {
            pTrans[j] = kChunkJoint.ReadFloat();
        }
        kBase.kTrans.x = pTrans[0];
        kBase.kTrans.y = pTrans[1];
        kBase.kTrans.z = pTrans[2];

        kJoint.mTransformFlags = 1;

        pRel[i].m_kParent = (unsigned int)kChunkJoint.Read32();
        pRel[i].m_kChild = (unsigned int)kChunkJoint.Read32();
        pRel[i].m_kSibling = (unsigned int)kChunkJoint.Read32();

        if (!pRel[i].m_kParent)
        {
            uNumRoots++;
            if (i)
            {
                JointRelatives kRel = pRel[0];
                pRel[0] = pRel[i];
                pRel[i] = kRel;

                ZephyrJointShared kTemp = m_pJoints[0];
                m_pJoints[0] = m_pJoints[i];
                m_pJoints[i] = kTemp;
            }
        }
    }

    for (i = 0; i < iNumJoints; i++)
    {
        if (m_kHashCRC2Joint.Store(m_pJoints[i].m_kCrc, (unsigned int*)(i + 1)))
        {
            ++m_kHashCRC2JointNumEntries;
        }
    }

    if (uNumRoots != 1)
    {
        return false;
    }

    if (pRel[0].m_kParent)
    {
        return false;
    }

    for (i = 0; i < iNumJoints; i++)
    {
        ZephyrJointShared& kJoint = m_pJoints[i];
        if (!InitJointHierarchy(pRel[i].m_kParent, kJoint.m_iParent))
        {
            return false;
        }
        if (!InitJointHierarchy(pRel[i].m_kChild, kJoint.m_iChild))
        {
            return false;
        }
        if (!InitJointHierarchy(pRel[i].m_kSibling, kJoint.m_iSibling))
        {
            return false;
        }
    }

    CalcBasePoseInverses(0);

    delete pRel;

    return true;
}

bool ZephyrSkeletonShared::InitJointHierarchy(unsigned int kCrc, char& iDest)
{
    if (kCrc)
    {
        iDest = (char)GetJointIdxByCrc(kCrc);
        if (iDest < 0)
        {
            return false;
        }
    }
    else
    {
        iDest = -1;
    }
    return true;
}
