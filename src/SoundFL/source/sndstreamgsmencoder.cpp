#include <new>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "snddsp.h"
#include "sndstreampacket.h"
#include "sndstreamgsmencoder.h"
#include "sndstreamsourceproducer.h"
#include "sndsourcetransport.h"

extern void __cdecl gsm_encode(char* pGsm, short* pFrame, unsigned char* pOut);

namespace Snd
{

void __cdecl gsm_option(void* pGsm, int iOption, unsigned char* pValue);

// FUNCTION: SWBF2 0x0074e040
void __cdecl gsm_option(void* pGsm, int iOption, unsigned char* pValue)
{
    char* const pThis = (char*)pGsm;

    if (iOption - 1 != 0)
    {
        return;
    }

    if (pValue != 0)
    {
        pThis[0x286] = (char)*pValue;
    }
}

// FUNCTION: SWBF2 0x007416c0
StreamGSMEncoder::StreamGSMEncoder()
    : mInputPacket(0),
      mInputPacketOffset(0),
      mInputPacketSize(0),
      mInputFrameOffset(0),
      mOutputPacketOffset(0),
      mFilter(0)
{
    mDecimate = false;
    mState.Init();
}

// FUNCTION: SWBF2 0x007419b0
void Snd::StreamGSMEncoder::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets, unsigned int uiNumPackets)
{
    void* pGSMEncoder = (void*)this;

    ((StreamSourceProducer*)pGSMEncoder)->SetPackets(uiPacketSize, pPackets,
                                                     uiNumPackets);
}

// FUNCTION: SWBF2 0x00741780
StreamGSMEncoder::~StreamGSMEncoder()
{
    if (mProducer[2] != 0)
    {
        Flush();
    }
}

// FUNCTION: SWBF2 0x00741870
void Snd::StreamGSMEncoder::Flush()
{

    StreamPacket* const pPacket = mInputPacket;
    if (pPacket != 0)
    {

        pPacket->Unlock();

        mInputPacket = 0;
        mInputPacketOffset = 0;
        mInputPacketSize = 0;
    }

    mInputFrameOffset = 0;
    mOutputPacketOffset = 0;

    DSP::HighLowPassFilter* const pFilter = mFilter;
    if (pFilter != 0)
    {
        pFilter->ResetState();
    }

    mState.Init();

    StreamSourceProducer::Flush();
}

// FUNCTION: SWBF2 0x007417f0
void* __fastcall sub_7417F0(void* pGSMEncoder, int, unsigned int uiFlags)
{
    ((StreamGSMEncoder*)pGSMEncoder)->StreamGSMEncoder::~StreamGSMEncoder();
    if (uiFlags & 1)
    {
        operator delete(pGSMEncoder);
    }
    return pGSMEncoder;
}

// FUNCTION: SWBF2 0x00741910
StreamPacket* StreamGSMEncoder::GetPacket(unsigned int uiOutput)
{
    SourceTransport* const pTransport = this;

    StreamPacket* pPacket;
    if (pTransport->GetCurrentPlayState() != SourceTransport::playState_playing ||
        (pPacket = StreamSourceProducer::GetPacket(uiOutput)) == 0)
    {
        Update();
        if (pTransport->GetCurrentPlayState() != SourceTransport::playState_playing)
        {
            return 0;
        }
        pPacket = StreamSourceProducer::GetPacket(uiOutput);
        if (pPacket == 0)
        {
            return 0;
        }
    }
    if (pPacket->mFlags & 1)
    {
        pTransport->Stop();
    }
    return pPacket;
}

// FUNCTION: SWBF2 0x007419c0
void StreamGSMEncoder::Update()
{
    if (StreamCodec::Update())
    {
        Flush();
    }

    if (!BufferPackets())
    {
        return;
    }

    StreamPacket* pOutput = GetReadPacket();
    while (pOutput != 0)
    {
        if (mInputPacket == 0)
        {
            mInputPacket = ConnectionGetPacket();
            if (mInputPacket == 0)
            {
                return;
            }
            mInputPacketSize = mInputPacket->mBufferUsed / 2;
            mInputPacketOffset = 0;
        }

        while (pOutput != 0)
        {
            bool bFlag1 = false;
            bool bFlag0 = false;
            bool bLast = false;
            short* const pInput = (short*)mInputPacket->mBuffer;
            unsigned char* const pOut = (unsigned char*)pOutput->mBuffer;
            unsigned int uiSamples = mInputPacketSize - mInputPacketOffset;

            if (mDecimate)
            {
                uiSamples /= 2;
                if (uiSamples != 0)
                {
                    unsigned int uiCount = uiSamples;
                    if (160 - mInputFrameOffset < uiCount)
                    {
                        uiCount = 160 - mInputFrameOffset;
                    }
                    uiCount *= 2;

                    short filtered[320];
                    mFilter->Process16(pInput + mInputPacketOffset, uiCount, filtered);
                    unsigned int i;
                    for (i = 0; i < uiCount; i += 2)
                    {
                        mInputFrame[mInputFrameOffset++] = filtered[i];
                    }
                    mInputPacketOffset += i;
                    uiSamples = 0;
                }
                else
                {
                    mInputPacketOffset = mInputPacketSize;
                }
            }

            if (mInputFrameOffset != 0)
            {
                unsigned int uiCount = 160 - mInputFrameOffset;
                if (uiSamples < uiCount)
                {
                    uiCount = uiSamples;
                }
                if (uiCount != 0)
                {
                    memcpy(&mInputFrame[mInputFrameOffset],
                           pInput + mInputPacketOffset, uiCount * 2);
                    mInputPacketOffset += uiCount;
                    mInputFrameOffset += uiCount;
                }
                if (mInputFrameOffset == 160)
                {
                    mState.Encode(&mInputFrame, (unsigned char (*)[33])(pOut + mOutputPacketOffset));
                    mOutputPacketOffset += 33;
                    mInputFrameOffset = 0;
                }
            }
            else if (uiSamples >= 160)
            {
                mState.Encode((short (*)[160])(pInput + mInputPacketOffset),
                              (unsigned char (*)[33])(pOut + mOutputPacketOffset));
                mInputPacketOffset += 160;
                mOutputPacketOffset += 33;
            }
            else
            {
                memcpy(mInputFrame, pInput + mInputPacketOffset, uiSamples * 2);
                mInputFrameOffset += uiSamples;
                mInputPacketOffset += uiSamples;
            }

            if (mInputPacketOffset == mInputPacketSize)
            {
                const unsigned short uiFlags = mInputPacket->mFlags;
                bFlag0 = (uiFlags & 1) != 0;
                bFlag1 = (uiFlags & 2) != 0;
                bLast = bFlag0 || bFlag1;

                if (bLast && mInputFrameOffset != 0 && mOutputPacketOffset < mProducer[2])
                {
                    memset(&mInputFrame[mInputFrameOffset], 0,
                           (160 - mInputFrameOffset) * 2);
                    mState.Encode(&mInputFrame, (unsigned char (*)[33])(pOut + mOutputPacketOffset));
                    mOutputPacketOffset += 33;
                    mInputFrameOffset = 0;
                    mState.Init();
                    if (mFilter != 0)
                    {
                        mFilter->ResetState();
                    }
                }

                mInputPacket->Unlock();
                mInputPacket = 0;
                mInputPacketOffset = 0;
                mInputPacketSize = 0;
            }

            if (mOutputPacketOffset == mProducer[2] || bLast)
            {
                const unsigned int uiPos = mPlaybackPosition;
                const unsigned int uiDecoded = (mOutputPacketOffset / 33) * 160;
                PacketComplete(pOutput, mOutputPacketOffset, uiDecoded,
                               bFlag0, bFlag1, uiPos);
                mOutputPacketOffset = 0;
                mPlaybackPosition = uiPos + uiDecoded;
                pOutput = 0;
            }

            if (mInputPacket == 0)
            {
                break;
            }
        }

        pOutput = GetReadPacket();
    }
}

}

