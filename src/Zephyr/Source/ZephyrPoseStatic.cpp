#include "ZephyrPoseStatic.h"
#include "ZephyrPoseDyn.h"
#include "ZephyrSkeletonShared.h"
#include "PebbleFL/Common/PblBitVector.h"

static const ZephyrTransform& GetBaseTransform(const ZephyrJointShared* pJoint)
{
    return *(const ZephyrTransform*)pJoint->m_kBaseTransform;
}

// FUNCTION: SWBF2 0x0072da90
template <int N>
ZephyrPoseStatic<N>::ZephyrPoseStatic() : m_pSkel(0)
{
}

// FUNCTION: SWBF2 0x0072db10
template <int N>
ZephyrPoseStatic<N>::~ZephyrPoseStatic()
{
    Close();
}

// FUNCTION: SWBF2 0x0072df20
template <int N>
void ZephyrPoseStatic<N>::Open(const ZephyrSkeleton<N>& kSkel)
{
    Close();

    m_pSkel = const_cast<ZephyrSkeleton<N>*>(&kSkel);
}

// FUNCTION: SWBF2 0x0072df10
template <int N>
void ZephyrPoseStatic<N>::Close()
{
}

// FUNCTION: SWBF2 0x0072dfd0
template <int N>
void ZephyrPoseStatic<N>::Set(const ZephyrPoseDyn<N>& kPose, bool bUseDefault)
{
    for (int i = 0; i < m_pSkel->GetNumJoints(); i++)
    {
        short iAnimIdx = kPose.GetAnimJointIdx((unsigned short)i);
        if (iAnimIdx >= 0)
        {
            kPose.GetJointTransform(m_pJoints[i], iAnimIdx);
        }
        else
        {

            if (bUseDefault)
            {
                m_pJoints[i] = GetBaseTransform(m_pSkel->GetJointShared(i));
            }
        }
    }
}

// FUNCTION: SWBF2 0x0072df40
template <int N>
void ZephyrPoseStatic<N>::Set(const ZephyrPoseDyn<N>& kPose, const PblBitVector<N>& kMask, bool bUseDefault)
{
    for (int i = 0; i < m_pSkel->GetNumJoints(); i++)
    {
        if (!kMask.IsBitSet(i))
        {
            continue;
        }

        short iAnimIdx = kPose.GetAnimJointIdx((unsigned short)i);
        if (iAnimIdx >= 0)
        {
            kPose.GetJointTransform(m_pJoints[i], iAnimIdx);
        }
        else
        {
            if (bUseDefault)
            {
                m_pJoints[i] = GetBaseTransform(m_pSkel->GetJointShared(i));
            }
        }
    }
}

// FUNCTION: SWBF2 0x0072e170
template <int N>
void ZephyrPoseStatic<N>::SetBasePose()
{
    for (int i = 0; i < m_pSkel->GetNumJoints(); i++)
    {
        m_pJoints[i] = GetBaseTransform(m_pSkel->GetJointShared(i));
    }
}

// FUNCTION: SWBF2 0x0072dc80
template <int N>
void ZephyrPoseStatic<N>::Blend(const ZephyrPoseDyn<N>& kPose, float fWeight)
{
    ZephyrTransform kT;

    for (int i = 0; i < m_pSkel->GetNumJoints(); i++)
    {
        short iAnimIdx = kPose.GetAnimJointIdx((unsigned short)i);
        if (iAnimIdx < 0)
        {

            continue;
        }

        if (!m_pJoints[i].IsValid())
        {
            kPose.GetJointTransform(m_pJoints[i], iAnimIdx);
            continue;
        }

        kPose.GetJointTransform(kT, iAnimIdx);

        m_pJoints[i].kQuat = Interpolate(m_pJoints[i].kQuat, kT.kQuat, fWeight);
        m_pJoints[i].kTrans = Interpolate(m_pJoints[i].kTrans, kT.kTrans, fWeight);
    }
}

// FUNCTION: SWBF2 0x0072db30
template <int N>
void ZephyrPoseStatic<N>::Blend(const ZephyrPoseDyn<N>& kPose, const PblBitVector<N>& kMask, float fWeight)
{
    ZephyrTransform kT;

    for (int i = 0; i < m_pSkel->GetNumJoints(); i++)
    {
        if (!kMask.IsBitSet(i))
        {
            continue;
        }

        short iAnimIdx = kPose.GetAnimJointIdx((unsigned short)i);
        if (iAnimIdx < 0)
        {
            continue;
        }

        if (!m_pJoints[i].IsValid())
        {
            kPose.GetJointTransform(m_pJoints[i], iAnimIdx);
            continue;
        }

        kPose.GetJointTransform(kT, iAnimIdx);

        m_pJoints[i].kQuat = Interpolate(m_pJoints[i].kQuat, kT.kQuat, fWeight);
        m_pJoints[i].kTrans = Interpolate(m_pJoints[i].kTrans, kT.kTrans, fWeight);
    }
}

template class ZephyrPoseStatic<32>;
template class ZephyrPoseStatic<64>;

// TEMPLATE: SWBF2 0x0072dad0
// ZephyrPoseStatic<64>::ZephyrPoseStatic<64>

// TEMPLATE: SWBF2 0x0072db20
// ZephyrPoseStatic<64>::~ZephyrPoseStatic<64>

// TEMPLATE: SWBF2 0x0072ddc0
// ZephyrPoseStatic<64>::Blend

// TEMPLATE: SWBF2 0x0072df30
// ZephyrPoseStatic<64>::Open

// TEMPLATE: SWBF2 0x0072e050
// ZephyrPoseStatic<64>::Set

// TEMPLATE: SWBF2 0x0072e0f0
// ZephyrPoseStatic<64>::Set

// TEMPLATE: SWBF2 0x0072e1c0
// ZephyrPoseStatic<64>::SetBasePose
