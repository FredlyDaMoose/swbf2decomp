#include "Renderable.h"

#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"

#include "RedEngineFL/Memory/engineAlloc.h"

// SYNTHETIC: SWBF2 0x00639955
// [thunk]:SkyObjectManager::SkyObjectClass::`scalar deleting destructor' adjustor{28}

// SYNTHETIC: SWBF2 0x00695054
// [thunk]:PrecipitationEffect::`scalar deleting destructor' adjustor{36}

// FUNCTION: SWBF2 0x006c3c60
Renderable::Renderable(int iPhase)
{
    _ePhase = iPhase;

}

// FUNCTION: SWBF2 0x006c3ca0
Renderable::Renderable()
{
    _ePhase = 3;

}

// FUNCTION: SWBF2 0x00410b80
bool Renderable::IsReflected()
{
    return true;
}

// FUNCTION: SWBF2 0x00410b90
bool Renderable::HasReflection()
{
    return false;
}

// FUNCTION: SWBF2 0x006c3ce0
Renderable::~Renderable()
{
}

// FUNCTION: SWBF2 0x006c3d20
void Renderable::Activate(int value)
{
    FLRenderer::AddRenderable(this, _ePhase, value);
}

// FUNCTION: SWBF2 0x006c3d40
void Renderable::Deactivate()
{
    FLRenderer::RemoveRenderable(this);
}

// FUNCTION: SWBF2 0x006c3d50
bool Renderable::IsActive()
{
    return FLRenderer::IsRenderableActive(this, _ePhase);
}