// FUNCTION: SWBF2 0x007500b0
int __cdecl gsm_asr(short value, int shift)
{
    if (shift >= 16)
    {
        return value < 0 ? -1 : 0;
    }
    if (shift <= -16)
    {
        return 0;
    }
    if (shift >= 0)
    {
        return value >> shift;
    }
    return (int)((unsigned int)(int)value << -shift);
}

// FUNCTION: SWBF2 0x00750540
short __cdecl gsm_mult(short a, short b)
{
    if (a == (short)0x8000 && b == (short)0x8000)
    {
        return 0x7fff;
    }
    return (short)(((int)a * b) >> 15);
}

// FUNCTION: SWBF2 0x00750590
int __cdecl gsm_norm(int value)
{
    static const unsigned char bitoff[256] = {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    if (!value)
    {
        _wassert(L"a != 0", L"source\\gsm-1.0.10\\gsm_encode.c", 0x66c);
    }
    if (value < 0)
    {
        if (value <= -1073741824)
        {
            return 0;
        }
        value = ~value;
    }
    const unsigned int magnitude = (unsigned int)value;
    if (magnitude & 0xffff0000u)
    {
        return magnitude & 0xff000000u ? bitoff[magnitude >> 24] - 1
                                      : bitoff[(magnitude >> 16) & 0xff] + 7;
    }
    return magnitude & 0xff00u ? bitoff[(magnitude >> 8) & 0xff] + 15
                               : bitoff[magnitude & 0xff] + 23;
}

// FUNCTION: SWBF2 0x00750000
int __cdecl Weighting_filter(const short* input, short* output)
{
    int result = 0;
    for (int i = 0; i < 40; ++i)
    {
        const short* p = input + i - 4;
        const int sum = 5741 * (p[3] + p[5]) + 2054 * (p[2] + p[6])
                      - 134 * (p[-1] + p[9]) - 374 * (p[0] + p[8])
                      + (p[4] << 13) + 4096;
        result = sum >> 13;
        if (result > 32767)
        {
            result = 32767;
        }
        else if (result < -32768)
        {
            result = -32768;
        }
        output[i] = (short)result;
    }
    return result;
}

// FUNCTION: SWBF2 0x0074f490
void __cdecl RPE_grid_positioning(unsigned short grid, const short* input, short* output)
{
    if (grid > 3)
    {
        _wassert(L"0 <= Mc && Mc <= 3", L"source\\gsm-1.0.10\\gsm_encode.c", 0x517);
    }
    short* p = output;
    int remaining = 13;
    switch (grid)
    {
    case 0: goto sample;
    case 1: goto zero1;
    case 2: goto zero2;
    case 3: *p++ = 0; break;
    default: return;
    }
    do
    {
zero2:  *p++ = 0;
zero1:  *p++ = 0;
sample: *p++ = *input++;
    } while (--remaining);
    for (int i = 0; i < 3 - grid; ++i)
    {
        *p++ = 0;
    }
}

static short gsm_saturate(int value)
{
    if (value > 32767)
    {
        return 32767;
    }
    if (value < -32768)
    {
        return -32768;
    }
    return (short)value;
}

// FUNCTION: SWBF2 0x0074e510
unsigned short* __cdecl APCM_quantization_xmaxc_to_exp_mant(short value, short* exponent, unsigned short* mantissa)
{
    short e = value > 15 ? (short)((value >> 3) - 1) : 0;
    short m = (short)(value - 8 * e);
    if (m)
    {
        while (m <= 7)
        {
            m = (short)(2 * m | 1);
            --e;
        }
        const unsigned short result = (unsigned short)(m - 8);
        if (e < -4 || e > 6)
        {
            _wassert(L"exp >= -4 && exp <= 6", L"source\\gsm-1.0.10\\gsm_encode.c", 0x47e);
        }
        if (result > 7)
        {
            _wassert(L"mant >= 0 && mant <= 7", L"source\\gsm-1.0.10\\gsm_encode.c", 0x47f);
        }
        *exponent = e;
        *mantissa = result;
    }
    else
    {
        *exponent = -4;
        *mantissa = 7;
    }
    return mantissa;
}

// FUNCTION: SWBF2 0x00751c10
int __cdecl Coefficients_0_12(const short* first, const short* second, short* output)
{
    int last = 0;
    for (int i = 0; i < 8; ++i)
    {
        const short quarter = gsm_saturate((first[i] >> 2) + (second[i] >> 2));
        last = gsm_saturate((first[i] >> 1) + quarter);
        output[i] = (short)last;
    }
    return last;
}

// FUNCTION: SWBF2 0x00751ce0
int __cdecl Coefficients_13_26(const short* first, const short* second, short* output)
{
    int last = 0;
    for (int i = 0; i < 8; ++i)
    {
        last = gsm_saturate((first[i] >> 1) + (second[i] >> 1));
        output[i] = (short)last;
    }
    return last;
}

// FUNCTION: SWBF2 0x00751d60
int __cdecl Coefficients_27_39(const short* first, const short* second, short* output)
{
    int last = 0;
    for (int i = 0; i < 8; ++i)
    {
        const short quarter = gsm_saturate((first[i] >> 2) + (second[i] >> 2));
        last = gsm_saturate((second[i] >> 1) + quarter);
        output[i] = (short)last;
    }
    return last;
}

// FUNCTION: SWBF2 0x0074fee0
void __cdecl Transformation_to_Log_Area_Ratios(short* ratios)
{
    for (int i = 0; i < 8; ++i)
    {
        const short input = ratios[i];
        short magnitude = input == (short)0x8000 ? 32767 : (input < 0 ? (short)-input : input);
        if (magnitude < 0)
        {
            _wassert(L"temp >= 0", L"source\\gsm-1.0.10\\gsm_encode.c", 0x337);
        }
        if (magnitude < 22118)
        {
            magnitude = (short)(magnitude >> 1);
        }
        else if (magnitude < 31130)
        {
            if (magnitude < 11059)
            {
                _wassert(L"temp >= 11059", L"source\\gsm-1.0.10\\gsm_encode.c", 0x33c);
            }
            magnitude = (short)(magnitude - 11059);
        }
        else
        {
            magnitude = (short)((magnitude - 26112) << 2);
        }
        ratios[i] = input < 0 ? (short)-magnitude : magnitude;
        if (ratios[i] == (short)0x8000)
        {
            _wassert(L"*r != MIN_WORD", L"source\\gsm-1.0.10\\gsm_encode.c", 0x345);
        }
    }
}

// FUNCTION: SWBF2 0x007524e0
int __cdecl LARp_to_rp(short* ratios)
{
    int result = 0;
    for (int i = 0; i < 8; ++i)
    {
        const short input = ratios[i];
        const int magnitude = input == (short)0x8000 ? 32767 : (input < 0 ? -input : input);
        const int mapped = magnitude < 11059 ? magnitude << 1
                         : magnitude < 20070 ? magnitude + 11059
                         : gsm_saturate((magnitude >> 2) + 26112);
        result = input < 0 ? -mapped : mapped;
        ratios[i] = (short)result;
    }
    return result;
}

static const short kGsmFac[8] = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };
static const short kGsmNrfac[8] = { 29128, 26215, 23832, 21846, 20165, 18725, 17476, 16384 };

