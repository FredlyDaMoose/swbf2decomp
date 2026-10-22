#pragma once

#include <stddef.h>

#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

// VTABLE: SWBF2 0x00783e78
class RedShape : public Renderable
{
public:

    // FUNCTION: SWBF2 0x006f07b0
    RedShape();

    virtual ~RedShape() {}

    // SYNTHETIC: SWBF2 0x006f0920
    // RedShape::`scalar deleting destructor'

    virtual void Render(int value) = 0;

    void SetColor(RedColor color)
    {
        _color = ((((unsigned int)color.a << 8 | color.r) << 8 | color.g) << 8) | color.b;
    }

    PblMatrix _Matrix;
    PblMatrix* _pMatrix;
    PblVector3 _vMin;
    PblVector3 _vMax;
    unsigned int _color;
};
 
 
 

// VTABLE: SWBF2 0x00783eb8
class RedSphereShape : public RedShape
{
public:
    // FUNCTION: SWBF2 0x006f0860
    RedSphereShape();
    // FUNCTION: SWBF2 0x006f08c0
    virtual ~RedSphereShape() {}

    // SYNTHETIC: SWBF2 0x006f0950
    // RedSphereShape::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006f2bb0
    virtual void Render(int value);
};

// VTABLE: SWBF2 0x00783e98
class RedTetrahedronShape : public RedShape
{
public:
    // FUNCTION: SWBF2 0x006f0880
    RedTetrahedronShape();
    // FUNCTION: SWBF2 0x006f08d0
    virtual ~RedTetrahedronShape() {}

    // SYNTHETIC: SWBF2 0x006f0980
    // RedTetrahedronShape::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006f2bc0
    virtual void Render(int value);
};
   