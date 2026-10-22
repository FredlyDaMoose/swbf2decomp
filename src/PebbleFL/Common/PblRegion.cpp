#include "PblRegion.h"

#include <math.h>
#include <stddef.h>

// FUNCTION: SWBF2 0x00729a40
bool PblRegion::IsPointInside(const PblVector3& rPoint) const
{
    switch (mType)
    {
    case type_sphere:
    {
        const float fDx = mMatrix.m[3][0] - rPoint.x;
        const float fDy = mMatrix.m[3][1] - rPoint.y;
        const float fDz = mMatrix.m[3][2] - rPoint.z;

        return mSize.w * mSize.w
             > fDy * fDy + fDx * fDx + fDz * fDz;
    }

    case type_box:
    {
        const float fDx = rPoint.x - mMatrix.m[3][0];
        const float fDy = rPoint.y - mMatrix.m[3][1];
        const float fDz = rPoint.z - mMatrix.m[3][2];

        const float fAlong0 = (float)fabs(mMatrix.m[0][1] * fDy
                                          + mMatrix.m[0][0] * fDx
                                          + mMatrix.m[0][2] * fDz);
        if (!(mSize.x > fAlong0))
        {
            return false;
        }

        const float fAlong1 = (float)fabs(mMatrix.m[1][1] * fDy
                                          + mMatrix.m[1][0] * fDx
                                          + mMatrix.m[1][2] * fDz);
        if (!(mSize.y > fAlong1))
        {
            return false;
        }

        const float fAlong2 = (float)fabs(mMatrix.m[2][1] * fDy
                                          + mMatrix.m[2][0] * fDx
                                          + mMatrix.m[2][2] * fDz);
        return mSize.z > fAlong2;
    }

    case type_cylinder:
    {
        const float fDx = rPoint.x - mMatrix.m[3][0];
        const float fDy = rPoint.y - mMatrix.m[3][1];
        const float fDz = rPoint.z - mMatrix.m[3][2];

        const float fAxial = (float)fabs(mMatrix.m[1][1] * fDy
                                         + mMatrix.m[1][0] * fDx
                                         + mMatrix.m[1][2] * fDz);
        if (fAxial >= mSize.y)
        {
            return false;
        }

        const float fRadial0 = mMatrix.m[0][1] * fDy
                             + mMatrix.m[0][0] * fDx
                             + mMatrix.m[0][2] * fDz;
        const float fRadial2 = mMatrix.m[2][1] * fDy
                             + mMatrix.m[2][0] * fDx
                             + mMatrix.m[2][2] * fDz;

        return mSize.x * mSize.x
             > fRadial2 * fRadial2 + fRadial0 * fRadial0;
    }

    case type_plane:

        return mSize.w > mMatrix.m[2][1] * rPoint.y
                       + mMatrix.m[2][0] * rPoint.x
                       + mMatrix.m[2][2] * rPoint.z;

    case type_box_aligned:
    {
        if (!(mSize.x > (float)fabs(rPoint.x - mMatrix.m[3][0])))
        {
            return false;
        }
        if (!(mSize.y > (float)fabs(rPoint.y - mMatrix.m[3][1])))
        {
            return false;
        }
        return mSize.z > (float)fabs(rPoint.z - mMatrix.m[3][2]);

    }

    case type_cylinder_xaligned:
    {
        const float fDx = rPoint.x - mMatrix.m[3][0];
        const float fDy = rPoint.y - mMatrix.m[3][1];
        const float fDz = rPoint.z - mMatrix.m[3][2];

        if (!(mSize.y > (float)fabs(fDx)))
        {
            return false;
        }
        return mSize.x * mSize.x > fDz * fDz + fDy * fDy;

    }

    case type_cylinder_yaligned:
    {
        const float fDx = rPoint.x - mMatrix.m[3][0];
        const float fDy = rPoint.y - mMatrix.m[3][1];
        const float fDz = rPoint.z - mMatrix.m[3][2];

        if (!(mSize.y > (float)fabs(fDy)))
        {
            return false;
        }
        return mSize.x * mSize.x > fDx * fDx + fDz * fDz;

    }

    case type_cylinder_zaligned:
    {
        const float fDx = rPoint.x - mMatrix.m[3][0];
        const float fDy = rPoint.y - mMatrix.m[3][1];
        const float fDz = rPoint.z - mMatrix.m[3][2];

        if (!(mSize.y > (float)fabs(fDz)))
        {
            return false;
        }
        return mSize.x * mSize.x > fDx * fDx + fDy * fDy;

    }

    default:
        return false;
    }
}

