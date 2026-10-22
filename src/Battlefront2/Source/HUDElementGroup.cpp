#include <d3dx9math.h>
#include "PebbleFL/Common/PblVector.h"

#include "HUDElementGroup.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

// GLOBAL: SWBF2 0x007e6940

char* HUD::ElementGroupBase::Factory::sHAlignStrings[3] =
{
    "Left", "Center", "Right"
};

// GLOBAL: SWBF2 0x007e694c

char* HUD::ElementGroupBase::Factory::sVAlignStrings[3] =
{
    "Top", "Center", "Bottom"
};

// FUNCTION: SWBF2 0x0054e1c0
HUD::ElementGroupBase::Factory::Factory(unsigned int uHashID)
    : HUD::Element::Factory(uHashID)
{
    mRect.Init("Rect", mProperties, Property::type_Vector2);
    mRect.InitVectorRange(Property::axis_X, 0.0f, 2.0f, 0.05f);
    mRect.InitVectorRange(Property::axis_Y, 0.0f, 2.0f, 0.05f);
    mRelativeSize.Init("RelativeSize", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mHAlign.Init("HAlign", mProperties, sHAlignStrings, 0, 2);
    mVAlign.Init("VAlign", mProperties, sVAlignStrings, 0, 2);
    mPropagateAlpha.Init("PropagateAlpha", mProperties, Property::type_Bool);
    mEventPosition.Init("EventPosition", mProperties, Property::type_Event);
    mEventScale.Init("EventScale", mProperties, Property::type_Event);
    mEventAlpha.Init("EventAlpha", mProperties, Property::type_Event);
    mEventRotation.Init("EventRotation", mProperties, Property::type_Event);
}

HUD::ElementGroup::Factory::Factory()
    : HUD::ElementGroupBase::Factory(0x5FB91E8Cu)
{
    mName = "Group";
}

static const unsigned int kRect = 0xeae44d07;
static const unsigned int kRelativeSize = 0x07c10754;
static const unsigned int kHAlign = 0x4a28cb3c;
static const unsigned int kVAlign = 0x5a9dcfe2;
static const unsigned int kPropagateAlpha = 0x09b602b0;
static const unsigned int kEventPosition = 0x7eb5b01c;
static const unsigned int kEventScale = 0x37f1161f;
static const unsigned int kEventAlpha = 0x41b355c1;
static const unsigned int kEventRotation = 0x36f7de41;

static inline RedGroupElement* GetGroupElement(const HUD::Element* pElement)
{
    return static_cast<RedGroupElement*>(pElement->mElement);
}

// FUNCTION: SWBF2 0x0054e3b0
HUD::ElementGroupBase::ElementGroupBase(RedGroupElement& group, const char* const pName,
                                       Factory::ItemCallback pCallback, void* const pUserData)
    : HUD::Element(group, pName, pCallback, pUserData),
      mWidth(1.0f),
      mHeight(1.0f),
      mRelativeMode(relativeMode_Viewport),
      mHAlign(hAlign_left),
      mVAlign(vAlign_top)
{
    mShowChildren = false;
    group.PropagateAlpha(false);
    mEventPosition.Init(EventPosition, this);
    mEventScale.Init(EventScale, this);
    mEventAlpha.Init(EventAlpha, this);
    mEventRotation.Init(EventRotation, this);
}

// FUNCTION: SWBF2 0x0054e4c0
HUD::ElementGroupBase::~ElementGroupBase()
{
    RemoveAllElements();
    mElementList.Clear();
}

// FUNCTION: SWBF2 0x0054e580
void HUD::ElementGroupBase::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                 void* const pUserData)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            ReadChild(config, data, pCallback, pUserData);
        }
        else
        {
            ReadData(config, data);
        }
    }
    PostReadSetup();
}

// FUNCTION: SWBF2 0x0054e610
unsigned int HUD::ElementGroupBase::GetWidthPixels() const
{
    return (int)GetWidthPixelsFloat();
}

// FUNCTION: SWBF2 0x0054e620
unsigned int HUD::ElementGroupBase::GetHeightPixels() const
{
    return (int)GetHeightPixelsFloat();
}

