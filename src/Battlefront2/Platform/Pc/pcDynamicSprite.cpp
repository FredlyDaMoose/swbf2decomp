#include "pcDynamicSprite.h"

#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

#include "RedEngineFL/Memory/engineAlloc.h"

// FUNCTION: SWBF2 0x00411940
void pcDynamicSprite::ClearViewportsCallback::Render(int)
{
    m_pOwner->ClearViewports();
}

// FUNCTION: SWBF2 0x004119c0
void pcDynamicSprite::Init()
{
    if (!s_Instance.IsActive())
    {
        s_Instance.Activate(0);
    }
}

pcDynamicSprite::pcDynamicSprite()
    : Renderable(2), m_ClearViewportsCallback(this)
{
    m_iNumViewports = 0;
    m_iNumViewportsX = 0;
    m_iNumTempBufferSprites = 0;
    m_bViewportsLocked = false;
}

// FUNCTION: SWBF2 0x00411ab0
pcDynamicSprite::~pcDynamicSprite()
{
    Deactivate();
}

// FUNCTION: SWBF2 0x00411b30
void pcDynamicSprite::Activate(int)
{
    RedTexture* target = RedRenderer::pcGetBuffer(PC_BUFFER_TEMP);
    m_iNumViewportsX = target->m_uiWidth >> 7;
    m_iNumViewports = (target->m_uiHeight >> 7) * m_iNumViewportsX;
    m_fViewportWidth = 128.0f / target->m_uiWidth;
    m_fViewportHeight = 128.0f / target->m_uiHeight;
    m_iNumTempBufferSprites = 0;
    m_bViewportsLocked = false;
    Renderable::Activate(-9999);
    m_ClearViewportsCallback.Activate(9999);
}

// FUNCTION: SWBF2 0x00411be0
void pcDynamicSprite::Deactivate()
{
    m_ClearViewportsCallback.Deactivate();
    Renderable::Deactivate();
    m_iNumViewports = 0;
    m_iNumViewportsX = 0;
    m_iNumTempBufferSprites = 0;
    m_bViewportsLocked = false;
}

// FUNCTION: SWBF2 0x00411c10
void pcDynamicSprite::Render(int)
{
    m_iNumTempBufferSprites = 0;
    m_bViewportsLocked = false;
}

// FUNCTION: SWBF2 0x00411c20
void pcDynamicSprite::ClearViewports()
{
    m_bViewportsLocked = true;
}

// GLOBAL: SWBF2 0x01eafda8
// SYNTHETIC: SWBF2 0x004014b0
// `dynamic initializer for 'pcDynamicSprite::s_Instance''
// SYNTHETIC: SWBF2 0x00766ce0
// `dynamic atexit destructor for 'pcDynamicSprite::s_Instance''
pcDynamicSprite pcDynamicSprite::s_Instance;

// SYNTHETIC: SWBF2 0x004119e0
// `dynamic initializer for 'pcDynamicSprite::s_Instance''
