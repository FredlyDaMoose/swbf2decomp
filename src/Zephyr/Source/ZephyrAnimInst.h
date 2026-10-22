#pragma once

#include "ZephyrAnim.h"
#include "ZephyrAnimJoint.h"
#include "ZephyrCommon.h"
#include "ZephyrSkeleton.h"
#include "ZephyrSkeletonShared.h"

template <int N>
class ZephyrAnimInst
{
public:
    ZephyrAnimInst();

    void SetAnim(const ZephyrAnim* pkAnim, const ZephyrSkeleton<N>* pSkel);

    unsigned short GetNumFrames() const { return m_pkAnim->m_u16NumFrames; }

    short GetAnimJointIdx(unsigned short u16Joint) const { return m_piAnimJointIdx[u16Joint]; }

    void GetRotationCached(PblQuaternion& kDst, unsigned short u16JointNum, unsigned short u16FrameNum) const;
    void GetTranslationCached(PblVector3& kDst, unsigned short u16JointNum, unsigned short u16FrameNum) const;

    void GetTransformCached(ZephyrTransform& kDst, unsigned short u16JointNum, unsigned short u16FrameNum) const
    {
        GetRotationCached(kDst.kQuat, u16JointNum, u16FrameNum);
        GetTranslationCached(kDst.kTrans, u16JointNum, u16FrameNum);
    }

private:

    int GetNumSkelJoints() const { return m_pSkel->m_pShared->m_uNumJoints; }

public:
    ZephyrAnimJoint::DecompQuatState m_pkJointQuatStates[N];
    ZephyrAnimJoint::DecompTranState m_pkJointTranStates[N];
    short m_piSkelJointIdx[N];
    short m_piAnimJointIdx[N];
    ZephyrAnim* m_pkAnim;
    ZephyrSkeleton<N>* m_pSkel;
};
 
