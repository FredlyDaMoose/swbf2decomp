#include "RedMoviePlayerRenderer.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

// FUNCTION: SWBF2 0x00726b50
void RedMoviePlayerRenderer::Render(const RedTexture* const pTexture, int iX, int iY,
                                    unsigned int uiWidth, unsigned int uiHeight, unsigned char ucFlags)
{
    RedBitmapElement* pBitmap = mBitmaps[mRenderBitmap++];
    pBitmap->Enable(true);
    pBitmap->SetTexture(pTexture->m_uiNameHash);
    pBitmap->SetRect((float)iX, (float)iY, (float)uiWidth + (float)iX,
                     (float)uiHeight + (float)iY);
    pBitmap->m_color.a = ucFlags;
}

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
void RemoveInterfaceElement(RedScreenGroupElement* pElement);
}

// FUNCTION: SWBF2 0x00726840
RedMoviePlayerRenderer::RedMoviePlayerRenderer()
    : mGroupElement(0)
{
    mBitmaps = 0;
    mNumBitmaps = 0;
    mRenderBitmap = 0;
}

// FUNCTION: SWBF2 0x00726880
RedBitmapElement** RedMoviePlayerRenderer::CloseRenderer(int iHeap)
{
    RedBitmapElement** ppBitmaps = mBitmaps;
    int iOldHeap = RedSetCurrentHeap(iHeap);

    for (unsigned int i = 0; i < mNumBitmaps; ++i)
    {
        mGroupElement.RemoveElement(mBitmaps[i]);
        delete mBitmaps[i];
    }
    FLRenderer::RemoveInterfaceElement(&mGroupElement);

    mBitmaps = 0;
    mNumBitmaps = 0;
    RedSetCurrentHeap(iOldHeap);
    return ppBitmaps;
}

// FUNCTION: SWBF2 0x00726900
void RedMoviePlayerRenderer::Disable()
{
    for (unsigned int i = 0; i < mNumBitmaps; ++i)
    {
        mBitmaps[i]->Enable(false);
        mBitmaps[i]->SetTexture(0u);
    }
    mGroupElement.Enable(false);
}

// FUNCTION: SWBF2 0x00726940
void RedMoviePlayerRenderer::EndFrame()
{
    mGroupElement.Enable(mRenderBitmap != 0);
    for (unsigned int i = mRenderBitmap; i < mNumBitmaps; ++i)
    {
        RedBitmapElement* pBitmap = mBitmaps[i];
        pBitmap->Enable(false);
        pBitmap->SetTexture(0u);
        pBitmap->SetRect(0.0f, 0.0f, 0.0f, 0.0f);
    }
    mRenderBitmap = 0;
}

// FUNCTION: SWBF2 0x007269c0
bool RedMoviePlayerRenderer::OpenRenderer(int iHeap, RedBitmapElement** ppBitmaps,
                                          unsigned int uiNumBitmaps)
{
    int iOldHeap = RedSetCurrentHeap(iHeap);

    unsigned int i;
    for (i = 0; i < uiNumBitmaps; ++i)
    {
        ppBitmaps[i] = new RedBitmapElement;
        if (!ppBitmaps[i])
        {
            while (i)
            {
                delete ppBitmaps[--i];
            }
            RedSetCurrentHeap(iOldHeap);
            return false;
        }
    }

    for (i = 0; i < uiNumBitmaps; ++i)
    {
        RedBitmapElement* pBitmap = ppBitmaps[i];
        pBitmap->Init();
        pBitmap->SetZOrder((unsigned char)(i - uiNumBitmaps - 1));
        pBitmap->SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);
        mGroupElement.AddElement(ppBitmaps[i]);
    }
    mGroupElement.SetZOrder(255);
    FLRenderer::AddInterfaceElement(&mGroupElement, 0.0f, 0.0f, false);

    RedSetCurrentHeap(iOldHeap);
    mBitmaps = ppBitmaps;
    mNumBitmaps = uiNumBitmaps;
    return true;
}