// FUNCTION: SWBF2 0x0054e630
float HUD::ElementGroupBase::GetWidthPixelsFloat() const
{
    return ConvertRelativeToPixels((RelativeMode)mRelativeMode, mWidth,
                                   GetContainerFrameWidth(), GetContainerViewWidth(),
                                   (float)RedInterfaceScreen::GetScreenWidth());
}

// FUNCTION: SWBF2 0x0054e690
float HUD::ElementGroupBase::GetHeightPixelsFloat() const
{
    return ConvertRelativeToPixels((RelativeMode)mRelativeMode, mHeight,
                                   GetContainerFrameHeight(), GetContainerViewHeight(),
                                   (float)RedInterfaceScreen::GetScreenHeight());
}

// FUNCTION: SWBF2 0x0054e6f0
bool HUD::ElementGroupBase::SetProperty(const unsigned int uHashID, const void* pValue)
{
    switch (uHashID)
    {
    case kRect:
        mWidth = ((const float*)pValue)[0];
        mHeight = ((const float*)pValue)[1];
        return true;
    case kRelativeSize:
        mRelativeMode = (unsigned char)*(const unsigned int*)pValue;
        return true;
    case kHAlign:
        SetAlignment((HAlign)*(const int*)pValue, (VAlign)mVAlign, true, true, true);
        return true;
    case kVAlign:
        SetAlignment((HAlign)mHAlign, (VAlign)*(const int*)pValue, true, true, true);
        return true;
    case kPropagateAlpha:
        GetGroupElement(this)->PropagateAlpha(*(const bool*)pValue);
        return true;
    case kEventPosition:
        Item::SetEvent(*(const char* const*)pValue, mEventPosition);
        return true;
    case kEventScale:
        Item::SetEvent(*(const char* const*)pValue, mEventScale);
        return true;
    case kEventAlpha:
        Item::SetEvent(*(const char* const*)pValue, mEventAlpha);
        return true;
    case kEventRotation:
        Item::SetEvent(*(const char* const*)pValue, mEventRotation);
        return true;
    }
    return Element::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0054e840
bool HUD::ElementGroupBase::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kRect:
        ((float*)pValue)[0] = mWidth;
        ((float*)pValue)[1] = mHeight;
        return true;
    case kRelativeSize:
        *(unsigned int*)pValue = mRelativeMode;
        return true;
    case kHAlign:
        *(unsigned int*)pValue = mHAlign;
        return true;
    case kVAlign:
        *(unsigned int*)pValue = mVAlign;
        return true;
    case kPropagateAlpha:
        *(bool*)pValue = GetGroupElement(this)->GetPropagateAlpha();
        return true;
    case kEventPosition:
        *(const char**)pValue = mEventPosition.GetRegisteredClassName(false);
        return true;
    case kEventScale:
        *(const char**)pValue = mEventScale.GetRegisteredClassName(false);
        return true;
    case kEventAlpha:
        *(const char**)pValue = mEventAlpha.GetRegisteredClassName(false);
        return true;
    case kEventRotation:
        *(const char**)pValue = mEventRotation.GetRegisteredClassName(false);
        return true;
    }
    return Element::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0054e970
const char* HUD::ElementGroupBase::PropertyTranslateEnum(const unsigned int uHashID,
                                                         const unsigned int uValue)
{
    switch (uHashID)
    {
    case kRelativeSize:
        return gRelativeModeStrings[uValue];
    case kHAlign:
        return Factory::sHAlignStrings[uValue];
    case kVAlign:
        return Factory::sVAlignStrings[uValue];
    }
    return Element::PropertyTranslateEnum(uHashID, uValue);
}

// FUNCTION: SWBF2 0x0054e9c0
bool HUD::ElementGroupBase::AddElement(HUD::Element* const pElement)
{
    if (pElement->mGroup)
    {
        pElement->mGroup->RemoveElement(pElement);
    }
    bool bResult = GetGroupElement(this)->AddElement(pElement->mElement);
    if (pElement->mElement->GetParent())
    {
        pElement->mGroup = this;
    }
    return bResult;
}

