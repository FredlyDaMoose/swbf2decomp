#ifndef SWBF2_GRAPHICS_PC_PCRENDERHELPERS_H
#define SWBF2_GRAPHICS_PC_PCRENDERHELPERS_H

#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMatrix.h"

static inline const PblVector3 operator+(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline const PblVector3 operator*(const PblVector3& a, float f)
{
    return PblVector3(a.x * f, a.y * f, a.z * f);
}

union RenderSortKey
{
    unsigned int id;
    struct
    {
        unsigned int depth : 13;
        unsigned int material : 16;
        unsigned int reserved : 3;
    } opaque;
    struct
    {
        unsigned int material : 16;
        unsigned int reserved : 2;
        unsigned int depth : 11;
        unsigned int pass : 2;
        unsigned int translucent : 1;
    } translucent;
};

// GLOBAL: SWBF2 0x007f58e8

// GLOBAL: SWBF2 0x007de2a0

// GLOBAL: SWBF2 0x007f58e0

// GLOBAL: SWBF2 0x007fa025

// GLOBAL: SWBF2 0x007fa026

// GLOBAL: SWBF2 0x007fa028

// GLOBAL: SWBF2 0x007fa048

// GLOBAL: SWBF2 0x007fa04c

// GLOBAL: SWBF2 0x007fa050

// GLOBAL: SWBF2 0x007fbe74

// GLOBAL: SWBF2 0x007fbeac

// GLOBAL: SWBF2 0x007fbeb0

// GLOBAL: SWBF2 0x007fbeb4

// GLOBAL: SWBF2 0x007fbeb8

// GLOBAL: SWBF2 0x008eecd0

// GLOBAL: SWBF2 0x009caee0

extern PblMatrix gMatrixIdentity;

#endif
