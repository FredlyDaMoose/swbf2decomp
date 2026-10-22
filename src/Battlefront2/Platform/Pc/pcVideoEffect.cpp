#include "pcVideoEffect.h"

// FUNCTION: SWBF2 0x00433e20
void VideoEffectRenderable::Render(int)
{
}

// GLOBAL: SWBF2 0x01e2fd58
static VideoEffectRenderable* s_pRenderable;

// FUNCTION: SWBF2 0x00433e30
void VideoEffect::Init()
{
    s_pRenderable = new VideoEffectRenderable;
}
