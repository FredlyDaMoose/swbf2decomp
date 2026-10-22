#include "ZephyrAnim.h"
#include "ZephyrAnimJoint.h"

// FUNCTION: SWBF2 0x0072f220
bool ZephyrAnim::Open(const ZephyrAnimD* pkAnimD, const ZephyrAnimJointD* pkJointD, ZephyrAnimJoint* pkJoints, char* pcData)
{
    m_kCrcName = pkAnimD->u32CrcName;
    m_u16NumFrames = pkAnimD->u16NumFrames;
    m_u16NumJoints = pkAnimD->u16NumJoints;
    m_pkJoints = pkJoints;

    for (int i = 0; i < m_u16NumJoints; i++)
    {
        if (!m_pkJoints[i].Open(this, pkJointD, pcData, (pkAnimD->u64TransFlags & (1 << i)) != 0))
        {
            return false;
        }
        ++pkJointD;
    }
    return true;
}
