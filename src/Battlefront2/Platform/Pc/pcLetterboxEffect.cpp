#include "pcLetterboxEffect.h"

// FUNCTION: SWBF2 0x0041f980
void LetterboxEffectRenderable::Render(int)
{
}

// GLOBAL: SWBF2 0x01e2fd2c
static LetterboxEffectRenderable* s_pRenderable;

// FUNCTION: SWBF2 0x0041f990
void LetterboxEffect::Init()
{
    s_pRenderable = new LetterboxEffectRenderable;
}

// FUNCTION: SWBF2 0x0041fa40
float Max(float fA, float fB)
{
    if (fA > fB)
    {
        return fA;
    }
    return fB;
}
