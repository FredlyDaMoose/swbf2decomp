#include "ZephyrAnimJoint.h"

#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "ZephyrAnim.h"

// FUNCTION: SWBF2 0x0072e440
void ZephyrAnimJoint::DecompTranToFrame(PblVector3& kTrans, unsigned short u16Frame,
                                        DecompTranState& kState) const
{
    if (!m_bTransDyn)
    {
        kTrans.x = m_fTranOffset[0];
        kTrans.y = m_fTranOffset[1];
        kTrans.z = m_fTranOffset[2];
        return;
    }

    if (kState.u16Frame == u16Frame + 1)
    {
        kTrans.x = (float)kState.i16PrevVal[0] * m_fTranScale + m_fTranOffset[0];
        kTrans.y = (float)kState.i16PrevVal[1] * m_fTranScale + m_fTranOffset[1];
        kTrans.z = (float)kState.i16PrevVal[2] * m_fTranScale + m_fTranOffset[2];
        return;
    }

    if (kState.u16Frame > u16Frame)
    {
        for (int i = 0; i < 3; ++i)
        {
            kState.pi8Dat[i] = m_pi8Tran[i] + 2;
            kState.i16Val[i] = (short)(((unsigned char)m_pi8Tran[i][1] << 8) | (unsigned char)m_pi8Tran[i][0]);
            kState.u8RLECount[i] = 0;
        }
        kState.u16Frame = 0;
    }

    if (kState.u16Frame != u16Frame)
    {
        for (int i = 0; i < 3; ++i)
        {
            unsigned short u16Cur = kState.u16Frame;
            if (kState.u8RLECount[i])
            {
                u16Cur = (unsigned short)(u16Cur + kState.u8RLECount[i]);
                if (u16Cur >= u16Frame)
                {
                    kState.u8RLECount[i] = (unsigned char)(u16Cur - u16Frame);
                    continue;
                }
                kState.u8RLECount[i] = 0;
            }

            char* pi8Dat = kState.pi8Dat[i];
            short i16Val = kState.i16Val[i];
            short i16PrevVal = kState.i16PrevVal[i];
            while (u16Cur < u16Frame)
            {
                if (*pi8Dat == (char)0x80)
                {
                    i16PrevVal = i16Val;
                    u16Cur = (unsigned short)(u16Cur + (unsigned char)pi8Dat[1]);
                    if (u16Cur > u16Frame)
                    {
                        kState.u8RLECount[i] = (unsigned char)(u16Cur - u16Frame);
                    }
                    pi8Dat += 2;
                }
                else
                {
                    ++u16Cur;
                    i16PrevVal = i16Val;
                    if (*pi8Dat == (char)0x81)
                    {
                        i16Val = (short)(((unsigned char)pi8Dat[2] << 8) | (unsigned char)pi8Dat[1]);
                        pi8Dat += 3;
                    }
                    else
                    {
                        i16Val = (short)(i16Val + *pi8Dat);
                        ++pi8Dat;
                    }
                }
            }
            kState.pi8Dat[i] = pi8Dat;
            kState.i16Val[i] = i16Val;
            kState.i16PrevVal[i] = i16PrevVal;
        }
        kState.u16Frame = u16Frame;
    }

    kTrans.x = (float)kState.i16Val[0] * m_fTranScale + m_fTranOffset[0];
    kTrans.y = (float)kState.i16Val[1] * m_fTranScale + m_fTranOffset[1];
    kTrans.z = (float)kState.i16Val[2] * m_fTranScale + m_fTranOffset[2];
}

// FUNCTION: SWBF2 0x0072e6b0
void ZephyrAnimJoint::GetTranslationUncached(PblVector3& kTrans, unsigned short u16Frame) const
{
    DecompTranState kState;
    kState.Invalidate();
    DecompTranToFrame(kTrans, u16Frame, kState);
}

