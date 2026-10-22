#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Util/ext/basevector.h"
#include "RedEngineFL/Util/ext/baselist.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "RedEngineFL/geometry/vector.h"
#include "RedEngineFL/geometry/polygons.h"
#include "RedEngineFL/geometry/Primitives.h"

namespace pvs { class ICamera; }
namespace pvs { class IDisplayable; }
namespace pvs { class IPortal; }
namespace pvs { class ISector; }
namespace pvs { class ITraverser; }
namespace pvs { class Sector; }
namespace pvs { class Portal; }
namespace pvs { class BaseCamera; }
namespace geo { struct Box; }

namespace pvs {

int GetCurrentFrame_();

class IManager
{
public:
    virtual void Update(const pvs::ICamera &, pvs::ITraverser *) = 0;
    virtual void RegisterDynamic(pvs::IDisplayable &) = 0;
    virtual void UnregisterDynamic(pvs::IDisplayable &) = 0;
    virtual void RegisterStatic(pvs::IDisplayable &) = 0;
    virtual void UnregisterStatic(pvs::IDisplayable &) = 0;
    virtual pvs::ISector & GetRootSector() = 0;
    virtual bool IsActive() = 0;
    virtual void SetActive(bool) = 0;
    virtual void Allocate(int, int) = 0;
    virtual pvs::IPortal * GetPortal(int) = 0;
    virtual pvs::ISector * GetSector(int) = 0;
    virtual pvs::IPortal * NewPortal() = 0;
    virtual pvs::ISector * NewSector() = 0;
    virtual pvs::IPortal * FindNearestPortal(const geo::Sphere &, float) = 0;
    // FUNCTION: SWBF2 0x006deec0
    virtual ~IManager() {}

    static IManager & GetInstance();
    static void SetInstance(IManager & rManager);
    static void ClearInstance();
};
}

namespace pvs {

class IPortal
{
public:
    virtual void SetActive(bool) = 0;
    virtual bool IsActive() = 0;
    virtual pvs::ISector * GetLeft() = 0;
    virtual pvs::ISector * GetRight() = 0;
    virtual void SetLeft(pvs::ISector *) = 0;
    virtual void SetRight(pvs::ISector *) = 0;
    virtual geo::Poly3D & GetPoly() = 0;
    virtual const geo::Poly3D & GetPoly() const = 0;
    virtual void Init(pvs::Sector *, pvs::Sector *) = 0;
    virtual void InitBound() = 0;
};
}

namespace pvs {

class ISector
{
public:
    virtual void RegisterDynamic(pvs::IDisplayable &) = 0;
    virtual void UnregisterDynamic(pvs::IDisplayable &) = 0;
    virtual void RegisterStatic(pvs::IDisplayable &) = 0;
    virtual void UnregisterStatic(pvs::IDisplayable &) = 0;
    virtual util::Vector<unsigned int> & GetStaticNames() = 0;
    virtual void AddPortal(pvs::IPortal *) = 0;
    virtual void SetBound(const geo::ExtrudedPoly3D &) = 0;
    virtual geo::ExtrudedPoly3D & GetBound() = 0;
};
}

namespace pvs {

// VTABLE: SWBF2 0x00783604
class Sector : public pvs::ISector
{
public:
    Sector();
    virtual ~Sector();

    virtual void RegisterDynamic(pvs::IDisplayable &);
    virtual void UnregisterDynamic(pvs::IDisplayable &);
    virtual void RegisterStatic(pvs::IDisplayable &);
    virtual void UnregisterStatic(pvs::IDisplayable &);
    virtual util::Vector<unsigned int> & GetStaticNames();
    virtual void AddPortal(pvs::IPortal *);
    virtual void SetBound(const geo::ExtrudedPoly3D &);
    virtual geo::ExtrudedPoly3D & GetBound();

