#include <string.h>
#include <d3dx9math.h>

#include "EntityFormation.h"
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x007eb7ec
PblList<EntityFormation> EntityFormation::sList;

// FUNCTION: SWBF2 0x004b99e0
EntityFormation::EntityFormation()
    : mRootIndex(0), mNodeMembers(0)
{
    mRootIsSlot = true;
    sList.Add((PblList<EntityFormation>::Node*)&mNode, this);
}

// FUNCTION: SWBF2 0x004b9a70
EntityFormation::~EntityFormation()
{
    sList.Remove((PblList<EntityFormation>::Node*)&mNode);
    if (mNodeMembers)
    {
        delete[] mNodeMembers;
        mNodeMembers = 0;
    }
}

// SYNTHETIC: SWBF2 0x004b9a40
// EntityFormation::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004b9b00
void EntityFormation::CalculatePoint(int i, const PblMatrix& matrix, PblVector3& point) const
{
    D3DXVECTOR3 v;
    D3DXVec3TransformNormal(&v, (const D3DXVECTOR3*)&GetPointPosition(i), (const D3DXMATRIX*)&matrix);
    point.x = v.x;
    point.y = v.y;
    point.z = v.z;
}

// FUNCTION: SWBF2 0x004b9b40
int EntityFormation::GetNumSlots() const
{
    int iNumSlots = GetNumPoints();
    if (!mRootIsSlot)
    {
        --iNumSlots;
    }
    return iNumSlots;
}

// FUNCTION: SWBF2 0x004b9b60
bool EntityFormation::RootIsSlot() const
{
    return mRootIsSlot;
}

// FUNCTION: SWBF2 0x004b9b70
int EntityFormation::GetRootIndex() const
{
    return mRootIndex;
}

// FUNCTION: SWBF2 0x004b9b80
bool EntityFormation::MemberIsSupported(unsigned int uMember, int iSlot) const
{
    if (mNodeMembers)
    {
        unsigned int uSlotMember = mNodeMembers[iSlot];
        if (uSlotMember && uSlotMember != uMember)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x004b9bb0
bool EntityFormation::Read(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0x84874D36u)
        {
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
                ReadScope(scope, child, ReadPropertyScope, 0,
                          ReadNodePropertyScope, this);
            }

            unsigned int uNumNodes = GetNumPoints();
            if (uNumNodes)
            {
                Node& root = mNodeArray.GetNode(mRootIndex);
                PblVector3 vRootPos = root.mPosition;
                PblQuaternion qInverse = root.mRotation.Inverse();
                for (unsigned int i = 0; i < uNumNodes; ++i)
                {
                    PblVector3& vPos = mNodeArray.GetNode(i).mPosition;
                    vPos.x -= vRootPos.x;
                    vPos.y -= vRootPos.y;
                    vPos.z -= vRootPos.z;
                    qInverse.RotateVector(vPos);
                }
            }
            break;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004b9da0
const EntityFormation* EntityFormation::Find(unsigned int uName)
{
    for (PblList<EntityFormation>::Itor it(sList); it; ++it)
    {
        if (it->GetName() == uName)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x004b9de0
void EntityFormation::DestroyAll()
{
    PblList<EntityFormation>::Itor it(sList);
    while (it)
    {
        EntityFormation* pFormation = it;
        ++it;
        delete pFormation;
    }
}

// FUNCTION: SWBF2 0x004b9e10
bool EntityFormation::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                        PathObject object, PathObjectType type, void* pUserData)
{
    if (data.GetId() == 0xC1E3FA81u)
    {
        ((EntityFormation*)object.path)->mRootIsSlot = data.GetFloatArg(0) > 0.0f;
        return true;
    }
    return RedPath::ReadPropertyScope(config, data, object, type, pUserData);
}

// FUNCTION: SWBF2 0x004b9e60
bool EntityFormation::ReadNodePropertyScope(PblConfig& config, PblConfig::Data& data,
                                            PathObject object, PathObjectType type, void* pUserData)
{
    EntityFormation* pFormation = (EntityFormation*)pUserData;
    switch (data.GetId())
    {
    case 0x20FD0E45u:
        if (data.GetFloatArg(0) > 0.0f)
        {
            pFormation->mRootIndex = (unsigned int)((char*)object.node - (char*)&pFormation->mNodeArray.GetNode(0)) / sizeof(Node);
        }
        return true;

    case 0x8129238Eu:
        if (!pFormation->mNodeMembers)
        {
            unsigned int uNumNodes = pFormation->GetNumPoints();
            pFormation->mNodeMembers = new unsigned int[uNumNodes];
            memset(pFormation->mNodeMembers, 0, uNumNodes * sizeof(unsigned int));
        }
        {
            unsigned int uMember = PblHash(data.GetStringArg(0));
            pFormation->mNodeMembers[(unsigned int)((char*)object.node - (char*)&pFormation->mNodeArray.GetNode(0)) / sizeof(Node)] = uMember;
        }
        return true;
    }
    return Node::ReadPropertyScope(config, data, object, type, pUserData);
}
