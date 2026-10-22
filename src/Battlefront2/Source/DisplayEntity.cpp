#include "DisplayEntity.h"

// FUNCTION: SWBF2 0x0048d620
void EntityElement::SetScale(float fScale)
{
    mScale = fScale;
}

// FUNCTION: SWBF2 0x0048d640
void EntityElement::SetSelected(bool bSelected)
{
    (void)bSelected;
}

// FUNCTION: SWBF2 0x0048d650
EntityElement::EntityElement()
    : mScale(1.0f)
    , mTranslation(0.0f, 0.0f, 0.0f)
    , mModel(0)
{
    mRotation._fX = 0.0f;
    mRotation._fY = 0.0f;
    mRotation._fZ = 0.0f;
    mRotation._fS = 1.0f;
}

// SYNTHETIC: SWBF2 0x0048d6f0
// EntityElement::`scalar deleting destructor'
// FUNCTION: SWBF2 0x0048d720
EntityElement::~EntityElement()
{
}

// FUNCTION: SWBF2 0x0048d730
void EntityElement::RenderUsingContext(const PblMatrix&, const RedColor&)
{
}
