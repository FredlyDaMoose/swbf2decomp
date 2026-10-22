#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <d3dx9math.h>

#include "EntityPath.h"
#include "Entity.h"
#include "GameObject.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblUnicode.h"

// GLOBAL: SWBF2 0x007eb800
PblList<EntityPath::BranchRegion> EntityPath::BranchRegion::sList;

// GLOBAL: SWBF2 0x007eb814
PblList<EntityPath> EntityPath::sList;

// FUNCTION: SWBF2 0x004d0da0
EntityPath::BranchRegion::BranchRegion(const PblVector3& vPosition, float fSize, unsigned int uHashID)
    : mPosition(vPosition), mSize(fSize), mHashID(uHashID)
{
    sList.Add(this, this);
}

// FUNCTION: SWBF2 0x004d0e10
EntityPath::BranchRegion::~BranchRegion()
{
    sList.Remove(this);
}

// FUNCTION: SWBF2 0x004d0e40
const EntityPath::BranchRegion* EntityPath::BranchRegion::FindByID(unsigned int uHashID)
{
    for (PblList<BranchRegion>::Itor it(sList); it; ++it)
    {
        if (it->GetHashID() == uHashID)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x004d0e70
void EntityPath::BranchRegion::DestroyAll()
{
    PblList<BranchRegion>::Itor it(sList);
    while (it)
    {
        BranchRegion* pRegion = it;
        ++it;
        delete pRegion;
    }
}

// FUNCTION: SWBF2 0x004d0eb0
EntityPath::BranchRegionFactory::BranchRegionFactory()
    : RedRegionFactory("entitypathbranch")
{
}

// FUNCTION: SWBF2 0x004d0f00
void* EntityPath::BranchRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName)
{
    if (desc.mTypeId != 0xAFD98518u)
    {
        return 0;
    }
    const char* pcID = strchr(pcName, ' ');
    if (!pcID || !strlen(pcID))
    {
        return 0;
    }
    return new BranchRegion(*(const PblVector3*)&desc.mMatrix.m[3][0], desc.mSize.w, PblHash(pcID));
}

// FUNCTION: SWBF2 0x004d0fb0
EntityPath::NodeProperties::NodeProperties()
    : mNumBranchPaths(5), mBranchProbability(0.0f), mCosMaxAngle(0.0f), mSpeedFactor(1.0f),
      mVariance(0.0f, 0.0f, 0.0f), mBranchInfoType(1), mBranchSet(0), mLandOnArrival(false)
{
    mBranchInfo.range = FLT_MAX;
}

// FUNCTION: SWBF2 0x004d1000
float EntityPath::NodeProperties::GetRange() const
{
    if (mBranchInfoType == 1)
    {
        return mBranchInfo.range;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x004d1010
const EntityPath::BranchRegion* EntityPath::NodeProperties::GetBranchRegion() const
{
    if (mBranchInfoType == 2)
    {
        return mBranchInfo.branchRegion;
    }
    return 0;
}

// FUNCTION: SWBF2 0x004d1020
void EntityPath::NodeProperties::CalculateNodePosition(const PblVector3* const pPosition,
                                                       const PblVector3* const pVariance,
                                                       const PblVector3* const pDirection,
                                                       PblVector3& vPosition) const
{
    if (pVariance->x == 0.0f && pVariance->y == 0.0f && pVariance->z == 0.0f)
    {
        vPosition = *pPosition;
    }
    else
    {
        vPosition = RandomVector(*pPosition, pVariance->x, pVariance->y, pVariance->z);
    }

    if (pDirection->x != 0.0f || pDirection->y != 0.0f || pDirection->z != 0.0f)
    {
        PblVector3 vDirection = *pDirection;
        PblMatrix matrix;
        EntityPath::BuildMatrix(vDirection, matrix, 0);
        PblVector3 vOffset = RandomVector(*(const PblVector3*)matrix.m[0], mVariance.x, mVariance.x, mVariance.x);
        vPosition.x += vOffset.x;
        vPosition.y += vOffset.y;
        vPosition.z += vOffset.z;
        vOffset = RandomVector(*(const PblVector3*)matrix.m[1], mVariance.y, mVariance.y, mVariance.y);
        vPosition.x += vOffset.x;
        vPosition.y += vOffset.y;
        vPosition.z += vOffset.z;
        vOffset = RandomVector(*(const PblVector3*)matrix.m[2], mVariance.z, mVariance.z, mVariance.z);
        vPosition.x += vOffset.x;
        vPosition.y += vOffset.y;
        vPosition.z += vOffset.z;
    }
}

// FUNCTION: SWBF2 0x004d11e0
PblVector3 EntityPath::NodeProperties::RandomVector(const PblVector3& v, float fX, float fY, float fZ)
{
    PblVector3 vResult;
    vResult.x = (PblRandom::GetGlobalFloat() - 0.5f) * fX + v.x;
    vResult.y = (PblRandom::GetGlobalFloat() - 0.5f) * fY + v.y;
    vResult.z = (PblRandom::GetGlobalFloat() - 0.5f) * fZ + v.z;
    return vResult;
}

// FUNCTION: SWBF2 0x004d1310
bool EntityPath::NodeProperties::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                                   RedPath::PathObject object,
                                                   RedPath::PathObjectType type, void* pUserData)
{
    EntityPath* pPath = (EntityPath*)pUserData;
    NodeProperties& props = pPath->mNodeProps[(unsigned int)((char*)object.node - (char*)&pPath->mNodeArray.GetNode(0)) / sizeof(RedPath::Node)];
    switch (data.GetId())
    {
    case 0x97D3961Fu:
        props.mNumBranchPaths = data.GetUint32Arg(0);
        props.mBranchSet |= 1;
        return true;
    case 0x5742BBFCu:
        props.mBranchProbability = data.GetFloatArg(0);
        return true;
    case 0x72824274u:
        props.mCosMaxAngle = PblMath::Cos(data.GetFloatArg(0) * 0.017453292f);
        props.mBranchSet |= 2;
        return true;
    case 0xFADC0CD2u:
    {
        float fRange = data.GetFloatArg(0);
        props.mBranchSet |= 4;
        props.mBranchInfoType = 1;
        props.mBranchInfo.range = fRange * fRange;
        return true;
    }
    case 0x7554BDFDu:
    {
        const BranchRegion* pRegion = BranchRegion::FindByID(PblHash(data.GetStringArg(0)));
        if (pRegion)
        {
            props.mBranchSet |= 4;
            props.mBranchInfo.branchRegion = (BranchRegion*)pRegion;
            props.mBranchInfoType = 2;
        }
        return true;
    }
    case 0x7B80C780u:
        props.mSpeedFactor = data.GetFloatArg(0);
        return true;
    case 0x858647B2u:
        props.mVariance.x = data.GetFloatArg(0);
        return true;
    case 0x86864945u:
        props.mVariance.y = data.GetFloatArg(0);
        return true;
    case 0x8386448Cu:
        props.mVariance.z = data.GetFloatArg(0);
        return true;
    case 0x3A390312u:
        props.mLandOnArrival = data.GetBoolArg(0);
        return true;
    }
    return RedPath::Node::ReadPropertyScope(config, data, object, type, pUserData);
}

// FUNCTION: SWBF2 0x004d14d0
EntityPath::EntityPath()
    : mNumClassIDs(0), mNodeProps(0), mSpeedFactor(1.0f), mVariance(0.0f, 0.0f, 0.0f),
      mCount(0), mMaxCount(0xFFFFFFFF), mMaxSquadronSize(0xFFFFFFFF), mDirection(0),
      mFrequency(1.0f), mEnableObject(0), mDirectionEntity(0)
{
    mPathRoll = false;
    mDifferentPath = true;
    mSingleDirection = false;
    mEnabled = true;
    sList.Add((PblList<EntityPath>::Node*)&mNode, this);
}

// FUNCTION: SWBF2 0x004d1630
EntityPath::~EntityPath()
{
    if (mNodeProps)
    {
        delete[] mNodeProps;
        mNodeProps = 0;
    }
    mEntityList.Clear();
    sList.Remove((PblList<EntityPath>::Node*)&mNode);
}

// FUNCTION: SWBF2 0x004d16e0
void EntityPath::DupeHeadAndTail()
{
    for (int i = GetNumPoints() - 3; i >= 0; --i)
    {
        mNodeArray.GetNode(i + 1) = mNodeArray.GetNode(i);
        mNodeProps[i + 1] = mNodeProps[i];
    }

    if (GetNumPoints() >= 4)
    {
        Node& next = mNodeArray.GetNode(2);
        Node& first = mNodeArray.GetNode(1);
        Node& head = mNodeArray.GetNode(0);
        head = first;
        head.mPosition.x += (first.mPosition.x - next.mPosition.x) * 0.2f;
        head.mPosition.y += (first.mPosition.y - next.mPosition.y) * 0.2f;
        head.mPosition.z += (first.mPosition.z - next.mPosition.z) * 0.2f;

        Node& prev = mNodeArray.GetNode(GetNumPoints() - 3);
        Node& last = mNodeArray.GetNode(GetNumPoints() - 2);
        Node& tail = mNodeArray.GetNode(GetNumPoints() - 1);
        tail = last;
        tail.mPosition.x += (last.mPosition.x - prev.mPosition.x) * 0.2f;
        tail.mPosition.y += (last.mPosition.y - prev.mPosition.y) * 0.2f;
        tail.mPosition.z += (last.mPosition.z - prev.mPosition.z) * 0.2f;

        mNodeProps[GetNumPoints() - 1] = mNodeProps[GetNumPoints() - 2];
    }
}

// FUNCTION: SWBF2 0x004d18f0
bool EntityPath::Read(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0x84874D36u)
        {
            sAllocExtraPathNodes = 2;
            if (data.GetNumArgs())
            {
                mName = PblHash(data.GetStringArg(0));
            }
            else
            {
                mName = 0;
            }

            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data child = scope.ReadNextData();
                if (child.GetId() == 0x514E905Au)
                {
                    mNodeProps = new NodeProperties[child.GetUint32Arg(0) + 2];
                }
                ReadScope(scope, child, ReadPropertyScope, 0,
                          NodeProperties::ReadPropertyScope, this);
            }
            sAllocExtraPathNodes = 0;
            DupeHeadAndTail();
            break;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004d1a90
const PblVector3* EntityPath::GetPointPosition(int i) const
{
    if (i < 0 || i >= GetNumPoints())
    {
        return 0;
    }
    return &RedPath::GetPointPosition(i);
}

// FUNCTION: SWBF2 0x004d1ac0
void EntityPath::CalculatePointPosition(int i, char cDirection, PblVector3& vPosition) const
{
    const NodeProperties& props = GetNodeProperties(i);
    PblVector3 vDirection;
    const PblVector3& vVariance = props.GetVariance();
    if (vVariance.x == 0.0f && vVariance.y == 0.0f && vVariance.z == 0.0f)
    {
        vDirection.x = 0.0f;
        vDirection.y = 0.0f;
        vDirection.z = 0.0f;
    }
    else
    {
        CalculateDirection(i, cDirection, vDirection);
    }
    props.CalculateNodePosition(&RedPath::GetPointPosition(i), &mVariance, &vDirection, vPosition);
}

// FUNCTION: SWBF2 0x004d1b60
void EntityPath::CalculateDirection(int i, char cDirection, PblVector3& vDirection) const
{
    const PblVector3* pFrom = &RedPath::GetPointPosition(i);
    const PblVector3* pTo = GetPointPosition(i + cDirection);
    if (pFrom && pTo)
    {
        PblVector3 vDelta;
        vDelta.x = pTo->x - pFrom->x;
        vDelta.y = pTo->y - pFrom->y;
        vDelta.z = pTo->z - pFrom->z;
        PblVector3 vUnit;
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)&vDelta);
        vDirection = vUnit;
    }
    else
    {
        vDirection.z = 0.0f;
        vDirection.y = 0.0f;
        vDirection.x = 0.0f;
    }
}

