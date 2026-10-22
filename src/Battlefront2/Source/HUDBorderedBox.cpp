#include "HUDBorderedBox.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

// FUNCTION: SWBF2 0x00543ee0
HUD::Item::Factory::Type HUD::Element::GetType() const
{
    return Factory::type_Element;
}

// FUNCTION: SWBF2 0x00543ef0
bool HUD::Element::WriteEnabled() const
{
    return mWriteEnabled && !mGroupNode.IsAttached();
}

// FUNCTION: SWBF2 0x00543f10
void HUD::ElementGroupBase::GetSize(float& fWidth, float& fHeight)
{
    fWidth = GetWidthPixelsFloat();
    fHeight = GetHeightPixelsFloat();
}

// FUNCTION: SWBF2 0x00543f40
HUD::ElementGroup::ElementGroup(const char* const pName, Factory::ItemCallback pCallback,
                                void* const pUserData)
    : HUD::ElementGroupBase(*static_cast<RedGroupElement*>(this), pName, pCallback, pUserData)
{
    RedGroupElement::Enable(false);
}

// SYNTHETIC: SWBF2 0x005440c0
// HUD::ElementGroup::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00544937
// [thunk]:HUD::ElementGroup::`scalar deleting destructor' adjustor{368}
// FUNCTION: SWBF2 0x00543fd0
HUD::ElementGroup::~ElementGroup()
{
    ElementGroupBase::RemoveAllElements();
}

// FUNCTION: SWBF2 0x00544040
void* HUD::ElementGroup::GetParent()
{
    return this;
}

// SYNTHETIC: SWBF2 0x00544050
// HUD::ElementGroupBase::Factory::~Factory

// SYNTHETIC: SWBF2 0x00544140
// HUD::Element::Factory::~Factory

// FUNCTION: SWBF2 0x005442c0
HUD::BorderedBox::Factory::Factory()
    : HUD::ElementGroupBase::Factory(0xB3A05A43u)
{
    mName = "BorderedBox";
    mBorder.Init("Border", mProperties, Property::type_Vector2);
    mBackground.Init("Background", mProperties, Property::type_Texture);
}

// FUNCTION: SWBF2 0x00544240
HUD::Item* HUD::BorderedBox::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                 void* const pUserData)
{
    HUD::Item* pItem = new BorderedBox(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005443a0
HUD::BorderedBox::BorderedBox(const char* const pName, Factory::ItemCallback pCallback,
                              void* const pUserData)
    : HUD::ElementGroup(pName, pCallback, pUserData),
      mTextureName(0),
      mBorderWidth(0.0f),
      mBorderHeight(0.0f),
      mTextureHash(0)
{
}

// SYNTHETIC: SWBF2 0x00544470
// HUD::BorderedBox::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0054492c
// [thunk]:HUD::BorderedBox::`scalar deleting destructor' adjustor{368}
// FUNCTION: SWBF2 0x005444a0
HUD::BorderedBox::~BorderedBox()
{
}

static const unsigned int kBorderHash = 0x1F523237u;
static const unsigned int kBackgroundHash = 0x4BABD89Du;

// FUNCTION: SWBF2 0x005445b0
bool HUD::BorderedBox::ReadData(PblConfig& config, PblConfig::Data& data)
{
    const unsigned int uId = data.GetId();
    if (uId == kBorderHash)
    {
        SetBorder(data.GetFloatArg(0), data.GetFloatArg(1));
        return true;
    }
    if (uId == kBackgroundHash)
    {
        SetTexture(data.GetStringArg(0));
        return true;
    }
    return ElementGroupBase::ReadData(config, data);
}

// FUNCTION: SWBF2 0x00544600
void HUD::BorderedBox::SetTexture(const char* const pName)
{
    FreeName(&mTextureName);
    CopyName(&mTextureName, pName, 0);
    mTextureHash = PblHash(pName);
    SetupElements();
}

// FUNCTION: SWBF2 0x00544650
void HUD::BorderedBox::SetBorder(float fBorderWidth, float fBorderHeight)
{
    mBorderWidth = fBorderWidth;
    mBorderHeight = fBorderHeight;
    SetupElements();
}

// FUNCTION: SWBF2 0x00544670
void HUD::BorderedBox::PostReadSetup()
{
    ElementGroupBase::PostReadSetup();
    SetupElements();
}

// FUNCTION: SWBF2 0x00544680
void HUD::BorderedBox::SetupElements()
{
    if (!mTextureHash || !(mWidth > 0.0f) || !(mHeight > 0.0f) || !(mBorderWidth >= 0.0f) ||
        !(mBorderHeight >= 0.0f))
    {
        return;
    }

    RedGroupElement::RemoveElement(&mBorderElement);
    mBorderElement.SetTextureHash(mTextureHash);
    const RedTexture* pTexture = mBorderElement.GetTexture();
    if (!pTexture)
    {
        return;
    }

    const float fWidth = GetWidthPixelsFloat();
    const float fHeight = GetHeightPixelsFloat();
    const float fU = mBorderWidth / (float)pTexture->GetWidth();
    const float fV = mBorderHeight / (float)pTexture->GetHeight();
    mBorderElement.SetBorderRect(-mBorderWidth, -mBorderHeight, mBorderWidth + fWidth,
                                  mBorderHeight + fHeight, mBorderWidth, mBorderHeight);
    mBorderElement.SetBorderTexCoords(0.0f, 0.0f, 1.0f, 1.0f, fU, fV, 0);
    mBorderElement.SetPosition(0.0f, 0.0f);
    RedGroupElement::AddElement(&mBorderElement);
    mBorderElement.Enable(true);
}

// FUNCTION: SWBF2 0x00544850
bool HUD::BorderedBox::SetProperty(const unsigned int uHashID, const void* pValue)
{
    const unsigned int* words = (const unsigned int*)pValue;
    if (uHashID == kBorderHash)
    {
        SetBorder(*(const float*)&words[0], *(const float*)&words[1]);
        return true;
    }
    if (uHashID == kBackgroundHash)
    {
        mTextureHash = words[1];
        SetupElements();
        return true;
    }
    if (uHashID == 0xEAE44D07u)
    {
        const bool accepted = ElementGroupBase::SetProperty(uHashID, pValue);
        if (accepted)
        {
            SetupElements();
        }
        return accepted;
    }
    return ElementGroupBase::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x005448d0
bool HUD::BorderedBox::GetProperty(const unsigned int uHashID, void* pValue)
{
    if (uHashID == kBorderHash)
    {
        float* values = (float*)pValue;
        values[0] = mBorderWidth;
        values[1] = mBorderHeight;
        return true;
    }
    if (uHashID == kBackgroundHash)
    {
        *(RedTexture**)pValue = RedTexture::Find(mTextureHash);
        return true;
    }
    return ElementGroupBase::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00544530
void HUD::BorderedBox::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("Background", uTabs, mTextureName, "");
    file.WriteProperty("Border", uTabs, mBorderWidth, mBorderHeight, 0.0f, 0.0f);
    ElementGroupBase::WriteData(file, uTabs);
}
