#include "sndlistener.h"
#include "sndengine.h"
#include "SoundFL/source/sndi3dl2reverbpreset.h"
#include "SoundFL/source/sndspace.h"

namespace Snd
{

// FUNCTION: SWBF2 0x007488b0
void Listener::Update()
{
    ListenerBase::Update();

    if (GetSpaceChanged() && GetReverbPresetChanged())
    {
        const I3DL2Reverb* const pReverb = (const I3DL2ReverbPreset*)
            (smActiveListeners > 1 ? Space::smDefaultSpace.mReverbPreset
                                   : mSpace->mReverbPreset);
        if (pReverb != Engine::GetGlobalReverb() && GetReverbEnable())
        {
            Engine::SetGlobalReverb(pReverb);
        }
    }
}

}
