#include "VoiceChatUtil.h"

// FUNCTION: SWBF2 0x006751e0
int VoiceChat::GetMSBIndex()
{
    unsigned int uiValue = 0x19;
    int iBits = 0;

    do
    {
        ++iBits;
        uiValue >>= 1;
    } while (uiValue);

    return iBits;
}

// FUNCTION: SWBF2 0x006750c0
unsigned char* VoiceChat::GetBitmapElementAndMask(unsigned char* const bitmap, unsigned int bit,
                                                  unsigned char& mask)
{
    const unsigned int element = bit >> 3;
    mask = (unsigned char)(1 << (bit - element * 8));
    return bitmap + element;
}

// FUNCTION: SWBF2 0x00675100
void VoiceChat::SetBit(unsigned char* const bitmap, unsigned int bit, bool value)
{
    unsigned char mask;
    unsigned char* element = GetBitmapElementAndMask(bitmap, bit, mask);
    if (value)
    {
        *element |= mask;
    }
    else
    {
        *element &= ~mask;
    }
}

// FUNCTION: SWBF2 0x006750f0
const unsigned char* VoiceChat::GetBitmapElementAndMask(const unsigned char* const bitmap,
                                                        unsigned int bit, unsigned char& mask)
{
    return GetBitmapElementAndMask(const_cast<unsigned char*>(bitmap), bit, mask);
}

// FUNCTION: SWBF2 0x00675130
bool VoiceChat::GetBit(const unsigned char* const bitmap, unsigned int bit)
{
    unsigned char mask;
    const unsigned char* element = GetBitmapElementAndMask(bitmap, bit, mask);
    return (*element & mask) != 0;
}

// FUNCTION: SWBF2 0x00675160
void VoiceChat::SetBits(unsigned char* const pBitmap, unsigned int bit,
                        unsigned int value, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
    {
        VoiceChat::SetBit(pBitmap, bit + i, (value & 1) != 0);
        value >>= 1;
    }
}

// FUNCTION: SWBF2 0x006751a0
unsigned int VoiceChat::GetBits(const unsigned char* const pBitmap, unsigned int bit,
                                unsigned int count)
{
    unsigned int value = 0;
    for (unsigned int i = bit + count; i > bit; )
    {
        value = (value << 1) | (unsigned int)VoiceChat::GetBit(pBitmap, --i);
    }
    return value;
}
