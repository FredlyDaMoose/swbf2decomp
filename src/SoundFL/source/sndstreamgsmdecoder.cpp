#include <new>
#include <emmintrin.h>
#include <assert.h>
#include <string.h>

#include "sndstreamgsmdecoder.h"
#include "sndstreamsourceproducer.h"

extern int __cdecl gsm_decode(const __m128i* state, unsigned char* frame, short* samples);

namespace Snd
{

// FUNCTION: SWBF2 0x0073d040
StreamGSMDecoder::StreamGSMDecoder()
    : mInputPacket(0),
      mInputPacketOffset(0),
      mInputPacketSize(0),
      mOutputPacketOffset(0),
      mFrameOffset(160)
{
    mDecimate = false;
    mState.Init();
}

// FUNCTION: SWBF2 0x0073d140
void* __fastcall sub_73D140(void* pDecoder, int, unsigned int uiFlags)
{
    ((StreamCodec*)pDecoder)->StreamCodec::~StreamCodec();

    if ((uiFlags & 1) != 0)
    {
        ::operator delete(pDecoder);
    }

    return pDecoder;
}

// FUNCTION: SWBF2 0x0073d250
StreamPacket* StreamGSMDecoder::GetPacket(unsigned int uiOutput)
{
    StreamNode* const pNode = this;

    if (pNode->GetCurrentPlayState() == SourceTransport::playState_playing)
    {
        StreamPacket* const pPacket = StreamSourceProducer::GetPacket(uiOutput);
        if (pPacket != 0)
        {
            if (pPacket->GetLast())
            {
                pNode->Stop();
            }
            return pPacket;
        }
    }

    Update();

    if (pNode->GetCurrentPlayState() != SourceTransport::playState_playing)
    {
        return 0;
    }

    StreamPacket* const pPacket = StreamSourceProducer::GetPacket(uiOutput);
    if (pPacket == 0)
    {
        return 0;
    }

    if (pPacket->GetLast())
    {
        pNode->Stop();
    }
    return pPacket;
}

// FUNCTION: SWBF2 0x0073d170
void StreamGSMDecoder::Flush()
{
    if (mInputPacket != 0)
    {
        mInputPacket->Unlock();
        mInputPacket = 0;
        mInputPacketOffset = 0;
        mInputPacketSize = 0;
    }

    mOutputPacketOffset = 0;
    mFrameOffset = 160;
    mState.Init();
    StreamSourceProducer::Flush();
}

// FUNCTION: SWBF2 0x0073d300
void StreamGSMDecoder::Update()
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
            mInputPacketSize = mInputPacket->mBufferUsed;
            mInputPacketOffset = 0;
        }

        do
        {
            if (pOutput == 0)
            {
                break;
            }

            bool bEnd = false;
            short* const pSamples = (short*)pOutput->mBuffer;
            unsigned char* const pInput = (unsigned char*)mInputPacket->mBuffer;
            bool bDiscontinuity = false;
            bool bLast = false;

            if (mDecimate)
            {
                if (mFrameOffset == 160)
                {
                    mState.Decode((unsigned char (*)[33])(pInput + mInputPacketOffset),
                                  mFrame);
                    mInputPacketOffset += 33;
                    mFrameOffset = 0;
                }

                unsigned int i;
                for (i = 0; i < 79; ++i)
                {
                    short* const pOut = pSamples + (mOutputPacketOffset + i * 2);
                    const short* const pIn = &(*mFrame)[mFrameOffset + i];
                    pOut[0] = pIn[0];
                    pOut[1] = (short)((pIn[0] + pIn[1]) >> 1);
                }

                short* const pOut = pSamples + (mOutputPacketOffset + i * 2);
                const short* const pIn = &(*mFrame)[mFrameOffset + i];
                pOut[0] = pIn[0];
                pOut[1] = pIn[0];

                mOutputPacketOffset += 160;
                mFrameOffset += 80;
            }
            else
            {
                mState.Decode((unsigned char (*)[33])(pInput + mInputPacketOffset),
                              (short (*)[160])(pSamples + mOutputPacketOffset));
                mInputPacketOffset += 33;
                mOutputPacketOffset += 160;
            }

            if (mInputPacketOffset == mInputPacket->mBufferUsed &&
                (!mDecimate || mFrameOffset == 160))
            {
                bLast = mInputPacket->GetLast();
                bDiscontinuity = mInputPacket->GetDiscontinuity();
                bEnd = bLast || bDiscontinuity;
                if (bEnd)
                {
                    mState.Init();
                }

                mInputPacket->Unlock();
                mInputPacket = 0;
                mInputPacketOffset = 0;
                mInputPacketSize = 0;
            }

            const unsigned int uiSamples = mOutputPacketOffset;
            if (uiSamples == mProducer[2] >> 1 || bEnd)
            {
                const unsigned int uiPos = mPlaybackPosition;
                PacketComplete(pOutput, uiSamples * 2, uiSamples, bLast,
                               bDiscontinuity, uiPos);
                mOutputPacketOffset = 0;
                pOutput = 0;
                mPlaybackPosition = uiPos + uiSamples;
            }
        } while (mInputPacket != 0);

        pOutput = GetReadPacket();
    }
}

