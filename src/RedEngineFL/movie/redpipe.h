#pragma once

template <class T, int N>
class RedPipe
{
public:

    T Peek(unsigned int uiIndex) const
    {
        unsigned int uiPos = mReadPos + uiIndex;
        if (uiPos >= N)
        {
            uiPos -= N;
        }
        return mElements[uiPos];
    }

    T Flow(const T& in)
    {
        T out = Peek(0);
        mElements[mReadPos] = in;
        if (++mReadPos == N)
        {
            mReadPos = 0;
        }
        return out;
    }

    bool Exists(const T& element) const
    {
        for (unsigned int i = 0; i < N; ++i)
        {
            if (mElements[i] == element)
            {
                return true;
            }
        }
        return false;
    }

    T mElements[N];
    unsigned short mReadPos;
};

template <class T, int N>
class RedPipePointer : public RedPipe<T, N>
{
public:
    RedPipePointer()
    {
        RedPipe<T, N>::mElements[0] = 0;
        RedPipe<T, N>::mReadPos = 0;
        mPipe.mReadPos = 0;
    }

    T GetRecent() const
    {
        T recent = 0;
        for (int i = N - 1; !recent && i >= 0; --i)
        {
            recent = RedPipe<T, N>::Peek(i);
        }
        return recent;
    }

    RedPipe<T, N> mPipe;
};
