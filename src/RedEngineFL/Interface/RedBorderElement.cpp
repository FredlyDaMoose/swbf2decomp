#include <math.h>
#include <string.h>

#include "RedBorderElement.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"

// FUNCTION: SWBF2 0x006f79e0
void RedBorderElement::SetBorderTexCoords(float fLeft, float fTop, float fRight, float fBottom,
                                       float fBorderX, float fBorderY, int  )
{
    float afY[4];
    afY[0] = fTop;
    afY[1] = fTop + fBorderY;
    afY[2] = fBottom - fBorderY;
    afY[3] = fBottom;

    const float fX1 = fLeft + fBorderX;
    const float fX2 = fRight - fBorderX;

    for (int r = 0; r < 4; ++r)
    {
        Vertex* pRow = &m_vertices[r * 4];
        const float fY = afY[r];
        pRow[0].u = fLeft;
        pRow[0].v = fY;
        pRow[1].u = fX1;
        pRow[1].v = fY;
        pRow[2].u = fX2;
        pRow[2].v = fY;
        pRow[3].u = fRight;
        pRow[3].v = fY;
    }
}

// FUNCTION: SWBF2 0x006f7500
RedBorderElement::RedBorderElement()
{
    mShadowMode = 0;
    m_eBlendMode = BLEND_ALPHA;
    m_pInterfaceShader = 0;
    memset(m_vertices, 0, sizeof(m_vertices));
    SetBorderTexCoords(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0);
}

// FUNCTION: SWBF2 0x006f75b0
void RedBorderElement::Cleanup()
{
    if (m_pInterfaceShader)
    {
        m_pInterfaceShader->Release();
        m_pInterfaceShader = 0;
    }
}

// SYNTHETIC: SWBF2 0x006f7580
// RedBorderElement::`scalar deleting destructor'
// FUNCTION: SWBF2 0x006f7ac0
RedBorderElement::~RedBorderElement()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x006f7680
const RedTexture* RedBorderElement::GetTexture() const
{
    if (m_pInterfaceShader)
    {
        return RedTexture::Find(m_pInterfaceShader->GetTexture());
    }
    return 0;
}

// FUNCTION: SWBF2 0x006f76b0
void RedBorderElement::Init()
{
    m_pInterfaceShader = (pcInterfaceShader*)pcRedShader::Create(
        PblHash("interface"), 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
    m_pInterfaceShader->m_mode = pcInterfaceShader::INTERFACE_MODE_BITMAP;
}

// FUNCTION: SWBF2 0x006f7850
void RedBorderElement::SetRoundedVertexPositions(float fLeft, float fTop, float fRight, float fBottom,
                                       float fBorderX, float fBorderY)
{
    fLeft = (float)floor(fLeft + 0.5f);
    fTop = (float)floor(fTop + 0.5f);
    fRight = (float)floor(fRight + 0.5f);
    fBottom = (float)floor(fBottom + 0.5f);
    fBorderX = (float)floor(fBorderX + 0.5f);
    fBorderY = (float)floor(fBorderY + 0.5f);

    float afY[4];
    afY[0] = fTop;
    afY[1] = fTop + fBorderY;
    afY[2] = fBottom - fBorderY;
    afY[3] = fBottom;

    const float fX1 = fLeft + fBorderX;
    const float fX2 = fRight - fBorderX;

    for (int r = 0; r < 4; ++r)
    {
        Vertex* pRow = &m_vertices[r * 4];
        const float fY = afY[r];
        pRow[0].x = fLeft;
        pRow[0].y = fY;
        pRow[1].x = fX1;
        pRow[1].y = fY;
        pRow[2].x = fX2;
        pRow[2].y = fY;
        pRow[3].x = fRight;
        pRow[3].y = fY;
    }
}

// FUNCTION: SWBF2 0x006f7a90
void RedBorderElement::SetTextureHash(unsigned int uiTextureHash)
{
    if (!m_pInterfaceShader)
    {
        Init();
    }
    m_pInterfaceShader->m_textureHash = uiTextureHash;
}

// FUNCTION: SWBF2 0x006f7c20
void RedBorderElement::GetBorderRect(float& fLeft, float& fTop, float& fRight, float& fBottom,
                                       float& fBorderX, float& fBorderY)
{
    fLeft = mBorderRect[0];
    fTop = mBorderRect[2];
    fRight = mBorderRect[1];
    fBottom = mBorderRect[3];
    fBorderX = mBorderRect[4];
    fBorderY = mBorderRect[5];
}

// FUNCTION: SWBF2 0x006f7b20
void RedBorderElement::CreateHotSpot(float fRadius)
{
    float fLeft, fTop, fRight, fBottom, fBorderX, fBorderY;
    GetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);

    fLeft -= fRadius;
    fTop -= fRadius;
    fRight += fRadius;
    fBottom += fRadius;

    if (!m_pHotSpot)
    {
        m_pHotSpot = new RedHotSpot;
    }
    m_pHotSpot->SetRect(fLeft, fTop, fRight, fBottom);
}