// FUNCTION: SWBF2 0x004d1c00
float EntityPath::CalculateSuitability(const PblVector3* const pPosition, const PblVector3* const pDirection,
                                       float fMinDot, float fRange, int i, char cDirection,
                                       const PblVector3* pDelta, float fDistSq) const
{
    PblVector3 vDelta;
    if (!pDelta)
    {
        const PblVector3& vPoint = RedPath::GetPointPosition(i);
        vDelta.x = vPoint.x - pPosition->x;
        vDelta.y = vPoint.y - pPosition->y;
        vDelta.z = vPoint.z - pPosition->z;
        pDelta = &vDelta;
    }
    if (fDistSq == -1.0f)
    {
        fDistSq = pDelta->x * pDelta->x + pDelta->y * pDelta->y + pDelta->z * pDelta->z;
    }

    float fSuitability = 1.0f;
    if (fDistSq != 0.0f)
    {
        fSuitability = fRange / fDistSq;
        if (fSuitability > 1.0f)
        {
            fSuitability = 1.0f;
        }
        else if (!(fSuitability > 0.0f))
        {
            return fSuitability;
        }
    }
    if (!pDirection)
    {
        return fSuitability;
    }

    float fHeading = 1.0f;
    if (fDistSq != 0.0f)
    {
        float fInvDist = 1.0f / (float)sqrt(fDistSq);
        float fDot = pDirection->x * (pDelta->x * fInvDist) + pDirection->y * (pDelta->y * fInvDist) +
                     pDirection->z * (pDelta->z * fInvDist);
        if (fMinDot > fDot)
        {
            return 0.0f;
        }
        fHeading = (fDot + 1.0f) * 0.5f + 0.0001f;
        if (fHeading == 0.0f)
        {
            return 0.0f;
        }
    }

    PblVector3 vPathDirection;
    CalculateDirection(i, cDirection, vPathDirection);
    float fInvDist = 1.0f / (float)sqrt(fDistSq);
    float fDot = (pDelta->x * fInvDist) * vPathDirection.x + (pDelta->y * fInvDist) * vPathDirection.y +
                 (pDelta->z * fInvDist) * vPathDirection.z;
    if (fMinDot > fDot)
    {
        return 0.0f;
    }
    float fResult = ((fDot + 1.0f) * 0.5f + 0.0001f) * (fHeading * fSuitability);
    if (0.0f > fResult)
    {
        return 0.0f;
    }
    if (fResult > 1.0f)
    {
        return 1.0f;
    }
    return fResult;
}

