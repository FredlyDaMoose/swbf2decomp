#include <string.h>

#include "HUDViewPort.h"
#include "CameraManager.h"
#include "HUDManager.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

static const unsigned int kViewport0Position = 0x0fbfc95e;
static const unsigned int kViewport1Position = 0x870b1061;
static const unsigned int kViewport2Position = 0xe123e230;
static const unsigned int kViewport3Position = 0x2e5f5873;
static const unsigned int kViewport0UseSafezone = 0xd7c3efff;
static const unsigned int kViewport1UseSafezone = 0xa97176e2;
static const unsigned int kViewport2UseSafezone = 0x5d23bd7d;
static const unsigned int kViewport3UseSafezone = 0x04abb610;
static const unsigned int kEventNameFilter = 0x4ee68dbe;

// FUNCTION: SWBF2 0x00568a50
HUD::Item* HUD::ViewPort::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                              void* const pUserData)
{
    HUD::Item* pItem = new ViewPort(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00568ad0
void HUD::ViewPort::GetSize(float& fWidth, float& fHeight)
{
    fWidth = 0.0f;
    fHeight = 0.0f;
}

// FUNCTION: SWBF2 0x00568af0
const char* HUD::ViewPort::PropertyTranslateEnum(const unsigned int uHashID,
                                                 const unsigned int uValue)
{
    return 0;
}

// FUNCTION: SWBF2 0x00568b00
HUD::ViewPort::Factory::Factory()
    : HUD::Element::Factory(0xE4ABBAC3u)
{
    mName = "ViewPort";

    mViewport0Position.Init("Viewport0Position", mProperties, Property::type_Vector3);
    mViewport0UseSafezone.Init("Viewport0UseSafezone", mProperties, Property::type_Bool);
    mViewport1Position.Init("Viewport1Position", mProperties, Property::type_Vector3);
    mViewport1UseSafezone.Init("Viewport1UseSafezone", mProperties, Property::type_Bool);
    mViewport2Position.Init("Viewport2Position", mProperties, Property::type_Vector3);
    mViewport2UseSafezone.Init("Viewport2UseSafezone", mProperties, Property::type_Bool);
    mViewport3Position.Init("Viewport3Position", mProperties, Property::type_Vector3);
    mViewport3UseSafezone.Init("Viewport3UseSafezone", mProperties, Property::type_Bool);
}

// SYNTHETIC: SWBF2 0x00568ca0
// HUD::ViewPort::Factory::~Factory

// FUNCTION: SWBF2 0x00568d10
HUD::ViewPort::ViewPort(const char* const pName, Factory::ItemCallback pCallback,
                        void* const pUserData)
    : HUD::Element(*static_cast<RedGroupElement*>(this), pName, pCallback, pUserData)
{
    mEventFilter = 0;
    mCurViewPort = 0;
    mItems = 0;
    mNumItems = 0;
    mViewportPosSet = 0;
}

// SYNTHETIC: SWBF2 0x00568db0
// HUD::ViewPort::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00569928
// [thunk]:HUD::ViewPort::`scalar deleting destructor' adjustor{256}
// FUNCTION: SWBF2 0x00568de0
HUD::ViewPort::~ViewPort()
{
    FreeEventFilter();
    if (mItems)
    {
        delete[] mItems;
        mItems = 0;
        mNumItems = 0;
    }
}

// FUNCTION: SWBF2 0x00568e70
void HUD::ViewPort::FreeEventFilter()
{
    if (mEventFilter)
    {
        delete[] mEventFilter;
        mEventFilter = 0;
    }
}

// FUNCTION: SWBF2 0x00568ea0
bool HUD::ViewPort::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    if (gScreenGroup)
    {
        switch (uHashID)
        {
        case kViewport0Position:
            gScreenGroup[1].SetPosition(*(const PblVector3*)pValue);
            mViewportPosSet |= 1;
            return true;
        case kViewport0UseSafezone:
            gScreenGroup[1].SetUseSafezone(*(const bool*)pValue);
            return true;
        case kViewport1Position:
            gScreenGroup[2].SetPosition(*(const PblVector3*)pValue);
            mViewportPosSet |= 2;
            return true;
        case kViewport1UseSafezone:
            gScreenGroup[2].SetUseSafezone(*(const bool*)pValue);
            return true;
        case kViewport2Position:
            gScreenGroup[3].SetPosition(*(const PblVector3*)pValue);
            mViewportPosSet |= 4;
            return true;
        case kViewport2UseSafezone:
            gScreenGroup[3].SetUseSafezone(*(const bool*)pValue);
            return true;
        case kViewport3Position:
            gScreenGroup[4].SetPosition(*(const PblVector3*)pValue);
            mViewportPosSet |= 8;
            return true;
        case kViewport3UseSafezone:
            gScreenGroup[4].SetUseSafezone(*(const bool*)pValue);
            return true;
        }
    }
    return Element::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00568ff0
bool HUD::ViewPort::GetProperty(const unsigned int uHashID, void* const pValue)
{
    if (gScreenGroup)
    {
        switch (uHashID)
        {
        case kViewport0Position:
            *(PblVector3*)pValue = gScreenGroup[1].GetPosition();
            return true;
        case kViewport0UseSafezone:
            *(bool*)pValue = gScreenGroup[1].GetUseSafezone();
            return true;
        case kViewport1Position:
            *(PblVector3*)pValue = gScreenGroup[2].GetPosition();
            return true;
        case kViewport1UseSafezone:
            *(bool*)pValue = gScreenGroup[2].GetUseSafezone();
            return true;
        case kViewport2Position:
            *(PblVector3*)pValue = gScreenGroup[3].GetPosition();
            return true;
        case kViewport2UseSafezone:
            *(bool*)pValue = gScreenGroup[3].GetUseSafezone();
            return true;
        case kViewport3Position:
            *(PblVector3*)pValue = gScreenGroup[4].GetPosition();
            return true;
        case kViewport3UseSafezone:
            *(bool*)pValue = gScreenGroup[4].GetUseSafezone();
            return true;
        }
    }
    return Element::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00569130
void HUD::ViewPort::Read(PblConfig& config, Factory::ItemCallback pCallback,
                         void* const pUserData)
{
    HUD::Item* apItems[512];

    if (mItems)
    {
        delete[] mItems;
    }
    mItems = apItems;
    mNumItems = 0;

    PblConfig configCopy(config, false);
    unsigned char uNumViewPorts = (unsigned char)(CameraManager::sInstance->m_iNumCam + 1);
    for (mCurViewPort = 1; mCurViewPort < uNumViewPorts; ++mCurViewPort)
    {
        bool bFirst = mCurViewPort == 1;
        PblConfig configPass(bFirst ? config : configCopy, bFirst);
        Item::SetEventFilter(mEventFilter, mCurViewPort);
        while (!configPass.IsDone())
        {
            PblConfig::Data data = configPass.ReadNextData();
            if (configPass.m_pChunk->PeekNextChildId() == 0x504f4353)
            {
                Item::Factory* pFactory = Item::Factory::FindByHashID(data.GetId());
                if (pFactory)
                {
                    const char* pName = data.GetNumArgs() ? data.GetStringArg(0) : "Unnamed";
                    HUD::Item* pItem = pFactory->CreateItem(pName, ItemConstructionCallback, this);
                    if (pItem)
                    {
                        PblConfig scope(configPass.ReadNextScope(), true);
                        pItem->Read(scope, ItemConstructionCallback, this);
                        pItem->mWriteEnabled = false;
                        if (bFirst && mNumItems < 512)
                        {
                            mItems[mNumItems++] = pItem;
                        }
                    }
                }
            }
            else
            {
                ReadData(config, data);
            }
        }
    }

    mItems = new HUD::Item*[mNumItems];
    memcpy(mItems, apItems, mNumItems * sizeof(HUD::Item*));
    PostReadSetup();
}

// FUNCTION: SWBF2 0x00569350
void HUD::ViewPort::PostReadSetup()
{
    Item::SetEventFilter(0, 0);
}

// FUNCTION: SWBF2 0x00569360
bool HUD::ViewPort::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kViewport0Position:
        gScreenGroup[1].SetPosition(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
        mViewportPosSet |= 1;
        return true;
    case kViewport0UseSafezone:
        gScreenGroup[1].SetUseSafezone(data.GetBoolArg(0));
        return true;
    case kViewport1Position:
        gScreenGroup[2].SetPosition(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
        mViewportPosSet |= 2;
        return true;
    case kViewport1UseSafezone:
        gScreenGroup[2].SetUseSafezone(data.GetBoolArg(0));
        return true;
    case kViewport2Position:
        gScreenGroup[3].SetPosition(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
        mViewportPosSet |= 4;
        return true;
    case kViewport2UseSafezone:
        gScreenGroup[3].SetUseSafezone(data.GetBoolArg(0));
        return true;
    case kViewport3Position:
        gScreenGroup[4].SetPosition(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
        mViewportPosSet |= 8;
        return true;
    case kViewport3UseSafezone:
        gScreenGroup[4].SetUseSafezone(data.GetBoolArg(0));
        return true;
    case kEventNameFilter:
    {
        FreeEventFilter();
        size_t uLength = strlen(data.GetStringArg(0));
        mEventFilter = new char[uLength + 1];
        if (mEventFilter)
        {
            strncpy(mEventFilter, data.GetStringArg(0), uLength);
            mEventFilter[uLength] = 0;
            Item::SetEventFilter(mEventFilter, mCurViewPort);
        }
        return true;
    }
    }
    return false;
}

// FUNCTION: SWBF2 0x00569640
void HUD::ViewPort::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("EventNameFilter", uTabs, mEventFilter, "");

    if (mViewportPosSet & 1)
    {
        const PblVector3& vPos = gScreenGroup[1].GetPosition();
        file.WriteProperty("Viewport0Position", uTabs, vPos.x, vPos.y, vPos.z, 0.0f, 0.0f, 0.0f);
    }
    file.WriteProperty("Viewport0UseSafezone", uTabs, (int)gScreenGroup[1].GetUseSafezone(), 0);
    if (mViewportPosSet & 2)
    {
        const PblVector3& vPos = gScreenGroup[2].GetPosition();
        file.WriteProperty("Viewport1Position", uTabs, vPos.x, vPos.y, vPos.z, 0.0f, 0.0f, 0.0f);
    }
    file.WriteProperty("Viewport1UseSafezone", uTabs, (int)gScreenGroup[2].GetUseSafezone(), 0);
    if (mViewportPosSet & 4)
    {
        const PblVector3& vPos = gScreenGroup[3].GetPosition();
        file.WriteProperty("Viewport2Position", uTabs, vPos.x, vPos.y, vPos.z, 0.0f, 0.0f, 0.0f);
    }
    file.WriteProperty("Viewport2UseSafezone", uTabs, (int)gScreenGroup[3].GetUseSafezone(), 0);
    if (mViewportPosSet & 8)
    {
        const PblVector3& vPos = gScreenGroup[4].GetPosition();
        file.WriteProperty("Viewport3Position", uTabs, vPos.x, vPos.y, vPos.z, 0.0f, 0.0f, 0.0f);
    }
    file.WriteProperty("Viewport3UseSafezone", uTabs, (int)gScreenGroup[4].GetUseSafezone(), 0);

    for (unsigned int i = 0; i < mNumItems; ++i)
    {
        unsigned char uViewport = 0;
        if (mItems[i]->GetType() == Factory::type_Element)
        {
            uViewport = static_cast<HUD::Element*>(mItems[i])->GetViewport();
            static_cast<HUD::Element*>(mItems[i])->SetViewport(0);
        }
        mItems[i]->mWriteEnabled = true;
        mItems[i]->Write(file, uTabs, 0);
        mItems[i]->mWriteEnabled = false;
        if (mItems[i]->GetType() == Factory::type_Element)
        {
            static_cast<HUD::Element*>(mItems[i])->SetViewport(uViewport);
        }
    }
}

// FUNCTION: SWBF2 0x00569900
void HUD::ViewPort::ItemConstructionCallback(HUD::Item* const pItem, void* const pUserData)
{
    if (pItem->GetType() == Factory::type_Element)
    {
        static_cast<HUD::Element*>(pItem)->SetViewport(static_cast<ViewPort*>(pUserData)->mCurViewPort);
    }
}
