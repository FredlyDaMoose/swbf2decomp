#include "ZephyrSkeleton.h"
#include "ZephyrSkeletonShared.h"
#include "ZephyrPoseStatic.h"
#include "PebbleFL/Common/PblBitVector.h"

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

// GLOBAL: SWBF2 0x009cf404
// SYNTHETIC: SWBF2 0x0040b0f0
// `dynamic initializer for 'kuiDummyRootHash''
static const unsigned int kuiDummyRootHash = PblTEMPHash("DummyRoot");

static inline void SetLocal(PblMatrix& kLocal, const PblQuaternion& kQuat, const PblVector3& kTrans)
{
    PblQuaternion kUnit;
    D3DXMatrixRotationQuaternion(&kLocal, D3DXQuaternionNormalize(&kUnit, &kQuat));
    kLocal.m[3][0] = kTrans.x;
    kLocal.m[3][1] = kTrans.y;
    kLocal.m[3][2] = kTrans.z;
    kLocal.m[3][3] = 1.0f;
}

template <int N>
static inline void BuildRootWorldMatrix(ZephyrSkeleton<N>& kSkel, const PblMatrix& kRootMatrix)
{
    PblMatrix kLocal;
    SetLocal(kLocal, kSkel.m_pPoseFinal->m_pJoints[0].kQuat, PblVector3(0.0f, 0.0f, 0.0f));
    PblMatrix kWorld;
    D3DXMatrixMultiply(&kWorld, &kLocal, &kRootMatrix);
    kSkel.m_pWorldMatrices[0] = kWorld;
}

template <int N>
static inline void BuildJointWorldMatrix(ZephyrSkeleton<N>& kSkel, char iJoint, const ZephyrJointShared* pJoint)
{
    if (pJoint->m_kCrc == kuiDummyRootHash)
    {
        kSkel.m_pWorldMatrices[iJoint] = kSkel.m_pWorldMatrices[pJoint->m_iParent];
        return;
    }

    const ZephyrTransform& kPose = kSkel.m_pPoseFinal->m_pJoints[iJoint];
    const ZephyrTransform& kBase = *(const ZephyrTransform*)pJoint->m_kBaseTransform;
    PblVector3 kTrans = (pJoint->mTransformFlags & 1) ? kPose.kTrans : kBase.kTrans;
    if (pJoint->mTransformFlags & 2)
    {
        kTrans = PblVector3(kTrans.x * pJoint->mScale, kTrans.y * pJoint->mScale, kTrans.z * pJoint->mScale);
    }

    PblMatrix kLocal;
    SetLocal(kLocal, kPose.kQuat, kTrans);
    PblMatrix kWorld;
    D3DXMatrixMultiply(&kWorld, &kLocal, &kSkel.m_pWorldMatrices[pJoint->m_iParent]);
    kSkel.m_pWorldMatrices[iJoint] = kWorld;
}

// FUNCTION: SWBF2 0x0072cc30
template <int N>
bool ZephyrSkeleton<N>::Open(const ZephyrSkeletonShared& kShared)
{
    m_pShared = const_cast<ZephyrSkeletonShared*>(&kShared);
    return true;
}

// FUNCTION: SWBF2 0x0072cc40
template <int N>
bool ZephyrSkeleton<N>::Open(const ZephyrSkeletonShared& kShared, const ZephyrPoseStatic<N>& kPoseFinal)
{
    m_pShared = const_cast<ZephyrSkeletonShared*>(&kShared);
    m_pPoseFinal = const_cast<ZephyrPoseStatic<N>*>(&kPoseFinal);
    return true;
}

// FUNCTION: SWBF2 0x0072cc20
template <int N>
void ZephyrSkeleton<N>::Close()
{
    m_pShared = 0;
}

// FUNCTION: SWBF2 0x0072bc30
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatrices(const PblMatrix& kRootMatrix)
{
    BuildRootWorldMatrix(*this, kRootMatrix);

    char iChild = m_pShared->m_pJoints[0].m_iChild;
    if (iChild >= 0)
    {
        BuildWorldMatricesRecurseDown(iChild);
    }
}

// FUNCTION: SWBF2 0x0072bd00
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatrices(const PblMatrix& kRootMatrix, PblBitVector<N>& kDirty)
{
    if (kDirty.IsBitSet(0))
    {
        BuildRootWorldMatrix(*this, kRootMatrix);
        kDirty.ClearBit(0);
    }

    char iChild = m_pShared->m_pJoints[0].m_iChild;
    if (iChild >= 0)
    {
        BuildWorldMatricesRecurseDown(iChild, kDirty);
    }
}

