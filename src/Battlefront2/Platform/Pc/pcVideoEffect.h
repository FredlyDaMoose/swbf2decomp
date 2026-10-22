#pragma once

#include "RedEngineFL/Application/Renderable.h"

// VTABLE: SWBF2 0x00796ac0
class VideoEffectRenderable : public Renderable
{
public:

    VideoEffectRenderable()
        : Renderable(9)
    {
    }

    // SYNTHETIC: SWBF2 0x00433df0
    // VideoEffectRenderable::`scalar deleting destructor'

    virtual void Render(int value);
};
 

class VideoEffect
{
public:
    static void Init();
};
