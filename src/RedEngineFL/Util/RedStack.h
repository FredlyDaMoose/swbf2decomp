#pragma once

class RedStack
{
public:
    RedStack() : mMemIdx(sMemIdx) {}
    ~RedStack() { sMemIdx = mMemIdx; }

    void* Alloc(int iSize)
    {
        if (iSize & 0xf)
        {
            iSize = (iSize & ~0xf) + 0x10;
        }
        void* p = &sMem[sMemIdx];
        sMemIdx += iSize;
        return p;
    }

    int mMemIdx;

    static char sMem[40960];
    static int sMemIdx;
};
 
