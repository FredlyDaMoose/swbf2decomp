#include "PblFourier.h"
#include "PblMath.h"

namespace PblFourier
{
static int FastFourier1D(PblComplex* pData, int iSize, int iSign);
static int FastFourier2D(PblComplex* pData, int iSizeX, int iSizeZ, int iSign);
}

// FUNCTION: SWBF2 0x0072b3d0
int PblFourier::FastFourier1D(PblComplex* pData, int iSize, int iSign)
{
    int j = 0;
    for (int i = 0; i < iSize - 1; ++i)
    {
        if (i < j)
        {
            PblComplex temp = pData[j];
            pData[j] = pData[i];
            pData[i] = temp;
        }
        int m = iSize >> 1;
        for (; m <= j; m >>= 1)
        {
            j -= m;
        }
        j += m;
    }

    for (int mmax = 1; iSize > mmax;)
    {
        int istep = mmax * 2;
        float fTheta = PblMath::fTwoPi / (float)istep * (float)iSign;
        float fTemp = PblMath::Sin(fTheta * 0.5f);
        float fWpr = fTemp * -2.0f * fTemp;
        float fWpi = PblMath::Sin(fTheta);
        float fWr = 1.0f;
        float fWi = 0.0f;
        for (int m = 0; m < mmax; ++m)
        {
            for (int i = m; i < iSize; i += istep)
            {
                PblComplex& a = pData[i];
                PblComplex& b = pData[i + mmax];
                float fTr = b.real * fWr - b.imag * fWi;
                float fTi = b.imag * fWr + b.real * fWi;
                b.real = a.real - fTr;
                b.imag = a.imag - fTi;
                a.real += fTr;
                a.imag += fTi;
            }
            float fOldWr = fWr;
            fWr += fWr * fWpr - fWi * fWpi;
            fWi += fWi * fWpr + fOldWr * fWpi;
        }
        mmax = istep;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0072b5d0
int PblFourier::FastFourier2D(PblComplex* pData, int iSizeX, int iSizeZ, int iSign)
{
    for (int x = 0; x < iSizeX; ++x)
    {
        FastFourier1D(pData + x * iSizeZ, iSizeZ, iSign);
    }

    PblComplex* pColumn = new PblComplex[iSizeX];
    for (int z = 0; z < iSizeZ; ++z)
    {
        for (int x = 0; x < iSizeX; ++x)
        {
            pColumn[x] = pData[z + x * iSizeZ];
        }
        FastFourier1D(pColumn, iSizeX, iSign);
        for (int x = 0; x < iSizeX; ++x)
        {
            pData[z + x * iSizeZ] = pColumn[x];
        }
    }
    delete[] pColumn;
    return 0;
}

// FUNCTION: SWBF2 0x0072b6b0
int PblFourier::InvFFT2D(PblComplex* pData, int iSizeX, int iSizeZ, bool bScale)
{
    FastFourier2D(pData, iSizeX, iSizeZ, 1);
    if (bScale)
    {
        int iCount = iSizeX * iSizeZ;
        for (int x = 0; x < iSizeX; ++x)
        {
            for (int z = 0; z < iSizeZ; ++z)
            {
                pData[x * iSizeZ + z].real /= (float)iCount;
                pData[x * iSizeZ + z].imag /= (float)iCount;
            }
        }
    }
    return 0;
}
