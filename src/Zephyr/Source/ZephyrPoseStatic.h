#pragma once

#include "ZephyrCommon.h"
#include "ZephyrSkeleton.h"

template <int N> class ZephyrPoseDyn;
template <int N> class PblBitVector;

template <int N>
class ZephyrPoseStatic
{
public:
    ZephyrPoseStatic();
    ~ZephyrPoseStatic();

    void Open(const ZephyrSkeleton<N>& kSkel);
    void Close();
    void SetBasePose();
    void Set(const ZephyrPoseDyn<N>& kPose, bool bForce);
    void Blend(const ZephyrPoseDyn<N>& kPose, float fWeight);

    void Set(const ZephyrPoseDyn<N>& kPose, const PblBitVector<N>& kMask, bool bForce);
    void Blend(const ZephyrPoseDyn<N>& kPose, const PblBitVector<N>& kMask, float fWeight);

    const ZephyrSkeleton<N>* GetSkeleton() const { return m_pSkel; }

    ZephyrSkeleton<N>* m_pSkel;
    ZephyrTransform m_pJoints[N];
};
 
