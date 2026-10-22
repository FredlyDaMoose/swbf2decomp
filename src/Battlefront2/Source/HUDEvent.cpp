#include <stdarg.h>
#include <stdio.h>

#include "HUDEvent.h"
#include "HUDEventQueue.h"
#include "GameLoop.h"
#include "HUDItem.h"
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x007eba5c
// SYNTHETIC: SWBF2 0x00403ff0
// `dynamic initializer for 'HUD::EventClass::sList''
// SYNTHETIC: SWBF2 0x00768b80
// `dynamic atexit destructor for 'HUD::EventClass::sList''
PblListSingle HUD::EventClass::sList;

// FUNCTION: SWBF2 0x0055dc30
HUD::EventClass::EventClass(unsigned int uHashID, Type eType, char const* const pName,
                            bool bFreeName)
    : mHashID(uHashID)
    , mType(eType)
{
    mRefCount = 1;
    mFreeName = bFreeName;

    mName = 0;

    HUD::CopyName(&mName, pName, 1);

    sList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0055de40
HUD::EventClass* HUD::EventClass::Create(Type eType, const char* pFormat, ...)
{
    char acName[0x400];

    va_list args;
    va_start(args, pFormat);
    _vsnprintf(acName, sizeof(acName), pFormat, args);
    va_end(args);

    return new EventClass(PblHash(acName), eType, acName, true);
}

// FUNCTION: SWBF2 0x0055db20
HUD::EventHandler::EventHandler()
{
    mFunc = 0;
    mData = 0;
    mClass = 0;
}

// FUNCTION: SWBF2 0x0055db70
HUD::EventHandler::~EventHandler()
{
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0055db50
void HUD::EventHandler::Init(void (*const pFunc)(const Event&, void* const), void* const pData)
{
    mFunc = pFunc;
    mData = pData;
}

// FUNCTION: SWBF2 0x0055dbd0
void HUD::EventHandler::HandleEvent(const Event& event) const
{
    mFunc(event, mData);
}

// FUNCTION: SWBF2 0x0055dde0
HUD::EventClass::Type HUD::EventClass::GetType() const
{
    return mType;
}

// FUNCTION: SWBF2 0x0055de10
void HUD::EventClass::Send(const Event& event) const
{

    for (const PblListDouble::Node* pNode = mHandlerList.mTerminator.mNext;
         pNode != &mHandlerList.mTerminator; pNode = pNode->mNext)
    {
        const EventHandler* pHandler = (const EventHandler*)((const char*)pNode - offsetof(EventHandler, mNode));
        pHandler->HandleEvent(event);
    }
}

// FUNCTION: SWBF2 0x0055dfe0
HUD::Event::Event(const EventClass* const pClass, bool bValue)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.boolValue = bValue;
}

// FUNCTION: SWBF2 0x0055e000
HUD::Event::Event(const EventClass* const pClass, float fValue)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.floatValue = fValue;
}

// FUNCTION: SWBF2 0x0055e020
HUD::Event::Event(const EventClass* const pClass, int iValue)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.intValue = iValue;
}

// FUNCTION: SWBF2 0x0055e0a0
HUD::Event::Event(const EventClass* const pClass, const RedColor* const pColor)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.color = const_cast<RedColor*>(pColor);
}

// FUNCTION: SWBF2 0x0055e0e0
HUD::Event::Event(const EventClass* const pClass, const PblVector3* const pVector)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.vector3 = const_cast<PblVector3*>(pVector);
}

// FUNCTION: SWBF2 0x0055e060
HUD::Event::Event(const EventClass* const pClass, const RedModel* const pModel)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.model = const_cast<RedModel*>(pModel);
}

// FUNCTION: SWBF2 0x0055e0c0
HUD::Event::Event(const EventClass* const pClass, const wchar_t* const pString)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.string = (unsigned short*)pString;
}

// FUNCTION: SWBF2 0x0055e100
void HUD::Event::Send() const
{
    mClass->Send(*this);
}

// FUNCTION: SWBF2 0x0055e120
const HUD::EventClass* HUD::Event::GetClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x0055e170
float HUD::Event::GetDataFloat() const
{
    switch (mClass->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
        return (float)mData.intValue;
    case EventClass::type_Float:
        return mData.floatValue;
    default:
        return 0.0f;
    }
}

