#pragma once

struct PblComplex
{
    float real;
    float imag;
};

namespace PblFourier
{
void __cdecl FFTShift1D(PblComplex* pData, int iSize);
void __cdecl FFTShift2D(PblComplex* pData, int iSizeX, int iSizeZ);
int __cdecl FFT1D(PblComplex* pData, int iSize);
int __cdecl InvFFT1D(PblComplex* pData, int iSize, bool bScale);
int __cdecl FFT2D(PblComplex* pData, int iSizeX, int iSizeZ);
int __cdecl InvFFT2D(PblComplex* pData, int iSizeX, int iSizeZ, bool bScale);

}
