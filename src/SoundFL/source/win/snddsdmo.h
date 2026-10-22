#pragma once

#include <stddef.h>
#include <mediaobj.h>

#include "snddsbuffer.h"
#include "snddsmediabuffer.h"

namespace Snd
{

class StreamFormat;

class MediaType : public DMO_MEDIA_TYPE
{
public:
    MediaType(const StreamFormat& rFormat);

    DSBuffer::WaveFormat mWaveformat;
};

class DSDMO
{
public:
    DSDMO();
    ~DSDMO();
    bool Create(const _GUID& rCLSID, const StreamFormat* const pInput,
                const StreamFormat* const pOutput);
    void Destroy();
    IMediaObject* GetMediaObject() const;
    bool IsValid() const;
    bool IsInputDataComplete();

    bool DeliverInputData(void* const pData, unsigned int uiSize);

    unsigned int GetOutputData(void* const pData, unsigned int uiSize);

    IMediaObject* mObject;
    IMediaObjectInPlace* mObjectInPlace;
    DSMediaBuffer mInputData;
};

 
 

 
 
 

}
