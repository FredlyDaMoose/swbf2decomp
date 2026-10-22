#pragma once

#include "RedEngineFL/Application/Renderable.h"

// VTABLE: SWBF2 0x0079657c
class LetterboxEffectRenderable : public Renderable
{
public:

    LetterboxEffectRenderable()
        : Renderable(9)
    {
    }

    // SYNTHETIC: SWBF2 0x0041f950
    // LetterboxEffectRenderable::`scalar deleting destructor'

    virtual void Render(int value);
};
 

class LetterboxEffect
{
public:
    static void Init();
};
