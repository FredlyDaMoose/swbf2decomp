#include <windows.h>
#include <mediaobj.h>

#include "snddsmediabuffer.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00750e70
unsigned long __stdcall DSMediaBuffer::AddRef()
{
    return InterlockedIncrement(&mRef);
}

// FUNCTION: SWBF2 0x00750f80
unsigned long DSMediaBuffer::GetRefCount() const
{
    return (unsigned long)mRef;
}

// FUNCTION: SWBF2 0x00750f90
long __stdcall DSMediaBuffer::QueryInterface(const _GUID& riid, void** ppvObject)
{
    if (ppvObject == NULL)
    {
        return E_POINTER;
    }

    if (riid == IID_IMediaBuffer || riid == IID_IUnknown)
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

// FUNCTION: SWBF2 0x00751020
unsigned long __stdcall DSMediaBuffer::Release()
{
    const long lRef = InterlockedDecrement(&mRef);
    if (lRef == 0)
    {
        if (mDynamic)
        {
            delete this;
            return lRef;
        }
        mData = 0;
        mMaxLength = 0;
        mLength = 0;
    }
    return lRef;
}

// FUNCTION: SWBF2 0x00751070
void DSMediaBuffer::SetBuffer(void* const pData, long lLength, long lMaxLength)
{
    mData = (unsigned char*)pData;
    mLength = (unsigned long)lLength;
    mMaxLength = (unsigned long)lMaxLength;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x00750e00
DSMediaBuffer::DSMediaBuffer()
    : mLength(0),
      mMaxLength(0),
      mRef(0),
      mData(NULL),
      mDynamic(false)
{

}
}

namespace Snd {

// FUNCTION: SWBF2 0x00750e90
long DSMediaBuffer::Create(unsigned char* pData, long lMaxLength, const _GUID& riid,
                           void** ppvObject, DSMediaBuffer* const pBuffer)
{
    DSMediaBuffer* pMediaBuffer = pBuffer;
    if (pMediaBuffer == NULL)
    {
        pMediaBuffer = new DSMediaBuffer(pData, lMaxLength);
        if (pMediaBuffer == NULL)
        {
            return E_OUTOFMEMORY;
        }
        pMediaBuffer->mDynamic = true;
    }

    pMediaBuffer->SetBuffer(pData, 0, lMaxLength);
    return pMediaBuffer->QueryInterface(riid, ppvObject);
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00750e30
DSMediaBuffer::~DSMediaBuffer()
{

}
}

namespace Snd {

// FUNCTION: SWBF2 0x00750e40
void* __fastcall sub_750E40(void* pBuffer, int, unsigned int uiFlags)
{
    ((DSMediaBuffer*)pBuffer)->DSMediaBuffer::~DSMediaBuffer();
    if (uiFlags & 1)
    {
        ::operator delete(pBuffer);
    }
    return pBuffer;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00750f10
long __stdcall DSMediaBuffer::GetBufferAndLength(unsigned char** ppBuffer, unsigned long* pulLength)
{
    if (!ppBuffer || !pulLength)
    {
        return E_POINTER;
    }
    *ppBuffer = mData;
    *pulLength = mLength;
    return S_OK;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00750f50
long __stdcall DSMediaBuffer::GetMaxLength(unsigned long* pulMaxLength)
{
    if (!pulMaxLength)
    {
        return E_POINTER;
    }
    *pulMaxLength = mMaxLength;
    return S_OK;
}
}

namespace Snd {

// FUNCTION: SWBF2 0x00751090
long __stdcall DSMediaBuffer::SetLength(unsigned long ulLength)
{
    if (ulLength > mMaxLength)
    {
        return E_INVALIDARG;
    }
    mLength = ulLength;
    return S_OK;
}
}

namespace Snd
{

DSMediaBuffer::DSMediaBuffer(unsigned char* pData, unsigned long ulMaxLength)
    : mLength(0),
      mMaxLength(ulMaxLength),
      mRef(0),
      mData(pData),
      mDynamic(false)
{
}
}
