#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblListDouble.h"
#include "PebbleFL/Common/PblListSingle.h"

namespace HUD { class File; }
namespace HUD { class EventHandler; }
#include "PebbleFL/Common/PblConfig.h"
#include "HUDEvent.h"
#include "PebbleFL/Common/PblFile.h"

namespace HUD {

class File : public PblFile
{
public:

    void WriteTabs(unsigned int uTabs);

    void WriteStringf(const char* pFormat, ...);

    void WriteProperty(const char* const pName, unsigned int uTabs, const char* const pValue,
                       const char* const pDefault);

    void WriteProperty(const char* const pName, unsigned int uTabs, int iValue, int iDefault);

    void WriteProperty(const char* const pName, unsigned int uTabs, int iValue0, int iValue1,
                       int iValue2, int iDefault0, int iDefault1, int iDefault2);

    void WriteProperty(const char* const pName, unsigned int uTabs, float fValue, float fDefault);

    void WriteProperty(const char* const pName, unsigned int uTabs, float fValue0, float fValue1,
                       float fDefault0, float fDefault1);

    void WriteProperty(const char* const pName, unsigned int uTabs, float fValue0, float fValue1,
                       float fValue2, float fDefault0, float fDefault1, float fDefault2);

    void WriteProperty(const char* const pName, unsigned int uTabs, float fValue0, float fValue1,
                       float fValue2, float fValue3, float fDefault0, float fDefault1,
                       float fDefault2, float fDefault3);
};
}

namespace HUD {
// VTABLE: SWBF2 0x007a31c4
class Item
{
public:

    // VTABLE: SWBF2 0x007a31f4

    class Factory
    {
    public:

        class Property
        {
        public:

            enum Type
            {
                type_Invalid = 0,
                type_Bool    = 1,
                type_Enum    = 2,
                type_Int     = 3,
                type_Float   = 4,
                type_Event   = 5,
                type_Color   = 6,
                type_Vector2 = 7,
                type_Vector3 = 8,
                type_Vector4 = 9,
                type_Model   = 10,
                type_Texture = 11
            };

            enum Axis
            {
                axis_Invalid = 0,
                axis_X       = 1,
                axis_Y       = 2,
                axis_Z       = 3,
                axis_W       = 4,
                axis_MAX     = 5
            };

            union Info
            {
                struct EnumRange
                {
                    char** enumStrings;
                    unsigned int minValue;
                    unsigned int maxValue;
                } enumRange;
                struct IntRange
                {
                    int minValue;
                    int maxValue;
                } intRange;
                struct FloatRange
                {
                    float minValue;
                    float maxValue;
                    float stepValue;
                } floatRange;
                struct VectorRange
                {
                    FloatRange range[4];
                } vectorRange;
            };

            struct Data
            {

                // FUNCTION: SWBF2 0x00563c90
                Data();

                char* mName;
                unsigned int mHashID;
                Type mType;
                Info mInfo;
            };

            // FUNCTION: SWBF2 0x00563cb0
            Property();

            // FUNCTION: SWBF2 0x00563d20
            ~Property();

            // FUNCTION: SWBF2 0x00563d60
            void Init(char* pName, PblListSingle& list, int iMin, int iMax);

            // FUNCTION: SWBF2 0x00563dc0
            void Init(char* pName, PblListSingle& list, float fMin, float fMax, float fStep);

            // FUNCTION: SWBF2 0x00563e40
            void Init(char* pName, PblListSingle& list, char** ppEnumStrings,
                      unsigned int uMinValue, unsigned int uMaxValue);

            // FUNCTION: SWBF2 0x00563eb0
            void Init(char* pName, PblListSingle& list, Type eType);

            // FUNCTION: SWBF2 0x00563f80
            void InitVectorRange(Axis eAxis, float fMin, float fMax, float fStep);

