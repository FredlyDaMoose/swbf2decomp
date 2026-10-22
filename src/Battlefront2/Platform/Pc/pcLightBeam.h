#pragma once

#include <stddef.h>

#include "pcSunFlare.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

struct VertexC
{
    CompressedPos pos;
    RedColor color;

    void SetPos(const PblVector3& v);
    void SetColor(const RedColor& c) { color = c; }
};
 
 
 
