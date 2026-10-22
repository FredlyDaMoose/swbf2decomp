#include "ZephyrPoseDyn.h"

// FUNCTION: SWBF2 0x0072cc90
template <int N>
ZephyrPoseDyn<N>::ZephyrPoseDyn()
    : m_pSkel(0),
      m_kRootTrans(0.0f, 0.0f, 0.0f),
      m_fAnimSpeed(30.0f),
      m_fCurT(0.0f),
      m_fLastT(0.0f),
      m_pEventList(0),
      m_bLoop(false),
      m_bPlayingBackward(false),
      m_bAnimFinished(false),
      m_bInterpolate(true)
{
}

// FUNCTION: SWBF2 0x0072cd70
template <int N>
ZephyrPoseDyn<N>::~ZephyrPoseDyn()
{
    Close();
}

// FUNCTION: SWBF2 0x0072d3f0
template <int N>
void ZephyrPoseDyn<N>::Open(const ZephyrSkeleton<N>& kSkel)
{
    Close();

    m_pSkel = const_cast<ZephyrSkeleton<N>*>(&kSkel);
}

// FUNCTION: SWBF2 0x0072cd90
template <int N>
void ZephyrPoseDyn<N>::Close()
{
    m_kAnim.SetAnim(NULL, NULL);
}

// FUNCTION: SWBF2 0x0072d430
template <int N>
void ZephyrPoseDyn<N>::SetAnimation(const ZephyrAnim* pkAnim, float fSpeed)
{
    m_kAnim.SetAnim(pkAnim, m_pSkel);

    if (pkAnim)
    {
        m_fCurT = 0.0f;
        m_bAnimFinished = false;
        ZephyrTransform kTemp;
        GetJointTransform(kTemp, 0);
        m_kRootTrans = kTemp.kTrans;
        m_fAnimSpeed = fSpeed;
    }
    else
    {
        m_fCurT = 0.0f;
        m_bAnimFinished = false;
        m_fAnimSpeed = fSpeed;
    }
}

// FUNCTION: SWBF2 0x0072d5b0
template <int N>
bool ZephyrPoseDyn<N>::SetFrameNum(int iFrameNum)
{
    float fTime = (float)(GetNumFrames() - 1);

    if (fTime > 0.0f)
    {
        fTime = (float)iFrameNum / fTime;
    }

    SetCurTime(fTime);

    return true;
}

// FUNCTION: SWBF2 0x0072d650
template <int N>
bool ZephyrPoseDyn<N>::SetFrameNumFloat(float fFrameNum)
{
    float fTime = (float)(GetNumFrames() - 1);

    if (fTime > 0.0f)
    {
        fTime = fFrameNum / fTime;
    }

    SetCurTime(fTime);

    return true;
}

// FUNCTION: SWBF2 0x0072d790
template <int N>
void ZephyrPoseDyn<N>::Update(float fElapsed)
{
    m_fLastT = m_fCurT;

    m_iEventsIter = 0;

    int iNumFrames = GetNumFrames();
    if (iNumFrames <= 1)
    {
        m_fCurT = 0.0f;
        if (!m_bLoop)
        {
            m_bAnimFinished = true;
        }
        return;
    }

    if (!m_bPlayingBackward)
    {

        if (m_bLoop || !m_bAnimFinished)
        {
            m_fCurT += m_fAnimSpeed * fElapsed / (float)(iNumFrames - 1);
        }

        if (m_fCurT > 1.0f)
        {
            if (m_bLoop)
            {
                m_fCurT -= (float)(int)m_fCurT;
            }
            else
            {
                m_fCurT = 1.0f;
            }

            m_bAnimFinished = true;
            return;
        }
    }
    else
    {

        if (m_bLoop || !m_bAnimFinished)
        {
            m_fCurT -= m_fAnimSpeed * fElapsed / (float)(iNumFrames - 1);
        }

        if (m_fCurT < 0.0f)
        {
            if (m_bLoop)
            {
                m_fCurT += (float)(1 - (int)m_fCurT);
            }
            else
            {
                m_fCurT = 0.0f;
            }

            m_bAnimFinished = true;
            return;
        }
    }

    if (m_bLoop)
    {
        m_bAnimFinished = false;
    }
}

