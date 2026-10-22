#pragma once

#include "RedInterfaceScreen.h"

class RedShadowBlob : public RedScreenGroupElement
{
public:
    struct BlobVertex
    {
        float intensity;
        float setValue;
        float blurValue;
        float pos[3];
    };

    // FUNCTION: SWBF2 0x006fc810
    static bool Create();

    static bool Destroy();

    static RedShadowBlob* Instance() { return sInstance; }

    void Update();

    static RedShadowBlob* sInstance;
    BlobVertex mData[20][15];
};
 
 
 