            const char* GetName() const;
            unsigned int GetHashID() const;
            Type GetType() const;
            unsigned int GetMinEnumValue() const;
            unsigned int GetMaxEnumValue() const;
            float GetMinFloatValue() const;
            float GetMaxFloatValue() const;
            float GetStepFloatValue() const;
            int GetMinIntValue() const;
            int GetMaxIntValue() const;
            float GetMinVectorValue(Axis eAxis) const;
            float GetMaxVectorValue(Axis eAxis) const;
            float GetStepVectorValue(Axis eAxis) const;

            PblListSingle::Node mNode;
            Data* mData;
        };

        enum Type
        {
            type_Invalid = 0,
            type_Item = 1,
            type_Element = 2
        };

        typedef void (__cdecl *ItemCallback)(Item* const, void* const);

        // FUNCTION: SWBF2 0x00564110
        Factory(unsigned int uHashID);

        // FUNCTION: SWBF2 0x005641a0
        virtual ~Factory();

        // SYNTHETIC: SWBF2 0x00564170
        // HUD::Item::Factory::`scalar deleting destructor'

        virtual Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                 void* const pUserData) = 0;

        Type GetType() const;
        const char* GetName() const;
        const Property* PropertyGetFirst() const;
        const Property* PropertyGetLast() const;
        const Property* PropertyGetNext(const Property* const pProperty) const;
        const Property* PropertyGetPrevious(const Property* const pProperty) const;

        static Factory* FindByHashID(unsigned int uHashID);
        static void DestroyAll();

        static PblListDouble sList;

        PblListSingle mProperties;
        unsigned int mHashID;
        PblListDouble::Node mNode;
        Type mType;
        char* mName;
    };

    virtual ~Item();
    // SYNTHETIC: SWBF2 0x005643b0
    // HUD::Item::`scalar deleting destructor'

    virtual Factory::Type GetType() const;
    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback,
                      void* const pUserData);
    virtual void Write(HUD::File& file, unsigned int uTabs, const char* pType);
    virtual bool WriteEnabled() const;
    virtual bool SetProperty(const unsigned int, const void *) = 0;
    virtual bool GetProperty(const unsigned int, void *) = 0;
    virtual const char * PropertyTranslateEnum(const unsigned int, const unsigned int) = 0;
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data) = 0;
    virtual void PostReadSetup() = 0;
    virtual void WriteData(HUD::File &, unsigned int) = 0;

    Item(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);

    void SetName(const char* const pName);
    void SetDisplayName(const char* const pName);
    const char* GetDisplayName() const;
    const Factory* GetFactory() const;

    static Item* GetFirst();
    static Item* GetNext(Item* const pItem);

    static PblListSingle sList;

    static EventClass* CreateEventA(const char* const pName, EventClass::Type eType);
    static bool ReadEvent(PblConfig::Data&, EventHandler&);
    static void SetEvent(const char*, EventHandler&);

    static void SetEventFilter(const char* const pFilter, unsigned int uIndex);

    static char* sEventFilter;
    static unsigned int sEventIndex;

protected:

    static void GetFilteredEventName(const char* const pName, char* const pBuffer,
                                     unsigned int uSize);
public:

    Item* SetFactory(const Factory* const pFactory);

    unsigned int GetNameHash() const;
    const char* GetName() const;

    void SetWriteEnabled(bool bEnable) { mWriteEnabled = bEnable; }

    unsigned int mNameHash;
    char* mName;
    char* mNameDisplay;
    const Factory* mFactory;
    PblListSingle::Node mItemNode;
    bool mWriteEnabled : 1;
};

void FreeName(char** ppName);

unsigned int TranslateString(const char* const* ppStrings, unsigned int uCount, const char* const pString);

// FUNCTION: SWBF2 0x00564890
unsigned int CopyName(char** ppDest, const char* const pSource, bool bUseUnnamed);
}

 
 
 
 
 
 
 
 

 
 
 
 
 
 
 
