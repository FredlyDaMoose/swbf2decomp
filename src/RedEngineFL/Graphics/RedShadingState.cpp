#include "RedShadingState.h"

#include "RedEngineFL/Memory/RedMemoryPool.h"

class RedModel
{
public:
    RedShadingState* CreateShadingState(unsigned int uiMaterialNameHash) const;

    RedShadingState* CreateDefaultShadingState(unsigned int uiRenderTypeHash) const;
};

// GLOBAL: SWBF2 0x009acad8
// SYNTHETIC: SWBF2 0x00408c20
// `dynamic initializer for 'RedShadingPose::Entry::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769ae0
// `dynamic atexit destructor for 'RedShadingPose::Entry::sMemoryPool''
MemoryPool RedShadingPose::Entry::sMemoryPool("RedShadingPose::Entry", 0xc);

// GLOBAL: SWBF2 0x009acb30
// SYNTHETIC: SWBF2 0x00408c40
// `dynamic initializer for 'RedShadingState::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769af0
// `dynamic atexit destructor for 'RedShadingState::sMemoryPool''
MemoryPool RedShadingState::sMemoryPool("RedShadingState", 0x20c);

void* RedShadingPose::Entry::operator new(unsigned int uiSize)
{
    return sMemoryPool.Allocate(uiSize);
}

void RedShadingPose::Entry::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

// FUNCTION: SWBF2 0x006ed390
RedColor& RedShadingState::AddColorParam(unsigned int uiHash)
{
    m_aValues[m_uiNumValues].type = TYPE_COLOR;
    if (m_ParamHash.Store(uiHash, &m_aValues[m_uiNumValues]))
    {
        ++m_ParamHashNumEntries;
    }
    return *(RedColor*)&m_aValues[m_uiNumValues++].value;
}

// FUNCTION: SWBF2 0x006ed3e0
int& RedShadingState::AddIntParam(unsigned int uiHash)
{
    m_aValues[m_uiNumValues].type = TYPE_INT;
    if (m_ParamHash.Store(uiHash, &m_aValues[m_uiNumValues]))
    {
        ++m_ParamHashNumEntries;
    }
    return *(int*)&m_aValues[m_uiNumValues++].value;
}

// FUNCTION: SWBF2 0x006ed430
RedTextureTransform& RedShadingState::AddTextureTransformParam(unsigned int uiHash)
{
    m_aValues[m_uiNumValues].type = TYPE_TEXTURETRANSFORM;
    if (m_ParamHash.Store(uiHash, &m_aValues[m_uiNumValues]))
    {
        ++m_ParamHashNumEntries;
    }
    m_uiNumValues += 4;
    return *(RedTextureTransform*)&m_aValues[m_uiNumValues - 4].value;
}

// FUNCTION: SWBF2 0x006ed480
void RedShadingPose::BindToModel(const RedModel* pModel)
{
    Reset();
    m_pModel = (RedModel*)pModel;
}

// FUNCTION: SWBF2 0x006ed4a0
RedShadingState* RedShadingPose::CreateDefaultShadingState(unsigned int uiRenderTypeHash)
{
    RedShadingState* pState = FindDefault(uiRenderTypeHash);
    if (pState)
    {
        return pState;
    }

    pState = m_pModel->CreateDefaultShadingState(uiRenderTypeHash);
    if (!pState)
    {
        return 0;
    }

    Entry* pEntry = new Entry;
    pEntry->uiMaterialNameHash = 0;
    pEntry->pState = pState;
    pEntry->pNext = m_pFirstEntry;
    m_pFirstEntry = pEntry;
    return pEntry->pState;
}

// FUNCTION: SWBF2 0x006ed4f0
RedShadingState* RedShadingPose::CreateShadingState(unsigned int uiMaterialNameHash)
{
    RedShadingState* pState = Find(uiMaterialNameHash);
    if (pState)
    {
        return pState;
    }

    pState = m_pModel->CreateShadingState(uiMaterialNameHash);
    if (!pState)
    {
        return 0;
    }

    Entry* pEntry = new Entry;
    pEntry->uiMaterialNameHash = uiMaterialNameHash;
    pEntry->pState = pState;
    pEntry->pNext = 0;
    if (m_pFirstEntry)
    {
        Entry* pLast = m_pFirstEntry;
        while (pLast->pNext)
        {
            pLast = pLast->pNext;
        }
        pLast->pNext = pEntry;
    }
    else
    {
        pEntry->pNext = 0;
        m_pFirstEntry = pEntry;
    }
    return pEntry->pState;
}

// FUNCTION: SWBF2 0x006ed570
RedShadingState* RedShadingPose::Find(unsigned int uiMaterialNameHash)
{
    if (!uiMaterialNameHash)
    {
        return 0;
    }
    for (Entry* pEntry = m_pFirstEntry; pEntry; pEntry = pEntry->pNext)
    {
        if (pEntry->uiMaterialNameHash == uiMaterialNameHash)
        {
            return pEntry->pState;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006ed5a0
RedShadingState* RedShadingPose::FindDefault(unsigned int uiRenderTypeHash)
{
    for (Entry* pEntry = m_pFirstEntry; pEntry && pEntry->uiMaterialNameHash == 0;
         pEntry = pEntry->pNext)
    {
        if (pEntry->pState->m_uiRenderTypeHash == uiRenderTypeHash)
        {
            return pEntry->pState;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006ed5d0
RedColor& RedShadingState::GetColorParam(unsigned int uiHash)
{
    return *(RedColor*)&m_ParamHash.Find(uiHash)->value;
}

// FUNCTION: SWBF2 0x006ed5f0
int& RedShadingState::GetIntParam(unsigned int uiHash)
{
    return *(int*)&m_ParamHash.Find(uiHash)->value;
}

// FUNCTION: SWBF2 0x006ed610
RedTextureTransform& RedShadingState::GetTextureTransformParam(unsigned int uiHash)
{
    return *(RedTextureTransform*)&m_ParamHash.Find(uiHash)->value;
}

// FUNCTION: SWBF2 0x006ed630
void RedShadingPose::Reset()
{
    while (m_pFirstEntry)
    {
        Entry* pNext = m_pFirstEntry->pNext;
        RedShadingState* pState = m_pFirstEntry->pState;

        pState->Release();
        delete m_pFirstEntry;
        m_pFirstEntry = pNext;
    }
}
