#pragma once

class ZephyrAnim;
class PblVector3;
class PblQuaternion;
struct ZephyrAnimJointD;

class ZephyrAnimJoint
{
public:

    static const unsigned short INVALID_FRAME = 0xffff;

    struct DecompQuatState
    {
        unsigned short u16Frame;
        char* pi8Dat[4];
        short i16Val[4];
        short i16PrevVal[4];
        unsigned char u8RLECount[4];
    };

    struct DecompTranState
    {

        void Invalidate() { u16Frame = INVALID_FRAME; }

        unsigned short u16Frame;
        char* pi8Dat[3];
        short i16Val[3];
        short i16PrevVal[3];
        unsigned char u8RLECount[3];
    };

    bool Open(const ZephyrAnim* pkAnim, const ZephyrAnimJointD* pkJointD, char* pcData, bool bTransDyn);

    void GetRotationCached(PblQuaternion& kDst, unsigned short u16Frame, DecompQuatState& kState) const
    {
        DecompQuatToFrame(kDst, u16Frame, kState);
    }
    void GetTranslationCached(PblVector3& kDst, unsigned short u16Frame, DecompTranState& kState) const
    {
        DecompTranToFrame(kDst, u16Frame, kState);
    }

    unsigned int GetNameCRC() const { return m_kCrcName; }

    void GetTranslationUncached(PblVector3& kTrans, unsigned short u16Frame) const;

private:

    void DecompQuatToFrame(PblQuaternion& kQuat, unsigned short u16Frame, DecompQuatState& kState) const;

    void DecompTranToFrame(PblVector3& kTrans, unsigned short u16Frame, DecompTranState& kState) const;

public:

    unsigned int m_kCrcName;
    char* m_pi8Quat[4];
    char* m_pi8Tran[3];
    float m_fTranOffset[3];
    float m_fTranScale;
    ZephyrAnim* m_pkAnim;
    bool m_bTransDyn;
};
 
 
 
