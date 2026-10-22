#pragma once

#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"

#define ZEPHYR_FILE_MAJOR_VERSION 8
#define ZEPHYR_FILE_MINOR_VERSION 0
#define ZEPHYR_ALIGN(uSize, iAlign) (((unsigned int)(uSize) + ((iAlign) - 1)) & ~(unsigned int)((iAlign) - 1))
#define ZEPHYR_HEADER_ID(a, b, c, d) \
    (((unsigned int)(a) << 24) | ((unsigned int)(b) << 16) | ((unsigned int)(c) << 8) | (unsigned int)(d))

struct ZephyrTransform
{
    ZephyrTransform()
    {
        kQuat._fX = 0.0f;
        kQuat._fY = 0.0f;
        kQuat._fZ = 0.0f;
        kQuat._fS = 1.0f;
    }

    bool IsValid() const { return kQuat._fS != 1000000.0f; }

    PblQuaternion kQuat;
    PblVector3 kTrans;
};
 
