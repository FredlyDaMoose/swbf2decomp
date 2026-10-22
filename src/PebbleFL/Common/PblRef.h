#ifndef SWBF2_SUPPORT_PBLREF_H
#define SWBF2_SUPPORT_PBLREF_H


// VTABLE: SWBF2 0x00771bcc
class PblRef
{
public:

    PblRef() : mValidFlag(1), mAliveFlag(1), mRefCount(1)
    {
    }

    virtual void Kill();

    virtual ~PblRef()
    {
        mValidFlag = 0;
        mAliveFlag = 0;
    }

    unsigned short AddRef()
    {
        ++mRefCount;
        return mRefCount;
    }

    // FUNCTION: SWBF2 0x0040cbb0
    unsigned short Release()
    {
        --mRefCount;
        unsigned short count = (unsigned short)mRefCount;
        if (count == 0)
        {
            delete this;
        }
        return count;
    }

    bool IsAlive() const
    {
        return mAliveFlag != 0;
    }

    unsigned short mValidFlag : 1;
    unsigned short mAliveFlag : 1;
    unsigned short mRefCount : 14;
};
 

 #endif
