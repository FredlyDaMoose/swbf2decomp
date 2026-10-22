#include "RedRegion.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <d3dx9math.h>

#include "RedEngineFL/Graphics/RedCamera.h"

// GLOBAL: SWBF2 0x007df740
// SYNTHETIC: SWBF2 0x00408660
// `dynamic initializer for 'RedRegionFactory::sList''
// SYNTHETIC: SWBF2 0x00769a30
// `dynamic atexit destructor for 'RedRegionFactory::sList''
PblListSingle RedRegionFactory::sList;

// GLOBAL: SWBF2 0x0099e7ec
// SYNTHETIC: SWBF2 0x00408670
// `dynamic initializer for 'RedRegionFactory::sRegionFactory''
// SYNTHETIC: SWBF2 0x00769a40
// `dynamic atexit destructor for 'RedRegionFactory::sRegionFactory''
RedRegionFactory RedRegionFactory::sRegionFactory;

// FUNCTION: SWBF2 0x006dc840
RedRegionFactory::RedRegionFactory(const char* pcNamePrefix)
    : mNamePrefix(pcNamePrefix)
{
    sList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x006dc8b0
RedRegionFactory::~RedRegionFactory()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x006dc770
RedRegion::RedRegion(RedRegionDesc& desc)
{
    switch (desc.mTypeId)
    {
    case 0xAFD98518u:
        desc.mType = type_sphere;
        break;
    case 0x70C67E32u:
        desc.mType = type_box;
        break;
    case 0xAD1EBFEFu:
        desc.mType = type_cylinder;
        break;
    case 0xCCCB0055u:
        desc.mType = type_plane;
        break;
    }
    Initialize(desc);
}

// FUNCTION: SWBF2 0x006dc810
RedRegion::RedRegion(const RedRegion& rOther)
    : PblRegion(rOther)
{
}

// FUNCTION: SWBF2 0x006dc830
RedRegion::RedRegion()
{
}

// FUNCTION: SWBF2 0x006dc8a0
RedRegion::~RedRegion()
{
}

// FUNCTION: SWBF2 0x006dc880
RedRegionFactory::RedRegionFactory()
    : mNamePrefix(0)
{
}

// FUNCTION: SWBF2 0x006dc930
void* RedRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    return new RedRegion(desc);
}

// FUNCTION: SWBF2 0x006dc9a0
const RedRegionFactory* RedRegionFactory::Find(const char* pcName)
{
    for (PblListSingle::Node* pNode = sList.mHead.mNext; pNode != &sList.mHead;
         pNode = pNode->mNext)
    {
        const RedRegionFactory* pFactory = (const RedRegionFactory*)
            ((char*)pNode - offsetof(RedRegionFactory, mNode));
        const char* pcPrefix = pFactory->mNamePrefix;
        if (pcPrefix && strncmp(pcPrefix, pcName, strlen(pcPrefix)) == 0)
        {
            return pFactory;
        }
    }
    return &sRegionFactory;
}