// FUNCTION: SWBF2 0x004d1e50
bool EntityPath::ClassIsSupported(unsigned int uClassID, bool bCheckCount) const
{
    if (bCheckCount && mCount >= mMaxCount)
    {
        return false;
    }
    if (!mNumClassIDs || !uClassID)
    {
        return true;
    }
    for (unsigned int i = 0; i < mNumClassIDs; ++i)
    {
        if (mClasses[i].GetID() == uClassID)
        {
            return !bCheckCount || mClasses[i].GetCount() < mClasses[i].GetMaxCount();
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004d1ed0
const EntityPath::NodeProperties& EntityPath::GetNodeProperties(unsigned int i) const
{
    return mNodeProps[i];
}

// FUNCTION: SWBF2 0x004d1ef0
bool EntityPath::Add(unsigned int uClassID, char cDirection, PblListDouble::Node* const pNode) const
{
    if (mCount >= mMaxCount)
    {
        return false;
    }
    ++mCount;
    if (mNumClassIDs)
    {
        unsigned int i;
        for (i = 0; i < mNumClassIDs; ++i)
        {
            if (mClasses[i].GetID() == uClassID)
            {
                break;
            }
        }
        if (i >= mNumClassIDs || !mClasses[i].Add())
        {
            --mCount;
            return false;
        }
    }
    if (pNode)
    {
        mEntityList.AddLast(pNode);
    }
    mDirectionEntity = cDirection;
    return true;
}

// FUNCTION: SWBF2 0x004d1f80
void EntityPath::Remove(unsigned int uClassID, PblListDouble::Node* const pNode) const
{
    --mCount;
    if (pNode)
    {
        pNode->Remove();
    }
    for (unsigned int i = 0; i < mNumClassIDs; ++i)
    {
        if (mClasses[i].GetID() == uClassID)
        {
            mClasses[i].Remove();
            break;
        }
    }
    if (!mCount)
    {
        mDirectionEntity = 0;
    }
}

// FUNCTION: SWBF2 0x004d1ff0
bool EntityPath::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                   RedPath::PathObject object,
                                   RedPath::PathObjectType type, void* pUserData)
{
    EntityPath* pPath = (EntityPath*)object.path;
    switch (data.GetId())
    {
    case 0x3CBCEB39u:
        pPath->mPathRoll = data.GetFloatArg(0) > 0.0f;
        return true;
    case 0xF22B0522u:
        pPath->mDifferentPath = data.GetFloatArg(0) > 0.0f;
        return true;
    case 0x3311AA8Eu:
        pPath->mSingleDirection = data.GetFloatArg(0) > 0.0f;
        return true;
    case 0x2FB31C01u:
        pPath->mFrequency = data.GetFloatArg(0);
        return true;
    case 0x5CAC8EFEu:
    case 0x5DAC9091u:
    case 0x5EAC9224u:
    case 0x5FAC93B7u:
    case 0x60AC954Au:
    case 0xAB3E0BFFu:
    {
        char acClass[256];
        strlcpy(acClass, data.GetStringArg(0), sizeof(acClass));
        char* pcMaxCount = strchr(acClass, ' ');
        if (pcMaxCount)
        {
            *pcMaxCount++ = 0;
        }
        pPath->mClasses[pPath->mNumClassIDs].SetID(PblHash(acClass));
        if (pcMaxCount)
        {
            pPath->mClasses[pPath->mNumClassIDs++].SetMaxCount(atol(pcMaxCount));
        }
        else
        {
            pPath->mClasses[pPath->mNumClassIDs++].SetMaxCount(0xFFFFFFFF);
        }
        return true;
    }
    case 0x60940EA9u:
        pPath->mMaxSquadronSize = (int)data.GetFloatArg(0);
        return true;
    case 0x95EA8376u:
        pPath->mMaxCount = data.GetUint32Arg(0);
        return true;
    case 0x7B80C780u:
        pPath->mSpeedFactor = data.GetFloatArg(0);
        return true;
    case 0x858647B2u:
        pPath->mVariance.x = data.GetFloatArg(0);
        return true;
    case 0x86864945u:
        pPath->mVariance.y = data.GetFloatArg(0);
        return true;
    case 0x8386448Cu:
        pPath->mVariance.z = data.GetFloatArg(0);
        return true;
    case 0x8D39BDE6u:
        pPath->mName = PblHash(data.GetStringArg(0));
        return true;
    case 0xC87ABC4Du:
        pPath->mEnableObject = PblHash(data.GetStringArg(0));
        return true;
    case 0xDF6DC76Au:
    {
        int iDirection = (int)data.GetFloatArg(0);
        if (iDirection < -1)
        {
            iDirection = -1;
        }
        else if (iDirection > 1)
        {
            iDirection = 1;
        }
        pPath->mDirection = (char)iDirection;
        return true;
    }
    }
    return RedPath::ReadPropertyScope(config, data, object, type, pUserData);
}

// FUNCTION: SWBF2 0x004d22c0
bool EntityPath::IsEnabled()
{
    if (!mEnabled)
    {
        return false;
    }
    if (!mEnableObject)
    {
        return true;
    }
    EntityEx* pEntity = EntityEx::mIdMap.Find(mEnableObject);
    if (pEntity && pEntity->IsRtti(GameObject::rttiHashGameObject))
    {
        return static_cast<GameObject*>(pEntity)->mIsAlive;
    }
    return true;
}

// FUNCTION: SWBF2 0x004d2320
const PblList<EntityPath>& EntityPath::GetPathList()
{
    return sList;
}

// FUNCTION: SWBF2 0x004d2330
void EntityPath::DestroyAll()
{
    PblList<EntityPath>::Itor it(sList);
    while (it)
    {
        EntityPath* pPath = it;
        ++it;
        delete pPath;
    }
}

// FUNCTION: SWBF2 0x004d2360
bool EntityPath::AllPathsIsClassSupported(unsigned int uClassID)
{
    for (PblList<EntityPath>::Itor it(sList); it; ++it)
    {
        if (it->ClassIsSupported(uClassID, false))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004d23a0
void EntityPath::EnablePathByName(unsigned int uName, bool bEnable)
{
    for (PblList<EntityPath>::Itor it(sList); it; ++it)
    {
        if (it->GetName() == uName)
        {
            it->mEnabled = bEnable;
        }
    }
}

// FUNCTION: SWBF2 0x004d23e0
float EntityPath::BuildMatrix(const PblVector3& vDirection, PblMatrix& matrix, PblMatrix* const pFlatMatrix)
{
    float fYaw = PblMath::Atan2(vDirection.x, vDirection.z);
    matrix.Reset();

    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fYaw * 0.5f);
    PblQuaternion qYaw;
    qYaw._fX = fSin * matrix.m[1][0];
    qYaw._fY = matrix.m[1][1] * fSin;
    qYaw._fZ = matrix.m[1][2] * fSin;
    qYaw._fS = fCos;
    PblVector3 vRight = qYaw.RotateVector(PblVector3(1.0f, 0.0f, 0.0f));

    matrix.m[2][0] = vDirection.x;
    matrix.m[2][1] = vDirection.y;
    matrix.m[2][2] = vDirection.z;
    matrix.m[2][3] = 0.0f;
    matrix.m[0][0] = vRight.x;
    matrix.m[0][1] = vRight.y;
    matrix.m[0][2] = vRight.z;
    matrix.m[0][3] = 0.0f;
    matrix.m[1][0] = vDirection.y * vRight.z - vDirection.z * vRight.y;
    matrix.m[1][1] = vDirection.z * vRight.x - vDirection.x * vRight.z;
    matrix.m[1][2] = vDirection.x * vRight.y - vDirection.y * vRight.x;
    matrix.m[1][3] = 0.0f;

    if (pFlatMatrix)
    {
        PblVector3 vFlat(matrix.m[2][0], 0.0f, matrix.m[2][2]);
        PblVector3 vForward;
        D3DXVec3Normalize((D3DXVECTOR3*)&vForward, (const D3DXVECTOR3*)&vFlat);
        pFlatMatrix->m[2][0] = vForward.x;
        pFlatMatrix->m[2][1] = vForward.y;
        pFlatMatrix->m[2][2] = vForward.z;
        pFlatMatrix->m[2][3] = 0.0f;
        pFlatMatrix->m[1][0] = 0.0f;
        pFlatMatrix->m[1][1] = 1.0f;
        pFlatMatrix->m[1][2] = 0.0f;
        pFlatMatrix->m[1][3] = 0.0f;
        pFlatMatrix->m[0][0] = pFlatMatrix->m[1][1] * pFlatMatrix->m[2][2] - pFlatMatrix->m[1][2] * pFlatMatrix->m[2][1];
        pFlatMatrix->m[0][1] = pFlatMatrix->m[1][2] * pFlatMatrix->m[2][0] - pFlatMatrix->m[1][0] * pFlatMatrix->m[2][2];
        pFlatMatrix->m[0][2] = pFlatMatrix->m[1][0] * pFlatMatrix->m[2][1] - pFlatMatrix->m[1][1] * pFlatMatrix->m[2][0];
        pFlatMatrix->m[0][3] = 0.0f;
    }
    return fYaw;
}