// FUNCTION: SWBF2 0x0072e210
void ZephyrAnimJoint::DecompQuatToFrame(PblQuaternion& kQuat, unsigned short u16Frame, DecompQuatState& kState) const
{
    const float fScale = 1.0f / 2047.0f;

    if (kState.u16Frame == u16Frame + 1)
    {
        kQuat._fX = (float)kState.i16PrevVal[0] * fScale;
        kQuat._fY = (float)kState.i16PrevVal[1] * fScale;
        kQuat._fZ = (float)kState.i16PrevVal[2] * fScale;
        kQuat._fS = (float)kState.i16PrevVal[3] * fScale;
        return;
    }

    if (kState.u16Frame > u16Frame)
    {
        for (int i = 0; i < 4; ++i)
        {
            kState.pi8Dat[i] = m_pi8Quat[i] + 2;
            kState.i16Val[i] = (short)(((unsigned char)m_pi8Quat[i][1] << 8) | (unsigned char)m_pi8Quat[i][0]);
            kState.u8RLECount[i] = 0;
        }
        kState.u16Frame = 0;
    }

    if (kState.u16Frame != u16Frame)
    {
        for (int i = 0; i < 4; ++i)
        {
            unsigned short u16Cur = kState.u16Frame;
            if (kState.u8RLECount[i])
            {
                u16Cur = (unsigned short)(u16Cur + kState.u8RLECount[i]);
                if (u16Cur >= u16Frame)
                {
                    kState.u8RLECount[i] = (unsigned char)(u16Cur - u16Frame);
                    continue;
                }
                kState.u8RLECount[i] = 0;
            }

            char* pi8Dat = kState.pi8Dat[i];
            short i16Val = kState.i16Val[i];
            short i16PrevVal = kState.i16PrevVal[i];
            while (u16Cur < u16Frame)
            {
                if (*pi8Dat == (char)0x80)
                {
                    i16PrevVal = i16Val;
                    u16Cur = (unsigned short)(u16Cur + (unsigned char)pi8Dat[1]);
                    if (u16Cur > u16Frame)
                    {
                        kState.u8RLECount[i] = (unsigned char)(u16Cur - u16Frame);
                    }
                    pi8Dat += 2;
                }
                else
                {
                    ++u16Cur;
                    i16PrevVal = i16Val;
                    if (*pi8Dat == (char)0x81)
                    {
                        i16Val = (short)(((unsigned char)pi8Dat[2] << 8) | (unsigned char)pi8Dat[1]);
                        pi8Dat += 3;
                    }
                    else
                    {
                        i16Val = (short)(i16Val + *pi8Dat);
                        ++pi8Dat;
                    }
                }
            }
            kState.pi8Dat[i] = pi8Dat;
            kState.i16Val[i] = i16Val;
            kState.i16PrevVal[i] = i16PrevVal;
        }
        kState.u16Frame = u16Frame;
    }

    kQuat._fX = (float)kState.i16Val[0] * fScale;
    kQuat._fY = (float)kState.i16Val[1] * fScale;
    kQuat._fZ = (float)kState.i16Val[2] * fScale;
    kQuat._fS = (float)kState.i16Val[3] * fScale;
}

// FUNCTION: SWBF2 0x0072e6f0
bool ZephyrAnimJoint::Open(const ZephyrAnim* pkAnim, const ZephyrAnimJointD* pkJointD, char* pcData, bool bTransDyn)
{
    m_pkAnim = const_cast<ZephyrAnim*>(pkAnim);
    m_bTransDyn = bTransDyn;
    m_kCrcName = pkJointD->u32CrcName;

    for (int i = 0; i < 4; i++)
    {
        m_pi8Quat[i] = pcData + pkJointD->u32Quat[i];
    }

    for (int i = 0; i < 3; i++)
    {
        m_pi8Tran[i] = m_bTransDyn ? pcData + pkJointD->u32Tran[i] : 0;
        m_fTranOffset[i] = pkJointD->fTranOffset[i];
    }
    m_fTranScale = pkJointD->fTranScale;
    return true;
}
