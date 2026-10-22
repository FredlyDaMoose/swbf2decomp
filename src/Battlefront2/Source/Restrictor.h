#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"

class Restrictor
{
public:

    Restrictor(int iMaxRestrictions) : mNumRestrictions(0), mMaxRestrictions(iMaxRestrictions) {}
    void Clear() { mNumRestrictions = 0; }

    bool AddRestriction(const PblVector3& vRestriction);

    PblVector3 RestrictMovement(const PblVector3& vDir, float& fDist);
    PblVector3 GetRestrictedMovement(const PblVector3& vDir, float fDist);
    void RestrictMovement(PblVector3& vMovement);

    PblVector3 mRestrictions[20];
    int mNumRestrictions;
    int mMaxRestrictions;
    float mHeightOffTerrain;
};
 
 
 
