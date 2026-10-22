#include "FlashyTextElement.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <string.h>

FlashyTextManager::FlashyTextManager()
{
}

FlashyTextManager::~FlashyTextManager()
{
}

// GLOBAL: SWBF2 0x01ebcff0
// SYNTHETIC: SWBF2 0x00403870
// `dynamic initializer for 'sFlashyTextManagerInstance''
// SYNTHETIC: SWBF2 0x00768730
// `dynamic atexit destructor for 'sFlashyTextManagerInstance''
FlashyTextManager sFlashyTextManagerInstance;

// FUNCTION: SWBF2 0x005269b0
void FlashyTextManager::Init(bool bMission)
{
    sFlashyTextManagerInstance.ActivateThread(0, bMission ? PER_FRAME : PER_TURN);
}

// GLOBAL: SWBF2 0x007e6670
static float cStateTimes[6] = { 0.0f, 0.1f, 0.05f, 0.05f, 0.15f, 0.15f };

// FUNCTION: SWBF2 0x00525f80
FlashyTextElement::FlashyTextElement(const char* pcName)
    : RedTextElement(pcName)
{
    CommonConstructor();
    PlatformConstructor();
}

// FUNCTION: SWBF2 0x00525fd0
FlashyTextElement::~FlashyTextElement()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x00526020
void FlashyTextElement::CommonConstructor()
{
    mState = FLASHY_STATE_DONE;
    mTime = 0.0f;
    mTimeDelay = 0.0f;
    mWidth = 0;
    mTail = 0;
    mTextYScale = 1.0f;
    mTextColor = RedColor::WHITE;
    mBGOffsetX = 0;
    mBGOffsetY = 0;
    mBGExpandX = 0;
    mBGExpandY = 0;
    mHashRight = 0;
    mHashMid = 0;
    mHashLeft = 0;
    mFlipped = false;
    mXFlipped = false;
    mRightJustifyBG = false;
    mDrawBG = false;
}

// FUNCTION: SWBF2 0x005260a0
void FlashyTextElement::SetBackgroundSize(char cOffsetX, char cOffsetY, char cExpandX, char cExpandY)
{
    mBGOffsetX = cOffsetX - 12;
    mBGOffsetY = cOffsetY;
    mBGExpandX = cExpandX;
    mBGExpandY = cExpandY;
}

// FUNCTION: SWBF2 0x005260d0
void FlashyTextElement::SetBackground(unsigned int uHashLeft, unsigned int uHashMid, unsigned int uHashRight)
{
    if (uHashLeft)
    {
        mDrawBG = true;
        mHashLeft = uHashLeft;
        mHashMid = uHashMid;
        mHashRight = uHashRight;
    }
    else
    {
        mDrawBG = false;
    }
}

