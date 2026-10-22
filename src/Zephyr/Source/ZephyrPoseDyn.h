#pragma once

#include "ZephyrAnimInst.h"
#include "ZephyrCommon.h"

class ZephyrEventList;

template <int N>
class ZephyrPoseDyn
{
public:
    ZephyrPoseDyn();
    ~ZephyrPoseDyn();

    void Open(const ZephyrSkeleton<N>& kSkel);
    void Close();
    void Update(float fElapsed);
    void SetAnimation(const ZephyrAnim* pkAnim, float fAnimSpeed);

    void GetJointTransform(ZephyrTransform& kTransform, unsigned short u16JointNum) const;

    short GetAnimJointIdx(unsigned short u16Joint) const { return m_kAnim.GetAnimJointIdx(u16Joint); }

    void GetJointTransform(ZephyrTransform& kTransform, unsigned short u16JointNum, float fT) const;

    PblVector3 GetRootTransDelta();
    bool SetFrameNum(int iFrameNum);
    bool SetFrameNumFloat(float fFrameNum);
    int GetNumFrames() const { return m_kAnim.GetNumFrames(); }

    // FUNCTION: SWBF2 0x00443e40
    void SetCurTime(float fTime)
    {
        m_fCurT = fTime;
        ZephyrTransform kTransform;
        GetJointTransform(kTransform, 0);
        m_kRootTrans = kTransform.kTrans;
    }
    void SetLoop(short i16Loop) { m_bLoop = (bool)i16Loop; }

    void PlayAnimForward() { m_bPlayingBackward = false; }

    void PlayAnimBackward() { m_bPlayingBackward = true; }

    void SetInterpolate(bool bInterpolate) { m_bInterpolate = bInterpolate; }

    ZephyrAnimInst<N> m_kAnim;
    ZephyrSkeleton<N>* m_pSkel;
    PblVector3 m_kRootTrans;
    float m_fAnimSpeed;
    float m_fCurT;
    float m_fLastT;
    ZephyrEventList* m_pEventList;
    short m_iEventsIter;
    bool m_bLoop;
    bool m_bPlayingBackward;
    bool m_bAnimFinished;
    bool m_bInterpolate;
};
 
