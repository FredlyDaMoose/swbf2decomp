#include "PortalReader.h"

#include "Entity.h"
#include "EntityProp.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Util/ext/hashtable.h"
#include "RedEngineFL/Util/ext/BaseString.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include <math.h>

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);
extern "C" geo::Vector3<float>* __stdcall D3DXVec3TransformCoord(geo::Vector3<float>* pOut, const geo::Vector3<float>* pV, const PblMatrix* pM);

namespace
{

struct RawArray
{
    void* _elements;
    int _numElements;
    int _size;
};
}

namespace pvs
{

// FUNCTION: SWBF2 0x00621110
PortalReader::PortalReader(IManager& manager)
    : manager_(manager)
{
}

// FUNCTION: SWBF2 0x00621130
void PortalReader::FindRegisterStatics(ISector& sector)
{
    for (unsigned int* it = sector.GetStaticNames().Begin(); it != sector.GetStaticNames().End(); ++it)
    {
        EntityEx* pEntity = EntityEx::GetEntityMap().Find(*it);
        if (pEntity && pEntity->IsRtti(EntityProp::GetRtti()))
        {
            IDisplayable& displayable = *static_cast<EntityProp*>(pEntity);
            displayable.SetPortalActive(true);
            sector.RegisterStatic(displayable);
        }
    }
}

// FUNCTION: SWBF2 0x006211d0
void PortalReader::Read(ISector& sector, PblConfig& config, const char* pcName)
{

    namedSectors_[pcName] = &sector;
    geo::ExtrudedPoly3D& bound = sector.GetBound();

    float fHeight = 0.0f;
    util::Vector<geo::Vector2<float> > points;
    util::Vector<util::BaseString> objects;
    util::BaseString object;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();

        static PblConfigSR base(data, "Base");
        if (base.GetNode(data))
        {
            bound.min_ = data.GetFloatArg(0);
            continue;
        }
        static PblConfigSR height(data, "Height");
        if (height.GetNode(data))
        {
            fHeight = data.GetFloatArg(0);
            continue;
        }
        static PblConfigSR point(data, "Point");
        if (point.GetNode(data))
        {
            geo::Vector2<float> v;
            v.x = data.GetFloatArg(0);
            v.y = data.GetFloatArg(1);
            points.Add(v);
            continue;
        }
        static PblConfigSR objectNode(data, "Object");
        if (!objectNode.GetNode(data))
        {
            break;
        }
        object.Set(data.GetStringArg(0));
        objects.Add(object);
    }

    sector.GetStaticNames().SetSize(objects.GetSize());
    util::BaseString* pName = objects.Begin();
    for (int i = 0; pName != objects.End(); ++pName, ++i)
    {
        const char* pcName = *pName;
        unsigned int uiHash = PblHash(pcName);
        sector.GetStaticNames()[i] = uiHash;
    }

    FindRegisterStatics(sector);

    bound.max_ = bound.min_ + fHeight;
    bound.points_.SetSize(points.GetSize());
    const geo::Vector2<float>* pPoint = points.Begin();
    for (geo::Vector2<float>* it = bound.points_.Begin(); it != bound.points_.End(); ++it, ++pPoint)
    {
        geo::Vector2<float> v;
        v.x = pPoint->x;
        v.y = -pPoint->y;
        *it = v;
    }

}

}

// FUNCTION: SWBF2 0x006215c0
void __fastcall DestroyPortalScratchArray(void* pArray)
{
    RawArray* pThis = (RawArray*)pArray;
    if (pThis->_elements)
    {
        RedFreeToHeap(__RedCurrHeap, pThis->_elements, 0);
        pThis->_elements = 0;
    }
    pThis->_numElements = 0;
    pThis->_size = 0;
}