// GLOBAL: SWBF2 0x007e2c9c
// SYNTHETIC: SWBF2 0x0040aef0
// `dynamic initializer for 'PblRegion::sList''
// SYNTHETIC: SWBF2 0x0076a0d0
// `dynamic atexit destructor for 'PblRegion::sList''
PblListSingle PblRegion::sList;

// FUNCTION: SWBF2 0x00727f00
PblRegion::PblRegion(const PblRegion& rOther)
{
    sList.AddLast(&mNode);
    mMatrix = rOther.mMatrix;
    mSize = rOther.mSize;
    mId = rOther.mId;
    mType = rOther.mType;
}

// FUNCTION: SWBF2 0x00727f70
PblRegion::PblRegion()
    : mId(0), mType(type_invalid)
{
    sList.AddLast(&mNode);
    mMatrix.Reset();
    mSize.x = 0.0f;
    mSize.y = 0.0f;
    mSize.z = 0.0f;
    mSize.w = 0.0f;
}

// FUNCTION: SWBF2 0x00727fd0
PblRegion::~PblRegion()
{
    mNode.Remove();
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

// FUNCTION: SWBF2 0x00728ef0
bool PblRegion::DoesSphereIntersect(const PblVector3& rCenter, float fRadius) const
{
    PblVector3 v;
    switch (mType)
    {
    case type_sphere:
    {
        const float fDx = rCenter.x - mMatrix.m[3][0];
        const float fDy = rCenter.y - mMatrix.m[3][1];
        const float fDz = rCenter.z - mMatrix.m[3][2];
        const float fR = mSize.w + fRadius;
        return fR * fR > fDy * fDy + fDx * fDx + fDz * fDz;
    }

    case type_box:
    {
        const PblMatrix inv = mMatrix.GetInverseFast();
        D3DXVec3TransformCoord(&v, &rCenter, &inv);
        return PblRegion::DoesSphereIntersectBox(*(const PblVector3*)&mSize, v, fRadius);
    }

    case type_cylinder:
    {
        const PblMatrix inv = mMatrix.GetInverseFast();
        D3DXVec3TransformCoord(&v, &rCenter, &inv);
        return PblRegion::DoesSphereIntersectYCylinder(*(const PblVector3*)&mSize, v, fRadius);
    }

    case type_plane:
        return mSize.w + fRadius > mMatrix.m[2][1] * rCenter.y
                                 + mMatrix.m[2][0] * rCenter.x
                                 + mMatrix.m[2][2] * rCenter.z;

    case type_box_aligned:
        v.x = rCenter.x - mMatrix.m[3][0];
        v.y = rCenter.y - mMatrix.m[3][1];
        v.z = rCenter.z - mMatrix.m[3][2];
        return PblRegion::DoesSphereIntersectBox(*(const PblVector3*)&mSize, v, fRadius);

    case type_cylinder_xaligned:
        v.x = rCenter.z - mMatrix.m[3][2];
        v.y = rCenter.x - mMatrix.m[3][0];
        v.z = rCenter.y - mMatrix.m[3][1];
        return PblRegion::DoesSphereIntersectYCylinder(*(const PblVector3*)&mSize, v, fRadius);

    case type_cylinder_yaligned:
        v.x = rCenter.x - mMatrix.m[3][0];
        v.y = rCenter.y - mMatrix.m[3][1];
        v.z = rCenter.z - mMatrix.m[3][2];
        return PblRegion::DoesSphereIntersectYCylinder(*(const PblVector3*)&mSize, v, fRadius);

    case type_cylinder_zaligned:
        v.x = rCenter.y - mMatrix.m[3][1];
        v.y = rCenter.z - mMatrix.m[3][2];
        v.z = rCenter.x - mMatrix.m[3][0];
        return PblRegion::DoesSphereIntersectYCylinder(*(const PblVector3*)&mSize, v, fRadius);
    }
    return false;
}

// FUNCTION: SWBF2 0x00729130
bool PblRegion::DoesSphereIntersectBox(const PblVector3& vSize, const PblVector3& vPoint, float fRadius)
{
    const float fAx = (float)fabs(vPoint.x);
    const float fAy = (float)fabs(vPoint.y);
    const float fAz = (float)fabs(vPoint.z);
    const float fDz = fAz > vSize.z ? fAz - vSize.z : 0.0f;
    const float fDy = fAy > vSize.y ? fAy - vSize.y : 0.0f;
    const float fDx = fAx > vSize.x ? fAx - vSize.x : 0.0f;
    return fRadius * fRadius > fDx * fDx + fDy * fDy + fDz * fDz;
}

// FUNCTION: SWBF2 0x007291d0
bool PblRegion::DoesSphereIntersectYCylinder(const PblVector3& vSize, const PblVector3& vPoint, float fRadius)
{
    const float fAxial = (float)fabs(vPoint.y);
    const float fAxialMax = vSize.y + fRadius;
    if (fAxial >= fAxialMax)
    {
        return false;
    }
    const float fRadial2 = vPoint.x * vPoint.x + vPoint.z * vPoint.z;
    const float fOuter = (vSize.x + fRadius) * (vSize.x + fRadius);
    if (fRadial2 >= fOuter)
    {
        return false;
    }
    if (vSize.y > fAxial)
    {
        return fOuter > fRadial2;
    }
    if (vSize.x * vSize.x >= fRadial2)
    {
        return fAxialMax > fAxial;
    }
    const float fDy = fAxial - vSize.y;
    const float fDr = (float)sqrt((double)fRadial2) - vSize.x;
    return fRadius * fRadius > fDr * fDr + fDy * fDy;
}

// FUNCTION: SWBF2 0x00728010
bool PblRegion::DoesAABoxIntersect(const PblVector3& vCenter, const PblVector3& vExtents) const
{
    float fAx, fAz;
    switch (mType)
    {
    case type_sphere:
    {
        fAx = (float)fabs(vCenter.x - mMatrix.m[3][0]);
        const float fAy = (float)fabs(vCenter.y - mMatrix.m[3][1]);
        fAz = (float)fabs(vCenter.z - mMatrix.m[3][2]);
        const float fDx = fAx > vExtents.x ? fAx - vExtents.x : 0.0f;
        const float fDy = fAy > vExtents.y ? fAy - vExtents.y : 0.0f;
        const float fDz = fAz > vExtents.z ? fAz - vExtents.z : 0.0f;
        return mSize.w * mSize.w > fDy * fDy + fDx * fDx + fDz * fDz;
    }

    case type_box:
    {
        const float f00 = (float)fabs(mMatrix.m[0][0]);
        const float f01 = (float)fabs(mMatrix.m[0][1]);
        const float f02 = (float)fabs(mMatrix.m[0][2]);
        const float f10 = (float)fabs(mMatrix.m[1][0]);
        const float f11 = (float)fabs(mMatrix.m[1][1]);
        const float f12 = (float)fabs(mMatrix.m[1][2]);
        const float f20 = (float)fabs(mMatrix.m[2][0]);
        const float f21 = (float)fabs(mMatrix.m[2][1]);
        const float f22 = (float)fabs(mMatrix.m[2][2]);
        if ((float)fabs(vCenter.x - mMatrix.m[3][0]) >= mSize.y * f10 + f00 * mSize.x + f20 * mSize.z)
        {
            return false;
        }
        if ((float)fabs(vCenter.y - mMatrix.m[3][1]) >= mSize.y * f11 + f01 * mSize.x + f21 * mSize.z)
        {
            return false;
        }
        if ((float)fabs(vCenter.z - mMatrix.m[3][2]) >= mSize.y * f12 + f02 * mSize.x + f22 * mSize.z)
        {
            return false;
        }
        const PblMatrix inv = mMatrix.GetInverseFast();
        PblVector3 v;
        D3DXVec3TransformCoord(&v, &vCenter, &inv);
        if ((float)fabs(v.x) >= vExtents.y * f01 + vExtents.x * f00 + vExtents.z * f02)
        {
            return false;
        }
        if ((float)fabs(v.y) >= vExtents.y * f11 + vExtents.x * f10 + vExtents.z * f12)
        {
            return false;
        }
        return (float)fabs(v.z) < vExtents.y * f21 + vExtents.x * f20 + vExtents.z * f22;
    }

    case type_plane:
    {
        const float fDist = vCenter.y * mMatrix.m[2][1] + vCenter.x * mMatrix.m[2][0]
                          + vCenter.z * mMatrix.m[2][2];
        if (mSize.w > fDist)
        {
            return true;
        }
        return mSize.w > fDist - ((float)fabs(mMatrix.m[2][1]) * vExtents.y
                                  + (float)fabs(mMatrix.m[2][0]) * vExtents.x
                                  + (float)fabs(mMatrix.m[2][2]) * vExtents.z);
    }

    case type_box_aligned:
        return mSize.x + vExtents.x > (float)fabs(vCenter.x - mMatrix.m[3][0])
            && mSize.y + vExtents.y > (float)fabs(vCenter.y - mMatrix.m[3][1])
            && mSize.z + vExtents.z > (float)fabs(vCenter.z - mMatrix.m[3][2]);

    case type_cylinder_xaligned:
        if ((float)fabs(vCenter.x - mMatrix.m[3][0]) >= mSize.y + vExtents.y)
        {
            return false;
        }
        fAx = (float)fabs(vCenter.z - mMatrix.m[3][2]);
        fAz = (float)fabs(vCenter.y - mMatrix.m[3][1]);
        break;

    case type_cylinder_yaligned:
        if ((float)fabs(vCenter.y - mMatrix.m[3][1]) >= mSize.y + vExtents.y)
        {
            return false;
        }
        fAx = (float)fabs(vCenter.x - mMatrix.m[3][0]);
        fAz = (float)fabs(vCenter.z - mMatrix.m[3][2]);
        break;

    case type_cylinder_zaligned:
        if ((float)fabs(vCenter.z - mMatrix.m[3][2]) >= mSize.y + vExtents.y)
        {
            return false;
        }
        fAx = (float)fabs(vCenter.y - mMatrix.m[3][1]);
        fAz = (float)fabs(vCenter.x - mMatrix.m[3][0]);
        break;

    default:
        return false;
    }

    if (fAx >= vExtents.x + mSize.x)
    {
        return false;
    }
    if (fAz >= vExtents.z + mSize.x)
    {
        return false;
    }
    if (vExtents.x > fAx && vExtents.z > fAz)
    {
        return true;
    }
    const float fDx = fAx - vExtents.x;
    const float fDz = fAz - vExtents.z;
    return mSize.x * mSize.x > fDz * fDz + fDx * fDx;
}

// FUNCTION: SWBF2 0x007286b0
bool PblRegion::DoesRayIntersect(const PblVector3& rStart, const PblVector3& rEnd) const
{
    switch (mType)
    {
    case type_sphere:
    {
        const float fDx = rEnd.x - rStart.x;
        const float fDy = rEnd.y - rStart.y;
        const float fDz = rEnd.z - rStart.z;
        const float fCx = mMatrix.m[3][0] - rStart.x;
        const float fCy = mMatrix.m[3][1] - rStart.y;
        const float fCz = mMatrix.m[3][2] - rStart.z;
        const float fR2 = mSize.w * mSize.w;
        const float fLen2 = fDx * fDx + fDy * fDy + fDz * fDz;
        const float fT = (fCx * fDx + fCy * fDy + fCz * fDz) / fLen2;
        const float fEx = fCx - fDx * fT;
        const float fEy = fCy - fDy * fT;
        const float fEz = fCz - fDz * fT;
        const float fDist2 = fEx * fEx + fEy * fEy + fEz * fEz;
        if (fDist2 > fR2)
        {
            return false;
        }
        if (0.0f > fT)
        {
            if (fR2 - fDist2 > fT * fT * fLen2)
            {
                return false;
            }
        }
        else if (fT > 1.0f)
        {
            if (fR2 - fDist2 > (1.0f - fT) * (1.0f - fT) * fLen2)
            {
                return false;
            }
        }
        return true;
    }

    case type_box:
    case type_box_aligned:
    {
        const float fHx = (rEnd.x - rStart.x) * 0.5f;
        const float fHy = (rEnd.y - rStart.y) * 0.5f;
        const float fHz = (rEnd.z - rStart.z) * 0.5f;
        const float fCx = (rStart.x + rEnd.x) * 0.5f - mMatrix.m[3][0];
        const float fCy = (rStart.y + rEnd.y) * 0.5f - mMatrix.m[3][1];
        const float fCz = (rStart.z + rEnd.z) * 0.5f - mMatrix.m[3][2];
        const float fE0 = (float)fabs(mMatrix.m[0][0] * fHx + mMatrix.m[0][1] * fHy + mMatrix.m[0][2] * fHz);
        if ((float)fabs(mMatrix.m[0][0] * fCx + mMatrix.m[0][1] * fCy + mMatrix.m[0][2] * fCz) > mSize.x + fE0)
        {
            return false;
        }
        const float fE1 = (float)fabs(mMatrix.m[1][0] * fHx + mMatrix.m[1][1] * fHy + mMatrix.m[1][2] * fHz);
        if ((float)fabs(mMatrix.m[1][0] * fCx + mMatrix.m[1][1] * fCy + mMatrix.m[1][2] * fCz) > mSize.y + fE1)
        {
            return false;
        }
        const float fE2 = (float)fabs(mMatrix.m[2][0] * fHx + mMatrix.m[2][1] * fHy + mMatrix.m[2][2] * fHz);
        if ((float)fabs(mMatrix.m[2][0] * fCx + mMatrix.m[2][1] * fCy + mMatrix.m[2][2] * fCz) > mSize.z + fE2)
        {
            return false;
        }

        const float fWx = fCz * fHy - fCy * fHz;
        const float fWy = fCx * fHz - fHx * fCz;
        const float fWz = fHx * fCy - fCx * fHy;
        if ((float)fabs(mMatrix.m[0][0] * fWx + mMatrix.m[0][1] * fWy + mMatrix.m[0][2] * fWz) >
            mSize.z * fE1 + mSize.y * fE2)
        {
            return false;
        }
        if ((float)fabs(mMatrix.m[1][0] * fWx + mMatrix.m[1][1] * fWy + mMatrix.m[1][2] * fWz) >
            mSize.z * fE0 + mSize.x * fE2)
        {
            return false;
        }
        return (float)fabs(mMatrix.m[2][0] * fWx + mMatrix.m[2][1] * fWy + mMatrix.m[2][2] * fWz) <=
               mSize.y * fE0 + mSize.x * fE1;
    }

    case type_cylinder:
    case type_cylinder_xaligned:
    case type_cylinder_yaligned:
    case type_cylinder_zaligned:
    {
        const float fStartY = mMatrix.m[0][1] * rStart.x + mMatrix.m[1][1] * rStart.y + mMatrix.m[2][1] * rStart.z
                            + mMatrix.m[3][1];
        const float fEndY = mMatrix.m[0][1] * rEnd.x + mMatrix.m[1][1] * rEnd.y + mMatrix.m[2][1] * rEnd.z
                          + mMatrix.m[3][1];
        const float fHalf = mSize.y;
        if (fStartY >= fHalf && fEndY >= fHalf)
        {
            return false;
        }
        const float fNegHalf = -fHalf;
        if (fNegHalf >= fStartY && fNegHalf >= fEndY)
        {
            return false;
        }
        const float fR2 = mSize.x * mSize.x;
        float fStartX = mMatrix.m[0][0] * rStart.x + mMatrix.m[1][0] * rStart.y + mMatrix.m[2][0] * rStart.z
                      + mMatrix.m[3][0];
        float fStartZ = mMatrix.m[0][2] * rStart.x + mMatrix.m[1][2] * rStart.y + mMatrix.m[2][2] * rStart.z
                      + mMatrix.m[3][2];
        float fEndX = mMatrix.m[0][0] * rEnd.x + mMatrix.m[1][0] * rEnd.y + mMatrix.m[2][0] * rEnd.z
                    + mMatrix.m[3][0];
        float fEndZ = mMatrix.m[0][2] * rEnd.x + mMatrix.m[1][2] * rEnd.y + mMatrix.m[2][2] * rEnd.z
                    + mMatrix.m[3][2];
        const float fDz = fEndZ - fStartZ;
        const float fDx = fEndX - fStartX;
        const float fDy = fEndY - fStartY;

        if (fStartY > fHalf)
        {
            const float fInv = 1.0f / fDy;
            const float fT = (fHalf - fStartY) * fInv;
            fStartZ = fT * fDz + fStartZ;
            fStartX = fT * fDx + fStartX;
            if (fNegHalf > fEndY)
            {
                const float fT2 = (fHalf + fEndY) * fInv;
                fEndZ = fEndZ - fT2 * fDz;
                fEndX = fEndX - fT2 * fDx;
            }
        }
        else if (fEndY > fHalf)
        {
            const float fInv = 1.0f / fDy;
            const float fT = (fEndY - fHalf) * fInv;
            fEndZ = fEndZ - fT * fDz;
            fEndX = fEndX - fT * fDx;
            if (fNegHalf > fStartY)
            {
                const float fT2 = (fNegHalf - fStartY) * fInv;
                fStartZ = fT2 * fDz + fStartZ;
                fStartX = fT2 * fDx + fStartX;
            }
        }
        if (fR2 > fStartZ * fStartZ + fStartX * fStartX)
        {
            return true;
        }
        if (fR2 > fEndZ * fEndZ + fEndX * fEndX)
        {
            return true;
        }
        const float fCross = fEndZ * fStartX - fEndX * fStartZ;
        const float fLen2 = fDz * fDz + fDx * fDx;
        if (fLen2 * fR2 > fCross * fCross)
        {
            return false;
        }
        return (fCross * fDz > fLen2 * fEndX) != (fCross * fDz > fLen2 * fStartX);
    }

    case type_plane:
        return mSize.w > mMatrix.m[2][0] * rStart.x + mMatrix.m[2][1] * rStart.y + mMatrix.m[2][2] * rStart.z
            && mMatrix.m[2][0] * rEnd.x + mMatrix.m[2][1] * rEnd.y + mMatrix.m[2][2] * rEnd.z > mSize.w;
    }
    return false;
}

// FUNCTION: SWBF2 0x007292b0
PblRegion* PblRegion::Find(unsigned int uId)
{
    for (PblListSingle::Itor it(sList); !it.IsDone(); ++it)
    {
        PblRegion* pRegion = (PblRegion*)((char*)it.GetNode() - offsetof(PblRegion, mNode));
        if (pRegion->mId == uId)
        {
            return pRegion;
        }
    }
    return 0;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x007292e0
void PblRegion::Initialize(const Description& desc)
{
    mMatrix = desc.mMatrix;
    mSize = desc.mSize;
    mId = desc.mNameId;
    mType = desc.mType;

    switch (mType)
    {
    case type_box:
    {
        float afLen[3];
        for (int i = 0; i < 3; ++i)
        {
            const float* r = mMatrix.m[i];
            afLen[i] = (float)sqrt((double)(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]));
        }

        unsigned int auMask[3] = { 7, 7, 7 };
        int i;
        for (i = 0; i < 3; ++i)
        {
            if ((float)fabs(mMatrix.m[i][0]) > 0.02f)
            {
                auMask[i] &= 1;
            }
            if ((float)fabs(mMatrix.m[i][1]) > 0.02f)
            {
                auMask[i] &= 2;
            }
            if ((float)fabs(mMatrix.m[i][2]) > 0.02f)
            {
                auMask[i] &= 4;
            }
            if (!auMask[i])
            {
                break;
            }
        }

        if (i == 3 && !(auMask[0] & auMask[1] & auMask[2]))
        {
            mType = type_box_aligned;
            for (int j = 0; j < 3; ++j)
            {
                if (auMask[j] & 1)
                {
                    mSize.x = afLen[j] * (&desc.mSize.x)[j];
                }
                if (auMask[j] & 2)
                {
                    mSize.y = afLen[j] * (&desc.mSize.x)[j];
                }
                if (auMask[j] & 4)
                {
                    mSize.z = afLen[j] * (&desc.mSize.x)[j];
                }
            }
            *(PblVector3*)mMatrix.m[0] = gVectorUnitX;
            mMatrix.m[0][3] = 0.0f;
            *(PblVector3*)mMatrix.m[1] = gVectorUnitY;
            mMatrix.m[1][3] = 0.0f;
            *(PblVector3*)mMatrix.m[2] = gVectorUnitZ;
            mMatrix.m[2][3] = 0.0f;
        }
        else
        {
            for (int j = 0; j < 3; ++j)
            {
                const float fInv = 1.0f / afLen[j];
                mMatrix.m[j][0] *= fInv;
                mMatrix.m[j][1] *= fInv;
                mMatrix.m[j][2] *= fInv;
                mMatrix.m[j][3] = 0.0f;
            }
            mSize.x *= afLen[0];
            mSize.y *= afLen[1];
            mSize.z *= afLen[2];
        }
        break;
    }

    case type_cylinder:
    {
        float afLen[3];
        for (int i = 0; i < 3; ++i)
        {
            const float* r = mMatrix.m[i];
            afLen[i] = (float)sqrt((double)(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]));
        }

        unsigned int uMask = 7;
        if ((float)fabs(mMatrix.m[1][0]) > 0.02f)
        {
            uMask &= 1;
        }
        if ((float)fabs(mMatrix.m[1][1]) > 0.02f)
        {
            uMask &= 2;
        }
        if ((float)fabs(mMatrix.m[1][2]) > 0.02f)
        {
            uMask &= 4;
        }

        mSize.y *= afLen[1];
        const float fX = mSize.x * afLen[0];
        const float fZ = mSize.z * afLen[2];
        const float fRadius = (float)sqrt((double)(fX * fX + fZ * fZ));
        mSize.x = fRadius;
        mSize.z = fRadius;

        if (!uMask)
        {
            for (int j = 0; j < 3; ++j)
            {
                const float fInv = 1.0f / afLen[j];
                mMatrix.m[j][0] *= fInv;
                mMatrix.m[j][1] *= fInv;
                mMatrix.m[j][2] *= fInv;
                mMatrix.m[j][3] = 0.0f;
            }
        }
        else if (uMask == 1)
        {
            mType = type_cylinder_xaligned;
            *(PblVector3*)mMatrix.m[0] = gVectorUnitY;
            mMatrix.m[0][3] = 0.0f;
            *(PblVector3*)mMatrix.m[1] = gVectorUnitZ;
            mMatrix.m[1][3] = 0.0f;
            *(PblVector3*)mMatrix.m[2] = gVectorUnitX;
            mMatrix.m[2][3] = 0.0f;
        }
        else if (uMask == 4)
        {
            mType = type_cylinder_zaligned;
            *(PblVector3*)mMatrix.m[0] = gVectorUnitZ;
            mMatrix.m[0][3] = 0.0f;
            *(PblVector3*)mMatrix.m[1] = gVectorUnitX;
            mMatrix.m[1][3] = 0.0f;
            *(PblVector3*)mMatrix.m[2] = gVectorUnitY;
            mMatrix.m[2][3] = 0.0f;
        }
        else
        {
            mType = type_cylinder_yaligned;
            *(PblVector3*)mMatrix.m[0] = gVectorUnitX;
            mMatrix.m[0][3] = 0.0f;
            *(PblVector3*)mMatrix.m[1] = gVectorUnitY;
            mMatrix.m[1][3] = 0.0f;
            *(PblVector3*)mMatrix.m[2] = gVectorUnitZ;
            mMatrix.m[2][3] = 0.0f;
        }
        break;
    }

    case type_plane:
    {
        PblVector3& vNormal = *(PblVector3*)mMatrix.m[2];
        PblVector3 v;
        D3DXVec3Normalize(&v, &vNormal);
        vNormal = v;
        mMatrix.m[2][3] = 0.0f;
        mSize.w = mMatrix.m[3][0] * vNormal.x + mMatrix.m[3][1] * vNormal.y
                + mMatrix.m[3][2] * vNormal.z;
        break;
    }
    }
}

// FUNCTION: SWBF2 0x00729e10
void PblRegion::Reset()
{
    sList.Clear();
}
