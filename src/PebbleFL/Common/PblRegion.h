#pragma once

#include "PblListSingle.h"
#include "PblMatrix.h"
#include "PblVector.h"

class __declspec(align(16)) PblRegion
{
public:

    enum Type
    {
        type_invalid = 0,
        type_sphere = 1,
        type_box = 2,
        type_cylinder = 3,
        type_plane = 4,
        type_box_aligned = 5,
        type_cylinder_xaligned = 6,
        type_cylinder_yaligned = 7,
        type_cylinder_zaligned = 8
    };

    bool IsPointInside(const PblVector3& rPoint) const;

    bool DoesSphereIntersect(const PblVector3& rCenter, float fRadius) const;

    bool DoesRayIntersect(const PblVector3& rStart, const PblVector3& rEnd) const;

    bool DoesAABoxIntersect(const PblVector3& vCenter, const PblVector3& vExtents) const;

    static bool DoesSphereIntersectBox(const PblVector3& vSize, const PblVector3& vPoint, float fRadius);
    static bool DoesSphereIntersectYCylinder(const PblVector3& vSize, const PblVector3& vPoint, float fRadius);

    PblRegion(const PblRegion& rOther);

    PblRegion();

    ~PblRegion();

    class __declspec(align(16)) Description
    {
    public:
        PblMatrix mMatrix;
        PblVector4 mSize;
        unsigned int mNameId;
        Type mType;
    };

protected:

    void Initialize(const Description& desc);

public:
    static PblListSingle sList;
    static void Reset();

    static PblRegion* Find(unsigned int uId);

    PblMatrix mMatrix;
    PblVector4 mSize;

    unsigned int mId;
    Type mType;

    PblListSingle::Node mNode;
};

 
