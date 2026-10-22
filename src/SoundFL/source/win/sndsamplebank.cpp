#include "sndsamplebank.h"

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndengine.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074c430
void SampleBank::LoadData(PblFileChunk& chunk, char const* pcName, unsigned int const* puiOffsets)
{
    (void)pcName;

    const unsigned int uiSize = GetSize();
    const unsigned int uiNumSamples = mNumSamples;

    if (Engine::GetMixConfig() == Engine::mixConfig_Disabled)
    {
        return;
    }

    unsigned char* pAddress = (unsigned char*)GetBaseAddress();

    if (puiOffsets)
    {
        PblFile* const pFile = chunk._pFile;
        const int iStart = pFile->Tell();
        for (unsigned int i = 0; i < uiNumSamples; ++i)
        {
            Sample& sample = mSamples[i];
            const unsigned int uiLength = sample.mAliasSample ? 0 : sample.mLength;
            void* pData;
            if (uiLength)
            {
                Engine::GetMixConfig();
                pData = sample.mAddress;
                pFile->Seek(iStart + puiOffsets[i], PblFile::SeekFromStart);
                pFile->ReadData(pData, uiLength);
            }
            else
            {
                pData = 0;
            }
            sample.UploadData(pData, uiLength);
        }
        pFile->Seek(iStart, PblFile::SeekFromStart);
        pFile->Seek(iStart + chunk._iSize + 8, PblFile::SeekFromStart);
    }
    else
    {
        chunk._pFile->ReadData(pAddress, uiSize);
        for (unsigned int i = 0; i < uiNumSamples; ++i)
        {
            Sample& sample = mSamples[i];
            const unsigned int uiLength = sample.mAliasSample ? 0 : sample.mLength;
            sample.UploadData(pAddress, uiLength);
            pAddress += uiLength;
        }
    }
}

}
