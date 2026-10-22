#pragma once

#include <stddef.h>

#include "PropModel.h"
#include "RedEngineFL/Util/RedSceneObject.h"

struct PropClusterLayer
{
    bool mCreateObjectsFlag;
    PblList<PropObject> mObjects;
};
 

// VTABLE: SWBF2 0x007ade28
class PropCluster : public RedSceneObject
{
public:
    PropCluster();
    virtual ~PropCluster();

    void Init(unsigned char id, float fSize);
    void DeleteObjects();
    void UncreateObjects(int iLayer);

    void Setup(int iMapIndex, float fX, float fZ);

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    const PropObject* FindPropObject(const PblVector3& vPos, bool bParam) const;

    PropClusterLayer mLayers[4];
    unsigned char mId;
    int mMapIndex;
    unsigned int mWasRenderedFlag;
    float mSize;
};
 
 
 
 
 
 

extern float sGrassDistanceScale;

void SetPropDistanceScale(float fValue);
