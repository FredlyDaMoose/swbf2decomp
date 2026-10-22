#include "HUDElementModel3D.h"
#include "RedEngineFL/Graphics/RedModel.h"

static const unsigned int kPositionHash         = 0x934f4e0a;
static const unsigned int kRotationHash         = 0x21ac415f;
static const unsigned int kScaleHash            = 0x82971c71;
static const unsigned int kScaleUniformHash     = 0x745450bf;
static const unsigned int kPositionRelativeHash = 0xf9f41ffa;
static const unsigned int kDepthHash            = 0xfe759eea;
static const unsigned int kEventMeshHash        = 0xf845adf2;
static const unsigned int kLightingHash         = 0x827eb1c9;
static const unsigned int kMeshHash             = 0xa100bebc;
static const unsigned int kHardpointHash        = 0x4e2c2692;
static const unsigned int kInheritMeshInfoHash  = 0x36ea8575;
static const unsigned int kMeshInfoHash         = 0x4b426ca0;

// GLOBAL: SWBF2 0x007eba38
// SYNTHETIC: SWBF2 0x00403fd0
// `dynamic initializer for 'HUD::ElementModel3D::sModelList''
// SYNTHETIC: SWBF2 0x00768b50
// `dynamic atexit destructor for 'HUD::ElementModel3D::sModelList''
PblListDouble HUD::ElementModel3D::sModelList;