// FUNCTION: SWBF2 0x0054ea10
void HUD::ElementGroupBase::RemoveElement(HUD::Element* const pElement)
{
    if (pElement->mGroup)
    {
        GetGroupElement(this)->RemoveElement(pElement->mElement);
        pElement->mGroup = 0;
        if (pElement->mGroupNode.IsAttached())
        {
            pElement->mGroupNode.Remove();
        }
    }
}

// FUNCTION: SWBF2 0x0054ea70
void HUD::ElementGroupBase::RemoveAllElements()
{
    RedGroupElement* pGroup = GetGroupElement(this);
    for (PblList<RedInterfaceElement>::Itor it = pGroup->GetChildrenList().Begin(); it;)
    {
        HUD::Element* pElement = static_cast<HUD::Element*>(it->GetParent());
        ++it;
        if (pElement)
        {
            RemoveElement(pElement);
        }
    }
    pGroup->RemoveAllElements();
}

// FUNCTION: SWBF2 0x0054eab0
bool HUD::ElementGroupBase::ReadChild(PblConfig& config, PblConfig::Data& data,
                                      Factory::ItemCallback pCallback, void* const pUserData)
{
    Item::Factory* pFactory = Item::Factory::FindByHashID(data.GetId());
    if (!pFactory)
    {
        return false;
    }

    const char* pName = data.GetNumArgs() ? data.GetStringArg(0) : "Unnamed";
    Item* pItem = pFactory->CreateItem(pName, pCallback, pUserData);
    if (!pItem)
    {
        return false;
    }

    PblConfig scope(config.ReadNextScope(), true);
    if (pFactory->GetType() == Item::Factory::type_Element)
    {
        HUD::Element* pElement = static_cast<HUD::Element*>(pItem);
        AddElement(pElement);
        mElementList.AddLast(&pElement->mGroupNode);
        pElement->SetViewport(GetViewport());
    }
    pItem->Read(scope, pCallback, pUserData);
    return true;
}

// FUNCTION: SWBF2 0x0054eb80
bool HUD::ElementGroupBase::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kRect:
    {
        const unsigned int uNumArgs = data.GetNumArgs();
        HAlign eHAlign = hAlign_left;
        VAlign eVAlign = vAlign_top;
        mWidth = data.GetFloatArg(0);
        mHeight = data.GetFloatArg(1);
        if (uNumArgs > 2)
        {
            unsigned int uIndex = TranslateString(Factory::sHAlignStrings, 3, data.GetStringArg(2));
            eHAlign = uIndex < 3 ? (HAlign)uIndex : hAlign_left;
        }
        if (uNumArgs > 3)
        {
            unsigned int uIndex = TranslateString(Factory::sVAlignStrings, 3, data.GetStringArg(3));
            eVAlign = uIndex < 3 ? (VAlign)uIndex : vAlign_top;
        }
        if (uNumArgs > 4)
        {

            int iMode;
            if (data.IsStringArg(4))
            {
                iMode = (int)TranslateString(gRelativeModeStrings, 3, data.GetStringArg(4));
            }
            else
            {
                iMode = data.GetBoolArg(4) ? relativeMode_Viewport : relativeMode_Pixels;
            }
            if (iMode >= 0)
            {
                mRelativeMode = (unsigned char)iMode;
            }
        }
        mHAlign = (unsigned char)eHAlign;
        mVAlign = (unsigned char)eVAlign;
        return true;
    }
    case kPropagateAlpha:
        GetGroupElement(this)->PropagateAlpha(data.GetBoolArg(0));
        return true;
    case kEventPosition:
        Item::ReadEvent(data, mEventPosition);
        return true;
    case kEventScale:
        Item::ReadEvent(data, mEventScale);
        return true;
    case kEventAlpha:
        Item::ReadEvent(data, mEventAlpha);
        return true;
    case kEventRotation:
        Item::ReadEvent(data, mEventRotation);
        return true;
    }
    return Element::ReadData(config, data);
}

