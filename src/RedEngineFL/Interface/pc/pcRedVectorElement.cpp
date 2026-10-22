#include "RedEngineFL/Interface/RedVectorElement.h"

#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <string.h>

// FUNCTION: SWBF2 0x006bf990
RedVectorElement::Point::Point()
{
}

// FUNCTION: SWBF2 0x006bfbb0
RedVectorElement::RedVectorElement()
{
    m_eBlendMode = BLEND_ALPHA;
    m_eStyle = VECTOR_STYLE_NORMAL;
    m_uiNumVertices = 0;
    m_pVertices = 0;
    m_pInterfaceShader = 0;
}

// FUNCTION: SWBF2 0x006bf9a0
RedVectorElement::~RedVectorElement()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x006bfb30
void RedVectorElement::SetBlendMode(BlendModeT eBlendMode)
{
    m_eBlendMode = eBlendMode;
}

// FUNCTION: SWBF2 0x006bfaf0
RedInterfaceElement::BlendModeT RedVectorElement::GetBlendMode() const
{
    return m_eBlendMode;
}

// FUNCTION: SWBF2 0x006bfc40
void RedVectorElement::Cleanup()
{
    Clear();

    if (m_pInterfaceShader)
    {
        m_pInterfaceShader->Release();
        m_pInterfaceShader = 0;
    }
}

// FUNCTION: SWBF2 0x006bfc90
void RedVectorElement::Clear()
{
    if (m_pVertices)
    {
        RedFreeToHeap(__RedCurrHeap, m_pVertices, 0);
        m_pVertices = 0;
    }

    m_uiNumVertices = 0;
}

// FUNCTION: SWBF2 0x006bfd80
void RedVectorElement::RenderUsingContext(const PblMatrix& mContext,
                                          const RedColor& color)
{
    if (!m_uiNumVertices)
    {
        return;
    }

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_LINESTRIP, 0x82, sizeof(Point), m_uiNumVertices, 0, false);

    void* pVertexData = pPrimitive->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    memcpy(pVertexData, m_pVertices, m_uiNumVertices * sizeof(Point));

    pPrimitive->EndVertexData();

    if (m_eBlendMode == BLEND_ADDITIVE)
    {
        m_pInterfaceShader->m_materialFlags |= 0x40;
    }
    else
    {
        m_pInterfaceShader->m_materialFlags &= ~0x40u;
    }

    m_pInterfaceShader->m_shadowEnable = (m_eStyle == VECTOR_STYLE_SHADOW);

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pInterfaceShader, mContext,
                                   color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006bfbf0
void RedVectorElement::Begin()
{
    if (!m_pInterfaceShader)
    {
        Init();
    }

    Clear();
    _Begin();

    m_curPoint.z = 0.0f;
    m_curPoint.y = 0.0f;
    m_curPoint.x = 0.0f;
    m_curPoint.color = RedColor::WHITE;
}

// FUNCTION: SWBF2 0x006bfcd0
void RedVectorElement::End()
{
    const unsigned int uiNumPoints = _GetNumPoints();
    const Point* pPoints = _GetPoints();
    const unsigned int uiSize = uiNumPoints * sizeof(Point);

    m_pVertices = (Point*)RedAllocFromHeap(__RedCurrHeap, uiSize, 0);
    m_uiNumVertices = uiNumPoints;
    memcpy(m_pVertices, pPoints, uiSize);

    _End();
}

// FUNCTION: SWBF2 0x006bfd30
void RedVectorElement::Init()
{
    m_pInterfaceShader = (pcInterfaceShader*)pcRedShader::Create(
        PblHash("Interface"), 0x82, 0, 0, 0, RedColor::WHITE, RedColor::BLACK,
        32.0f);

    m_pInterfaceShader->m_mode = pcInterfaceShader::INTERFACE_MODE_VECTOR;
}

// FUNCTION: SWBF2 0x006bfe60
RedVectorElement::Point* RedVectorElement::_AddPoint(const Point& point,
                                                     bool bConnected)
{
    if (!bConnected && _GetNumPoints())
    {
        Point* pBridge = _AddPoint();
        *pBridge = pBridge[-1];
        pBridge->color.a = 0;

        pBridge = _AddPoint();
        *pBridge = point;
        pBridge->color.a = 0;
    }

    Point* pPoint = _AddPoint();
    *pPoint = point;
    return pPoint;
}