// FUNCTION: SWBF2 0x0073d2f0
void Snd::StreamGSMDecoder::SetPackets(unsigned int uiPacketSize, StreamPacket* pPackets, unsigned int uiNumPackets)
{
    void* pDecoder = (void*)this;
    ((StreamSourceProducer*)pDecoder)->SetPackets(uiPacketSize, pPackets,
                                                  uiNumPackets);
}

}

extern unsigned short* __cdecl APCM_quantization_xmaxc_to_exp_mant(short value, short* exponent,
                                           unsigned short* mantissa);
extern int __cdecl APCM_inverse_quantization(const short* coded, unsigned short mantissa,
                               short exponent, short* output);
extern void __cdecl RPE_grid_positioning(unsigned short grid, const short* input,
                                short* output);
extern void __cdecl Decoding_of_the_coded_Log_Area_Ratios(const short* coded, short* decoded);
extern int __cdecl Coefficients_0_12(const short* previous, const short* current, short* output);
extern int __cdecl Coefficients_13_26(const short* previous, const short* current, short* output);
extern int __cdecl Coefficients_27_39(const short* previous, const short* current, short* output);
extern int __cdecl LARp_to_rp(short* ratios);

static short gsmDecoderSaturate(int value)
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

static short gsmDecoderMultiply(short a, short b)
{
    if (a == (short)0x8000 && b == (short)0x8000)
    {
        return 32767;
    }
    return (short)(((int)a * b + 16384) >> 15);
}

// FUNCTION: SWBF2 0x0074eea0
int __cdecl Gsm_RPE_Decoding(int state, short maximum, short grid,
                       short* coded, int output)
{
    (void)state;
    short exponent;
    unsigned short mantissa;
    short reconstructed[13];
    APCM_quantization_xmaxc_to_exp_mant(maximum, &exponent, &mantissa);
    APCM_inverse_quantization(coded, mantissa, exponent, reconstructed);

    typedef int (__cdecl* GridFn)(unsigned short, const short*, short*);
    return ((GridFn)RPE_grid_positioning)((unsigned short)grid, reconstructed, (short*)output);
}

// FUNCTION: SWBF2 0x007518b0
int __cdecl Gsm_Long_Term_Synthesis_Filtering(int state, short lag, short gain, int residual, short* history)
{
    short* const nrp = (short*)(state + 0x270);
    if ((unsigned short)(lag - 40) > 80)
    {
        lag = *nrp;
    }
    *nrp = lag;
    if (lag < 40 || lag > 120)
    {
        _wassert(L"Nr >= 40 && Nr <= 120", L"source\\gsm-1.0.10\\long_term.c", 0x3a0);
    }
    static const short kGain[4] = { 3277, 11469, 21299, 32767 };
    const short factor = kGain[gain];
    if (factor == (short)0x8000)
    {
        _wassert(L"brp != MIN_WORD", L"source\\gsm-1.0.10\\long_term.c", 0x3a9);
    }
    int result = 0;
    for (int i = 0; i < 40; ++i)
    {
        result = gsmDecoderSaturate((int)gsmDecoderMultiply(factor, history[i - lag]) +
                                    ((short*)residual)[i]);
        history[i] = (short)result;
    }
    memmove(history - 120, history - 80, 120 * sizeof(short));
    return result;
}

int __cdecl Gsm_Short_Term_Synthesis_Filter(Snd::GSM* state, short* coded, int input, int output);

// FUNCTION: SWBF2 0x007526f0
int __cdecl Short_term_synthesis_filtering(int state, int ratios, int count, short* input, short* output)
{
    short* memory = (short*)(state + 0x272);
    int result = count;
    for (int i = 0; i < count; ++i)
    {
        short sample = input[i];
        short* coeff = (short*)ratios;
        for (int j = 7; j >= 0; --j)
        {
            sample = gsmDecoderSaturate((int)sample - gsmDecoderMultiply(coeff[j], memory[j]));
            memory[j + 1] = gsmDecoderSaturate((int)memory[j] + gsmDecoderMultiply(sample, coeff[j]));
        }
        memory[0] = sample;
        output[i] = sample;
        result = count - i - 1;
    }
    return result;
}