// FUNCTION: SWBF2 0x00555c50
HUD::Item* HUD::ElementModel3D::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                    void* const pUserData)
{
    HUD::Item* pItem = new ElementModel3D(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00555cd0
void* HUD::ElementModel3D::GetParent()
{
    return this;
}

// FUNCTION: SWBF2 0x00555ce0
HUD::ElementModel3D::MeshInfo::MeshInfo()
    : mMeshID(0), mDepth(0.01f), mMeshName(0)
{
    mMatrix.Reset();
    mMatrixInfo.SetMatrix(&mMatrix);
}

// FUNCTION: SWBF2 0x00555d30
HUD::ElementModel3D::MeshInfo::~MeshInfo()
{
    FreeName(&mMeshName);
}

// FUNCTION: SWBF2 0x00555d50
bool HUD::ElementModel3D::MeshInfo::Read(PblConfig& config, const bool bInScreenGroup)
{
    while (!config.IsDone())
    {
        if (config.m_pChunk->PeekNextChildId() == 0x41544144)
        {
            PblConfig::Data data = config.ReadNextData();
            bool bRead = mMatrixInfo.ReadScale(data, 0);
            bRead |= mMatrixInfo.ReadRotation(data, 0);
            bRead |= mMatrixInfo.ReadPosition(data, 0);
            if (!bRead && data.GetId() == kDepthHash)
            {
                mDepth = data.GetFloatArg(0);
            }
        }
        else if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            PblConfig scope(config.ReadNextScope(), true);
            Read(scope, bInScreenGroup);
        }
    }
    mMatrixInfo.ApplyScale();
    return true;
}

// FUNCTION: SWBF2 0x00555e50
bool HUD::ElementModel3D::MeshInfo::Write(HUD::File& file, const float fFrameWidth,
                                          const float fFrameHeight, const float fViewWidth,
                                          const float fViewHeight, unsigned int uTabs,
                                          const bool bInScreenGroup)
{
    mMatrixInfo.WritePosition(file, uTabs, fFrameWidth, fFrameHeight, fViewWidth, fViewHeight, 0,
                              bInScreenGroup);
    mMatrixInfo.WriteRotation(file, uTabs, 0);
    mMatrixInfo.WriteScale(file, uTabs, 0);
    file.WriteProperty("Depth", uTabs, mDepth, 0.01f);
    return true;
}

// FUNCTION: SWBF2 0x00555ee0
unsigned int HUD::ElementModel3D::MeshInfo::GetMeshID() const
{
    return mMeshID;
}

// FUNCTION: SWBF2 0x00555ef0
PblMatrix& HUD::ElementModel3D::MeshInfo::GetMatrix()
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00555f00
HUD::Matrix& HUD::ElementModel3D::MeshInfo::GetMatrixInfo()
{
    return mMatrixInfo;
}

// FUNCTION: SWBF2 0x00555f10
void HUD::ElementModel3D::MeshInfo::SetMeshName(const char* const pName)
{
    mMeshID = PblHash(pName);
    FreeName(&mMeshName);
    CopyName(&mMeshName, pName, 1);
}

// FUNCTION: SWBF2 0x00555f50
const char* HUD::ElementModel3D::MeshInfo::GetMeshName() const
{
    return mMeshName;
}

// FUNCTION: SWBF2 0x00555f60
HUD::ElementModel3D::MeshInfo& HUD::ElementModel3D::MeshInfo::operator=(MeshInfo& other)
{
    mMeshID = other.mMeshID;
    mMatrix = other.mMatrix;
    mMatrixInfo = other.mMatrixInfo;
    mDepth = other.mDepth;
    mMeshName = other.mMeshName;
    other.mMeshName = 0;
    return *this;
}

// FUNCTION: SWBF2 0x00555fd0
void HUD::ElementModel3D::MeshInfo::ApplyRelativePosition(float fFrameWidth, float fFrameHeight,
                                                          float fViewWidth, float fViewHeight,
                                                          bool bInScreenGroup)
{
    mMatrixInfo.ApplyRelativeMode(fFrameWidth, fFrameHeight, fViewWidth, fViewHeight, bInScreenGroup);
}

// FUNCTION: SWBF2 0x00555ff0
HUD::ElementModel3D::Factory::Factory()
    : HUD::Element::Factory(0x1ECCDEFDu)
{
    mName = "Model3D";

    mLighting.Init("Lighting", mProperties, Property::type_Bool);
    mDepth.Init("Depth", mProperties, 0.0f, 10.0f, 0.0001f);
    mModel.Init("Mesh", mProperties, Property::type_Model);
    mEventMesh.Init("EventMesh", mProperties, Property::type_Event);
}

// FUNCTION: SWBF2 0x00556130
HUD::ElementModel3D::ElementModel3D(const char* const pName, Factory::ItemCallback pCallback,
                                    void* const pUserData)
    : HUD::Element(*static_cast<Red3DModelElementLite*>(this), pName, pCallback, pUserData),
      mMeshInfo(0), mNumMeshInfo(0), mInheritedMeshInfo(false), mInheritedMeshInfoName(0),
      mMeshName(0), mHardpointName(0)
{
    Red3DModelElementLite* pElement = static_cast<Red3DModelElementLite*>(mElement);
    pElement->EnableLighting(true);
    pElement->SetDepth(0.01f);
    Red3DModelElementLite::Enable(false);
    mEventMesh.Init(EventMesh, this);
    sModelList.AddLast(&mModelNode);
}

// SYNTHETIC: SWBF2 0x006d6cb0
// Red3DModelElementLite::`scalar deleting destructor'
// FUNCTION: SWBF2 0x00556290
Red3DModelElementLite::~Red3DModelElementLite()
{
}

// SYNTHETIC: SWBF2 0x00557103
// [thunk]:HUD::ElementModel3D::`scalar deleting destructor' adjustor{256}
// FUNCTION: SWBF2 0x005562a0
HUD::ElementModel3D::~ElementModel3D()
{
    FreeName(&mMeshName);
    FreeName(&mHardpointName);
    FreeName(&mInheritedMeshInfoName);
    if (mMeshInfo && mInheritedMeshInfo)
    {
        delete[] mMeshInfo;
        mMeshInfo = 0;
    }
    if (mModelNode.IsAttached())
    {
        mModelNode.Remove();
    }
}

// FUNCTION: SWBF2 0x005563b0
void HUD::ElementModel3D::Read(PblConfig& config, Factory::ItemCallback, void* const)
{
    unsigned int uNumMeshInfo = 0;
    MeshInfo aMeshInfo[256];
    MeshInfo* pNext = aMeshInfo;
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            if (data.GetId() == kMeshInfoHash)
            {
                ++uNumMeshInfo;
                MeshInfo* pMeshInfo = pNext++;
                pMeshInfo->SetMeshName(data.GetStringArg(0));
                PblConfig scope(config.ReadNextScope(), true);
                pMeshInfo->Read(scope, InScreenGroup());
            }
        }
        else
        {
            ReadData(config, data);
        }
    }
    if (uNumMeshInfo)
    {
        mNumMeshInfo = uNumMeshInfo;
        MeshInfo* pMeshInfo = new MeshInfo[uNumMeshInfo];
        mInheritedMeshInfo = false;
        mMeshInfo = pMeshInfo;
        for (unsigned int i = 0; i < mNumMeshInfo; ++i)
        {
            mMeshInfo[i] = aMeshInfo[i];
        }
    }
    PostReadSetup();
}

