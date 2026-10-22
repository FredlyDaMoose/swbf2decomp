#pragma once

#include "sndsource.h"
#include "sndsource2d.h"
#include "sndsource3d.h"

namespace Snd
{

class Source2D3D : public Source, public Source2D, public Source3D::Properties
{
public:

    Source2D3D() : mFlags(0) {}

    void Set3DMode(bool b3DMode);

    bool Get3DMode() const { return (mFlags & 1) != 0; }

    unsigned char mFlags;

protected:

    bool Read(PblFileChunk& chunk, unsigned int uiPropertyID);

    bool Get3DModeChanged()
    {
        const bool bChanged = (mFlags & 4) != 0;
        mFlags &= (unsigned char)~4u;
        return bChanged;
    }
    float CalculatePitch(const Source3D* pSource3D) const;
};

 

}
