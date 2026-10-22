#include "RedPath.h"
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x007df840
RedPath::List RedPath::sPathList;

// GLOBAL: SWBF2 0x009a0818
int RedPath::sAllocExtraPathNodes;

// FUNCTION: SWBF2 0x006dde20
RedPath::NodeArray::NodeArray() : mNodes(0), mNumNodes(0)
{
}

// FUNCTION: SWBF2 0x006dde30
RedPath::RedPath() : mName(0), mType(Bad)
{
}

// FUNCTION: SWBF2 0x006dde80
RedPath::~RedPath()
{
    sPathList.RemoveAll();
}

// FUNCTION: SWBF2 0x006ddfa0
template void PblList<RedPath>::RemoveAll();

// FUNCTION: SWBF2 0x006dded0
void RedPath::List::Add(RedPath* const pPath)
{
    PblList<RedPath>::Add(&pPath->mNode, pPath);
}

// FUNCTION: SWBF2 0x006ddf00
void RedPath::AddToPathList(RedPath* const pPath)
{
    sPathList.Add(pPath);
}

// FUNCTION: SWBF2 0x006ddf20
bool RedPath::NodeArray::AllocNodes(unsigned int uNumNodes)
{
    if (uNumNodes)
    {
        mNodes = new Node[uNumNodes];
        if (mNodes)
        {
            mNumNodes = uNumNodes;
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006ddfe0
void RedPath::ClearPaths()
{
    sPathList.RemoveAll();
}

// FUNCTION: SWBF2 0x006ddff0
const RedPath* RedPath::List::Find(unsigned int uName) const
{
    for (PblList<RedPath>::Itor it(*this); it; ++it)
    {
        if (it->GetName() == uName)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006de030
void RedPath::NodeArray::FreeNodes()
{
    if (mNodes)
    {
        delete[] mNodes;
        mNumNodes = 0;
    }
}

// FUNCTION: SWBF2 0x006de050
unsigned int RedPath::GetName() const
{
    return mName;
}

// FUNCTION: SWBF2 0x006de6d0
void RedPath::SetName(unsigned int uName)
{
    mName = uName;
}

// FUNCTION: SWBF2 0x006de060
RedPath::Node& RedPath::NodeArray::GetNode(unsigned int i)
{
    return mNodes[i];
}

// FUNCTION: SWBF2 0x006de080
const RedPath::Node& RedPath::NodeArray::GetNode(unsigned int i) const
{
    return mNodes[i];
}

// FUNCTION: SWBF2 0x006de0a0
const RedPath::Node& RedPath::GetNode(int i) const
{
    return mNodeArray.GetNode(i);
}

// FUNCTION: SWBF2 0x006de0b0
unsigned int RedPath::NodeArray::GetNumNodes() const
{
    return mNumNodes;
}

// FUNCTION: SWBF2 0x006de0c0
int RedPath::GetNumPoints() const
{
    return mNodeArray.GetNumNodes();
}

// FUNCTION: SWBF2 0x006de0d0
const PblVector3& RedPath::GetPointPosition(int i) const
{
    return GetNode(i).mPosition;
}

// FUNCTION: SWBF2 0x006de0e0
const PblQuaternion& RedPath::GetPointRotation(int i) const
{
    return GetNode(i).mRotation;
}

// FUNCTION: SWBF2 0x006de100
RedPath::SplineType RedPath::GetSplineType() const
{
    return mSplineType;
}

// FUNCTION: SWBF2 0x006de110
bool RedPath::Read(PblConfig& config)
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
                          Node::ReadPropertyScope, 0);
            }
            break;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006de240
bool RedPath::Node::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                      PathObject object, PathObjectType type, void* pUserData)
{
    return true;
}

// FUNCTION: SWBF2 0x006de250
bool RedPath::ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                PathObject object, PathObjectType type, void* pUserData)
{
    return true;
}

// FUNCTION: SWBF2 0x006de260
bool RedPath::Node::ReadScope(PblConfig& config, PblConfig::Data& data,
                              bool (__cdecl* pPropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                              void* pUserData)
{
    switch (data.GetId())
    {
    case 0x11DE6CDCu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data child = scope.ReadNextData();
                if (pPropFn)
                {
                    PathObject object;
                    object.node = this;
                    pPropFn(scope, child, object, pathObjectType_PathNode, pUserData);
                }
            }
        }
        return true;
    case 0x21AC415Fu:
        mRotation._fS = data.GetFloatArg(0);
        mRotation._fX = data.GetFloatArg(1);
        mRotation._fY = data.GetFloatArg(2);
        mRotation._fZ = data.GetFloatArg(3);
        return true;
    case 0x934F4E0Au:
        mPosition.x = data.GetFloatArg(0);
        mPosition.y = data.GetFloatArg(1);
        mPosition.z = -data.GetFloatArg(2);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006de3a0
bool RedPath::NodeArray::ReadScope(PblConfig& config, PblConfig::Data& data,
                                   bool (__cdecl* pPropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                                   void* pUserData)
{
    if (data.GetId() != 0x514E905Au)
    {
        return false;
    }

    AllocNodes(data.GetUint32Arg(0) + sAllocExtraPathNodes);

    PblConfig scope(config.ReadNextScope(), true);
    unsigned int i = 0;
    while (!scope.IsDone())
    {
        PblConfig::Data child = scope.ReadNextData();
        if (child.GetId() == 0xB1C14E0Du)
        {
            PblConfig nodeScope(scope.ReadNextScope(), true);
            while (!nodeScope.IsDone())
            {
                PblConfig::Data prop = nodeScope.ReadNextData();
                GetNode(i).ReadScope(nodeScope, prop, pPropFn, pUserData);
            }
            ++i;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006de520
bool RedPath::ReadScope(PblConfig& config, PblConfig::Data& data,
                        bool (__cdecl* pPathPropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                        void* pPathUserData,
                        bool (__cdecl* pNodePropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                        void* pNodeUserData)
{
    switch (data.GetId())
    {
    case 0x89348E46u:
        mType = (Type)data.GetUint32Arg(0);
        return true;
    case 0x11DE6CDCu:
        {
            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data child = scope.ReadNextData();
                if (pPathPropFn)
                {
                    PathObject object;
                    object.path = this;
                    pPathPropFn(scope, child, object, pathObjectType_Path, pPathUserData);
                }
            }
        }
        return true;
    case 0x8134A814u:
        {
            unsigned int uSpline = PblHash(data.GetStringArg(0));
            if (uSpline == 0x002DBEBEu)
            {
                mSplineType = CatmullRom;
            }
            else if (uSpline == 0x9C265311u)
            {
                mSplineType = Hermite;
            }
            else
            {
                mSplineType = uSpline == 0xD00594C0u ? Linear : None;
            }
        }
        break;
    case 0x67F79061u:
    case 0x9259CD79u:
    case 0xD7BDF66Fu:
    case 0xD872E2A5u:
        break;
    default:
        if (mNodeArray.ReadScope(config, data, pNodePropFn, pNodeUserData))
        {
            return true;
        }
        break;
    }
    return false;
}

// SYNTHETIC: SWBF2 0x006ddea0
// RedPath::`scalar deleting destructor'