// FUNCTION: SWBF2 0x0055e140
bool HUD::Event::GetDataBool() const
{
    switch (mClass->GetType())
    {
    case EventClass::type_Bool:
        return mData.boolValue;
    case EventClass::type_Int:
    case EventClass::type_Uint:
    case EventClass::type_Float:
        return mData.uintValue != 0;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x0055e080
HUD::Event::Event(const EventClass* const pClass, const RedTexture* const pTexture)
{
    mClass = const_cast<EventClass*>(pClass);
    mData.texture = const_cast<RedTexture*>(pTexture);
}

// FUNCTION: SWBF2 0x0055dc00
const char* HUD::EventHandler::GetRegisteredClassName(bool bEmptyIfMissing) const
{
    if (mClass)
    {
        return mClass->mName;
    }
    return bEmptyIfMissing ? "" : 0;
}

// FUNCTION: SWBF2 0x0055ddf0
const char* HUD::EventClass::GetName() const
{
    return mName;
}

// FUNCTION: SWBF2 0x0055df10
HUD::EventClass* HUD::EventClass::GetFirst()
{
    PblListSingle::Node* node = sList.mHead.mNext;
    return node == &sList.mHead ? 0 : reinterpret_cast<EventClass*>(reinterpret_cast<char*>(node) - offsetof(EventClass, mNode));
}

// FUNCTION: SWBF2 0x0055df30
HUD::EventClass* HUD::EventClass::GetLast()
{
    PblListSingle::Node* node = sList.mHead.mNext;
    if (node == &sList.mHead)
    {
        return 0;
    }
    while (node->mNext != &sList.mHead)
    {
        node = node->mNext;
    }
    return reinterpret_cast<EventClass*>(reinterpret_cast<char*>(node) - offsetof(EventClass, mNode));
}

// FUNCTION: SWBF2 0x0055df60
HUD::EventClass* HUD::EventClass::GetNext(EventClass* pEvent)
{
    PblListSingle::Node* node = pEvent->mNode.mNext;
    return node == &sList.mHead ? 0 : reinterpret_cast<EventClass*>(reinterpret_cast<char*>(node) - offsetof(EventClass, mNode));
}

// FUNCTION: SWBF2 0x0055df80
HUD::EventClass* HUD::EventClass::GetPrevious(EventClass* pEvent)
{
    PblListSingle::Node* const current = &pEvent->mNode;
    PblListSingle::Node* node = current;
    while (node->mNext != current)
    {
        node = node->mNext;
    }
    return node == &sList.mHead ? 0 : reinterpret_cast<EventClass*>(reinterpret_cast<char*>(node) - offsetof(EventClass, mNode));
}

// FUNCTION: SWBF2 0x0055e220
void HUD::EventQueue::AddEvent(const HUD::Event* pEvent, float fDelay)
{
    if (!pEvent)
    {
        return;
    }
    for (int i = 0; i < 32; ++i)
    {
        HUD::DelayedEvent& slot = HUD::gEventList[i];
        if (slot.activationTime < 0.0f)
        {
            slot.e = *pEvent;
            slot.activationTime = GameLoop::GetMissionTime() + fDelay;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0055e110
void HUD::Event::SendDelayed(float fDelay) const
{
    HUD::EventQueue::AddEvent(this, fDelay);
}

// FUNCTION: SWBF2 0x0055dcd0
HUD::EventClass::~EventClass()
{
    if (mFreeName)
    {
        FreeName(&mName);
    }
    mHandlerList.Clear();

    if (mNode.mNext)
    {
        mNode.Remove();
    }
    mHandlerList.mTerminator.mNext = 0;
    mHandlerList.mTerminator.mPrev = 0;
}

// FUNCTION: SWBF2 0x0055dd50
void HUD::EventClass::RegisterEventHandler(EventHandler& handler)
{
    if (handler.mNode.IsAttached())
    {
        handler.mNode.Remove();
    }
    mHandlerList.AddLast(&handler.mNode);
    handler.mClass = this;
}

// FUNCTION: SWBF2 0x0055de00
unsigned int HUD::EventClass::AddRef()
{
    return ++mRefCount;
}

// FUNCTION: SWBF2 0x0055dee0
HUD::EventClass* HUD::EventClass::FindByHashID(unsigned int uHashID)
{
    for (PblListSingle::Node* pNode = sList.mHead.mNext; pNode != &sList.mHead; pNode = pNode->mNext)
    {
        EventClass* pEvent = (EventClass*)((char*)pNode - offsetof(EventClass, mNode));
        if (pEvent->mHashID == uHashID)
        {
            return pEvent;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0055dfa0
void HUD::EventClass::DestroyAll()
{
    PblListSingle::Node* pNode = sList.mHead.mNext;
    while (pNode != &sList.mHead)
    {
        EventClass* pEvent = (EventClass*)((char*)pNode - offsetof(EventClass, mNode));
        pNode = pNode->mNext;
        delete pEvent;
    }
    sList.Clear();
}

// FUNCTION: SWBF2 0x0055e130
HUD::Event::Data HUD::Event::GetData() const
{
    return mData;
}

// FUNCTION: SWBF2 0x0055e040
HUD::Event* InitializeEventWithData(HUD::Event* pEvent, HUD::EventClass* pClass, HUD::Event::Data data)
{
    pEvent->mClass = pClass;
    pEvent->mData = data;
    return pEvent;
}
