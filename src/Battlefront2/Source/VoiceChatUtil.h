#ifndef VOICECHATUTIL_H
#define VOICECHATUTIL_H

namespace VoiceChat
{
unsigned char* GetBitmapElementAndMask(unsigned char* const bitmap, unsigned int bit,
                                       unsigned char& mask);
void SetBit(unsigned char* const bitmap, unsigned int bit, bool value);

const unsigned char* GetBitmapElementAndMask(const unsigned char* const bitmap, unsigned int bit,
                                             unsigned char& mask);
bool GetBit(const unsigned char* const bitmap, unsigned int bit);

int GetMSBIndex();

void SetBits(unsigned char* const bitmap, unsigned int bit, unsigned int value, unsigned int count);
unsigned int GetBits(const unsigned char* const bitmap, unsigned int bit, unsigned int count);
}

#endif