// FUNCTION: SWBF2 0x0074e070
int __cdecl APCM_inverse_quantization(const short* coded, unsigned short mantissa,
                        short exponent, short* output)
{
    if (mantissa > 7)
    {
        _wassert(L"mant >= 0 && mant <= 7", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4f1);
    }
    const short factor = kGsmFac[mantissa];
    const short shift = gsm_saturate(6 - exponent);
    const short rounding = (short)gsm_asr(1, (short)(1 - shift));
    int result = 0;
    for (int i = 0; i < 13; ++i)
    {
        const short signedSample = (short)(2 * coded[i] - 7);
        if ((unsigned short)coded[i] > 7)
        {
            _wassert(L"*xMc >= 0 && *xMc <= 7", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4f9);
        }
        if (signedSample < -7 || signedSample > 7)
        {
            _wassert(L"temp <= 7 && temp >= -7", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4fd);
        }
        const short scaled = (short)(signedSample << 12);
        const short product = (short)(((int)factor * scaled + 16384) >> 15);
        const short sum = gsm_saturate(product + rounding);
        result = gsm_asr(sum, shift);
        output[i] = (short)result;
    }
    return result;
}

// FUNCTION: SWBF2 0x0074e240
short* __cdecl APCM_quantization(const short* input, short* coded,
                           short* mantissa, short* exponent, short* xmaxc)
{
    short maximum = 0;
    for (int i = 0; i < 13; ++i)
    {
        const short magnitude = input[i] == (short)0x8000 ? 32767
                              : input[i] < 0 ? (short)-input[i] : input[i];
        if (magnitude > maximum)
        {
            maximum = magnitude;
        }
    }
    short exp = 0;
    short test = 0;
    short temp = (short)(maximum >> 9);
    for (int i = 0; i < 6; ++i)
    {
        test |= temp <= 0;
        temp = (short)(temp >> 1);
        if (exp > 5)
        {
            _wassert(L"exp <= 5", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4aa);
        }
        if (!test)
        {
            ++exp;
        }
    }
    if (exp < 0 || exp > 6)
    {
        _wassert(L"exp >= 0 && exp <= 6", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4ae);
    }
    if (exp + 5 < 0 || exp + 5 > 11)
    {
        _wassert(L"temp <= 11 && temp >= 0", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4b1);
    }
    const short encodedMaximum = gsm_saturate((maximum >> (exp + 5)) + (exp << 3));
    unsigned short mant;
    APCM_quantization_xmaxc_to_exp_mant(encodedMaximum, &exp, &mant);
    if (exp < -4096 || exp > 4096)
    {
        _wassert(L"exp >= -4096 && exp <= 4096", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4c8);
    }
    if (mant > 7)
    {
        _wassert(L"mant >= 0 && mant <= 7", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4c9);
    }
    const short inverse = kGsmNrfac[mant];
    const short shift = (short)(6 - exp);
    for (int i = 0; i < 13; ++i)
    {
        if ((unsigned short)shift >= 16)
        {
            _wassert(L"temp1 >= 0 && temp1 < 16", L"source\\gsm-1.0.10\\gsm_encode.c", 0x4d0);
        }
        const short scaled = (short)((unsigned short)input[i] << shift);
        const short product = (short)(((int)scaled * inverse) >> 15);
        coded[i] = (short)((product >> 12) + 4);
    }
    *mantissa = (short)mant;
    *exponent = exp;
    *xmaxc = encodedMaximum;
    return xmaxc;
}

