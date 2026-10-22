#ifndef SWBF2_GRAPHICS_PC_PCRENDERSTATEHELPERS_H
#define SWBF2_GRAPHICS_PC_PCRENDERSTATEHELPERS_H

#include "RedEngineFL/Application/Renderable.h"

namespace FLRenderer
{
class RenderableList : public PblOrderedList<Renderable, int>
{
public:
    void Add(Renderable* pRenderable, int iOrder);
    void Remove(Renderable* pRenderable);

    void Render() const;

    void RenderReflections(unsigned int uiFlags) const;
};

extern RenderableList _RenderableLists[10];
}
 

#include "RedEngineFL/Application/pc/pcRedDropShadow.h"

// GLOBAL: SWBF2 0x007dfa84

// GLOBAL: SWBF2 0x008f6e04

// GLOBAL: SWBF2 0x008f6e08

namespace FLRenderer
{
extern unsigned int m_numPotentialReflections;
extern Renderable* m_potentialReflections[50];

void AddRenderable(Renderable* pRenderable, int ePhase, int iOrder);
bool IsRenderableActive(const Renderable* pRenderable, int ePhase);
void RemoveRenderable(Renderable* pRenderable);
}

#endif
