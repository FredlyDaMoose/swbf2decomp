#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "HUDItem.h"
#include "HUDEvent.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"

// GLOBAL: SWBF2 0x00789c2c
static char* sDefaultNames[2] = { "", "Unnamed" };

// FUNCTION: SWBF2 0x00543ed0
bool HUD::Item::WriteEnabled() const
{
    return mWriteEnabled;
}

// FUNCTION: SWBF2 0x00563930
HUD::Item::Factory::Type HUD::Item::GetType() const
{
    return Factory::type_Item;
}

// FUNCTION: SWBF2 0x00563940
void HUD::File::WriteTabs(unsigned int uTabs)
{
    for (unsigned int i = uTabs; i; --i)
    {
        WriteString("    ");
    }
}

// FUNCTION: SWBF2 0x00563970
void HUD::File::WriteStringf(const char* pFormat, ...)
{
    char acBuffer[1024];
    va_list args;
    va_start(args, pFormat);
    _vsnprintf(acBuffer, sizeof(acBuffer), pFormat, args);
    WriteString(acBuffer);
}

// FUNCTION: SWBF2 0x005639b0
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs,
                              const char* const pValue, const char* const pDefault)
{
    if (pValue && strlen(pValue) && strcmp(pValue, pDefault))
    {
        WriteTabs(uTabs);
        WriteStringf("%s(\"%s\")\n", pName, pValue);
    }
}

// FUNCTION: SWBF2 0x00563a20
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs, int iValue,
                              int iDefault)
{
    if (iValue != iDefault)
    {
        WriteTabs(uTabs);
        WriteStringf("%s(%d)\n", pName, iValue);
    }
}

// FUNCTION: SWBF2 0x00563a50
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs, int iValue0,
                              int iValue1, int iValue2, int iDefault0, int iDefault1,
                              int iDefault2)
{
    if (iValue0 != iDefault0 || iValue1 != iDefault1 || iValue2 != iDefault2)
    {
        WriteTabs(uTabs);
        WriteStringf("%s(%d, %d, %d)\n", pName, iValue0, iValue1, iValue2);
    }
}

// FUNCTION: SWBF2 0x00563aa0
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs, float fValue,
                              float fDefault)
{
    if (fValue != fDefault)
    {
        WriteTabs(uTabs);
        WriteStringf("%s(%f)\n", pName, fValue);
    }
}

// FUNCTION: SWBF2 0x00563af0
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs, float fValue0,
                              float fValue1, float fDefault0, float fDefault1)
{
    if (fValue0 != fDefault0 || fValue1 != fDefault1)
    {
        WriteTabs(uTabs);
        WriteStringf("%s(%f, %f)\n", pName, fValue0, fValue1);
    }
}

// FUNCTION: SWBF2 0x00563b60
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs, float fValue0,
                              float fValue1, float fValue2, float fDefault0, float fDefault1,
                              float fDefault2)
{
    if (fValue0 != fDefault0 || fValue1 != fDefault1 || fValue2 != fDefault2)
    {
        WriteTabs(uTabs);
        WriteStringf("%s(%f, %f, %f)\n", pName, fValue0, fValue1, fValue2);
    }
}

// FUNCTION: SWBF2 0x00563bf0
void HUD::File::WriteProperty(const char* const pName, unsigned int uTabs, float fValue0,
                              float fValue1, float fValue2, float fValue3, float fDefault0,
                              float fDefault1, float fDefault2, float fDefault3)
{
    if (fValue0 != fDefault0 || fValue1 != fDefault1 || fValue2 != fDefault2 ||
        fValue3 != fDefault3)
    {
        WriteTabs(uTabs);
        WriteStringf("%s(%f, %f, %f, %f)\n", pName, fValue0, fValue1, fValue2, fValue3);
    }
}

// FUNCTION: SWBF2 0x00563c90
HUD::Item::Factory::Property::Data::Data()
{
    mName = "Unknown";
    mHashID = 0;
    mType = type_Invalid;
}

// FUNCTION: SWBF2 0x00563cb0
HUD::Item::Factory::Property::Property()
{
    mData = new Data();
}