static int gsm_saturate_long(__int64 value)
{
    if (value > 0x7fffffffLL)
    {
        return 0x7fffffff;
    }
    if (value < (-0x7fffffffLL - 1))
    {
        return (int)0x80000000u;
    }
    return (int)value;
}

static short gsm_multiply_round(short a, short b)
{
    if (a == (short)0x8000 && b == (short)0x8000)
    {
        return 0x7fff;
    }
    return (short)(((int)a * b + 16384) >> 15);
}

struct gsm_state
{
    short dp0[280];
    short z1;
    int L_z2;
    int mp;
    short u[8];
    short LARpp[2][8];
    short j;
    short ltp_cut;
    short nrp;
    short v[9];
    short msr;
    char verbose, fast, wav_fmt;
    unsigned char frame_index, frame_chain;
};
 
 
 
 
 
 
 

// FUNCTION: SWBF2 0x0074ec60
int __cdecl Gsm_Preprocess(void* state, const short* input, short* output)
{
    gsm_state* p = (gsm_state*)state;
    short previous = p->z1;
    int accumulator = p->L_z2;
    short preemphasis = (short)p->mp;
    for (int i = 0; i < 160; ++i)
    {
        const short scaledInput = (short)((input[i] >> 3) << 2);
        if (scaledInput < -0x4000)
        {
            _wassert(L"SO >= -0x4000", L"source\\gsm-1.0.10\\gsm_encode.c", 0x5b1);
        }
        if (scaledInput > 0x3ffc)
        {
            _wassert(L"SO <= 0x3FFC", L"source\\gsm-1.0.10\\gsm_encode.c", 0x5b2);
        }
        const short difference = (short)(scaledInput - previous);
        if (difference == (short)0x8000)
        {
            _wassert(L"s1 != MIN_WORD", L"source\\gsm-1.0.10\\gsm_encode.c", 0x5c2);
        }
        previous = scaledInput;
        const short high = (short)(accumulator >> 15);
        const short low = (short)(accumulator - ((int)high << 15));
        const int recursive = gsm_multiply_round(low, 32735);
        accumulator = gsm_saturate_long((__int64)((int)difference << 15)
                                         + recursive + (int)high * 32735);
        const int rounded = gsm_saturate_long((__int64)accumulator + 16384);
        const short feedback = gsm_multiply_round(preemphasis, -28180);
        preemphasis = (short)(rounded >> 15);
        output[i] = gsm_saturate(preemphasis + feedback);
    }
    p->z1 = previous;
    p->L_z2 = accumulator;
    p->mp = preemphasis;
    return preemphasis;
}

// FUNCTION: SWBF2 0x007519d0
int __cdecl Long_term_analysis_filtering(short gain, short lag, const short* history,
                        const short* input, short* predicted, short* residual)
{
    static const short kGains[4] = { 3277, 11469, 21299, 32767 };
    if (gain < 0 || gain > 3)
    {
        return gain;
    }
    int result = gain;
    for (int i = 0; i < 40; ++i)
    {
        const short estimate = gsm_multiply_round(kGains[gain], history[i - lag]);
        predicted[i] = estimate;
        result = gsm_saturate(input[i] - estimate);
        residual[i] = (short)result;
    }
    return result;
}

extern void __cdecl Calculation_of_the_LTP_parameters(const short*, const short*, short*, short*);

// FUNCTION: SWBF2 0x007517c0
int __cdecl Gsm_Long_Term_Predictor(void*, const short* input, const short* history,
                        short* residual, short* predicted, short* lag, short* gain)
{
    const wchar_t* file = L"source\\gsm-1.0.10\\long_term.c";
    if (!input)
    {
        _wassert(L"d", file, 0x370);
    }
    if (!history)
    {
        _wassert(L"dp", file, 0x370);
    }
    if (!residual)
    {
        _wassert(L"e", file, 0x370);
    }
    if (!predicted)
    {
        _wassert(L"dpp", file, 0x371);
    }
    if (!lag)
    {
        _wassert(L"Nc", file, 0x371);
    }
    if (!gain)
    {
        _wassert(L"bc", file, 0x371);
    }
    Calculation_of_the_LTP_parameters(input, history, gain, lag);
    return Long_term_analysis_filtering(*gain, *lag, history, input, predicted, residual);
}

// FUNCTION: SWBF2 0x007525d0
void __cdecl Short_term_analysis_filtering(void* state, const short* coefficients,
                         int count, short* samples)
{
    short* memory = ((gsm_state*)state)->u;
    for (int sample = 0; sample < count; ++sample)
    {
        short di = samples[sample];
        short saved = di;
        for (int i = 0; i < 8; ++i)
        {
            const short ui = memory[i];
            const short coefficient = coefficients[i];
            memory[i] = saved;
            saved = gsm_saturate(ui + gsm_multiply_round(coefficient, di));
            di = gsm_saturate(di + gsm_multiply_round(coefficient, ui));
        }
        samples[sample] = di;
    }
}

