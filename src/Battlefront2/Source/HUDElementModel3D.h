#pragma once

#include <stddef.h>
#include "HUDElement.h"
#include "RedEngineFL/Interface/Red3DModelElementLite.h"

class RedModel;

namespace HUD {
// VTABLE: SWBF2 0x007a136c
class ElementModel3D : public HUD::Element, public Red3DModelElementLite
{
public:

    // VTABLE: SWBF2 0x007a1350

    class Factory : public HUD::Element::Factory
    {
    public:
        // FUNCTION: SWBF2 0x00555ff0
        Factory();

        // SYNTHETIC: SWBF2 0x005560e0
        // HUD::ElementModel3D::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mLighting;
        Property mDepth;
        Property mModel;
        Property mEventMesh;
    };

    // VTABLE: SWBF2 0x007a135c
    class MeshInfo
    {
    public:
        MeshInfo();
        virtual ~MeshInfo();
        // SYNTHETIC: SWBF2 0x00556600
        // HUD::ElementModel3D::MeshInfo::`vector deleting destructor'
        virtual bool Read(PblConfig& config, const bool bInScreenGroup);
        virtual bool Write(HUD::File& file, const float fFrameWidth, const float fFrameHeight,
                           const float fViewWidth, const float fViewHeight, unsigned int uTabs,
                           const bool bInScreenGroup);

        MeshInfo& operator=(MeshInfo& other);

        unsigned int GetMeshID() const;
        PblMatrix& GetMatrix();
        HUD::Matrix& GetMatrixInfo();
        void SetMeshName(const char* const pName);
        const char* GetMeshName() const;
        void ApplyRelativePosition(float fFrameWidth, float fFrameHeight, float fViewWidth,
                                   float fViewHeight, bool bInScreenGroup);

        void SetDepth(float fDepth) { mDepth = fDepth; }
        float GetDepth() const { return mDepth; }

        unsigned int mMeshID;
        PblMatrix mMatrix;
        HUD::Matrix mMatrixInfo;
        float mDepth;
        char* mMeshName;
    };

    ElementModel3D(const char* const pName = 0, Factory::ItemCallback pCallback = 0,
                   void* const pUserData = 0);
    virtual ~ElementModel3D();
    // SYNTHETIC: SWBF2 0x00556260
    // HUD::ElementModel3D::`scalar deleting destructor'

    ElementModel3D& operator=(const ElementModel3D& other);

    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback,
                      void* const pUserData);
    virtual bool SetProperty(const unsigned int, const void*);
    virtual bool GetProperty(const unsigned int, void*);
    virtual void GetSize(float& fWidth, float& fHeight);

    virtual void* GetParent();

    void SetModel(const RedModel* const pModel);
    void SetModel(unsigned int uModelHash);
    void SetModel(const char* const pName);

    static ElementModel3D* FindByHashID(unsigned int uHashID);

protected:
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    void SetMesh(const RedModel* const pModel);
    void SetMeshName(const char* const pName);
    MeshInfo* FindMeshInfoByHashID(unsigned int uHashID);

    static void EventMesh(const HUD::Event& event, void* const pData);

public:

    static PblListDouble sModelList;

    HUD::EventHandler mEventMesh;
    MeshInfo mDefault;
    MeshInfo* mMeshInfo;
    unsigned int mNumMeshInfo;
    PblListDouble::Node mModelNode;
    bool mInheritedMeshInfo : 1;
    char* mInheritedMeshInfoName;
    char* mMeshName;
    char* mHardpointName;
};
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

 
 
 
