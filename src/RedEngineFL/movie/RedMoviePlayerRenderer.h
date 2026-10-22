#pragma once

#include "RedEngineFL/Interface/RedInterfaceScreen.h"

class RedBitmapElement;
class RedTexture;

class RedMoviePlayerRenderer
{
public:
    // FUNCTION: SWBF2 0x00726840
    RedMoviePlayerRenderer();

    // FUNCTION: SWBF2 0x00726870
    ~RedMoviePlayerRenderer() {}

    bool OpenRenderer(int iHeap, RedBitmapElement** ppBitmaps,
                      unsigned int uiNumBitmaps);
    RedBitmapElement** CloseRenderer(int iHeap);
    void EndFrame();

    void Render(const RedTexture* const pTexture, int iX, int iY,
                unsigned int uiWidth, unsigned int uiHeight, unsigned char ucFlags);

protected:
    void Disable();

public:

    RedBitmapElement** mBitmaps;
    unsigned int mNumBitmaps;
    unsigned int mRenderBitmap;
    RedScreenGroupElement mGroupElement;
};

 
