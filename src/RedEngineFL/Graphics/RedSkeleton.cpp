#include "RedSkeleton.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedPose.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

static const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

static PblMatrix* _NewAlignedMatrices(unsigned int uiCount)
{
    unsigned int uiSize = (uiCount > 0x3ffffff) ? 0xffffffff : uiCount * sizeof(PblMatrix);
    if (uiSize > 0xffffffef)
    {
        return 0;
    }
    char* p = new char[uiSize + 16];
    if (!p)
    {
        return 0;
    }
    unsigned char uPad = (unsigned char)(16 - ((unsigned int)p & 15));
    p += uPad;
    p[-1] = (char)uPad;
    return (PblMatrix*)p;
}

// GLOBAL: SWBF2 0x009a0a18
RedSkeleton* RedSkeleton::s_pCurrentSkeleton;

// GLOBAL: SWBF2 0x009a0a2c
// SYNTHETIC: SWBF2 0x00408ae0
// `dynamic initializer for 'RedSkeleton::_HashTable''

PblHashTable<RedSkeleton, 512> RedSkeleton::_HashTable;

// FUNCTION: SWBF2 0x006e87e0
PblMatrix* RedSkeleton::GetBoneMatrixByName(unsigned int uiName) const
{
    for (unsigned int i = 0; i < m_uiNumBones; ++i)
    {
        if (m_auiNameHash[i] == uiName)
        {
            return &m_amDefaultPose[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006e8820
PblMatrix* RedSkeleton::GetInvBoneMatrixByName(unsigned int uiName) const
{
    const RedSkeleton* pSkeleton = (const RedSkeleton*)this;
    for (unsigned int i = 0; i < pSkeleton->m_uiNumBones; ++i)
    {
        if (pSkeleton->m_auiNameHash[i] == uiName)
        {
            return &pSkeleton->m_amDefaultPoseInv[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006e8860
void RedSkeleton::GetSkinMatrices(PblMatrix* pMatrices, const unsigned char* pIndices,
                                  unsigned int uiCount) const
{
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        pMatrices[i] = m_amDefaultPoseInv[pIndices[i]];
    }
}

// FUNCTION: SWBF2 0x006e88c0
RedSkeleton* RedSkeleton::Read(PblFileChunk& chunk)
{
    RedSkeleton* pSkeleton = new RedSkeleton;
    pSkeleton->_Read(chunk);
    s_pCurrentSkeleton = pSkeleton;
    return pSkeleton;
}

// FUNCTION: SWBF2 0x006e8920
void RedSkeleton::SetupPose(RedPose& pose) const
{
    for (unsigned int i = 0; i < m_uiNumBones; ++i)
    {
        if (m_auiNameHash[i] && !pose._Table.Find(m_auiNameHash[i]))
        {
            if (pose._Table.Store(m_auiNameHash[i], &m_amDefaultPose[i]))
            {
                ++pose._iNumEntries;
            }
        }
    }
}

// FUNCTION: SWBF2 0x006e8990
unsigned int RedSkeleton::_PostProcess(const PblMatrix& mParent, unsigned int uiBone)
{
    unsigned int uiCount = 1;
    PblMatrix mWorld;
    D3DXMatrixMultiply(&mWorld, &m_amDefaultPose[uiBone], &mParent);
    m_amDefaultPose[uiBone] = mWorld;
    m_amDefaultPoseInv[uiBone] = m_amDefaultPose[uiBone].GetInverseFast();
    for (int iChild = m_aConnections[uiBone].iChild; iChild >= 0;
         iChild = m_aConnections[iChild].iSibling)
    {
        uiCount += _PostProcess(m_amDefaultPose[uiBone], iChild);
    }
    return uiCount;
}

// FUNCTION: SWBF2 0x006e8a90
void RedSkeleton::_Read(PblFileChunk& chunk)
{
    char acName[256];

    m_uiNumBones = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4f464e49:
        {
            child.ReadString(acName, sizeof(acName));
            m_uiNameHash = PblHash(acName);
            m_uiNumBones = child._pFile->Read32();

            if (m_uiNumBones)
            {
                m_amDefaultPose = _NewAlignedMatrices(m_uiNumBones);
                m_amDefaultPoseInv = _NewAlignedMatrices(m_uiNumBones);
                m_aConnections = new Connection[m_uiNumBones];
                m_auiNameHash = new unsigned int[m_uiNumBones];
            }
            for (unsigned int i = 0; i < m_uiNumBones; ++i)
            {
                m_aConnections[i].Reset();
            }
            break;
        }
        case 0x454d414e:
            for (unsigned int i = 0; i < m_uiNumBones; ++i)
            {
                child.ReadString(acName, sizeof(acName));
                m_auiNameHash[i] = PblTEMPHash(acName);
            }
            break;
        case 0x4d524658:
            for (unsigned int i = 0; i < m_uiNumBones; ++i)
            {
                PblMatrix m;
                m.m[0][0] = child._pFile->ReadFloat();
                m.m[0][1] = child._pFile->ReadFloat();
                m.m[0][2] = child._pFile->ReadFloat();
                m.m[1][0] = child._pFile->ReadFloat();
                m.m[1][1] = child._pFile->ReadFloat();
                m.m[1][2] = child._pFile->ReadFloat();
                m.m[2][0] = child._pFile->ReadFloat();
                m.m[2][1] = child._pFile->ReadFloat();
                m.m[2][2] = child._pFile->ReadFloat();
                m.m[3][0] = child._pFile->ReadFloat();
                m.m[3][1] = child._pFile->ReadFloat();
                m.m[3][2] = child._pFile->ReadFloat();
                m.m[0][3] = 0.0f;
                m.m[1][3] = 0.0f;
                m.m[2][3] = 0.0f;
                m.m[3][3] = 1.0f;
                m_amDefaultPose[i] = m;
            }
            break;
        case 0x544e5250:
            for (unsigned int i = 0; i < m_uiNumBones; ++i)
            {
                child.ReadString(acName, sizeof(acName));
                unsigned int uiParentHash = PblTEMPHash(acName);
                for (unsigned int j = 0; j < m_uiNumBones; ++j)
                {
                    if (m_auiNameHash[j] == uiParentHash)
                    {
                        m_aConnections[i].iParent = (char)j;
                        m_aConnections[i].iSibling = m_aConnections[j].iChild;
                        m_aConnections[j].iChild = (char)i;
                        break;
                    }
                }
            }
            break;
        }
    }
    _PostProcess(_IdentityMatrix, 0);
}