// FUNCTION: SWBF2 0x0054ed60
void HUD::ElementGroupBase::PostReadSetup()
{
    Element::PostReadSetup();
    HAlign eHAlign = (HAlign)mHAlign;
    VAlign eVAlign = (VAlign)mVAlign;
    mHAlign = hAlign_left;
    mVAlign = vAlign_top;
    SetAlignment(eHAlign, eVAlign, true, true, true);
}

// FUNCTION: SWBF2 0x0054ed90
void HUD::ElementGroupBase::WriteData(HUD::File& file, unsigned int uTabs)
{
    if (mWidth != 1.0f || mHeight != 1.0f || mHAlign != hAlign_left || mVAlign != vAlign_top ||
        mRelativeMode != relativeMode_Viewport)
    {
        file.WriteTabs(uTabs);
        file.WriteStringf("Rect(%f, %f, \"%s\", \"%s\", \"%s\")\n",
                          ConvertPixelsToRelative((RelativeMode)mRelativeMode,
                                                  GetWidthPixelsFloat(),
                                                  GetContainerFrameWidth(),
                                                  GetContainerViewWidth(),
                                                  (float)RedInterfaceScreen::GetScreenWidth()),
                          ConvertPixelsToRelative((RelativeMode)mRelativeMode,
                                                  GetHeightPixelsFloat(),
                                                  GetContainerFrameHeight(),
                                                  GetContainerViewHeight(),
                                                  (float)RedInterfaceScreen::GetScreenHeight()),
                          Factory::sHAlignStrings[mHAlign], Factory::sVAlignStrings[mVAlign],
                          gRelativeModeStrings[mRelativeMode]);
    }
    file.WriteProperty("PropagateAlpha", uTabs, (int)GetGroupElement(this)->GetPropagateAlpha(), 0);
    file.WriteProperty("EventPosition", uTabs, mEventPosition.GetRegisteredClassName(false), "");
    file.WriteProperty("EventRotation", uTabs, mEventRotation.GetRegisteredClassName(false), "");
    file.WriteProperty("EventScale", uTabs, mEventScale.GetRegisteredClassName(false), "");
    file.WriteProperty("EventAlpha", uTabs, mEventAlpha.GetRegisteredClassName(false), "");

    SetAlignment((HAlign)mHAlign, (VAlign)mVAlign, false, true, true);
    Element::WriteData(file, uTabs);
    SetAlignment((HAlign)mHAlign, (VAlign)mVAlign, false, false, true);

    for (PblListDouble::Node* pNode = mElementList.mTerminator.mNext;
         pNode != &mElementList.mTerminator; pNode = pNode->mNext)
    {
        HUD::Element* pElement =
            (HUD::Element*)((char*)pNode - offsetof(HUD::Element, mGroupNode));
        pElement->Write(file, uTabs, 0);
    }
}

// FUNCTION: SWBF2 0x0054f020
void HUD::ElementGroupBase::SetAlignment(HAlign eHAlign, VAlign eVAlign, bool bReapply, bool bAdd,
                                         bool bStore)
{
    const float fWidth = GetWidthPixelsFloat();
    const float fHeight = GetHeightPixelsFloat();
    const float afAlign[3] = { 0.0f, 0.5f, 1.0f };
    const float fSign = bAdd ? 1.0f : -1.0f;
    PblVector3 vPos = mElement->GetPosition();
    vPos.x += afAlign[mHAlign] * fWidth * fSign;
    vPos.y += afAlign[mVAlign] * fHeight * fSign;
    mElement->SetPosition(vPos);
    if (bStore)
    {
        mHAlign = (unsigned char)eHAlign;
        mVAlign = (unsigned char)eVAlign;
    }
    if (bReapply)
    {
        SetAlignment((HAlign)mHAlign, (VAlign)mVAlign, false, false, false);
    }
}

