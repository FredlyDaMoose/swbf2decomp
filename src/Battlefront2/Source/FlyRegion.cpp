#include "FlyRegion.h"

// FUNCTION: SWBF2 0x0052bd10
FlyRegionManager::FlyRegionManager()
    : RedRegionFactory("danger")
{
}

// GLOBAL: SWBF2 0x007ec178

// SYNTHETIC: SWBF2 0x00403a30
// `dynamic initializer for 'FlyRegionManager::sRegions''
// SYNTHETIC: SWBF2 0x007687b0
// `dynamic atexit destructor for 'FlyRegionManager::sRegions''
ListPool<RedRegion*, int> FlyRegionManager::sRegions(5);

// FUNCTION: SWBF2 0x0052bd60
void FlyRegionManager::Reset()
{
    sRegions.Clear();
}

// FUNCTION: SWBF2 0x0052bd80
void* FlyRegionManager::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    if (desc.mTypeId != 0x70C67E32u)
    {
        return 0;
    }

    desc.mSize.y += 1000.0f;
    RedRegion* pRegion = new RedRegion(desc);
    sRegions.Append(pRegion);
    return pRegion;
}

// FUNCTION: SWBF2 0x0052be30
bool FlyRegionManager::IsInside(const PblVector3& rPoint, float fRadius)
{
    for (int i = 0; i < sRegions.GetLength(); ++i)
    {
        RedRegion region(*sRegions[i]);
        region.mSize.x += fRadius;
        region.mSize.z += fRadius;
        if (region.IsPointInside(rPoint))
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0052bf00
bool FlyRegionManager::RayTest(const PblVector3& rStart, const PblVector3& rEnd, float fRadius)
{
    for (int i = 0; i < sRegions.GetLength(); ++i)
    {
        RedRegion region(*sRegions[i]);
        region.mSize.x += fRadius;
        region.mSize.z += fRadius;
        if (region.DoesRayIntersect(rStart, rEnd))
        {
            return true;
        }
    }
    return false;
}