extern void __cdecl Decoding_of_the_coded_Log_Area_Ratios(const short*, short*);

// FUNCTION: SWBF2 0x007522c0
void __cdecl Gsm_Short_Term_Analysis_Filter(void* state, const short* codedRatios, short* samples)
{
    gsm_state* p = (gsm_state*)state;
    const short oldIndex = p->j;
    short* current = p->LARpp[oldIndex];
    const short newIndex = (short)(oldIndex ^ 1);
    p->j = newIndex;
    short* previous = p->LARpp[newIndex];
    short ratios[8];
    Decoding_of_the_coded_Log_Area_Ratios(codedRatios, current);
    Coefficients_0_12(previous, current, ratios);
    LARp_to_rp(ratios);
    Short_term_analysis_filtering(state, ratios, 13, samples);
    Coefficients_13_26(previous, current, ratios);
    LARp_to_rp(ratios);
    Short_term_analysis_filtering(state, ratios, 14, samples + 13);
    Coefficients_27_39(previous, current, ratios);
    LARp_to_rp(ratios);
    Short_term_analysis_filtering(state, ratios, 13, samples + 27);
    memcpy(ratios, current, sizeof(ratios));
    LARp_to_rp(ratios);
    Short_term_analysis_filtering(state, ratios, 120, samples + 40);
}

extern void __cdecl Autocorrelation(short*, int*);
extern void __cdecl Reflection_coefficients(const int*, short*);
extern void __cdecl Quantization_and_coding(short*);
extern void __cdecl RPE_grid_selection(const short*, short*, short*);

// FUNCTION: SWBF2 0x0074ea10
void __cdecl Gsm_Coder(void* state, const short* input, short* codedRatios,
                        short* lag, short* gain, short* grid,
                        short* maximum, short* codedSamples)
{
    // GLOBAL: SWBF2 0x01e2f838
    static short e[50];
    short samples[160];
    int correlations[9];
    short weighted[40];
    short selected[13];
    short reconstructed[13];
    short mantissa, exponent;
    gsm_state* p = (gsm_state*)state;
    short* history = p->dp0 + 120;
    short* prediction = history;
    short* residual = e + 5;

    Gsm_Preprocess(state, input, samples);
    Autocorrelation(samples, correlations);
    Reflection_coefficients(correlations, codedRatios);
    Transformation_to_Log_Area_Ratios(codedRatios);
    Quantization_and_coding(codedRatios);
    Gsm_Short_Term_Analysis_Filter(state, codedRatios, samples);

    for (int block = 0; block < 4; ++block)
    {
        Gsm_Long_Term_Predictor(state, samples + block * 40, history,
                    residual, prediction, lag + block, gain + block);
        Weighting_filter(residual, weighted);
        RPE_grid_selection(weighted, selected, grid + block);
        APCM_quantization(selected, codedSamples + block * 13,
                    &mantissa, &exponent, maximum + block);
        APCM_inverse_quantization(codedSamples + block * 13, (unsigned short)mantissa,
                    exponent, reconstructed);
        RPE_grid_positioning((unsigned short)grid[block], reconstructed, residual);
        for (int i = 0; i < 40; ++i)
        {
            history[i] = gsm_saturate(residual[i] + prediction[i]);
        }
        history += 40;
        prediction += 40;
    }
    memcpy(p->dp0, p->dp0 + 160, 120 * sizeof(short));
}

typedef short word;
typedef long longword;
typedef unsigned long ulongword;

#define MIN_WORD (-32767 - 1)
#define MAX_WORD 32767
#define SASR(x, by) ((x) >= 0 ? (x) >> (by) : (~(-((x) + 1) >> (by))))
#define GSM_MULT_R(a, b) (SASR(((longword)(a) * (longword)(b) + 16384), 15))
#define GSM_MULT(a, b) (SASR(((longword)(a) * (longword)(b)), 15))
#define GSM_ADD(a, b) \
    ((ulongword)((ltmp = (longword)(a) + (longword)(b)) - MIN_WORD) > \
        MAX_WORD - MIN_WORD ? (ltmp > 0 ? MAX_WORD : MIN_WORD) : ltmp)
#define GSM_SUB(a, b) \
    ((ltmp = (longword)(a) - (longword)(b)) >= MAX_WORD \
    ? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)
#define GSM_ABS(a) ((a) < 0 ? ((a) == MIN_WORD ? MAX_WORD : -(a)) : (a))
#define GSM_MAGIC 0xD

