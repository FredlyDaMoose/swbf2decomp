#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"

// VTABLE: SWBF2 0x007834cc
class RedPath
{
public:

    RedPath();
    virtual ~RedPath();
    virtual bool Read(PblConfig &);

    enum Type
    {
        OneWay = 0,
        RoundTrip = 1,
        Loop = 2,
        Bad = 3
    };
    enum SplineType
    {
        None = 0,
        Linear = 1,
        Hermite = 2,
        CatmullRom = 3
    };

    union PathObject;
    enum PathObjectType
    {
        pathObjectType_Invalid = 0,
        pathObjectType_Path = 1,
        pathObjectType_PathNode = 2
    };

    class Node
    {
    public:
        Node()
        {
            mRotation._fX = 0.0f;
            mRotation._fY = 0.0f;
            mRotation._fZ = 0.0f;
            mRotation._fS = 1.0f;
        }

        static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                      PathObject object, PathObjectType type, void* pUserData);

        bool ReadScope(PblConfig& config, PblConfig::Data& data,
                       bool (__cdecl* pPropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                       void* pUserData);

        PblVector3 mPosition;
        PblQuaternion mRotation;
    };

    class NodeArray
    {
    public:

        NodeArray();
        ~NodeArray() { FreeNodes(); }

        bool AllocNodes(unsigned int uNumNodes);
        void FreeNodes();
        unsigned int GetNumNodes() const;
        const Node& GetNode(unsigned int i) const;
        Node& GetNode(unsigned int i);

        bool ReadScope(PblConfig& config, PblConfig::Data& data,
                       bool (__cdecl* pPropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                       void* pUserData);

        Node* mNodes;
        unsigned int mNumNodes;
    };

    class List : public PblList<RedPath>
    {
    public:
        void Add(RedPath* const pPath);
        const RedPath* Find(unsigned int uName) const;
    };

    union PathObject
    {
        RedPath* path;
        Node* node;
    };

    static void AddToPathList(RedPath* const pPath);
    static void ClearPaths();

    unsigned int GetName() const;

    void SetName(unsigned int uName);
    SplineType GetSplineType() const;
    int GetNumPoints() const;
    const Node& GetNode(int i) const;
    const PblVector3& GetPointPosition(int i) const;
    const PblQuaternion& GetPointRotation(int i) const;

    bool ReadScope(PblConfig& config, PblConfig::Data& data,
                   bool (__cdecl* pPathPropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                   void* pPathUserData,
                   bool (__cdecl* pNodePropFn)(PblConfig&, PblConfig::Data&, PathObject, PathObjectType, void*),
                   void* pNodeUserData);

protected:
    static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                  PathObject object, PathObjectType type, void* pUserData);

public:
    PblList<RedPath>::Node mNode;
    unsigned int mName;
    NodeArray mNodeArray;
    Type mType;
    SplineType mSplineType;

    static List sPathList;

    static const RedPath* Find(unsigned int uName) { return sPathList.Find(uName); }

    static int sAllocExtraPathNodes;
};
 
 
 
 
 
 