namespace pvs
{

// FUNCTION: SWBF2 0x006215f0
void PortalReader::Read(IPortal& portal, PblConfig& config, IManager& manager, const char* pcName)
{
    geo::Vector3<float> lower;
    geo::Vector3<float> upper;
    geo::Vector3<float> position;
    PblQuaternion rotation;
    rotation._fX = 0.0f;
    rotation._fY = 0.0f;
    rotation._fZ = 0.0f;
    rotation._fS = 1.0f;
    float fWidth = 0.0f;
    float fHeight = 0.0f;
    util::BaseString sector1;
    util::BaseString sector2;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();

        static PblConfigSR lowerCorner(data, "LowerCorner");
        if (lowerCorner.GetNode(data))
        {
            lower = geo::Vector3<float>(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            continue;
        }
        static PblConfigSR upperCorner(data, "UpperCorner");
        if (upperCorner.GetNode(data))
        {
            upper = geo::Vector3<float>(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            continue;
        }
        static PblConfigSR positionNode(data, "Position");
        if (positionNode.GetNode(data))
        {
            position = geo::Vector3<float>(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            continue;
        }
        static PblConfigSR rotationNode(data, "Rotation");
        if (rotationNode.GetNode(data))
        {
            rotation._fX = -data.GetFloatArg(3);
            rotation._fY = data.GetFloatArg(0);
            rotation._fZ = -data.GetFloatArg(1);
            rotation._fS = data.GetFloatArg(2);
            continue;
        }
        static PblConfigSR widthNode(data, "width");
        if (widthNode.GetNode(data))
        {
            fWidth = data.GetFloatArg(0);
            continue;
        }
        static PblConfigSR heightNode(data, "height");
        if (heightNode.GetNode(data))
        {
            fHeight = data.GetFloatArg(0);
            continue;
        }
        static PblConfigSR sector1Node(data, "Sector1");
        if (sector1Node.GetNode(data))
        {
            sector1.Set(data.GetStringArg(0));
            continue;
        }
        static PblConfigSR sector2Node(data, "Sector2");
        if (!sector2Node.GetNode(data))
        {
            break;
        }
        sector2.Set(data.GetStringArg(0));
    }

    const char* pcLeft = sector1;
    portal.SetLeft(namedSectors_[pcLeft]);
    const char* pcRight = sector2;
    portal.SetRight(namedSectors_[pcRight]);

    ISector* pRoot = &manager.GetRootSector();
    if (!portal.GetLeft())
    {
        portal.SetLeft(pRoot);
    }
    if (!portal.GetRight())
    {
        portal.SetRight(pRoot);
    }

    geo::Poly3D quad;

    geo::Vector3<float> d(upper.x - lower.x, upper.y - lower.y, upper.z - lower.z);
    if ((float)sqrt((double)(d.x * d.x + d.y * d.y + d.z * d.z)) > 0.0f)
    {
        geo::Poly3D& poly = portal.GetPoly();
        poly.points_.SetSize(4);
        poly.points_[0] = geo::Vector3<float>(upper.x, upper.z, upper.y);
        poly.points_[1] = geo::Vector3<float>(lower.x + 0.0f, lower.z + d.z, lower.y + 0.0f);
        poly.points_[2] = geo::Vector3<float>(lower.x, lower.z, lower.y);
        poly.points_[3] = geo::Vector3<float>(upper.x - 0.0f, upper.z - d.z, upper.y - 0.0f);
    }
    else
    {
        float fHalfWidth = fWidth * 0.5f;
        float fHalfHeight = fHeight * 0.5f;
        geo::Vector3<float> corners[4] = {
            geo::Vector3<float>(fHalfWidth, -fHalfHeight, 0.0f),
            geo::Vector3<float>(fHalfWidth, fHalfHeight, 0.0f),
            geo::Vector3<float>(-fHalfWidth, fHalfHeight, 0.0f),
            geo::Vector3<float>(-fHalfWidth, -fHalfHeight, 0.0f),
        };
        position.z = -position.z;

        PblQuaternion unit;
        PblMatrix transform;
        D3DXMatrixRotationQuaternion(&transform, D3DXQuaternionNormalize(&unit, &rotation));
        transform.m[3][0] = position.x;
        transform.m[3][1] = position.y;
        transform.m[3][2] = position.z;
        transform.m[3][3] = 1.0f;

        portal.GetPoly().points_.SetSize(4);
        const geo::Vector3<float>* pCorner = corners;
        for (geo::Vector3<float>* it = portal.GetPoly().points_.Begin(); it != portal.GetPoly().points_.End(); ++it, ++pCorner)
        {
            geo::Vector3<float> v;
            D3DXVec3TransformCoord(&v, pCorner, &transform);
            *it = v;
        }
    }

    portal.InitBound();
}

// FUNCTION: SWBF2 0x00622100
void PortalReader::Read(IManager& manager, PblConfig& config)
{
    PblConfig cfg(config, true);
    int iNumPortals = 0;

    while (!cfg.IsDone())
    {
        PblConfig::Data data = cfg.ReadNextData();

        static PblConfigSR sector(data, "Sector");
        if (sector.GetNode(data))
        {
            const char* pcName = data.GetStringArg(0);
            PblConfig scope(cfg.ReadNextScope(), true);
            Read(*manager.NewSector(), scope, pcName);
        }
        else
        {
            static PblConfigSR portal(data, "Portal");
            if (!portal.GetNode(data))
            {
                break;
            }
            PblConfig scope(cfg.ReadNextScope(), true);
            Read(*manager.NewPortal(), scope, manager, data.GetStringArg(0));
            ++iNumPortals;
        }
    }

    ISector& root = manager_.GetRootSector();
    for (int i = 0; i < iNumPortals; ++i)
    {
        IPortal* pPortal = manager.GetPortal(i);
        ISector* pLeft = pPortal->GetLeft();
        ISector* pRight = pPortal->GetRight();
        pLeft->AddPortal(pPortal);
        pRight->AddPortal(pPortal);
    }

    PblHashTable<EntityEx, 1024>& map = EntityEx::GetEntityMap();
    for (PblHashTable<EntityEx, 1024>::Itor it = map.Begin(); it != map.End(); ++it)
    {
        EntityEx* pEntity = it;
        if (pEntity && pEntity->IsRtti(EntityProp::GetRtti()))
        {
            IDisplayable& displayable = *static_cast<EntityProp*>(pEntity);
            if (!displayable.GetSector())
            {
                displayable.SetPortalActive(true);
                root.RegisterStatic(displayable);
            }
        }
    }
}

}

typedef util::HashTable<pvs::ISector*, char const*, util::TNewArrayAlloc<pvs::ISector*> > PortalReaderSectorTable_;
typedef util::HashTable_<pvs::ISector*, char const*, PortalReaderSectorTable_::ElemVecT,
                         PortalReaderSectorTable_::KeyVecT, PortalReaderSectorTable_::HashVecT>
    PortalReaderSectorTableBase_;
 
 
 
 
 
 

// TEMPLATE: SWBF2 0x006223b0
// util::HashTable_<pvs::ISector *,char const *,util::Vector<pvs::ISector *,util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > > > >::HashTable_
template PortalReaderSectorTableBase_::HashTable_();

// TEMPLATE: SWBF2 0x00622770
// util::HashTable_<pvs::ISector *,char const *,util::Vector<pvs::ISector *,util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > > > >::Get
template pvs::ISector*& PortalReaderSectorTableBase_::Get(char const* const&);

// TEMPLATE: SWBF2 0x00622da0
// util::HashTable_<pvs::ISector *,char const *,util::Vector<pvs::ISector *,util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > > > >::FindIndex_
template unsigned int PortalReaderSectorTableBase_::FindIndex_(unsigned int) const;

// TEMPLATE: SWBF2 0x00622bc0
// util::HashTable<pvs::ISector *,char const *,util::TNewArrayAlloc<pvs::ISector *> >::Reallocate
template void PortalReaderSectorTable_::Reallocate(int);

// TEMPLATE: SWBF2 0x006230c0
// util::Vector<pvs::ISector *,util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > > >::SetSize_
template bool util::Vector<pvs::ISector*, util::BaseArray<pvs::ISector*, util::TypedArrayAllocator<pvs::ISector*, util::TNewArrayAlloc<pvs::ISector*> > > >::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x00623170
// util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > >::Reserve_
template bool util::BaseArray<pvs::ISector*, util::TypedArrayAllocator<pvs::ISector*, util::TNewArrayAlloc<pvs::ISector*> > >::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x00623230
// util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > > >::SetSize_
template bool util::Vector<char const*, util::BaseArray<char const*, util::TypedArrayAllocator<char const*, util::TNewArrayAlloc<pvs::ISector*> > > >::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x006232e0
// util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > >::Reserve_
template bool util::BaseArray<char const*, util::TypedArrayAllocator<char const*, util::TNewArrayAlloc<pvs::ISector*> > >::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x006233a0
// util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > > >::SetSize_
template bool util::Vector<unsigned int, util::BaseArray<unsigned int, util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<pvs::ISector*> > > >::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x00623450
// util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > >::Reserve_
template bool util::BaseArray<unsigned int, util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<pvs::ISector*> > >::Reserve_(int, bool);

// FUNCTION: SWBF2 0x00622380
void __fastcall DestroyPortalPodArray(void* pArray)
{
    RawArray* pThis = (RawArray*)pArray;
    if (pThis->_elements)
    {
        RedFreeToHeap(__RedCurrHeap, pThis->_elements, 0);
        pThis->_elements = 0;
    }
    pThis->_numElements = 0;
    pThis->_size = 0;
}

// FUNCTION: SWBF2 0x00622480
template PblHashTable<EntityEx, 1024>::Itor PblHashTable<EntityEx, 1024>::Begin() const;

// FUNCTION: SWBF2 0x006224c0
template const PblHashTable<EntityEx, 1024>::Itor& PblHashTable<EntityEx, 1024>::Itor::operator++();

// FUNCTION: SWBF2 0x006224b0
void __fastcall DestroyPortalBaseStringArray(void* pArray)
{
    ((util::BaseArray<util::BaseString>*)pArray)->~BaseArray();
}

// TEMPLATE: SWBF2 0x006228a0
// util::Vector<util::BaseString,util::BaseArray<util::BaseString,util::TNewArrayAlloc<util::BaseString> > >::Add
template bool util::Vector<util::BaseString>::Add(util::BaseString const&);

// TEMPLATE: SWBF2 0x00622eb0
// util::BaseArray<util::BaseString,util::TNewArrayAlloc<util::BaseString> >::~BaseArray
template util::BaseArray<util::BaseString>::~BaseArray();

// TEMPLATE: SWBF2 0x00623510
// util::BaseArray<util::BaseString,util::TNewArrayAlloc<util::BaseString> >::Reserve_
template bool util::BaseArray<util::BaseString>::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x006237a0
// util::TypedArrayAllocator<util::BaseString,util::NewAllocator>::Allocate_
template util::BaseString* util::TypedArrayAllocator<util::BaseString, util::NewAllocator>::Allocate_(int&);

// TEMPLATE: SWBF2 0x00622930
// util::Vector<char,util::BaseArray<char,util::TNewArrayAlloc<char> > >::SetSize_
template bool util::Vector<char>::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x00622f40
// util::BaseArray<char,util::TNewArrayAlloc<char> >::Reserve_
template bool util::BaseArray<char>::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x006227f0
// util::Vector<unsigned int,util::BaseArray<unsigned int,util::TNewArrayAlloc<unsigned int> > >::SetSize_
template bool util::Vector<unsigned int>::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x00622df0
// util::BaseArray<unsigned int,util::TNewArrayAlloc<unsigned int> >::Reserve_
template bool util::BaseArray<unsigned int>::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x006236b0
// util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> >::Allocate_
template pvs::ISector** util::TypedArrayAllocator<pvs::ISector*, util::TNewArrayAlloc<pvs::ISector*> >::Allocate_(int&);

 
 

// TEMPLATE: SWBF2 0x006225a0
// util::Vector<geo::Vector2<float>,util::BaseArray<geo::Vector2<float>,util::TNewArrayAlloc<geo::Vector2<float> > > >::Add
template bool util::Vector<geo::Vector2<float> >::Add(geo::Vector2<float> const&);

// TEMPLATE: SWBF2 0x00622610
// util::Vector<geo::Vector2<float>,util::BaseArray<geo::Vector2<float>,util::TNewArrayAlloc<geo::Vector2<float> > > >::SetSize_
template bool util::Vector<geo::Vector2<float> >::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x006229e0
// util::BaseArray<geo::Vector2<float>,util::TNewArrayAlloc<geo::Vector2<float> > >::Reserve_
template bool util::BaseArray<geo::Vector2<float> >::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x00623020
// util::TypedArrayAllocator<geo::Vector2<float>,util::NewAllocator>::Allocate_
template geo::Vector2<float>* util::TypedArrayAllocator<geo::Vector2<float>, util::NewAllocator>::Allocate_(int&);

// TEMPLATE: SWBF2 0x006226c0
// util::Vector<geo::Vector3<float>,util::BaseArray<geo::Vector3<float>,util::TNewArrayAlloc<geo::Vector3<float> > > >::SetSize_
template bool util::Vector<geo::Vector3<float> >::SetSize_(int, bool);

// TEMPLATE: SWBF2 0x00622ad0
// util::BaseArray<geo::Vector3<float>,util::TNewArrayAlloc<geo::Vector3<float> > >::Reserve_
template bool util::BaseArray<geo::Vector3<float> >::Reserve_(int, bool);

// TEMPLATE: SWBF2 0x00623070
// util::TypedArrayAllocator<geo::Vector3<float>,util::NewAllocator>::Allocate_
template geo::Vector3<float>* util::TypedArrayAllocator<geo::Vector3<float>, util::NewAllocator>::Allocate_(int&);

// TEMPLATE: SWBF2 0x00623700
// util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> >::Allocate_
template char const** util::TypedArrayAllocator<char const*, util::TNewArrayAlloc<pvs::ISector*> >::Allocate_(int&);

// TEMPLATE: SWBF2 0x00623750
// util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> >::Allocate_
template unsigned int* util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<pvs::ISector*> >::Allocate_(int&);

 
 
