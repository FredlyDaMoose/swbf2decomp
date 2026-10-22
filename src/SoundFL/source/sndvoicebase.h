#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "sndallocator.h"
#include "sndsource2d3d.h"
#include "sndsource3d.h"
#include "sndsourcetransport.h"

namespace Snd
{

class Sample;
class VoiceVirtualManager;

class VoiceBase : public Source3D,
                  public Source2D3D,
                  public SourceTransport,
                  public Allocator<VoiceBase>
{
protected:

    VoiceBase();

    ~VoiceBase();

public:

    enum Flags
    {
        flags_locked        = 1,
        flags_updateSample  = 2
    };

    void Lock(bool bLocked);

    bool GetLocked() const { return (mFlags & flags_locked) != 0; }

protected:

    bool GetSampleChanged()
    {
        const bool bChanged = (mFlags & flags_updateSample) != 0;
        mFlags &= (unsigned char)~(unsigned int)flags_updateSample;
        return bChanged;
    }
public:

    void RemoveFromManager();

    void SetSample(const Sample* pSample);

    void CopyParameters(const VoiceBase& rOther, bool bSample);

    PblListSingle::Node mNode;
    VoiceVirtualManager* mManager;
    Sample* mSample;
    unsigned char mFlags;
};

 

}