// FUNCTION: SWBF2 0x007523c0
int __cdecl Gsm_Short_Term_Synthesis_Filter(Snd::GSM* state, short* coded, int input, int output)
{
    short* const previous = state->LARpp[state->j];
    state->j ^= 1;
    short* const current = state->LARpp[state->j];
    short ratios[8];
    Decoding_of_the_coded_Log_Area_Ratios(coded, previous);
    Coefficients_0_12(current, previous, ratios);
    LARp_to_rp(ratios);
    Short_term_synthesis_filtering((int)state, (int)ratios, 13, (short*)input, (short*)output);
    Coefficients_13_26(current, previous, ratios);
    LARp_to_rp(ratios);
    Short_term_synthesis_filtering((int)state, (int)ratios, 14, (short*)input + 13, (short*)output + 13);
    Coefficients_27_39(current, previous, ratios);
    LARp_to_rp(ratios);
    Short_term_synthesis_filtering((int)state, (int)ratios, 13, (short*)input + 27, (short*)output + 27);
    memcpy(ratios, previous, sizeof(ratios));
    LARp_to_rp(ratios);
    return Short_term_synthesis_filtering((int)state, (int)ratios, 120, (short*)input + 40, (short*)output + 40);
}

// FUNCTION: SWBF2 0x00750630
unsigned int __cdecl Gsm_Decoder(Snd::GSM* state, int codedLar,
                                 int lag, int gain, short* grid,
                                 int maximum, short* coded, short* output)
{
    short samples[160];
    short residual[40];
    short* const history = state->dp0 + 120;
    for (int block = 0; block < 4; ++block)
    {
        Gsm_RPE_Decoding((int)state, ((short*)maximum)[block], grid[block],
                    coded + 13 * block, (int)residual);
        Gsm_Long_Term_Synthesis_Filtering((int)state, ((short*)lag)[block], ((short*)gain)[block],
                    (int)residual, history);
        memcpy(samples + 40 * block, history, 40 * sizeof(short));
    }
    Gsm_Short_Term_Synthesis_Filter(state, (short*)codedLar, (int)samples, (int)output);
    short previous = state->msr;
    unsigned int result = 0;
    for (int i = 0; i < 160; ++i)
    {
        previous = gsmDecoderSaturate((int)output[i] +
            (short)(((int)previous * 28180 + 16384) >> 15));
        result = (unsigned short)(gsmDecoderSaturate(2 * (int)previous) & ~7);
        output[i] = (short)result;
    }
    state->msr = previous;
    return result;
}