// FUNCTION: SWBF2 0x00526110
void FlashyTextElement::RenderUsingContext(const PblMatrix& mContext, const RedColor& color)
{
    float fLeft, fTop, fRight, fBottom;
    GetRect(fLeft, fTop, fRight, fBottom);

    TextHAlignT eHAlign;
    TextVAlignT eVAlign;
    GetTextAlign(eHAlign, eVAlign);
    if (eHAlign != TEXT_HALIGN_LEFT)
    {
        float fTextWidth = fRight - fLeft;
        float fShift;
        if (mRightJustifyBG)
        {
            fShift = (mWidth ? (float)mWidth : fTextWidth) - fTextWidth;
        }
        else
        {
            fShift = ((mWidth ? (float)mWidth : fTextWidth) - fTextWidth) * 0.5f;
        }
        fRight -= fShift;
        fLeft -= fShift;
    }

    if (mDrawBG && mState != FLASHY_STATE_DELAY)
    {
        int iExpandX = mBGExpandX;
        int iExpandY = mBGExpandY;
        float fX = (float)mBGOffsetX + fLeft - (float)iExpandX;
        float fY = (float)mBGOffsetY + fTop - (float)iExpandY;
        float fWidth = (float)(iExpandX * 2) + (mWidth ? (float)mWidth : fRight - fLeft) + (float)mTail;
        float fHeight = (float)(iExpandY * 2) + (fBottom - fTop);
        if (mState == FLASHY_STATE_BG)
        {
            fWidth = mTime / cStateTimes[FLASHY_STATE_BG] * fWidth;
        }

        RedColor bgColor = color;
        if (mHashMid != 0x83245ab6 && m_eStyle != TEXT_STYLE_SELECTED)
        {
            bgColor.b = 0x96;
            bgColor.g = 0x96;
            bgColor.r = 0x96;
        }

        float fY1 = fHeight + fY;
        float fCapX = fHeight + fX;
        if (mXFlipped)
        {
            RenderBitmap(mContext, bgColor, mHashLeft, fCapX, fY, fX, fY1);
            float fMidX = fWidth + fX;
            RenderBitmap(mContext, bgColor, mHashMid, fMidX, fY, fCapX, fY1);
            if (mFlipped)
            {
                RenderBitmap(mContext, bgColor, mHashRight, fMidX + fHeight, fY1, fMidX, fY);
            }
            else
            {
                RenderBitmap(mContext, bgColor, mHashRight, fMidX + fHeight, fY, fMidX, fY1);
            }
        }
        else
        {
            if (mFlipped)
            {
                RenderBitmap(mContext, bgColor, mHashLeft, fX, fY1, fCapX, fY);
            }
            else
            {
                RenderBitmap(mContext, bgColor, mHashLeft, fX, fY, fCapX, fY1);
            }
            float fMidX = fWidth + fX;
            RenderBitmap(mContext, bgColor, mHashMid, fCapX, fY, fMidX, fY1);
            if (mFlipped)
            {
                RenderBitmap(mContext, bgColor, mHashRight, fMidX, fY1, fMidX + fHeight, fY);
            }
            else
            {
                RenderBitmap(mContext, bgColor, mHashRight, fMidX, fY, fMidX + fHeight, fY1);
            }
        }
    }

    float fCursor = 0.0f;
    if (mState == FLASHY_STATE_DELAY2 || mState == FLASHY_STATE_CURSOR)
    {
        if (mState == FLASHY_STATE_CURSOR)
        {
            fCursor = mTime / cStateTimes[FLASHY_STATE_CURSOR];
        }
        float fX = (fRight - fLeft) * fCursor + fLeft;
        RenderBitmap(mContext, color, 0xe336320f, fX, fTop, (fBottom - fTop) + fX, fBottom);
    }

    if (mState >= FLASHY_STATE_EXPAND)
    {
        float fScale = 1.0f;
        if (mState == FLASHY_STATE_EXPAND)
        {
            fScale = mTime / cStateTimes[FLASHY_STATE_EXPAND];
        }

        PblMatrix mScaled = mContext;
        mScaled.m[1][0] = mContext.m[1][0] * fScale;
        mScaled.m[1][1] = mContext.m[1][1] * fScale;
        mScaled.m[1][2] = mContext.m[1][2] * fScale;
        mScaled.m[1][3] = 0.0f;
        mScaled.m[3][1] = mContext.m[3][1] - (1.0f - fScale) * 0.5f * (fBottom - fTop);

        RedColor fade = RedColor::WHITE;
        if (mState == FLASHY_STATE_ALPHAIN)
        {
            fade.a = (unsigned char)(int)(mTime / cStateTimes[FLASHY_STATE_ALPHAIN] * 255.0f);
        }

        RedColor textColor = mTextColor;
        if (!mDrawBG && m_eStyle == TEXT_STYLE_SELECTED)
        {
            float fGreen = color.a * 1.2f + 50.0f;
            if (!(255.0f > fGreen))
            {
                fGreen = 255.0f;
            }
            textColor.b = 100;
            textColor.g = (unsigned char)(int)fGreen;
            textColor.r = 0xff;
            textColor.a = 0xff;
        }

        RedTextElement::RenderUsingContext(mScaled, fade * textColor * color);
    }
}

