#pragma once

#include <stddef.h>
#include <mediaobj.h>

namespace Snd
{

class DSMediaBuffer : public IMediaBuffer
{
public:

    DSMediaBuffer();
    DSMediaBuffer(unsigned char* pData, unsigned long ulMaxLength);
    virtual ~DSMediaBuffer();
    static long Create(unsigned char* pData, long lMaxLength, const _GUID& riid,
                       void** ppvObject, DSMediaBuffer* const pBuffer);

    unsigned long GetRefCount() const;

    unsigned int GetLength() const;

    void SetBuffer(void* const pData, long lLength, long lMaxLength);

    virtual long __stdcall QueryInterface(const _GUID& riid, void** ppvObject);

    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();

    virtual long __stdcall SetLength(unsigned long ulLength);
    virtual long __stdcall GetMaxLength(unsigned long* pulMaxLength);
    virtual long __stdcall GetBufferAndLength(unsigned char** ppBuffer, unsigned long* pulLength);

    unsigned long mLength;
    unsigned long mMaxLength;
    long mRef;
    unsigned char* mData;
    bool mDynamic;
};

 
 
 
 
 

}