// FUNCTION: SWBF2 0x00750820
int __cdecl gsm_decode(const __m128i* s, unsigned char* c, short* target)
{
    short LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13 * 4];

    if (((*c >> 4) & 0x0F) != 0xD)
    {
        return -1;
    }

    LARc[0]  = (short)((*c++ & 0xF) << 2);
    LARc[0] |= (*c >> 6) & 0x3;
    LARc[1]  = *c++ & 0x3F;
    LARc[2]  = (*c >> 3) & 0x1F;
    LARc[3]  = (short)((*c++ & 0x7) << 2);
    LARc[3] |= (*c >> 6) & 0x3;
    LARc[4]  = (*c >> 2) & 0xF;
    LARc[5]  = (short)((*c++ & 0x3) << 2);
    LARc[5] |= (*c >> 6) & 0x3;
    LARc[6]  = (*c >> 3) & 0x7;
    LARc[7]  = *c++ & 0x7;

    Nc[0]  = (*c >> 1) & 0x7F;
    bc[0]  = (short)((*c++ & 0x1) << 1);
    bc[0] |= (*c >> 7) & 0x1;
    Mc[0]  = (*c >> 5) & 0x3;
    xmaxc[0]  = (short)((*c++ & 0x1F) << 1);
    xmaxc[0] |= (*c >> 7) & 0x1;
    xmc[0]  = (*c >> 4) & 0x7;
    xmc[1]  = (*c >> 1) & 0x7;
    xmc[2]  = (short)((*c++ & 0x1) << 2);
    xmc[2] |= (*c >> 6) & 0x3;
    xmc[3]  = (*c >> 3) & 0x7;
    xmc[4]  = *c++ & 0x7;
    xmc[5]  = (*c >> 5) & 0x7;
    xmc[6]  = (*c >> 2) & 0x7;
    xmc[7]  = (short)((*c++ & 0x3) << 1);
    xmc[7] |= (*c >> 7) & 0x1;
    xmc[8]  = (*c >> 4) & 0x7;
    xmc[9]  = (*c >> 1) & 0x7;
    xmc[10]  = (short)((*c++ & 0x1) << 2);
    xmc[10] |= (*c >> 6) & 0x3;
    xmc[11]  = (*c >> 3) & 0x7;
    xmc[12]  = *c++ & 0x7;

    Nc[1]  = (*c >> 1) & 0x7F;
    bc[1]  = (short)((*c++ & 0x1) << 1);
    bc[1] |= (*c >> 7) & 0x1;
    Mc[1]  = (*c >> 5) & 0x3;
    xmaxc[1]  = (short)((*c++ & 0x1F) << 1);
    xmaxc[1] |= (*c >> 7) & 0x1;
    xmc[13]  = (*c >> 4) & 0x7;
    xmc[14]  = (*c >> 1) & 0x7;
    xmc[15]  = (short)((*c++ & 0x1) << 2);
    xmc[15] |= (*c >> 6) & 0x3;
    xmc[16]  = (*c >> 3) & 0x7;
    xmc[17]  = *c++ & 0x7;
    xmc[18]  = (*c >> 5) & 0x7;
    xmc[19]  = (*c >> 2) & 0x7;
    xmc[20]  = (short)((*c++ & 0x3) << 1);
    xmc[20] |= (*c >> 7) & 0x1;
    xmc[21]  = (*c >> 4) & 0x7;
    xmc[22]  = (*c >> 1) & 0x7;
    xmc[23]  = (short)((*c++ & 0x1) << 2);
    xmc[23] |= (*c >> 6) & 0x3;
    xmc[24]  = (*c >> 3) & 0x7;
    xmc[25]  = *c++ & 0x7;

    Nc[2]  = (*c >> 1) & 0x7F;
    bc[2]  = (short)((*c++ & 0x1) << 1);
    bc[2] |= (*c >> 7) & 0x1;
    Mc[2]  = (*c >> 5) & 0x3;
    xmaxc[2]  = (short)((*c++ & 0x1F) << 1);
    xmaxc[2] |= (*c >> 7) & 0x1;
    xmc[26]  = (*c >> 4) & 0x7;
    xmc[27]  = (*c >> 1) & 0x7;
    xmc[28]  = (short)((*c++ & 0x1) << 2);
    xmc[28] |= (*c >> 6) & 0x3;
    xmc[29]  = (*c >> 3) & 0x7;
    xmc[30]  = *c++ & 0x7;
    xmc[31]  = (*c >> 5) & 0x7;
    xmc[32]  = (*c >> 2) & 0x7;
    xmc[33]  = (short)((*c++ & 0x3) << 1);
    xmc[33] |= (*c >> 7) & 0x1;
    xmc[34]  = (*c >> 4) & 0x7;
    xmc[35]  = (*c >> 1) & 0x7;
    xmc[36]  = (short)((*c++ & 0x1) << 2);
    xmc[36] |= (*c >> 6) & 0x3;
    xmc[37]  = (*c >> 3) & 0x7;
    xmc[38]  = *c++ & 0x7;

    Nc[3]  = (*c >> 1) & 0x7F;
    bc[3]  = (short)((*c++ & 0x1) << 1);
    bc[3] |= (*c >> 7) & 0x1;
    Mc[3]  = (*c >> 5) & 0x3;
    xmaxc[3]  = (short)((*c++ & 0x1F) << 1);
    xmaxc[3] |= (*c >> 7) & 0x1;
    xmc[39]  = (*c >> 4) & 0x7;
    xmc[40]  = (*c >> 1) & 0x7;
    xmc[41]  = (short)((*c++ & 0x1) << 2);
    xmc[41] |= (*c >> 6) & 0x3;
    xmc[42]  = (*c >> 3) & 0x7;
    xmc[43]  = *c++ & 0x7;
    xmc[44]  = (*c >> 5) & 0x7;
    xmc[45]  = (*c >> 2) & 0x7;
    xmc[46]  = (short)((*c++ & 0x3) << 1);
    xmc[46] |= (*c >> 7) & 0x1;
    xmc[47]  = (*c >> 4) & 0x7;
    xmc[48]  = (*c >> 1) & 0x7;
    xmc[49]  = (short)((*c++ & 0x1) << 2);
    xmc[49] |= (*c >> 6) & 0x3;
    xmc[50]  = (*c >> 3) & 0x7;
    xmc[51]  = *c++ & 0x7;

    Gsm_Decoder((Snd::GSM*)s, (int)LARc, (int)Nc, (int)bc, Mc, (int)xmaxc, xmc, target);
    return 0;
}