#define GSM_ASSERT(e, file, line) \
    (void)((!!(e)) || (_wassert(_CRT_WIDE(#e), file, line), 0))
#define GSM_ENCODE_C L"source\\gsm-1.0.10\\gsm_encode.c"
#define LONG_TERM_C L"source\\gsm-1.0.10\\long_term.c"

// GLOBAL: SWBF2 0x007e3c70
static const word gsm_DLB[4] = { 6554, 16384, 26214, 32767 };

static word gsm_div(word num, word denum)
{
    longword L_num = num;
    longword L_denum = denum;
    word div = 0;
    int k = 15;

    GSM_ASSERT(num >= 0 && denum >= num, GSM_ENCODE_C, 0x6bf);
    if (num == 0)
    {
        return 0;
    }

    while (k--)
    {
        div <<= 1;
        L_num <<= 1;
        if (L_num >= L_denum)
        {
            L_num -= L_denum;
            div++;
        }
    }
    return div;
}

// FUNCTION: SWBF2 0x0074e5d0
void __cdecl Autocorrelation(short* s, int* L_ACF)
{
    int k, i;
    word temp, smax, scalauto;

    smax = 0;
    for (k = 0; k <= 159; k++)
    {
        temp = GSM_ABS(s[k]);
        if (temp > smax)
        {
            smax = temp;
        }
    }

    if (smax == 0)
    {
        scalauto = 0;
    }
    else
    {
        GSM_ASSERT(smax > 0, GSM_ENCODE_C, 0x265);
        scalauto = (word)(4 - gsm_norm((longword)smax << 16));
    }

    if (scalauto > 0)
    {
#define SCALE(n) \
        case n: for (k = 0; k <= 159; k++) \
                    s[k] = (word)GSM_MULT_R(s[k], 16384 >> (n - 1)); \
            break;

        switch (scalauto)
        {
        SCALE(1)
        SCALE(2)
        SCALE(3)
        SCALE(4)
        }
#undef SCALE
    }

    {
        word* sp = s;
        word sl = *sp;

#define STEP(k) L_ACF[k] += ((longword)sl * sp[-(k)]);
#define NEXTI sl = *++sp

        for (k = 9; k--; L_ACF[k] = 0);

        STEP(0);
        NEXTI;
        STEP(0); STEP(1);
        NEXTI;
        STEP(0); STEP(1); STEP(2);
        NEXTI;
        STEP(0); STEP(1); STEP(2); STEP(3);
        NEXTI;
        STEP(0); STEP(1); STEP(2); STEP(3); STEP(4);
        NEXTI;
        STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5);
        NEXTI;
        STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6);
        NEXTI;
        STEP(0); STEP(1); STEP(2); STEP(3); STEP(4); STEP(5); STEP(6); STEP(7);

        for (i = 8; i <= 159; i++)
        {
            NEXTI;

            STEP(0);
            STEP(1); STEP(2); STEP(3); STEP(4);
            STEP(5); STEP(6); STEP(7); STEP(8);
        }

        for (k = 9; k--; L_ACF[k] <<= 1);
#undef STEP
#undef NEXTI
    }

    if (scalauto > 0)
    {
        GSM_ASSERT(scalauto <= 4, GSM_ENCODE_C, 0x2ba);
        for (k = 160; k--; *s++ <<= scalauto);
    }
}

// FUNCTION: SWBF2 0x0074fac0
void __cdecl Reflection_coefficients(const int* L_ACF, short* r)
{
    int i, m, n;
    word temp;
    longword ltmp;
    word ACF[9];
    word P[9];
    word K[9];

    if (L_ACF[0] == 0)
    {
        for (i = 8; i--; *r++ = 0);
        return;
    }

    GSM_ASSERT(L_ACF[0] != 0, GSM_ENCODE_C, 0x2f1);
    temp = (word)gsm_norm(L_ACF[0]);

    GSM_ASSERT(temp >= 0 && temp < 32, GSM_ENCODE_C, 0x2f4);

    for (i = 0; i <= 8; i++)
    {
        ACF[i] = (word)SASR(L_ACF[i] << temp, 16);
    }

    for (i = 1; i <= 7; i++)
    {
        K[i] = ACF[i];
    }
    for (i = 0; i <= 8; i++)
    {
        P[i] = ACF[i];
    }

    for (n = 1; n <= 8; n++, r++)
    {
        temp = P[1];
        temp = GSM_ABS(temp);
        if (P[0] < temp)
        {
            for (i = n; i <= 8; i++)
            {
                *r++ = 0;
            }
            return;
        }

        *r = gsm_div(temp, P[0]);

        GSM_ASSERT(*r >= 0, GSM_ENCODE_C, 0x30c);
        if (P[1] > 0)
        {
            *r = -*r;
        }
        GSM_ASSERT(*r != MIN_WORD, GSM_ENCODE_C, 0x30e);
        if (n == 8)
        {
            return;
        }

        temp = (word)GSM_MULT_R(P[1], *r);
        P[0] = (word)GSM_ADD(P[0], temp);

        for (m = 1; m <= 8 - n; m++)
        {
            temp = (word)GSM_MULT_R(K[m], *r);
            P[m] = (word)GSM_ADD(P[m + 1], temp);

            temp = (word)GSM_MULT_R(P[m + 1], *r);
            K[m] = (word)GSM_ADD(K[m], temp);
        }
    }
}

// FUNCTION: SWBF2 0x0074ef00
void __cdecl Quantization_and_coding(short* LAR)
{
    word temp;
    longword ltmp;

#define STEP(A, B, MAC, MIC) \
        temp = (word)GSM_MULT(A, *LAR); \
        temp = (word)GSM_ADD(temp, B); \
        temp = (word)GSM_ADD(temp, 256); \
        temp = (word)SASR(temp, 9); \
        *LAR = temp > MAC ? MAC - MIC : (temp < MIC ? 0 : temp - MIC); \
        LAR++;

    STEP(20480, 0, 31, -32);
    STEP(20480, 0, 31, -32);
    STEP(20480, 2048, 15, -16);
    STEP(20480, -2560, 15, -16);

    STEP(13964, 94, 7, -8);
    STEP(15360, -1792, 7, -8);
    STEP(8534, -341, 3, -4);
    STEP(9036, -1144, 3, -4);

#undef STEP
}

// FUNCTION: SWBF2 0x0074f550
void __cdecl RPE_grid_selection(const short* x, short* xM, short* Mc_out)
{
    int i;
    longword L_result, L_temp;
    longword EM;
    word Mc;
    longword L_common_0_3;

    EM = 0;
    Mc = 0;

#define STEP(m, i) L_temp = SASR(x[m + 3 * i], 2); \
                   L_result += L_temp * L_temp;

    L_result = 0;
    STEP(0, 1); STEP(0, 2); STEP(0, 3); STEP(0, 4);
    STEP(0, 5); STEP(0, 6); STEP(0, 7); STEP(0, 8);
    STEP(0, 9); STEP(0, 10); STEP(0, 11); STEP(0, 12);
    L_common_0_3 = L_result;

    STEP(0, 0);
    L_result <<= 1;
    EM = L_result;

    L_result = 0;
    STEP(1, 0);
    STEP(1, 1); STEP(1, 2); STEP(1, 3); STEP(1, 4);
    STEP(1, 5); STEP(1, 6); STEP(1, 7); STEP(1, 8);
    STEP(1, 9); STEP(1, 10); STEP(1, 11); STEP(1, 12);
    L_result <<= 1;
    if (L_result > EM)
    {
        Mc = 1;
        EM = L_result;
    }

    L_result = 0;
    STEP(2, 0);
    STEP(2, 1); STEP(2, 2); STEP(2, 3); STEP(2, 4);
    STEP(2, 5); STEP(2, 6); STEP(2, 7); STEP(2, 8);
    STEP(2, 9); STEP(2, 10); STEP(2, 11); STEP(2, 12);
    L_result <<= 1;
    if (L_result > EM)
    {
        Mc = 2;
        EM = L_result;
    }

    L_result = L_common_0_3;
    STEP(3, 12);
    L_result <<= 1;
    if (L_result > EM)
    {
        Mc = 3;
        EM = L_result;
    }
#undef STEP

    for (i = 0; i <= 12; i++)
    {
        xM[i] = x[Mc + 3 * i];
    }
    *Mc_out = Mc;
}

// FUNCTION: SWBF2 0x007511b0
void __cdecl Calculation_of_the_LTP_parameters(const short* d, const short* dp, short* bc_out, short* Nc_out)
{
    int k, lambda;
    word Nc, bc;
    word wt[40];

    longword L_max, L_power;
    word R, S, dmax, scal;
    word temp;

    dmax = 0;
    for (k = 0; k <= 39; k++)
    {
        temp = d[k];
        temp = GSM_ABS(temp);
        if (temp > dmax)
        {
            dmax = temp;
        }
    }

    temp = 0;
    if (dmax == 0)
    {
        scal = 0;
    }
    else
    {
        GSM_ASSERT(dmax > 0, LONG_TERM_C, 0xb8);
        temp = (word)gsm_norm((longword)dmax << 16);
    }

    if (temp > 6)
    {
        scal = 0;
    }
    else
    {
        scal = (word)(6 - temp);
    }

    GSM_ASSERT(scal >= 0, LONG_TERM_C, 0xbf);

    for (k = 0; k <= 39; k++)
    {
        wt[k] = (word)SASR(d[k], scal);
    }

    L_max = 0;
    Nc = 40;

    for (lambda = 40; lambda <= 120; lambda++)
    {
#define STEP(k) (longword)wt[k] * dp[k - lambda]
        longword L_result;

        L_result  = STEP(0);  L_result += STEP(1);
        L_result += STEP(2);  L_result += STEP(3);
        L_result += STEP(4);  L_result += STEP(5);
        L_result += STEP(6);  L_result += STEP(7);
        L_result += STEP(8);  L_result += STEP(9);
        L_result += STEP(10); L_result += STEP(11);
        L_result += STEP(12); L_result += STEP(13);
        L_result += STEP(14); L_result += STEP(15);
        L_result += STEP(16); L_result += STEP(17);
        L_result += STEP(18); L_result += STEP(19);
        L_result += STEP(20); L_result += STEP(21);
        L_result += STEP(22); L_result += STEP(23);
        L_result += STEP(24); L_result += STEP(25);
        L_result += STEP(26); L_result += STEP(27);
        L_result += STEP(28); L_result += STEP(29);
        L_result += STEP(30); L_result += STEP(31);
        L_result += STEP(32); L_result += STEP(33);
        L_result += STEP(34); L_result += STEP(35);
        L_result += STEP(36); L_result += STEP(37);
        L_result += STEP(38); L_result += STEP(39);
#undef STEP

        if (L_result > L_max)
        {
            Nc = (word)lambda;
            L_max = L_result;
        }
    }

    *Nc_out = Nc;

    L_max <<= 1;

    GSM_ASSERT(scal <= 100 && scal >= -100, LONG_TERM_C, 0xf4);
    L_max = L_max >> (6 - scal);

    GSM_ASSERT(Nc <= 120 && Nc >= 40, LONG_TERM_C, 0xf7);

    L_power = 0;
    for (k = 0; k <= 39; k++)
    {
        longword L_temp;

        L_temp = SASR(dp[k - Nc], 3);
        L_power += L_temp * L_temp;
    }
    L_power <<= 1;

    if (L_max <= 0)
    {
        *bc_out = 0;
        return;
    }
    if (L_max >= L_power)
    {
        *bc_out = 3;
        return;
    }

    temp = (word)gsm_norm(L_power);

    R = (word)SASR(L_max << temp, 16);
    S = (word)SASR(L_power << temp, 16);

    for (bc = 0; bc <= 2; bc++)
    {
        if (R <= gsm_mult(S, gsm_DLB[bc]))
        {
            break;
        }
    }
    *bc_out = bc;
}

// FUNCTION: SWBF2 0x00751e30
void __cdecl Decoding_of_the_coded_Log_Area_Ratios(const short* LARc, short* LARpp)
{
    word temp1;
    longword ltmp;

#define STEP(B, MIC, INVA) \
        temp1 = (word)(GSM_ADD(*LARc++, MIC) << 10); \
        temp1 = (word)GSM_SUB(temp1, B << 1); \
        temp1 = (word)GSM_MULT_R(INVA, temp1); \
        *LARpp++ = (word)GSM_ADD(temp1, temp1);

    STEP(0, -32, 13107);
    STEP(0, -32, 13107);
    STEP(2048, -16, 13107);
    STEP(-2560, -16, 13107);

    STEP(94, -8, 19223);
    STEP(-1792, -8, 17476);
    STEP(-341, -4, 31454);
    STEP(-1144, -4, 29708);

#undef STEP
}

// FUNCTION: SWBF2 0x007500f0
void __cdecl gsm_encode(char* pGsm, short* pFrame, unsigned char* pOut)
{
    word LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13 * 4];
    unsigned char* c = pOut;

    Gsm_Coder(pGsm, pFrame, LARc, Nc, bc, Mc, xmaxc, xmc);

    *c++ =   ((GSM_MAGIC & 0xF) << 4)
           | ((LARc[0] >> 2) & 0xF);
    *c++ =   ((LARc[0] & 0x3) << 6)
           | (LARc[1] & 0x3F);
    *c++ =   ((LARc[2] & 0x1F) << 3)
           | ((LARc[3] >> 2) & 0x7);
    *c++ =   ((LARc[3] & 0x3) << 6)
           | ((LARc[4] & 0xF) << 2)
           | ((LARc[5] >> 2) & 0x3);
    *c++ =   ((LARc[5] & 0x3) << 6)
           | ((LARc[6] & 0x7) << 3)
           | (LARc[7] & 0x7);
    *c++ =   ((Nc[0] & 0x7F) << 1)
           | ((bc[0] >> 1) & 0x1);
    *c++ =   ((bc[0] & 0x1) << 7)
           | ((Mc[0] & 0x3) << 5)
           | ((xmaxc[0] >> 1) & 0x1F);
    *c++ =   ((xmaxc[0] & 0x1) << 7)
           | ((xmc[0] & 0x7) << 4)
           | ((xmc[1] & 0x7) << 1)
           | ((xmc[2] >> 2) & 0x1);
    *c++ =   ((xmc[2] & 0x3) << 6)
           | ((xmc[3] & 0x7) << 3)
           | (xmc[4] & 0x7);
    *c++ =   ((xmc[5] & 0x7) << 5)
           | ((xmc[6] & 0x7) << 2)
           | ((xmc[7] >> 1) & 0x3);
    *c++ =   ((xmc[7] & 0x1) << 7)
           | ((xmc[8] & 0x7) << 4)
           | ((xmc[9] & 0x7) << 1)
           | ((xmc[10] >> 2) & 0x1);
    *c++ =   ((xmc[10] & 0x3) << 6)
           | ((xmc[11] & 0x7) << 3)
           | (xmc[12] & 0x7);
    *c++ =   ((Nc[1] & 0x7F) << 1)
           | ((bc[1] >> 1) & 0x1);
    *c++ =   ((bc[1] & 0x1) << 7)
           | ((Mc[1] & 0x3) << 5)
           | ((xmaxc[1] >> 1) & 0x1F);
    *c++ =   ((xmaxc[1] & 0x1) << 7)
           | ((xmc[13] & 0x7) << 4)
           | ((xmc[14] & 0x7) << 1)
           | ((xmc[15] >> 2) & 0x1);
    *c++ =   ((xmc[15] & 0x3) << 6)
           | ((xmc[16] & 0x7) << 3)
           | (xmc[17] & 0x7);
    *c++ =   ((xmc[18] & 0x7) << 5)
           | ((xmc[19] & 0x7) << 2)
           | ((xmc[20] >> 1) & 0x3);
    *c++ =   ((xmc[20] & 0x1) << 7)
           | ((xmc[21] & 0x7) << 4)
           | ((xmc[22] & 0x7) << 1)
           | ((xmc[23] >> 2) & 0x1);
    *c++ =   ((xmc[23] & 0x3) << 6)
           | ((xmc[24] & 0x7) << 3)
           | (xmc[25] & 0x7);
    *c++ =   ((Nc[2] & 0x7F) << 1)
           | ((bc[2] >> 1) & 0x1);
    *c++ =   ((bc[2] & 0x1) << 7)
           | ((Mc[2] & 0x3) << 5)
           | ((xmaxc[2] >> 1) & 0x1F);
    *c++ =   ((xmaxc[2] & 0x1) << 7)
           | ((xmc[26] & 0x7) << 4)
           | ((xmc[27] & 0x7) << 1)
           | ((xmc[28] >> 2) & 0x1);
    *c++ =   ((xmc[28] & 0x3) << 6)
           | ((xmc[29] & 0x7) << 3)
           | (xmc[30] & 0x7);
    *c++ =   ((xmc[31] & 0x7) << 5)
           | ((xmc[32] & 0x7) << 2)
           | ((xmc[33] >> 1) & 0x3);
    *c++ =   ((xmc[33] & 0x1) << 7)
           | ((xmc[34] & 0x7) << 4)
           | ((xmc[35] & 0x7) << 1)
           | ((xmc[36] >> 2) & 0x1);
    *c++ =   ((xmc[36] & 0x3) << 6)
           | ((xmc[37] & 0x7) << 3)
           | (xmc[38] & 0x7);
    *c++ =   ((Nc[3] & 0x7F) << 1)
           | ((bc[3] >> 1) & 0x1);
    *c++ =   ((bc[3] & 0x1) << 7)
           | ((Mc[3] & 0x3) << 5)
           | ((xmaxc[3] >> 1) & 0x1F);
    *c++ =   ((xmaxc[3] & 0x1) << 7)
           | ((xmc[39] & 0x7) << 4)
           | ((xmc[40] & 0x7) << 1)
           | ((xmc[41] >> 2) & 0x1);
    *c++ =   ((xmc[41] & 0x3) << 6)
           | ((xmc[42] & 0x7) << 3)
           | (xmc[43] & 0x7);
    *c++ =   ((xmc[44] & 0x7) << 5)
           | ((xmc[45] & 0x7) << 2)
           | ((xmc[46] >> 1) & 0x3);
    *c++ =   ((xmc[46] & 0x1) << 7)
           | ((xmc[47] & 0x7) << 4)
           | ((xmc[48] & 0x7) << 1)
           | ((xmc[49] >> 2) & 0x1);
    *c++ =   ((xmc[49] & 0x3) << 6)
           | ((xmc[50] & 0x7) << 3)
           | (xmc[51] & 0x7);
}
