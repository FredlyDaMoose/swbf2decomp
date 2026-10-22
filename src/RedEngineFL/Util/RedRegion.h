#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblListSingle.h"
#include "PebbleFL/Common/PblRegion.h"

class RedRegionDesc;
class RedCamera;

// VTABLE: SWBF2 0x00783474
class RedRegionFactory
{
public:

    RedRegionFactory(const char* pcNamePrefix);

    RedRegionFactory();

    virtual ~RedRegionFactory();

    // SYNTHETIC: SWBF2 0x006dc900
    // RedRegionFactory::`scalar deleting destructor'

    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;
    const char* GetPrefix() const;
    virtual void ReadParameter(const char* pcRegionName, const char* pcParameter,
                               unsigned int uiIndex, unsigned int uiCount,
                               const char* pcValue, unsigned int uiValueIndex,
                               void* pDesc) const;

protected:

    void ProcessRegionName(const char* const pcName, void* const pRegion) const;

    static unsigned int GetParameterIndex(const char** const ppcNames,
                                          unsigned int uiNumNames,
                                          const char* const pcParameter,
                                          unsigned int uiLength);

public:

    static const RedRegionFactory* Find(const char* pcName);

    static void RemoveAll() { sList.mHead.mNext = &sList.mHead; }

    static const RedRegionFactory* GetDefault() { return &sRegionFactory; }

    const char* mNamePrefix;
    PblListSingle::Node mNode;

    static PblListSingle sList;

    static RedRegionFactory sRegionFactory;
};
 
 
 

class RedRegionDesc : public PblRegion::Description
{
public:
    unsigned int mTypeId;
};
 
 

class RedRegion : public PblRegion
{
public:

    RedRegion(RedRegionDesc& desc);
    RedRegion();
    RedRegion(const RedRegion& rOther);

    ~RedRegion();

    bool IsVisible(const RedCamera* pCamera, float fExtra) const;

    static RedRegion* Find(unsigned int uId) { return (RedRegion*)PblRegion::Find(uId); }

    void SetProperty(unsigned int, const char*) {}

    float GetIntersectionFactor(const PblVector3& vCenter, float fRadius) const;

protected:

    float _GetAABoxSphereIntersectionFactor(const PblVector3& vSize,
                                            const PblVector3& vPos, float fRadius) const;
    float _GetYCylinderSphereIntersectionFactor(const PblVector3& vSize,
                                                const PblVector3& vPos, float fRadius) const;
};
 