// FUNCTION: SWBF2 0x006f7c70
void RedBorderElement::SetBorderRect(float fLeft, float fTop, float fRight, float fBottom,
                                       float fBorderX, float fBorderY)
{
    mBorderRect[0] = fLeft;
    mBorderRect[1] = fRight;
    mBorderRect[2] = fTop;
    mBorderRect[3] = fBottom;
    mBorderRect[4] = fBorderX;
    mBorderRect[5] = fBorderY;
    SetRoundedVertexPositions(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
}

// FUNCTION: SWBF2 0x006f7470
RedBorderElement::RedBorderElement(const char* pcName)
    : RedInterfaceElement(pcName)
{
    mShadowMode = 0;
    m_eBlendMode = BLEND_ALPHA;
    m_pInterfaceShader = 0;
    memset(m_vertices, 0, sizeof(m_vertices));
    SetBorderTexCoords(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0);
}

// FUNCTION: SWBF2 0x006f7600
RedInterfaceElement::BlendModeT RedBorderElement::GetBlendMode() const
{
    return m_eBlendMode;
}

// FUNCTION: SWBF2 0x006f7700
bool RedBorderElement::IsBorderElement() const
{
    return true;
}

// FUNCTION: SWBF2 0x006f7710
void RedBorderElement::BeforeRender()
{
}

// FUNCTION: SWBF2 0x006f7720
void RedBorderElement::RenderUsingContext(const PblMatrix& mContext, const RedColor& color)
{
    static const unsigned short sIndices[28] = {
        0, 4, 1, 5, 2, 6, 3, 7, 7, 4,
        4, 8, 5, 9, 6, 10, 7, 11, 11, 8,
        8, 12, 9, 13, 10, 14, 11, 15
    };

    if (!m_pInterfaceShader)
    {
        return;
    }

    BeforeRender();

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x202, sizeof(Vertex), 16, 28, false);

    void* pVertexData = pPrimitive->BeginVertexData();
    unsigned short* pIndexData = pPrimitive->BeginIndexData();
    if (!pVertexData)
    {
        return;
    }

    memcpy(pVertexData, m_vertices, sizeof(m_vertices));
    memcpy(pIndexData, sIndices, sizeof(sIndices));

    pPrimitive->EndVertexData();
    pPrimitive->EndIndexData();

    if (m_eBlendMode == BLEND_ADDITIVE)
    {
        m_pInterfaceShader->m_materialFlags |= 0x40;
    }
    else
    {
        m_pInterfaceShader->m_materialFlags &= ~0x40u;
    }

    m_pInterfaceShader->m_shadowEnable = (mShadowMode == 1);

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pInterfaceShader, mContext, color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006f7840
void RedBorderElement::SetBlendMode(BlendModeT eMode)
{
    m_eBlendMode = eMode;
}

// FUNCTION: SWBF2 0x006f7cf0
void RedBorderElement::SetAlignedBorderRect(float fWidth, float fHeight,
                                        int eHorizontal, int eVertical,
                                        float fBorderX, float fBorderY)
{
    float fLeft = 0.0f, fRight = 0.0f;
    if (eHorizontal == 0)
    {
        fRight = fWidth;
    }
    else if (eHorizontal == 1)
    {
        fLeft = fWidth * -0.5f;
        fRight = -fLeft;
    }
    else if (eHorizontal == 2)
    {
        fLeft = -fWidth;
    }

    float fTop = 0.0f, fBottom = 0.0f;
    if (eVertical == 0)
    {
        fBottom = fHeight;
    }
    else if (eVertical == 1)
    {
        fTop = fHeight * -0.5f;
        fBottom = -fTop;
    }
    else if (eVertical == 2)
    {
        fTop = -fHeight;
    }

    SetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
}
