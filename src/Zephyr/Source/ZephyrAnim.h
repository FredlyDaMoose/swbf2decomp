#pragma once

struct ZephyrAnimBlockHeader
{
    unsigned int uiID;
    unsigned int uiSize;
};
 

struct ZephyrAnimFileHeaderD
{
    ZephyrAnimBlockHeader kHeader;
    unsigned short u16MajorVersion;
    unsigned short u16MinorVersion;
    unsigned int u32NumTotalJoints;
    unsigned int u32DatChunkSize;
    unsigned short u16NumAnims;
    unsigned char u8DebugLevel;
    unsigned char u8Padding;
};
 

struct ZephyrAnimD
{
    unsigned __int64 u64TransFlags;
    unsigned int u32CrcName;
    unsigned short u16NumFrames;
    unsigned short u16NumJoints;
};
 

struct ZephyrAnimJointD
{
    unsigned int u32CrcName;
    unsigned int u32Quat[4];
    unsigned int u32Tran[3];
    float fTranOffset[3];
    float fTranScale;
};
 

class ZephyrAnimJoint;

class ZephyrAnim
{
public:
    bool Open(const ZephyrAnimD* pkAnimD, const ZephyrAnimJointD* pkJointD, ZephyrAnimJoint* pkJoints, char* pcData);
    unsigned int GetNameCRC() const { return m_kCrcName; }

    unsigned short GetNumFrames() const { return m_u16NumFrames; }

    ZephyrAnimJoint* m_pkJoints;
    unsigned int m_kCrcName;
    unsigned short m_u16NumFrames;
    unsigned short m_u16NumJoints;
};
 
