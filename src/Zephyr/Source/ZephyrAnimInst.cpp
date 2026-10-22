#include "ZephyrAnimInst.h"

// FUNCTION: SWBF2 0x0072f2b0
template <int N>
ZephyrAnimInst<N>::ZephyrAnimInst() : m_pkAnim(0), m_pSkel(0)
{
}

// FUNCTION: SWBF2 0x0072f3f0
template <int N>
void ZephyrAnimInst<N>::SetAnim(const ZephyrAnim* pkAnim, const ZephyrSkeleton<N>* pSkel)
{
    if (pkAnim == m_pkAnim)
    {
        return;
    }
    m_pkAnim = const_cast<ZephyrAnim*>(pkAnim);
    m_pSkel = const_cast<ZephyrSkeleton<N>*>(pSkel);
    if (!pkAnim)
    {
        return;
    }

    for (int i = 0; i < GetNumSkelJoints(); i++)
    {
        m_piAnimJointIdx[i] = -1;
    }

    for (unsigned short j = 0; j < m_pkAnim->m_u16NumJoints; j++)
    {
        short iSkel = (short)((int)pSkel->m_pShared->m_kHashCRC2Joint.Find(
                                  m_pkAnim->m_pkJoints[j].GetNameCRC()) - 1);
        if (iSkel >= 0)
        {
            m_piAnimJointIdx[iSkel] = j;
        }
        m_piSkelJointIdx[j] = iSkel;
        m_pkJointQuatStates[j].u16Frame = ZephyrAnimJoint::INVALID_FRAME;
        m_pkJointTranStates[j].u16Frame = ZephyrAnimJoint::INVALID_FRAME;
    }
}

template class ZephyrAnimInst<32>;

template ZephyrAnimInst<64>::ZephyrAnimInst();

template void ZephyrAnimInst<64>::SetAnim(const ZephyrAnim* pkAnim, const ZephyrSkeleton<64>* pSkel);

// TEMPLATE: SWBF2 0x0072f500
// ZephyrAnimInst<64>::SetAnim

// TEMPLATE: SWBF2 0x0072f2d0
// ZephyrAnimInst<64>::ZephyrAnimInst<64>

// FUNCTION: SWBF2 0x0072f2f0
template <int N>
void ZephyrAnimInst<N>::GetRotationCached(PblQuaternion& kDst, unsigned short u16JointNum, unsigned short u16FrameNum) const
{
    m_pkAnim->m_pkJoints[u16JointNum].GetRotationCached(
        kDst, u16FrameNum, const_cast<ZephyrAnimJoint::DecompQuatState&>(m_pkJointQuatStates[u16JointNum]));
}

// TEMPLATE: SWBF2 0x0072f330
// ZephyrAnimInst<64>::GetRotationCached

// FUNCTION: SWBF2 0x0072f370
template <int N>
void ZephyrAnimInst<N>::GetTranslationCached(PblVector3& kDst, unsigned short u16JointNum, unsigned short u16FrameNum) const
{
    m_pkAnim->m_pkJoints[u16JointNum].GetTranslationCached(
        kDst, u16FrameNum, const_cast<ZephyrAnimJoint::DecompTranState&>(m_pkJointTranStates[u16JointNum]));
}

// TEMPLATE: SWBF2 0x0072f3b0
// ZephyrAnimInst<64>::GetTranslationCached

template void ZephyrAnimInst<32>::GetRotationCached(PblQuaternion&, unsigned short, unsigned short) const;
template void ZephyrAnimInst<32>::GetTranslationCached(PblVector3&, unsigned short, unsigned short) const;
template void ZephyrAnimInst<64>::GetRotationCached(PblQuaternion&, unsigned short, unsigned short) const;
template void ZephyrAnimInst<64>::GetTranslationCached(PblVector3&, unsigned short, unsigned short) const;
