#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedColor.h"
#include "RedTextureTransform.h"

class RedModel;
class MemoryPool;

class RedShadingState : public PblRef
{
public:

    RedShadingState(unsigned int uiRenderTypeHash);
    static void* operator new(unsigned int uiSize);
    static void operator delete(void* pBlock);

    static MemoryPool sMemoryPool;

    enum ValueTypeT
    {
        TYPE_NONE = 0,
        TYPE_INT = 1,
        TYPE_FLOAT = 2,
        TYPE_COLOR = 3,
        TYPE_VECTOR = 4,
        TYPE_TEXTURETRANSFORM = 5,
        TYPE_MATRIX = 6,
        TYPE_PTR = 7,
        NUM_TYPES = 8,
        TYPE_FORCE_DWORD = 0x7fffffff
    };

    struct Value
    {
        ValueTypeT type;
        unsigned int value;
    };

    int& AddIntParam(unsigned int uiHash);
    RedColor& AddColorParam(unsigned int uiHash);
    RedTextureTransform& AddTextureTransformParam(unsigned int uiHash);

    int& GetIntParam(unsigned int uiHash);
    RedColor& GetColorParam(unsigned int uiHash);
    RedTextureTransform& GetTextureTransformParam(unsigned int uiHash);

    unsigned int m_uiRenderTypeHash;
    unsigned int m_uiNumValues;
    Value m_aValues[31];

    int m_ParamHashNumEntries;
    PblHashTable<Value, 32> m_ParamHash;
};
 
 
 

class RedShadingPose
{
public:
    struct Entry
    {
        static void* operator new(unsigned int uiSize);
        static void operator delete(void* pBlock);

        static MemoryPool sMemoryPool;

        unsigned int uiMaterialNameHash;
        RedShadingState* pState;
        Entry* pNext;
    };

    void BindToModel(const RedModel* pModel);
    RedShadingState* CreateDefaultShadingState(unsigned int uiRenderTypeHash);
    RedShadingState* CreateShadingState(unsigned int uiMaterialNameHash);
    RedShadingState* Find(unsigned int uiMaterialNameHash);
    RedShadingState* FindDefault(unsigned int uiRenderTypeHash);
    void Reset();

    Entry* m_pFirstEntry;
    RedModel* m_pModel;
};
 
 