    bool Contains(const geo::Vector3<float> &);
    bool Overlap(const geo::Sphere &);
    void Create();
    geo::Sphere & GetSphereBound();
    bool GetTravMark();
    void SetTravMark(bool);
    void SetActiveTravMark(bool);
    bool TestWindingOrder();
    void Process(pvs::IDisplayable &, const pvs::ICamera &, const geo::Box &, pvs::ITraverser *);

    pvs::Sector * PortalCrossingCheck(const geo::Sphere &, const geo::Vector3<float> &, float &);

    void Traverse(const pvs::ICamera &, const geo::Box &, const geo::Frustum &, pvs::ITraverser *, pvs::Sector *);

    bool GetActiveTravMark();

    float windingMult_;
    bool tmark_;
    bool atmark_;
    bool initdone_;
    util::Vector<pvs::Portal *> portals_;
    util::List<pvs::IDisplayable *, util::SLNode<pvs::IDisplayable *> > dynamics_;
    util::Vector<pvs::IDisplayable *> statics_;
    util::Vector<unsigned int> staticNames_;
    geo::ExtrudedPoly3D bound_;
    geo::Sphere sbound_;
};
}
 
 
 
 
 
 
 
 
 
 
 

namespace pvs {

class SectorCache
{
public:
    struct Entry
    {
        Entry() : sect_(0), nextCheckDist_(0) {}

        pvs::Sector* sect_;
        float nextCheckDist_;
        geo::Vector3<float> lastCheckPos_;
    };

    util::Vector<Entry> entries_;
};
}
 

namespace pvs {

// VTABLE: SWBF2 0x0078365c
class Manager : public pvs::IManager
{
public:
    Manager();
    virtual ~Manager();

    virtual void Update(const pvs::ICamera &, pvs::ITraverser *);
    virtual void RegisterDynamic(pvs::IDisplayable &);
    virtual void UnregisterDynamic(pvs::IDisplayable &);
    virtual void RegisterStatic(pvs::IDisplayable &);
    virtual void UnregisterStatic(pvs::IDisplayable &);
    virtual pvs::ISector & GetRootSector();
    virtual bool IsActive();
    virtual void SetActive(bool);
    virtual void Allocate(int, int);
    virtual pvs::IPortal * GetPortal(int);
    virtual pvs::ISector * GetSector(int);
    virtual pvs::IPortal * NewPortal();
    virtual pvs::ISector * NewSector();
    virtual pvs::IPortal * FindNearestPortal(const geo::Sphere &, float);

    pvs::Sector * FindSector(const pvs::IDisplayable &);

    pvs::Sector * FindSector(const geo::Sphere &);

    void FindSectors(const pvs::BaseCamera &, util::Vector<pvs::Sector *> &, pvs::SectorCache &);

    void GetCamFrustum(const pvs::ICamera &, geo::Frustum &);

protected:
    virtual void UpdateDynamics();

    void UpdateDynamics(pvs::Sector &);
    virtual void TraversePortals(const pvs::ICamera &, pvs::ITraverser *);

public:
    pvs::SectorCache scache_;
    PblMatrix saveCamTrans_;
    geo::Vector3<float> saveCamPos_;
    pvs::Sector* saveCamSect_;
    bool lastSaveCam_;
    util::Vector<pvs::Sector *> camsectors_;
    util::Vector<pvs::Sector *> sectors_;
    util::Vector<pvs::Portal *> portals_;
    pvs::Sector rootSect_;
    pvs::Sector* curCamSector_;
    util::Vector<pvs::IDisplayable *> dynamics_;
};
}
 
 
 
 
 
 
 
 
 
 
 
 

namespace pvs {

// VTABLE: SWBF2 0x0078362c
class Portal : public pvs::IPortal
{
public:
    Portal() : active_(true), clipMark_(false) {}
    virtual ~Portal();