// FUNCTION: SWBF2 0x0054f110
void HUD::ElementGroupBase::EventPosition(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != EventClass::type_Vector3)
    {
        return;
    }

    ElementGroupBase* pGroup = (ElementGroupBase*)pData;
    PblVector3 vPos = *event.GetData().vector3;
    vPos.x = ConvertRelativeToPixels((RelativeMode)pGroup->mRelativeMode, vPos.x,
                                     pGroup->GetContainerFrameWidth(),
                                     pGroup->GetContainerViewWidth(),
                                     (float)RedInterfaceScreen::GetScreenWidth());
    vPos.y = ConvertRelativeToPixels((RelativeMode)pGroup->mRelativeMode, vPos.y,
                                     pGroup->GetContainerFrameHeight(),
                                     pGroup->GetContainerViewHeight(),
                                     (float)RedInterfaceScreen::GetScreenHeight());
    pGroup->mElement->SetPosition(vPos);
    pGroup->SetAlignment((HAlign)pGroup->mHAlign, (VAlign)pGroup->mVAlign, false, false, true);
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x0054f240
void HUD::ElementGroupBase::EventScale(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != EventClass::type_Vector3)
    {
        return;
    }

    ElementGroupBase* pGroup = (ElementGroupBase*)pData;
    const PblVector3 vScale = *event.GetData().vector3;
    PblMatrix mMatrix = pGroup->mElement->GetMatrix();
    const float afScale[3] = { vScale.x, vScale.y, vScale.z };
    for (int i = 0; i < 3; ++i)
    {
        D3DXVECTOR3 vAxis;
        D3DXVec3Normalize(&vAxis, (const D3DXVECTOR3*)mMatrix.m[i]);
        mMatrix.m[i][0] = vAxis.x * afScale[i];
        mMatrix.m[i][1] = vAxis.y * afScale[i];
        mMatrix.m[i][2] = vAxis.z * afScale[i];
        mMatrix.m[i][3] = 0.0f;
    }
    pGroup->mElement->SetMatrix(mMatrix);
}

// FUNCTION: SWBF2 0x0054f3c0
void HUD::ElementGroupBase::EventAlpha(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != EventClass::type_Float)
    {
        return;
    }

    ElementGroupBase* pGroup = (ElementGroupBase*)pData;
    const int iAlpha = (int)(event.GetData().floatValue * 255.0f);
    if (pGroup->mElement)
    {
        pGroup->mElement->SetAlphaOnly((unsigned char)iAlpha);
    }
}

// FUNCTION: SWBF2 0x0054f410
void HUD::ElementGroupBase::EventRotation(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != EventClass::type_Vector3)
    {
        return;
    }

    ElementGroupBase* pGroup = (ElementGroupBase*)pData;
    const PblVector3* pRotation = event.GetData().vector3;
    const PblMatrix mCurrent = pGroup->mElement->GetMatrix();
    const PblVector3 vPosition = mCurrent.GetTranslation();

    const float fDegToRad = 0.017453292f;
    PblMatrix mResult = _IdentityMatrix;
    D3DXMATRIX mTemp;
    D3DXMatrixRotationX((D3DXMATRIX*)&mResult, pRotation->x * fDegToRad);
    D3DXMatrixMultiply((D3DXMATRIX*)&mResult, (D3DXMATRIX*)&mResult,
                       D3DXMatrixRotationY(&mTemp, pRotation->y * fDegToRad));
    D3DXMatrixMultiply((D3DXMATRIX*)&mResult, (D3DXMATRIX*)&mResult,
                       D3DXMatrixRotationZ(&mTemp, pRotation->z * fDegToRad));

    const PblVector3& vX = mCurrent.GetXAxis();
    const PblVector3& vY = mCurrent.GetYAxis();
    const PblVector3& vZ = mCurrent.GetZAxis();
    D3DXMatrixMultiply((D3DXMATRIX*)&mResult, (D3DXMATRIX*)&mResult,
                       D3DXMatrixScaling(&mTemp,
                                         sqrtf(vX.x * vX.x + vX.y * vX.y + vX.z * vX.z),
                                         sqrtf(vY.x * vY.x + vY.y * vY.y + vY.z * vY.z),
                                         sqrtf(vZ.x * vZ.x + vZ.y * vZ.y + vZ.z * vZ.z)));

    mResult.m[3][0] = vPosition.x;
    mResult.m[3][1] = vPosition.y;
    mResult.m[3][2] = vPosition.z;
    mResult.m[3][3] = 1.0f;
    pGroup->mElement->SetMatrix(mResult);
}
