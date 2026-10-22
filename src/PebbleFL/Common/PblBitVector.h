#pragma once

template <int N>
class PblBitVector
{
public:
    enum { NUM_UINTS = (N + 31) / 32 };

    PblBitVector() {}

    void ClearAll()
    {
        for (unsigned int i = 0; i < NUM_UINTS; ++i)
        {
            m_auiData[i] = 0;
        }
    }

    void Or(const PblBitVector& rOther)
    {
        for (unsigned int i = 0; i < NUM_UINTS; ++i)
        {
            m_auiData[i] |= rOther.m_auiData[i];
        }
    }

    void SetAll()
    {
        for (unsigned int i = 0; i < NUM_UINTS; ++i)
        {
            m_auiData[i] = 0xffffffff;
        }
    }
    void ClearBit(unsigned int uBit)
    {
        m_auiData[uBit >> 5] &= ~(1 << (uBit & 31));
    }

    void SetBit(unsigned int uBit)
    {
        m_auiData[uBit >> 5] |= 1 << (uBit & 31);
    }
    void InvertAll()
    {
        for (unsigned int i = 0; i < N / 32; ++i)
        {
            m_auiData[i] = ~m_auiData[i];
        }
        if (N % 32)
        {
            m_auiData[N / 32] ^= (1u << (N % 32)) - 1;
        }
    }
    void And(const PblBitVector& rOther)
    {
        for (unsigned int i = 0; i < NUM_UINTS; ++i)
        {
            m_auiData[i] &= rOther.m_auiData[i];
        }
    }
    const PblBitVector operator~() const
    {
        PblBitVector result(*this);
        result.InvertAll();
        return result;
    }
    bool IsBitSet(unsigned int uBit) const
    {
        return (m_auiData[uBit >> 5] & (1 << (uBit & 31))) != 0;
    }

    unsigned int m_auiData[NUM_UINTS];
};