    virtual void SetActive(bool);
    virtual bool IsActive();
    virtual pvs::ISector * GetLeft();
    virtual pvs::ISector * GetRight();
    virtual void SetLeft(pvs::ISector *);
    virtual void SetRight(pvs::ISector *);
    virtual geo::Poly3D & GetPoly();
    virtual const geo::Poly3D & GetPoly() const;
    virtual void Init(pvs::Sector *, pvs::Sector *);
    virtual void InitBound();

    void FindNormal();

    bool Clip(const pvs::ICamera &, geo::Box &, const geo::Frustum &, const pvs::Sector &);

    pvs::Sector* GetOther(pvs::Sector* pSect) { return left_ == pSect ? right_ : left_; }

    pvs::Sector* left_;
    pvs::Sector* right_;
    bool active_;
    bool clipMark_;
    unsigned int travmark_;
    geo::Vector3<float> norm_;
    geo::Poly3D poly_;
    geo::Sphere sbound_;
};
}
 
 
 
 
 
 

namespace pvs { class Manager; }
namespace geo { struct Box; }

namespace pvs {

class IDisplayable
{
public:
    virtual void OnTraverse(pvs::ITraverser &) = 0;
    virtual void SetActive(int) = 0;
    virtual bool IsActive(int) const = 0;
    virtual void SetProcessed(int) = 0;
    virtual bool IsProcessed(int) const = 0;
    virtual void GetAABound(geo::Box &) const = 0;
    virtual void GetSphereBound(geo::Sphere &) const = 0;
    virtual bool PortalClip() const = 0;
    virtual void SetPortalClip(bool) = 0;
    virtual bool PortalActive() const = 0;
    virtual void SetPortalActive(bool) = 0;
    virtual bool Dynamic() const = 0;
    virtual void SetDynamic(bool) = 0;
    virtual pvs::Sector * GetSector() = 0;
    virtual void SetSector(pvs::Sector *) = 0;
    virtual void Update(pvs::Manager &) = 0;
};

template <class T>
class BaseDisplayable : public T
{
public:
    BaseDisplayable() : sector_(0), flags_(0), aframe_(0), pframe_(0) {}

    virtual ~BaseDisplayable() {}

    virtual void OnTraverse(pvs::ITraverser &);
    virtual void SetActive(int);
    virtual bool IsActive(int) const;
    virtual void SetProcessed(int);
    virtual bool IsProcessed(int) const;
    virtual bool PortalClip() const;
    virtual void SetPortalClip(bool);
    virtual bool PortalActive() const;
    virtual void SetPortalActive(bool);
    virtual bool Dynamic() const;
    virtual void SetDynamic(bool);
    virtual pvs::Sector * GetSector();
    virtual void SetSector(pvs::Sector *);
    virtual void Update(pvs::Manager &);

    pvs::Sector* sector_;
    unsigned int flags_;
    int aframe_;
    int pframe_;
};

// VTABLE: SWBF2 0x007835bc
class DynDisplayable : public pvs::BaseDisplayable<pvs::IDisplayable>
{
public:
    DynDisplayable();
    virtual ~DynDisplayable();

    virtual void SetSector(pvs::Sector *);
    virtual void Update(pvs::Manager &);

    float nextCheckDist_;
    geo::Vector3<float> lastCheckPos_;
};

class ICamera
{
public:
    virtual void GetPos(geo::Vector3<float> &) const = 0;
    virtual void GetViewTrans(PblMatrix &) const = 0;
    virtual void GetInvViewTrans(PblMatrix &) const = 0;
};

// VTABLE: SWBF2 0x00772db4 pvs::DynDisplayable
// VTABLE: SWBF2 0x00772dfc pvs::ICamera
class BaseCamera : public pvs::DynDisplayable, public pvs::ICamera
{
public:
    BaseCamera() {}
    virtual ~BaseCamera();

    virtual void GetAABound(geo::Box &) const;
    virtual void GetSphereBound(geo::Sphere &) const;
};
}
 
 
 
 
 