// FUNCTION: SWBF2 0x00556660
void HUD::ElementModel3D::GetSize(float& fWidth, float& fHeight)
{
    fWidth = 0.0f;
    fHeight = 0.0f;
}

// FUNCTION: SWBF2 0x00556680
bool HUD::ElementModel3D::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    Red3DModelElementLite* pElement = static_cast<Red3DModelElementLite*>(mElement);
    const RedModel* pModel = pElement->GetModel();
    MeshInfo* pMeshInfo = FindMeshInfoByHashID(pModel ? pModel->GetName() : 0);
    if (pMeshInfo)
    {
        const float* pfValue = (const float*)pValue;
        bool bMeshInfoProperty = true;
        switch (uHashID)
        {
        case kPositionHash:
        {
            PblMatrix& matrix = pMeshInfo->GetMatrix();
            matrix.m[3][0] = pfValue[0];
            matrix.m[3][1] = pfValue[1];
            matrix.m[3][2] = pfValue[2];
            matrix.m[3][3] = 1.0f;
            SetModel(pElement->GetModel());
            break;
        }
        case kRotationHash:
            pMeshInfo->GetMatrixInfo().SetRotation(pfValue[0], pfValue[1], pfValue[2]);
            SetModel(pElement->GetModel());
            break;
        case kScaleUniformHash:
        {
            HUD::Matrix& info = pMeshInfo->GetMatrixInfo();
            info.ApplyRotation();
            info.SetScale(pfValue[0], pfValue[0], pfValue[0]);
            info.ApplyScale();
            SetModel(pElement->GetModel());
            break;
        }
        case kScaleHash:
        {
            HUD::Matrix& info = pMeshInfo->GetMatrixInfo();
            info.ApplyRotation();
            info.SetScale(pfValue[0], pfValue[1], pfValue[2]);
            info.ApplyScale();
            SetModel(pElement->GetModel());
            break;
        }
        case kPositionRelativeHash:
            pMeshInfo->GetMatrixInfo().SetRelativeMode(*(const RelativeMode*)pValue);
            SetModel(pElement->GetModel());
            break;
        case kDepthHash:
            pMeshInfo->SetDepth(*pfValue);
            if (pMeshInfo == &mDefault)
            {
                pElement->SetDepth(*pfValue);
            }
            return true;
        default:
            bMeshInfoProperty = false;
            break;
        }
        if (bMeshInfoProperty)
        {
            if (pMeshInfo != &mDefault)
            {
                return true;
            }
            return HUD::Element::SetProperty(uHashID, pValue);
        }
    }

    switch (uHashID)
    {
    case kDepthHash:
        pElement->SetDepth(*(const float*)pValue);
        return true;
    case kEventMeshHash:
        Item::SetEvent(*(const char* const*)pValue, mEventMesh);
        return true;
    case kLightingHash:
        pElement->EnableLighting(*(const bool*)pValue);
        return true;
    case kMeshHash:
        if (pValue)
        {
            SetModel(*(const RedModel* const*)pValue);
        }
        return true;
    }
    return HUD::Element::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x005568c0
