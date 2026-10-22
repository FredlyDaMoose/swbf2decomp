#pragma once

#include <stddef.h>

#include "RedEngineFL/Application/Renderable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class Aimer;

// VTABLE: SWBF2 0x007aa74c
class MuzzleFlashRenderer : public Renderable
{
public:

    struct MuzzleFlashData
    {
        Aimer* m_pAimer;
        PblVector3 mDir;
        float mRadius;
        float mWidth;
        float mLength;
        RedColor mColor;
    };

    MuzzleFlashRenderer();
    virtual ~MuzzleFlashRenderer();

    virtual void Render(int value);

    void RenderFlash(MuzzleFlashData const& data, RedColor const& color) const;

    void RenderStreak(MuzzleFlashData const& data, RedColor const& color) const;

    MuzzleFlashData mMuzzleFlashData[2][30];
    int mCurrentBuffer;
    int mNumMuzzleFlashes;
    int mOldNumMuzzleFlashes;
};
 
 
 
 
 

namespace MuzzleFlash
{
bool Destroy();
}