// FUNCTION: SWBF2 0x0072cdc0
template <int N>
void ZephyrPoseDyn<N>::GetJointTransform(ZephyrTransform& kTransform, unsigned short u16JointNum, float fT) const
{
    int iCurFrame = (int)((float)(m_kAnim.GetNumFrames() - 1) * fT);
    m_kAnim.GetTransformCached(kTransform, u16JointNum, (unsigned short)iCurFrame);

    if (m_bInterpolate)
    {
        int nextFrame = iCurFrame + 1;
        ZephyrTransform nextTransform;

        if (nextFrame >= (int)m_kAnim.GetNumFrames())
        {
            if (m_bLoop)
            {
                m_kAnim.GetTransformCached(nextTransform, u16JointNum, 1);
                PblVector3 tmp;
                m_kAnim.GetTranslationCached(tmp, u16JointNum, m_kAnim.GetNumFrames() - 1);
                nextTransform.kTrans.x += tmp.x;
                nextTransform.kTrans.y += tmp.y;
                nextTransform.kTrans.z += tmp.z;
            }
            else
            {
                m_kAnim.GetTransformCached(nextTransform, u16JointNum, m_kAnim.GetNumFrames() - 1);
            }
        }
        else
        {
            m_kAnim.GetTransformCached(nextTransform, u16JointNum, (unsigned short)nextFrame);
        }

        float perc2NextFrame = fT * (m_kAnim.GetNumFrames() - 1);
        perc2NextFrame -= (float)(int)perc2NextFrame;

        kTransform.kQuat = Interpolate(kTransform.kQuat, nextTransform.kQuat, perc2NextFrame);

        kTransform.kTrans.x = (nextTransform.kTrans.x - kTransform.kTrans.x) * perc2NextFrame + kTransform.kTrans.x;
        kTransform.kTrans.y = (nextTransform.kTrans.y - kTransform.kTrans.y) * perc2NextFrame + kTransform.kTrans.y;
        kTransform.kTrans.z = (nextTransform.kTrans.z - kTransform.kTrans.z) * perc2NextFrame + kTransform.kTrans.z;
    }
}

// FUNCTION: SWBF2 0x0072cda0
template <int N>
void ZephyrPoseDyn<N>::GetJointTransform(ZephyrTransform& kTransform, unsigned short u16JointNum) const
{
    GetJointTransform(kTransform, u16JointNum, m_fCurT);
}

template <int N>
PblVector3 ZephyrPoseDyn<N>::GetRootTransDelta()
{
    ZephyrTransform kTemp;

    PblVector3 kLastRootTrans = m_kRootTrans;
    GetJointTransform(kTemp, 0);
    m_kRootTrans = kTemp.kTrans;

    if (m_bLoop)
    {
        if (!m_bPlayingBackward && (m_fLastT > m_fCurT))
        {

            GetJointTransform(kTemp, 0, 1.0f);
            PblVector3 kBeforeLoop(kTemp.kTrans.x - kLastRootTrans.x, kTemp.kTrans.y - kLastRootTrans.y,
                                   kTemp.kTrans.z - kLastRootTrans.z);
            GetJointTransform(kTemp, 0, 0.0f);
            return PblVector3((m_kRootTrans.x - kTemp.kTrans.x) + kBeforeLoop.x,
                              (m_kRootTrans.y - kTemp.kTrans.y) + kBeforeLoop.y,
                              (m_kRootTrans.z - kTemp.kTrans.z) + kBeforeLoop.z);
        }
        else if (m_bPlayingBackward && (m_fLastT < m_fCurT))
        {

            GetJointTransform(kTemp, 0, 0.0f);
            PblVector3 kBeforeLoop(kTemp.kTrans.x - kLastRootTrans.x, kTemp.kTrans.y - kLastRootTrans.y,
                                   kTemp.kTrans.z - kLastRootTrans.z);
            GetJointTransform(kTemp, 0, 1.0f);
            return PblVector3((m_kRootTrans.x - kTemp.kTrans.x) + kBeforeLoop.x,
                              (m_kRootTrans.y - kTemp.kTrans.y) + kBeforeLoop.y,
                              (m_kRootTrans.z - kTemp.kTrans.z) + kBeforeLoop.z);
        }
    }

    return PblVector3(m_kRootTrans.x - kLastRootTrans.x, m_kRootTrans.y - kLastRootTrans.y,
                      m_kRootTrans.z - kLastRootTrans.z);
}

template class ZephyrPoseDyn<32>;
template class ZephyrPoseDyn<64>;

// TEMPLATE: SWBF2 0x0072cd80
// ZephyrPoseDyn<64>::~ZephyrPoseDyn<64>

// TEMPLATE: SWBF2 0x0072cf90
// ZephyrPoseDyn<64>::GetJointTransform

// TEMPLATE: SWBF2 0x0072cfb0
// ZephyrPoseDyn<64>::GetJointTransform

// TEMPLATE: SWBF2 0x0072d180
// ZephyrPoseDyn<64>::GetRootTransDelta

// TEMPLATE: SWBF2 0x0072d410
// ZephyrPoseDyn<64>::Open

// TEMPLATE: SWBF2 0x0072d4f0
// ZephyrPoseDyn<64>::SetAnimation

// TEMPLATE: SWBF2 0x0072d6f0
// ZephyrPoseDyn<64>::SetFrameNumFloat

// TEMPLATE: SWBF2 0x0072cd00
// ZephyrPoseDyn<64>::ZephyrPoseDyn<64>

// TEMPLATE: SWBF2 0x0072d910
// ZephyrPoseDyn<64>::Update