bool HUD::ElementModel3D::GetProperty(const unsigned int uHashID, void* const pValue)
{
    Red3DModelElementLite* pElement = static_cast<Red3DModelElementLite*>(mElement);
    const RedModel* pModel = pElement->GetModel();
    MeshInfo* pMeshInfo = FindMeshInfoByHashID(pModel ? pModel->GetName() : 0);
    if (pMeshInfo)
    {
        float* pfValue = (float*)pValue;
        switch (uHashID)
        {
        case kPositionHash:
        {
            const PblMatrix& matrix = pMeshInfo->GetMatrix();
            pfValue[0] = matrix.m[3][0];
            pfValue[1] = matrix.m[3][1];
            pfValue[2] = matrix.m[3][2];
            return true;
        }
        case kRotationHash:
            *(PblVector3*)pValue = pMeshInfo->GetMatrixInfo().GetRotation();
            return true;
        case kScaleUniformHash:
        {
            const PblVector3 scale = pMeshInfo->GetMatrixInfo().GetScale();
            float fScale = scale.x > scale.y ? scale.x : scale.y;
            *pfValue = fScale > scale.z ? fScale : scale.z;
            return true;
        }
        case kScaleHash:
            *(PblVector3*)pValue = pMeshInfo->GetMatrixInfo().GetScale();
            return true;
        case kPositionRelativeHash:
            *(RelativeMode*)pValue = pMeshInfo->GetMatrixInfo().GetRelativeMode();
            return true;
        case kDepthHash:
            *pfValue = pMeshInfo->GetDepth();
            return true;
        }
    }

    switch (uHashID)
    {
    case kDepthHash:
        *(float*)pValue = pElement->GetDepth();
        return true;
    case kEventMeshHash:
        *(const char**)pValue = mEventMesh.GetRegisteredClassName(false);
        return true;
    case kLightingHash:
        *(bool*)pValue = pElement->IsLightningEnabled();
        return true;
    case kMeshHash:
        *(const RedModel**)pValue = pModel;
        return true;
    }
    return HUD::Element::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00556ab0
void HUD::ElementModel3D::SetModel(const RedModel* const pModel)
{
    Red3DModelElementLite::SetModel(pModel);
    SetMesh(pModel);
}

// FUNCTION: SWBF2 0x00556ae0
void HUD::ElementModel3D::SetModel(unsigned int uModelHash)
{
    Red3DModelElementLite::SetModel(uModelHash);
    SetMesh(GetModel());
}

// FUNCTION: SWBF2 0x00556b10
void HUD::ElementModel3D::SetModel(const char* const pName)
{
    Red3DModelElementLite::SetModel(pName);
    SetMeshName(pName);
    SetMesh(GetModel());
}

// FUNCTION: SWBF2 0x00556b40
HUD::ElementModel3D& HUD::ElementModel3D::operator=(const ElementModel3D& other)
{
    HUD::Element::operator=(other);
    SetModel(other.GetModel());
    EnableLighting(other.IsLightningEnabled());
    SetDepth(other.GetDepth());
    SetBone(other.GetBone());
    return *this;
}

// FUNCTION: SWBF2 0x00556ba0
void HUD::ElementModel3D::SetMesh(const RedModel* const pModel)
{
    RedInterfaceElement* pElement = mElement;
    if (pModel)
    {
        MeshInfo* pMeshInfo = FindMeshInfoByHashID(pModel->GetName());
        if (pMeshInfo)
        {
            pElement->SetMatrix(pMeshInfo->GetMatrix());
            static_cast<Red3DModelElementLite*>(pElement)->SetDepth(pMeshInfo->GetDepth());
        }
    }
}

// FUNCTION: SWBF2 0x00556be0
void HUD::ElementModel3D::SetMeshName(const char* const pName)
{
    FreeName(&mMeshName);
    CopyName(&mMeshName, pName, 0);
}

// FUNCTION: SWBF2 0x00556c10
bool HUD::ElementModel3D::ReadData(PblConfig& config, PblConfig::Data& data)
{
    Red3DModelElementLite* pElement = static_cast<Red3DModelElementLite*>(mElement);
    switch (data.GetId())
    {
    case kLightingHash:
        pElement->EnableLighting(data.GetBoolArg(0));
        return true;
    case kHardpointHash:
    {
        const char* pName = data.GetStringArg(0);
        CopyName(&mHardpointName, pName, 0);
        pElement->SetBone(pName);
        return true;
    }
    case kInheritMeshInfoHash:
    {
        const char* pName = data.GetStringArg(0);
        FreeName(&mInheritedMeshInfoName);
        CopyName(&mInheritedMeshInfoName, pName, 0);
        ElementModel3D* pModel3D = FindByHashID(PblHash(pName));
        if (pModel3D)
        {
            mMeshInfo = pModel3D->mMeshInfo;
            mInheritedMeshInfo = true;
            mNumMeshInfo = pModel3D->mNumMeshInfo;
        }
        return true;
    }
    case kMeshHash:
        SetModel(data.GetStringArg(0));
        return true;
    case kEventMeshHash:
        Item::ReadEvent(data, mEventMesh);
        return true;
    case kDepthHash:
        pElement->SetDepth(data.GetFloatArg(0));
        return true;
    }
    return HUD::Element::ReadData(config, data);
}

// FUNCTION: SWBF2 0x00556d70
void HUD::ElementModel3D::PostReadSetup()
{
    HUD::Element::PostReadSetup();
    mDefault.GetMatrix() = mElement->m_mLocal;
    for (unsigned int i = 0; i < mNumMeshInfo; ++i)
    {
        mMeshInfo[i].ApplyRelativePosition(GetContainerFrameWidth(), GetContainerFrameHeight(),
                                           GetContainerViewWidth(), GetContainerViewHeight(),
                                           InScreenGroup());
    }
}

// FUNCTION: SWBF2 0x00556e30
void HUD::ElementModel3D::WriteData(HUD::File& file, unsigned int uTabs)
{
    Red3DModelElementLite* pElement = static_cast<Red3DModelElementLite*>(mElement);
    pElement->SetMatrix(mDefault.GetMatrix());
    file.WriteProperty("Mesh", uTabs, mMeshName, "");
    file.WriteProperty("Lighting", uTabs, (int)pElement->IsLightningEnabled(), 1);
    file.WriteProperty("Depth", uTabs, pElement->GetDepth(), 0.01f);
    file.WriteProperty("Hardpoint", uTabs, mHardpointName, "");
    file.WriteProperty("EventMesh", uTabs, mEventMesh.GetRegisteredClassName(true), "");
    file.WriteProperty("InheritMeshInfo", uTabs, mInheritedMeshInfoName, "");
    HUD::Element::WriteData(file, uTabs);
    if (!mInheritedMeshInfo)
    {
        for (unsigned int i = 0; i < mNumMeshInfo; ++i)
        {
            file.WriteTabs(uTabs);
            file.WriteStringf("MeshInfo(\"%s\")\n", mMeshInfo[i].GetMeshName());
            file.WriteTabs(uTabs);
            file.WriteString("{\n");
            mMeshInfo[i].Write(file, GetContainerFrameWidth(), GetContainerFrameHeight(),
                               GetContainerViewWidth(), GetContainerViewHeight(), uTabs + 1,
                               InScreenGroup());
            file.WriteTabs(uTabs);
            file.WriteString("}\n\n");
        }
    }
    SetMesh(pElement->GetModel());
}

// FUNCTION: SWBF2 0x00557010
HUD::ElementModel3D::MeshInfo* HUD::ElementModel3D::FindMeshInfoByHashID(unsigned int uHashID)
{
    for (unsigned int i = 0; i < mNumMeshInfo; ++i)
    {
        if (mMeshInfo[i].GetMeshID() == uHashID)
        {
            return &mMeshInfo[i];
        }
    }
    return &mDefault;
}

// FUNCTION: SWBF2 0x00557070
HUD::ElementModel3D* HUD::ElementModel3D::FindByHashID(unsigned int uHashID)
{
    for (PblListDouble::Node* pNode = sModelList.mTerminator.mNext; pNode != &sModelList.mTerminator;
         pNode = pNode->mNext)
    {
        ElementModel3D* pModel3D =
            (ElementModel3D*)((char*)pNode - offsetof(ElementModel3D, mModelNode));
        if (pModel3D->GetNameHash() == uHashID)
        {
            return pModel3D;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005570b0
void HUD::ElementModel3D::EventMesh(const HUD::Event& event, void* const pData)
{
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Uint:
        ((ElementModel3D*)pData)->SetModel(event.GetData().uintValue);
        break;
    case EventClass::type_Model:
        ((ElementModel3D*)pData)->SetModel(event.GetData().model);
        break;
    }
}