// FUNCTION: SWBF2 0x005268c0
void FlashyTextElement::StartFlashiness()
{
    mState = FLASHY_STATE_DELAY;
    mTime = 0.0f;
    if (!FlashyTextManager::IsElementActive(this))
    {
        FlashyTextManager::AddToUpdateList(this);
    }
}

// FUNCTION: SWBF2 0x005268e0
bool FlashyTextElement::UpdateFlashiness(float fDeltaTime)
{
    cStateTimes[FLASHY_STATE_DELAY] = mTimeDelay;
    mTime += fDeltaTime;
    if (mTime >= cStateTimes[mState])
    {
        do
        {
            mTime -= cStateTimes[mState];
            mState = mState + 1;
            if (mState == FLASHY_STATE_DONE)
            {
                FlashyTextManager::RemoveFromUpdateList(this);
                break;
            }
        } while (mTime >= cStateTimes[mState]);
    }
    return true;
}

// FUNCTION: SWBF2 0x005269d0
void FlashyTextManager::Clean()
{
    sFlashyTextManagerInstance.mFTEList.RemoveAll();
}

// FUNCTION: SWBF2 0x005269e0
bool FlashyTextManager::Update(float fDeltaTime)
{
    for (PblList<FlashyTextElement>::Itor it(sFlashyTextManagerInstance.mFTEList); (FlashyTextElement*)it; ++it)
    {
        it->UpdateFlashiness(fDeltaTime);
    }
    return true;
}

// FUNCTION: SWBF2 0x00526a20
void FlashyTextManager::AddToUpdateList(FlashyTextElement* pElement)
{
    sFlashyTextManagerInstance.mFTEList.Add(&pElement->mNode, pElement);
}

// FUNCTION: SWBF2 0x00526a60
void FlashyTextManager::RemoveFromUpdateList(FlashyTextElement* pElement)
{
    sFlashyTextManagerInstance.mFTEList.Remove(&pElement->mNode);
}

// FUNCTION: SWBF2 0x00526aa0
bool FlashyTextManager::IsElementActive(FlashyTextElement* pElement)
{
    return pElement->mNode.IsInList();
}

// FUNCTION: SWBF2 0x00414720
void FlashyTextElement::PlatformConstructor()
{
    m_pInterfaceBitmapShader = 0;
}

// FUNCTION: SWBF2 0x00414730
void FlashyTextElement::Init()
{
    m_pInterfaceBitmapShader = (pcInterfaceShader*)pcRedShader::Create(
        0x84ea5130, 0x202, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);

    m_pInterfaceBitmapShader->m_mode = pcInterfaceShader::INTERFACE_MODE_BITMAP;
}

// FUNCTION: SWBF2 0x00414770
void FlashyTextElement::Cleanup()
{
    if (m_pInterfaceBitmapShader)
    {
        m_pInterfaceBitmapShader->Release();
        m_pInterfaceBitmapShader = 0;
    }
}

// FUNCTION: SWBF2 0x004147c0
void FlashyTextElement::RenderBitmap(const PblMatrix& mContext, const RedColor& color,
                                     unsigned int uHash, float fX0, float fY0,
                                     float fX1, float fY1)
{
    // GLOBAL: SWBF2 0x007ec3f0
    static FlashyVertexT vertices[4] = {
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f },
    };

    m_pInterfaceBitmapShader->SetTexture(uHash);
    vertices[0].x = fX0;
    vertices[0].y = fY0;
    vertices[1].x = fX0;
    vertices[1].y = fY1;
    vertices[2].x = fX1;
    vertices[2].y = fY0;
    vertices[3].x = fX1;
    vertices[3].y = fY1;
    m_pInterfaceBitmapShader->m_materialFlags &= ~0x40u;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x202, sizeof(FlashyVertexT), 4, 0, false);

    void* pVertexData = pPrimitive->BeginVertexData();
    if (!pVertexData)
    {
        return;
    }

    memcpy(pVertexData, vertices, sizeof(vertices));

    pPrimitive->EndVertexData();

    RedRenderer::pcRenderPrimitive(pPrimitive, m_pInterfaceBitmapShader,
                                   mContext, color, 0x200, 0, 0, 0);

    pPrimitive->Release();
}