// FUNCTION: SWBF2 0x00563d20
HUD::Item::Factory::Property::~Property()
{
    delete mData;
    mData = 0;

    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00563d60
void HUD::Item::Factory::Property::Init(char* pName, PblListSingle& list, int iMin, int iMax)
{
    if (!mData)
    {
        return;
    }

    mData->mName = pName;
    mData->mHashID = PblHash(pName);
    mData->mType = type_Int;
    mData->mInfo.intRange.minValue = iMin;
    mData->mInfo.intRange.maxValue = iMax;

    list.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00563dc0
void HUD::Item::Factory::Property::Init(char* pName, PblListSingle& list,
                                        float fMin, float fMax, float fStep)
{
    if (!mData)
    {
        return;
    }

    mData->mName = pName;
    mData->mHashID = PblHash(pName);
    mData->mType = type_Float;
    mData->mInfo.floatRange.minValue = fMin;
    mData->mInfo.floatRange.maxValue = fMax;
    mData->mInfo.floatRange.stepValue = fStep;

    list.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00563e40
void HUD::Item::Factory::Property::Init(char* pName, PblListSingle& list,
                                        char** ppEnumStrings,
                                        unsigned int uMinValue, unsigned int uMaxValue)
{
    if (!mData)
    {
        return;
    }

    mData->mName = pName;
    mData->mHashID = PblHash(pName);
    mData->mType = type_Enum;
    mData->mInfo.enumRange.enumStrings = ppEnumStrings;
    mData->mInfo.enumRange.minValue = uMinValue;
    mData->mInfo.enumRange.maxValue = uMaxValue;

    list.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00563eb0
void HUD::Item::Factory::Property::Init(char* pName, PblListSingle& list, Type eType)
{
    if (!mData)
    {
        return;
    }

    mData->mName = pName;
    mData->mHashID = PblHash(pName);
    mData->mType = eType;

    if (mData->mType == type_Vector2 || mData->mType == type_Vector3 ||
        mData->mType == type_Vector4)
    {
        for (int i = 0; i < 4; ++i)
        {
            mData->mInfo.vectorRange.range[i].minValue = -FLT_MAX;
            mData->mInfo.vectorRange.range[i].maxValue = FLT_MAX;
            mData->mInfo.vectorRange.range[i].stepValue = 1.0f;
        }
    }

    list.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x00563f80
void HUD::Item::Factory::Property::InitVectorRange(Axis eAxis, float fMin, float fMax,
                                                   float fStep)
{
    if (!mData)
    {
        return;
    }

    Info::FloatRange& range = mData->mInfo.vectorRange.range[eAxis - 1];
    range.minValue = fMin;
    range.maxValue = fMax;
    range.stepValue = fStep;
}

// FUNCTION: SWBF2 0x00563fc0
const char* HUD::Item::Factory::Property::GetName() const
{
    return mData ? mData->mName : 0;
}

// FUNCTION: SWBF2 0x00563fd0
unsigned int HUD::Item::Factory::Property::GetHashID() const
{
    return mData ? mData->mHashID : 0;
}

// FUNCTION: SWBF2 0x00563fe0
HUD::Item::Factory::Property::Type HUD::Item::Factory::Property::GetType() const
{
    return mData ? mData->mType : type_Invalid;
}

// FUNCTION: SWBF2 0x00563ff0
unsigned int HUD::Item::Factory::Property::GetMinEnumValue() const
{
    return mData ? mData->mInfo.enumRange.minValue : 0;
}

// FUNCTION: SWBF2 0x00564000
unsigned int HUD::Item::Factory::Property::GetMaxEnumValue() const
{
    return mData ? mData->mInfo.enumRange.maxValue : 0;
}

// FUNCTION: SWBF2 0x00564010
float HUD::Item::Factory::Property::GetMinFloatValue() const
{
    return mData ? mData->mInfo.floatRange.minValue : 0.0f;
}

// FUNCTION: SWBF2 0x00564030
float HUD::Item::Factory::Property::GetMaxFloatValue() const
{
    return mData ? mData->mInfo.floatRange.maxValue : 0.0f;
}

// FUNCTION: SWBF2 0x00564050
float HUD::Item::Factory::Property::GetStepFloatValue() const
{
    return mData ? mData->mInfo.floatRange.stepValue : 0.0f;
}

// FUNCTION: SWBF2 0x00564070
int HUD::Item::Factory::Property::GetMinIntValue() const
{
    return mData ? mData->mInfo.intRange.minValue : 0;
}

// FUNCTION: SWBF2 0x00564080
int HUD::Item::Factory::Property::GetMaxIntValue() const
{
    return mData ? mData->mInfo.intRange.maxValue : 0;
}

// FUNCTION: SWBF2 0x00564090
float HUD::Item::Factory::Property::GetMinVectorValue(Axis eAxis) const
{
    return mData ? mData->mInfo.vectorRange.range[eAxis - 1].minValue : 0.0f;
}

// FUNCTION: SWBF2 0x005640b0
float HUD::Item::Factory::Property::GetMaxVectorValue(Axis eAxis) const
{
    return mData ? mData->mInfo.vectorRange.range[eAxis - 1].maxValue : 0.0f;
}

// FUNCTION: SWBF2 0x005640e0
float HUD::Item::Factory::Property::GetStepVectorValue(Axis eAxis) const
{
    return mData ? mData->mInfo.vectorRange.range[eAxis - 1].stepValue : 0.0f;
}

// GLOBAL: SWBF2 0x007eba78
// SYNTHETIC: SWBF2 0x00404080
// `dynamic initializer for 'HUD::Item::Factory::sList''
// SYNTHETIC: SWBF2 0x00768be0
// `dynamic atexit destructor for 'HUD::Item::Factory::sList''
PblListDouble HUD::Item::Factory::sList;

// FUNCTION: SWBF2 0x00564110
HUD::Item::Factory::Factory(unsigned int uHashID)
    : mHashID(uHashID)
{
    mType = type_Item;
    mName = "Unnamed";
    sList.AddFirst(&mNode);
}

// FUNCTION: SWBF2 0x005641a0
HUD::Item::Factory::~Factory()
{
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x005641e0
HUD::Item::Factory::Type HUD::Item::Factory::GetType() const
{
    return mType;
}

// FUNCTION: SWBF2 0x005641f0
const char* HUD::Item::Factory::GetName() const
{
    return mName;
}

// FUNCTION: SWBF2 0x00564200
const HUD::Item::Factory::Property* HUD::Item::Factory::PropertyGetFirst() const
{
    if (mProperties.IsEmpty())
    {
        return 0;
    }
    const PblListSingle::Node* pNode = mProperties.mHead.mNext;
    return pNode != &mProperties.mHead ? (const Property*)pNode : 0;
}

// FUNCTION: SWBF2 0x00564220
const HUD::Item::Factory::Property* HUD::Item::Factory::PropertyGetLast() const
{
    if (mProperties.IsEmpty())
    {
        return 0;
    }
    const PblListSingle::Node* pLast = 0;
    for (const PblListSingle::Node* pNode = mProperties.mHead.mNext;
         pNode != &mProperties.mHead; pNode = pNode->mNext)
    {
        pLast = pNode;
    }
    return pLast != &mProperties.mHead ? (const Property*)pLast : 0;
}

// FUNCTION: SWBF2 0x00564250
const HUD::Item::Factory::Property* HUD::Item::Factory::PropertyGetNext(
    const Property* const pProperty) const
{
    const PblListSingle::Node* pNode = pProperty->mNode.mNext;
    return pNode != &mProperties.mHead ? (const Property*)pNode : 0;
}

// FUNCTION: SWBF2 0x00564270
const HUD::Item::Factory::Property* HUD::Item::Factory::PropertyGetPrevious(
    const Property* const pProperty) const
{
    const PblListSingle::Node* pPrev;
    const PblListSingle::Node* pNode = &pProperty->mNode;
    do
    {
        pPrev = pNode;
        pNode = pNode->mNext;
    } while (pNode != &pProperty->mNode);
    return pPrev != &mProperties.mHead ? (const Property*)pPrev : 0;
}

// FUNCTION: SWBF2 0x005642a0
HUD::Item::Factory* HUD::Item::Factory::FindByHashID(unsigned int uHashID)
{
    for (PblListDouble::Node* pNode = sList.mTerminator.mNext; pNode != &sList.mTerminator;
         pNode = pNode->mNext)
    {
        Factory* pFactory = (Factory*)((char*)pNode - offsetof(Factory, mNode));
        if (pFactory->mHashID == uHashID)
        {
            return pFactory;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005642d0
void HUD::Item::Factory::DestroyAll()
{
    PblListDouble::Node* pNode = sList.mTerminator.mNext;
    while (pNode != &sList.mTerminator)
    {
        Factory* pFactory = (Factory*)((char*)pNode - offsetof(Factory, mNode));
        pNode = pNode->mNext;
        delete pFactory;
    }
    sList.Clear();
}

// GLOBAL: SWBF2 0x007eba98
// SYNTHETIC: SWBF2 0x00404090
// `dynamic initializer for 'HUD::Item::sList''
// SYNTHETIC: SWBF2 0x00768c00
// `dynamic atexit destructor for 'HUD::Item::sList''
PblListSingle HUD::Item::sList;

// FUNCTION: SWBF2 0x00564310
HUD::Item::Item(const char* const pName, Factory::ItemCallback pCallback,
                void* const pUserData)
    : mNameHash(0), mName(0), mNameDisplay(0)
{
    mWriteEnabled = true;
    if (pName)
    {
        SetName(pName);
    }
    sList.AddLast(&mItemNode);
    if (pCallback)
    {
        pCallback(this, pUserData);
    }
}

// FUNCTION: SWBF2 0x005643e0
HUD::Item::~Item()
{
    if (mNameDisplay != mName)
    {
        FreeName(&mNameDisplay);
    }
    FreeName(&mName);
    mItemNode.Remove();
}

// FUNCTION: SWBF2 0x00564470
unsigned int HUD::Item::GetNameHash() const
{
    return mNameHash;
}

// GLOBAL: SWBF2 0x01e56fb8
char* HUD::Item::sEventFilter;
// GLOBAL: SWBF2 0x01e56fb4
unsigned int HUD::Item::sEventIndex;

// FUNCTION: SWBF2 0x00564690
void HUD::Item::GetFilteredEventName(const char* const pName, char* const pBuffer,
                                     unsigned int uSize)
{
    if (sEventFilter)
    {
        const char* pSrc = pName;
        const char* pFilter = sEventFilter;
        while (*pSrc == *pFilter && *pSrc && *pFilter)
        {
            ++pSrc;
            ++pFilter;
        }
        if (*pFilter == '%')
        {
            const unsigned int uLen = (unsigned int)(pSrc - pName);
            memcpy(pBuffer, pName, uLen);
            pBuffer[uLen] = 0;
            _snprintf(pBuffer + uLen, uSize - 1, "%d", sEventIndex);
            while (*pSrc >= '0' && *pSrc <= '9' && *pSrc)
            {
                ++pSrc;
            }
            strlcat(pBuffer, pSrc, uSize - 1);
            return;
        }
    }
    strncpy(pBuffer, pName, uSize - 1);
}

// FUNCTION: SWBF2 0x0055dbf0
HUD::EventClass* HUD::EventHandler::GetRegisteredClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x0055ddb0
void HUD::EventClass::UnregisterEventHandler(HUD::EventHandler& handler)
{
    handler.mNode.Remove();
    handler.mClass = 0;
}

// FUNCTION: SWBF2 0x005647f0
void HUD::Item::SetEvent(const char* pName, EventHandler& handler)
{
    if (handler.GetRegisteredClass())
    {
        handler.GetRegisteredClass()->UnregisterEventHandler(handler);
    }
    if (pName)
    {
        EventClass* pClass = EventClass::FindByHashID(PblHash(pName));
        if (pClass)
        {
            pClass->RegisterEventHandler(handler);
        }
    }
}

// FUNCTION: SWBF2 0x00564840
void HUD::Item::SetEventFilter(const char* const pFilter, unsigned int uIndex)
{
    sEventFilter = (char*)pFilter;
    sEventIndex = uIndex;
}

// FUNCTION: SWBF2 0x00564480
void HUD::Item::SetName(const char* const pName)
{
    mNameHash = PblHash(pName);
    if (mNameDisplay != mName)
    {
        FreeName(&mNameDisplay);
    }
    FreeName(&mName);

    char acName[512];
    GetFilteredEventName(pName, acName, sizeof(acName));
    CopyName(&mName, acName, 1);
    if (!mNameDisplay)
    {
        mNameDisplay = mName;
    }
}

// FUNCTION: SWBF2 0x005644f0
const char* HUD::Item::GetName() const
{
    return mName;
}

// FUNCTION: SWBF2 0x00564850
HUD::Item* HUD::Item::GetFirst()
{
    PblListSingle::Node* pNode = sList.mHead.mNext;
    if (pNode == &sList.mHead)
    {
        return 0;
    }
    return (Item*)((char*)pNode - offsetof(Item, mItemNode));
}

// FUNCTION: SWBF2 0x00564870
HUD::Item* HUD::Item::GetNext(Item* const pItem)
{
    PblListSingle::Node* pNode = pItem->mItemNode.mNext;
    if (pNode == &sList.mHead)
    {
        return 0;
    }
    return (Item*)((char*)pNode - offsetof(Item, mItemNode));
}

// FUNCTION: SWBF2 0x00564500
void HUD::Item::SetDisplayName(const char* const pName)
{
    if (mNameDisplay == mName)
    {
        mNameDisplay = 0;
    }
    FreeName(&mNameDisplay);
    CopyName(&mNameDisplay, pName, 0);
}

// FUNCTION: SWBF2 0x00564530
const char* HUD::Item::GetDisplayName() const
{
    return mNameDisplay;
}

// FUNCTION: SWBF2 0x00564540
HUD::Item* HUD::Item::SetFactory(const Factory* const pFactory)
{
    mFactory = pFactory;
    return this;
}

// FUNCTION: SWBF2 0x00564550
const HUD::Item::Factory* HUD::Item::GetFactory() const
{
    return mFactory;
}

// FUNCTION: SWBF2 0x00564560
void HUD::Item::Read(PblConfig& config, Factory::ItemCallback pCallback,
                     void* const pUserData)
{
    while (!config.IsDone())
    {
        if (config.m_pChunk->PeekNextChildId() == 0x41544144)
        {
            PblConfig::Data data = config.ReadNextData();
            ReadData(config, data);
        }
        else if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            PblConfig scope(config.ReadNextScope(), true);
            Read(scope, pCallback, pUserData);
        }
    }
    PostReadSetup();
}

// FUNCTION: SWBF2 0x00564610
void HUD::Item::Write(HUD::File& file, unsigned int uTabs, const char* pType)
{
    file.WriteTabs(uTabs);
    if (!pType)
    {
        pType = GetFactory()->GetName();
    }
    file.WriteStringf("%s(\"%s\")\n", pType, GetName());
    file.WriteTabs(uTabs);
    file.WriteString("{\n");
    WriteData(file, uTabs + 1);
    file.WriteTabs(uTabs);
    file.WriteString("}\n\n");
}

// FUNCTION: SWBF2 0x00564740
bool HUD::Item::ReadEvent(PblConfig::Data& data, EventHandler& handler)
{
    char acName[512];
    GetFilteredEventName(data.GetStringArg(0), acName, sizeof(acName));
    EventClass* pClass = EventClass::FindByHashID(PblHash(acName));
    if (pClass)
    {
        pClass->RegisterEventHandler(handler);
    }
    return true;
}

// FUNCTION: SWBF2 0x00564790
HUD::EventClass* HUD::Item::CreateEventA(const char* const pName, EventClass::Type eType)
{
    char acName[512];
    GetFilteredEventName(pName, acName, sizeof(acName));
    EventClass* pClass = EventClass::FindByHashID(PblHash(acName));
    if (pClass)
    {
        pClass->AddRef();
        return pClass;
    }
    return EventClass::Create(eType, acName);
}

// FUNCTION: SWBF2 0x00564890
unsigned int HUD::CopyName(char** ppDest, const char* const pSource, bool bUseUnnamed)
{
    if (pSource)
    {

        size_t uLength = strlen(pSource) + 1;

        *ppDest = new char[uLength];
        if (*ppDest)
        {
            strncpy(*ppDest, pSource, uLength);
        }
    }

    if (!*ppDest)
    {
        *ppDest = bUseUnnamed ? sDefaultNames[1] : sDefaultNames[0];
    }

    return (unsigned int)(*ppDest);
}

// FUNCTION: SWBF2 0x005648f0
void HUD::FreeName(char** ppName)
{
    if (*ppName)
    {
        unsigned int i;
        for (i = 0; i < sizeof(sDefaultNames) / sizeof(sDefaultNames[0]); ++i)
        {
            if (*ppName == sDefaultNames[i])
            {
                break;
            }
        }
        if (i == sizeof(sDefaultNames) / sizeof(sDefaultNames[0]))
        {
            delete[] *ppName;
        }
        *ppName = 0;
    }
}
