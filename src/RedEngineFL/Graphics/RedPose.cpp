#include <d3dx9math.h>

#include "RedPose.h"
#include "RedSkeleton.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;
#define _IdentityMatrix (gMatrixIdentity)

// FUNCTION: SWBF2 0x006ddc70
void RedPose::BindToSkeleton(RedSkeleton const* pSkeleton) const
{
    m_pSkeleton = (RedSkeleton*)pSkeleton;
}

// FUNCTION: SWBF2 0x006ddc80
void RedPose::GetHardSkinMatrices(PblMatrix* pMatrices, unsigned char const* pIndices,
                                  unsigned int uiCount) const
{
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        pMatrices[i] = _GetBoneMatrix(pIndices[i]);
    }
}

// FUNCTION: SWBF2 0x006ddce0
void RedPose::GetSkinMatrices(PblMatrix* pMatrices, unsigned char const* pIndices,
                              unsigned int uiCount) const
{
    for (unsigned int i = 0; i < uiCount; ++i)
    {
        unsigned int uiBone = pIndices[i];
        PblMatrix const& rBone = _GetBoneMatrix(uiBone);
        if (&rBone == &_IdentityMatrix)
        {
            pMatrices[i] = m_pSkeleton->m_amDefaultPoseInv[uiBone];
        }
        else
        {
            PblMatrix mSkin;
            D3DXMatrixMultiply((D3DXMATRIX*)&mSkin,
                               (const D3DXMATRIX*)&m_pSkeleton->m_amDefaultPoseInv[uiBone],
                               (const D3DXMATRIX*)&rBone);
            pMatrices[i] = mSkin;
        }
    }
}

// FUNCTION: SWBF2 0x006ddde0
PblMatrix const& RedPose::_GetBoneMatrix(int iBone) const
{
    PblMatrix const* pMatrix = _Table.Find(m_pSkeleton->m_auiNameHash[iBone]);
    return pMatrix ? *pMatrix : _IdentityMatrix;
}

// FUNCTION: SWBF2 0x006ddb30
template <int N>
unsigned int RedPose::ConvertFromZephyrPose(const ZephyrSkeleton<N>& kSkel)
{
    unsigned int uiNumJoints = kSkel.m_pShared->m_uNumJoints;
    for (unsigned int i = 0; i < uiNumJoints; ++i)
    {
        unsigned int uiCrc = kSkel.m_pShared->m_pJoints[i].m_kCrc;
        if (_Table.Find(uiCrc) && _Table.Remove(uiCrc))
        {
            --_iNumEntries;
        }
        if (_Table.Store(uiCrc, &kSkel.m_pWorldMatrices[i]))
        {
            ++_iNumEntries;
        }
    }
    return uiNumJoints;
}

template unsigned int RedPose::ConvertFromZephyrPose<32>(const ZephyrSkeleton<32>& kSkel);
template unsigned int RedPose::ConvertFromZephyrPose<64>(const ZephyrSkeleton<64>& kSkel);

// TEMPLATE: SWBF2 0x006ddbd0
// RedPose::ConvertFromZephyrPose<64>

// FUNCTION: SWBF2 0x006ddb10
template <int N>
unsigned int RedPose::ConvertFromZephyrPose(const ZephyrPoseStatic<N>& kPose)
{
    return ConvertFromZephyrPose(*kPose.GetSkeleton());
}

template unsigned int RedPose::ConvertFromZephyrPose<32>(const ZephyrPoseStatic<32>& kPose);
