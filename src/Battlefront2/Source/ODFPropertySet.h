#pragma once

namespace ODFPropertySet
{
    enum Type
    {
        type_Invalid = 0,
        type_String = 1,
        type_Float = 2,
        type_UInt8 = 3,
        type_Int8 = 4,
        type_Int16 = 5,
        type_UInt16 = 6,
        type_Int32 = 7,
        type_UInt32 = 8,
        type_Int64 = 9,
        type_UInt64 = 10,
        type_Max = 11
    };

    struct ODFProperty
    {
        unsigned int id;
        Type type;
        unsigned int memberOffset;
    };

    extern char* sScanfFormats[10];

    bool __cdecl SetProperty(const ODFProperty* const pProperties, unsigned int uNumProperties,
                             const void* const pObject, unsigned int uProperty,
                             const char* const pszValue);
}
