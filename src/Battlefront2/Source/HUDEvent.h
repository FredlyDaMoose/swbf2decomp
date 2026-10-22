#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblListDouble.h"
#include "PebbleFL/Common/PblListSingle.h"

class PblVector3;
class RedColor;
class RedModel;
class RedTexture;

namespace HUD {

class EventHandler;
class Event;

class EventClass
{
public:

    enum Type
    {
        type_Invalid = 0,
        type_Bool    = 1,
        type_Int     = 2,
        type_Uint    = 3,
        type_Float   = 4,
        type_Model   = 5,
        type_Texture = 6,
        type_Color   = 7,
        type_String  = 8,
        type_Vector3 = 9
    };

    // FUNCTION: SWBF2 0x0055dc30
    EventClass(unsigned int uHashID, Type eType, char const* const pName, bool bFreeName);

    ~EventClass();

    // FUNCTION: SWBF2 0x0055de40
    static EventClass* Create(Type eType, const char* pFormat, ...);

    const char* GetName() const;

    void RegisterEventHandler(EventHandler& handler);

    void UnregisterEventHandler(EventHandler& handler);
    unsigned int AddRef();

    static EventClass* FindByHashID(unsigned int uHashID);
    static EventClass* GetFirst();
    static EventClass* GetLast();
    static EventClass* GetNext(EventClass* pEvent);
    static EventClass* GetPrevious(EventClass* pEvent);

    Type GetType() const;

    void Send(const Event& event) const;

    static PblListSingle sList;

    static void DestroyAll();

    unsigned int mHashID;
    Type mType;
    PblListDouble mHandlerList;
    PblListSingle::Node mNode;
    unsigned int mRefCount;
    char* mName;
    bool mFreeName;
};

class Event;

class EventHandler
{
public:
    EventHandler();
    ~EventHandler();
    const char* GetRegisteredClassName(bool) const;

    void Init(void (*const pFunc)(const Event&, void* const), void* const pData);

    void HandleEvent(const Event& event) const;

    EventClass* GetRegisteredClass() const;

    void (*mFunc)(const Event&, void* const);
    void* mData;
    PblListDouble::Node mNode;
    EventClass* mClass;
};

class Event
{
public:
    union Data
    {
        bool boolValue;
        float floatValue;
        int intValue;
        unsigned int uintValue;
        RedModel* model;
        RedTexture* texture;
        RedColor* color;
        unsigned short* string;
        PblVector3* vector3;
    };

    Event() : mClass(0) {}
    ~Event() {}

    Event(const EventClass* const pClass, bool bValue);

    Event(const EventClass* const pClass, float fValue);
    Event(const EventClass* const pClass, int iValue);
    Event(const EventClass* const pClass, const RedColor* const pColor);

    Event(const EventClass* const pClass, const PblVector3* const pVector);

    Event(const EventClass* const pClass, const RedModel* const pModel);
    Event(const EventClass* const pClass, const wchar_t* const pString);

    Event(const EventClass* const pClass, const RedTexture* const pTexture);

    void Send() const;

    void SendDelayed(float fDelay) const;

    const EventClass* GetClass() const;
    float GetDataFloat() const;

    bool GetDataBool() const;

    Data GetData() const;

    EventClass* mClass;
    Data mData;
};

}

 
 
 
 
 
 
 
 

 
