#pragma once

#include <string.h>

#include "PebbleFL/Common/PblFile.h"

namespace RedScreenShot
{

    struct Pixel
    {
        float r;
        float g;
        float b;
    };

    struct GammaRamp
    {
        unsigned char red[256];
        unsigned char green[256];
        unsigned char blue[256];
    };

    struct SamplePoint
    {
        float x;
        float y;
    };
}

template <class T>
class Accumulator
{
public:
    Accumulator();
    ~Accumulator();

    bool Open();

    void Close()
    {
        if (m_file.IsOpen())
        {
            m_file.Close();
        }
        PblFile::Delete(m_fileName);
    }
    void Clear(int iValue, int iNumBytes);

    T Get(int iIndex)
    {
        Seek(iIndex);
        return m_buffer[m_bufferOffset];
    }
    void Set(int iIndex, const T& value)
    {
        Seek(iIndex);
        m_buffer[m_bufferOffset] = value;
        m_bufferDirty = true;
    }

protected:
    void Seek(int iIndex);

public:
    char m_fileName[50];
    T* m_buffer;
    int m_curPos;
    int m_bufferOffset;
    int m_bufferSize;
    bool m_bufferDirty;
    PblFile m_file;
};
 

namespace RedScreenShot
{
    bool StitchImage(Accumulator<RedScreenShot::Pixel>* pAccumulators, int iTilesX, int iTilesY,
                     int iSamplesPerPixel, const RedScreenShot::GammaRamp& gamma, const char* pcFileName,
                     unsigned short uiWidth, unsigned short uiHeight);
    void TakeScreenShot();
}

namespace RedScreenShot
{
    extern int size_increase;
    extern int samples_per_pixel;
    extern float gamma_correction;
    void __cdecl GPUSync();
    void __cdecl GetFrameBufferDimensions(unsigned int& uiWidth, unsigned int& uiHeight);
    bool __cdecl GetPoissonDiscSamples(SamplePoint* pSamples, int iNumSamples, const SamplePoint& start,
                                       float fMinDist, const SamplePoint& min, const SamplePoint& max);
    bool __cdecl GetScreenShotFileName(char* const pcFileName);
    void __cdecl AccumulateFrameBuffer(Accumulator<Pixel>& accum);
}
