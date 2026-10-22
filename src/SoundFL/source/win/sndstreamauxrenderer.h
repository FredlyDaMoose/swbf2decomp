#pragma once

#include <stddef.h>

#include "snddsbufferrenderer.h"

namespace Snd
{

class StreamFormat;

class StreamAuxRenderer : public DSBufferRenderer
{
public:
    StreamAuxRenderer();
    virtual ~StreamAuxRenderer();
    bool Open(unsigned char ucDeviceIndex, StreamFormat const& cFormat,
              float fBufferMs);

private:
    virtual void UpdateObject();

public:
    bool mIsOpen;
    IDirectSound* mDevice;
};
 
 
 

void __fastcall StreamAuxRenderer__Close(void* pAuxRenderer);

}