// FUNCTION: SWBF2 0x0072bde0
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatrices(const PblMatrix& kRootMatrix, const PblBitVector<N>& kMask)
{
    PblBitVector<N> kDirty;
    kDirty.SetAll();

    BuildRootWorldMatrix(*this, kRootMatrix);
    kDirty.ClearBit(0);

    int iNumJoints = GetNumJoints();
    for (int i = 0; i < iNumJoints; i++)
    {
        if (kMask.IsBitSet(i) && kDirty.IsBitSet(i))
        {
            BuildWorldMatricesRecurseUp((char)i, kDirty);
        }
    }
}

// FUNCTION: SWBF2 0x0072bef0
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatrices(const PblMatrix& kRootMatrix, int iJoint)
{
    BuildRootWorldMatrix(*this, kRootMatrix);

    if (iJoint)
    {
        BuildWorldMatricesRecurseUp((char)iJoint);
    }
}

// FUNCTION: SWBF2 0x0072bfb0
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatrices(const PblMatrix& kRootMatrix, int iJoint, PblBitVector<N>& kDirty)
{
    if (kDirty.IsBitSet(0))
    {
        BuildRootWorldMatrix(*this, kRootMatrix);
        kDirty.ClearBit(0);
    }

    if (iJoint)
    {
        BuildWorldMatricesRecurseUp((char)iJoint, kDirty);
    }
}

// FUNCTION: SWBF2 0x0072c150
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatricesRecurseDown(char iJoint)
{

    do
    {
        const ZephyrJointShared* pJoint = &m_pShared->m_pJoints[iJoint];
        BuildJointWorldMatrix(*this, iJoint, pJoint);

        if (pJoint->m_iSibling >= 0)
        {
            BuildWorldMatricesRecurseDown(pJoint->m_iSibling);
        }

        iJoint = pJoint->m_iChild;
    } while (iJoint >= 0);
}

// FUNCTION: SWBF2 0x0072c360
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatricesRecurseDown(char iJoint, PblBitVector<N>& kDirty)
{
    do
    {
        const ZephyrJointShared* pJoint = &m_pShared->m_pJoints[iJoint];
        if (kDirty.IsBitSet(iJoint))
        {
            BuildJointWorldMatrix(*this, iJoint, pJoint);
            kDirty.ClearBit(iJoint);
        }

        if (pJoint->m_iSibling >= 0)
        {
            BuildWorldMatricesRecurseDown(pJoint->m_iSibling, kDirty);
        }

        iJoint = pJoint->m_iChild;
    } while (iJoint >= 0);
}

// FUNCTION: SWBF2 0x0072c7d0
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatricesRecurseUp(char iJoint)
{
    const ZephyrJointShared* pJoint = &m_pShared->m_pJoints[iJoint];
    if (pJoint->m_iParent)
    {
        BuildWorldMatricesRecurseUp(pJoint->m_iParent);
    }

    BuildJointWorldMatrix(*this, iJoint, pJoint);
}

// FUNCTION: SWBF2 0x0072c9e0
template <int N>
void ZephyrSkeleton<N>::BuildWorldMatricesRecurseUp(char iJoint, PblBitVector<N>& kDirty)
{
    kDirty.ClearBit(iJoint);

    const ZephyrJointShared* pJoint = &m_pShared->m_pJoints[iJoint];
    if (kDirty.IsBitSet(pJoint->m_iParent))
    {
        BuildWorldMatricesRecurseUp(pJoint->m_iParent, kDirty);
    }

    BuildJointWorldMatrix(*this, iJoint, pJoint);
}

template class ZephyrSkeleton<32>;
template class ZephyrSkeleton<64>;

// TEMPLATE: SWBF2 0x0072bc00
// ZephyrSkeleton<64>::ZephyrSkeleton<64>

// TEMPLATE: SWBF2 0x0072bc20
// ZephyrSkeleton<64>::~ZephyrSkeleton<64>

// TEMPLATE: SWBF2 0x0072cc60
// ZephyrSkeleton<64>::Open

// TEMPLATE: SWBF2 0x0072cc70
// ZephyrSkeleton<64>::Open

// TEMPLATE: SWBF2 0x0072c080
// ZephyrSkeleton<64>::BuildWorldMatrices

// TEMPLATE: SWBF2 0x0072c5c0
// ZephyrSkeleton<64>::BuildWorldMatricesRecurseDown
