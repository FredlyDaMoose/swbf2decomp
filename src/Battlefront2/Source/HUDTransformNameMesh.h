#pragma once

#include <stddef.h>
#include "HUDTransform.h"

class RedModel;

namespace HUD {
class TransformNameMesh : public HUD::Transform
{
public:

    TransformNameMesh(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);
    virtual ~TransformNameMesh();

    // VTABLE: SWBF2 0x007a32cc

    class Factory : public HUD::Item::Factory
    {
    public:
        Factory();

        // SYNTHETIC: SWBF2 0x00564b70
        // HUD::TransformNameMesh::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    class NameMesh
    {
    public:
        NameMesh();
        ~NameMesh();

        NameMesh& operator=(NameMesh& other);

        void Init(unsigned int uHashID, unsigned int uMeshID, bool bInherited);
        void Init(const char* pName, const char* pMeshName);

        unsigned int GetNameHash() const;
        unsigned int GetMeshNameHash() const;
        const char* GetName() const;
        const char* GetMeshName() const;
        bool IsInherited() const;

        const RedModel* GetMesh(const char* const pWhere);

        static int __cdecl CompareSort(const void* p1, const void* p2);

        static int __cdecl CompareSearch(const void* p1, const void* p2);

        union Mesh
        {
            unsigned int meshID;
            RedModel* mesh;
        };

        unsigned int mHashID;
        Mesh mMesh;
        char* mName;
        char* mMeshName;
        bool mDisplayedWarning : 1;
        bool mInherited : 1;
        bool mMeshIsID : 1;
    };

    void Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData);

protected:
    bool ReadData(PblConfig& config, PblConfig::Data& data);
    void WriteData(HUD::File& file, unsigned int uTabs);

    static TransformNameMesh* FindByHashID(unsigned int uHashID);

    NameMesh* FindNameMesh(unsigned int uHashID);

    static void EventInput(const HUD::Event& event, void* const pData);

public:
    static PblListSingle sTransformNameMeshList;

    NameMesh* mMapping;
    unsigned int mNumMappings;
    PblListSingle::Node mTransformNameMeshNode;
    char* mInheritNames[16];
    unsigned int mNumInheritNames;
};
}
 
 
 
 
 
 
 
