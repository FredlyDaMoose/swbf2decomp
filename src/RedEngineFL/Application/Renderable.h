#ifndef SWBF2_GRAPHICS_RENDERABLE_H
#define SWBF2_GRAPHICS_RENDERABLE_H


#include "Battlefront2/Source/Thread.h"

// VTABLE: SWBF2 0x00772acc
class Renderable
{
public:

    Renderable();

    Renderable(int iPhase);

    virtual ~Renderable();

    // SYNTHETIC: SWBF2 0x006c3cf0
    // Renderable::`scalar deleting destructor'

    virtual void Activate(int value);

    virtual void Deactivate();

    virtual bool IsActive();

    virtual void Render(int value) = 0;

    virtual bool IsReflected();

    virtual bool HasReflection();

    int _ePhase;

    PblOrderedList<Renderable, int>::Node _RenderableNode;
};

 
 
 
 #endif
