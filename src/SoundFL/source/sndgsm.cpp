#include <emmintrin.h>
#include <string.h>

#include "sndgsm.h"

extern int __cdecl gsm_decode(const __m128i* state, unsigned char* frame, short* samples);
extern void __cdecl gsm_encode(char* pGsm, short* pFrame, unsigned char* pOut);

namespace Snd
{

void __cdecl gsm_option(void* pGsm, int iOption, unsigned char* pValue);

// FUNCTION: SWBF2 0x0074bca0
void GSM::Decode(unsigned char (*pFrame)[33], short (*pSamples)[160])
{
    ::gsm_decode((const __m128i*)this, *pFrame, *pSamples);
}

// FUNCTION: SWBF2 0x0074bcc0
void GSM::Encode(short (*pSamples)[160], unsigned char (*pFrame)[33])
{
    ::gsm_encode((char*)this, *pSamples, *pFrame);
}

// FUNCTION: SWBF2 0x0074bce0
void GSM::Init()
{
    memset(this, 0, sizeof(GSM));
    nrp = 40;

    {
        unsigned char ucValue = 1;
        gsm_option(this, 2, &ucValue);
    }
    {
        unsigned char ucValue = 1;
        gsm_option(this, 3, &ucValue);
    }
}

}