// FUNCTION: SWBF2 0x006dca00
float RedRegion::GetIntersectionFactor(const PblVector3& vCenter, float fRadius) const
{
    const PblVector3& vSize = *(const PblVector3*)&mSize;
    const PblVector3& vPos = *(const PblVector3*)&mMatrix.m[3][0];
    PblVector3 vLocal;

    switch (mType)
    {
    case type_sphere:
    {
        float fDX = vCenter.x - vPos.x;
        float fDY = vCenter.y - vPos.y;
        float fDZ = vCenter.z - vPos.z;
        float fDist2 = fDX * fDX + fDY * fDY + fDZ * fDZ;
        float fOuter = mSize.w + fRadius;
        if (fDist2 >= fOuter * fOuter)
        {
            return 0.0f;
        }
        float fInner = mSize.w - fRadius;
        if (fInner > 0.0f && fInner * fInner >= fDist2)
        {
            return 1.0f;
        }
        return (fOuter - sqrtf(fDist2)) / (fOuter - fInner);
    }
    case type_box:
    {
        PblMatrix mInverse = mMatrix.GetInverseFast();
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vCenter,
                               (const D3DXMATRIX*)&mInverse);
        return _GetAABoxSphereIntersectionFactor(vSize, vLocal, fRadius);
    }
    case type_cylinder:
    {
        PblMatrix mInverse = mMatrix.GetInverseFast();
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vCenter,
                               (const D3DXMATRIX*)&mInverse);
        return _GetYCylinderSphereIntersectionFactor(vSize, vLocal, fRadius);
    }
    case type_plane:
    {
        float fDist = vCenter.x * mMatrix.m[2][0] + vCenter.y * mMatrix.m[2][1] +
                      vCenter.z * mMatrix.m[2][2] - mSize.w;
        if (fDist >= fRadius)
        {
            return 0.0f;
        }
        if (-fRadius >= fDist)
        {
            return 1.0f;
        }
        return (fRadius - fDist) * 0.5f / fRadius;
    }
    case type_box_aligned:
        vLocal.x = vCenter.x - vPos.x;
        vLocal.y = vCenter.y - vPos.y;
        vLocal.z = vCenter.z - vPos.z;
        return _GetAABoxSphereIntersectionFactor(vSize, vLocal, fRadius);

    case type_cylinder_xaligned:
        vLocal.x = vCenter.z - vPos.z;
        vLocal.y = vCenter.x - vPos.x;
        vLocal.z = vCenter.y - vPos.y;
        return _GetYCylinderSphereIntersectionFactor(vSize, vLocal, fRadius);
    case type_cylinder_yaligned:
        vLocal.x = vCenter.x - vPos.x;
        vLocal.y = vCenter.y - vPos.y;
        vLocal.z = vCenter.z - vPos.z;
        return _GetYCylinderSphereIntersectionFactor(vSize, vLocal, fRadius);
    case type_cylinder_zaligned:
        vLocal.x = vCenter.y - vPos.y;
        vLocal.y = vCenter.z - vPos.z;
        vLocal.z = vCenter.x - vPos.x;
        return _GetYCylinderSphereIntersectionFactor(vSize, vLocal, fRadius);
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x006dcdd0
unsigned int RedRegionFactory::GetParameterIndex(const char** const ppcNames,
                                                 unsigned int uiNumNames,
                                                 const char* const pcParameter,
                                                 unsigned int uiLength)
{
    for (unsigned int i = 0; i < uiNumNames; ++i)
    {
        if (_strnicmp(ppcNames[i], pcParameter, uiLength) == 0)
        {
            return i + 1;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006dce10
const char* RedRegionFactory::GetPrefix() const
{
    return mNamePrefix;
}

// FUNCTION: SWBF2 0x006dce20
bool RedRegion::IsVisible(const RedCamera* pCamera, float fExtra) const
{
    const PblVector3& vPos = *(const PblVector3*)&mMatrix.m[3][0];

    switch (mType)
    {
    case type_sphere:
        return pCamera->ClassifySphere(vPos, fExtra + mSize.w, -1.0f) != RedCamera::VISIBILITY_NONE;
    case type_box:
        return pCamera->ClassifyBox(mMatrix, PblVector3(fExtra + mSize.x, mSize.y + fExtra,
                                                        mSize.z + fExtra)) != RedCamera::VISIBILITY_NONE;
    case type_cylinder:
        return pCamera->ClassifyBox(mMatrix, PblVector3(mSize.x + fExtra, mSize.y + fExtra,
                                                        mSize.z + fExtra)) != RedCamera::VISIBILITY_NONE;
    case type_plane:
        return pCamera->ClassifyPlane(*(const PblVector3*)&mMatrix.m[2][0],
                                      fExtra + mSize.w) != RedCamera::VISIBILITY_NONE;
    case type_box_aligned:
        return pCamera->ClassifyAABox(vPos, PblVector3(fExtra + mSize.x, mSize.y + fExtra,
                                                       mSize.z + fExtra)) != RedCamera::VISIBILITY_NONE;
    case type_cylinder_xaligned:
        return pCamera->ClassifyAABox(vPos, PblVector3(mSize.y + fExtra, mSize.x + fExtra,
                                                       mSize.x + fExtra)) != RedCamera::VISIBILITY_NONE;
    case type_cylinder_yaligned:
        return pCamera->ClassifyAABox(vPos, PblVector3(mSize.x + fExtra, mSize.y + fExtra,
                                                       mSize.z + fExtra)) != RedCamera::VISIBILITY_NONE;
    case type_cylinder_zaligned:
        return pCamera->ClassifyAABox(vPos, PblVector3(mSize.x + fExtra, mSize.x + fExtra,
                                                       mSize.y + fExtra)) != RedCamera::VISIBILITY_NONE;
    }
    return false;
}

// FUNCTION: SWBF2 0x006dd0e0
void RedRegionFactory::ProcessRegionName(const char* const pcName, void* const pRegion) const
{
    enum
    {
        STATE_NAME_START = 1,
        STATE_NAME = 2,
        STATE_VALUE_START = 3,
        STATE_VALUE = 4
    };

    int iState = STATE_NAME_START;
    const char* pc = pcName + strlen(mNamePrefix);
    while (*pc != ' ' && *pc != 0)
    {
        ++pc;
    }

    unsigned int uiIndex = 0;
    unsigned int uiNameLen = 0;
    unsigned int uiValueLen = 0;
    char acName[512];
    char acValue[512];

    for (;;)
    {
        switch (iState)
        {
        case STATE_NAME_START:
            uiNameLen = 0;
            if (*pc != ' ')
            {
                uiNameLen = 1;
                acName[0] = *pc;
                acName[1] = 0;
                iState = STATE_NAME;
            }
            break;
        case STATE_NAME:
            if (*pc == '=')
            {
                uiValueLen = 0;
                iState = STATE_VALUE_START;
            }
            else if (*pc == ' ' || *pc == 0)
            {
                _snprintf(acValue, sizeof(acValue), "param%d", uiIndex);
                ReadParameter(pcName, acValue, 0, uiIndex, acName, uiNameLen, pRegion);
                iState = STATE_NAME_START;
                ++uiIndex;
            }
            else
            {
                acName[uiNameLen++] = *pc;
                acName[uiNameLen] = 0;
            }
            break;
        case STATE_VALUE_START:
            uiValueLen = 0;
            if (*pc != ' ')
            {
                uiValueLen = 1;
                acValue[0] = *pc;
                acValue[1] = 0;
                iState = STATE_VALUE;
            }
            break;
        case STATE_VALUE:
            if (*pc == ' ' || *pc == 0)
            {
                ReadParameter(pcName, acName, uiNameLen, uiIndex, acValue, uiValueLen, pRegion);
                iState = STATE_NAME_START;
                ++uiIndex;
            }
            else
            {
                acValue[uiValueLen++] = *pc;
                acValue[uiValueLen] = 0;
            }
            break;
        }
        if (*pc == 0)
        {
            break;
        }
        ++pc;
    }
}

// FUNCTION: SWBF2 0x006dd2c0
void RedRegionFactory::ReadParameter(const char* pcRegionName, const char* pcParameter,
                                     unsigned int uiParameterLen, unsigned int uiIndex,
                                     const char* pcValue, unsigned int uiValueLen,
                                     void* pDesc) const
{
}

// FUNCTION: SWBF2 0x006dd2d0
float RedRegion::_GetAABoxSphereIntersectionFactor(const PblVector3& vSize,
                                                   const PblVector3& vPos,
                                                   float fRadius) const
{
    float fAbsX = (float)fabs(vPos.x);
    float fAbsY = (float)fabs(vPos.y);
    float fAbsZ = (float)fabs(vPos.z);
    float fOutZ = fAbsZ > vSize.z ? fAbsZ - vSize.z : 0.0f;
    float fOutY = fAbsY > vSize.y ? fAbsY - vSize.y : 0.0f;
    float fOutX = fAbsX > vSize.x ? fAbsX - vSize.x : 0.0f;
    float fDist2 = fOutX * fOutX + fOutY * fOutY + fOutZ * fOutZ;
    if (fDist2 >= fRadius * fRadius)
    {
        return 0.0f;
    }
    if (fDist2 > 0.0f)
    {
        return (fRadius - sqrtf(fDist2)) * 0.5f / fRadius;
    }

    float fDepth = vSize.x - fAbsX;
    if (fDepth > vSize.y - fAbsY)
    {
        fDepth = vSize.y - fAbsY;
    }
    if (fDepth > vSize.z - fAbsZ)
    {
        fDepth = vSize.z - fAbsZ;
    }
    if (fDepth >= fRadius)
    {
        return 1.0f;
    }
    return (fDepth + fRadius) * 0.5f / fRadius;
}

// FUNCTION: SWBF2 0x006dd410
float RedRegion::_GetYCylinderSphereIntersectionFactor(const PblVector3& vSize,
                                                       const PblVector3& vPos,
                                                       float fRadius) const
{
    float fAbsY = (float)fabs(vPos.y);
    float fOuterH = vSize.y + fRadius;
    if (fAbsY >= fOuterH)
    {
        return 0.0f;
    }
    float fOuterR = vSize.x + fRadius;
    float fDist2 = vPos.x * vPos.x + vPos.z * vPos.z;
    if (fDist2 >= fOuterR * fOuterR)
    {
        return 0.0f;
    }
    float fInnerH = vSize.y - fRadius;
    float fInnerR = vSize.x - fRadius;
    if (fInnerR >= 0.0f && fInnerR * fInnerR >= fDist2)
    {
        if (fInnerH >= 0.0f && fInnerH >= fAbsY)
        {
            return 1.0f;
        }
        return (fOuterH - fAbsY) * 0.5f / fRadius;
    }

    float fDist = sqrtf(fDist2);
    if (fInnerH > 0.0f && fInnerH > fAbsY)
    {
        return (fOuterR - fDist) * 0.5f / fRadius;
    }
    if (vSize.x >= fDist)
    {
        if (vSize.y >= fAbsY)
        {
            float fDepth = vSize.y - fAbsY;
            if (fDepth > vSize.x - fDist)
            {
                fDepth = vSize.x - fDist;
            }
            return (fDepth + fRadius) * 0.5f / fRadius;
        }
        return (fOuterH - fAbsY) * 0.5f / fRadius;
    }
    if (vSize.y >= fAbsY)
    {
        return (fOuterR - fDist) * 0.5f / fRadius;
    }

    float fDY = fAbsY - vSize.y;
    float fDR = fDist - vSize.x;
    return (fRadius - sqrtf(fDY * fDY + fDR * fDR)) * 0.5f / fRadius;
}
