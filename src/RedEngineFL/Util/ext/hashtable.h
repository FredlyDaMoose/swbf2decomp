#pragma once

#include "basevector.h"

namespace util
{

class Hash
{
public:
    static unsigned int From(const char* pcKey)
    {
        if (!pcKey || !*pcKey)
        {
            return 0;
        }

        unsigned int uiHash = 0x811c9dc5u;
        for (; *pcKey; ++pcKey)
        {
            uiHash = (uiHash ^ (unsigned int)(*pcKey | 0x20)) * 0x01000193u;
        }
        return uiHash;
    }

    static unsigned int From(unsigned int uiKey)
    {
        return uiKey * 0xD2D84A61u ^ 0x87CD360Bu;
    }
};

template <class ELEM, class KEY, class ELEMVEC, class KEYVEC, class HASHVEC>
class HashTable_
{
public:

    // FUNCTION: SWBF2 0x006d7070
    HashTable_()
        : size_(0), reserveMult_(2.0f)
    {
        Clear();
    }

    void Clear()
    {
        elements_.Clear();
        hashes_.Clear();
        keys_.Clear();
        size_ = 0;
    }

    ~HashTable_() {}

    // FUNCTION: SWBF2 0x0052b080
    unsigned int FindIndex_(unsigned int uiHash) const
    {
        const unsigned int uiSize = (unsigned int)hashes_.GetSize();
        if (uiSize == 0)
        {
            return 0;
        }

        unsigned int uiIndex = uiHash % uiSize;
        while (hashes_[(int)uiIndex] != 0 && hashes_[(int)uiIndex] != uiHash)
        {
            uiIndex = (uiIndex + 1) % uiSize;
        }

        return uiIndex;
    }

    ELEM& Get(const KEY& rKey)
    {
        const unsigned int uiHash = Hash::From(rKey);
        const unsigned int uiIndex = FindIndex_(uiHash);

        if (hashes_[(int)uiIndex] == 0)
        {
            elements_[(int)uiIndex] = ELEM();
            hashes_[(int)uiIndex] = uiHash;
            keys_[(int)uiIndex] = rKey;
            size_++;
        }

        return elements_[(int)uiIndex];
    }

    class Itor
    {
    public:
        Itor(HashTable_& rTable) : table_(&rTable), pos_(0) {}

        int GetPos() const { return pos_; }

        bool operator!=(const Itor& rOther) const
        {
            return pos_ != rOther.pos_ || table_ != rOther.table_;
        }

        ELEM& operator*() { return table_->elements_[pos_]; }

        const Itor& operator++()
        {
            ++pos_;
            while (pos_ < table_->hashes_.GetSize() && table_->hashes_[pos_] == 0)
            {
                ++pos_;
            }
            return *this;
        }

    private:
        friend class HashTable_;
        Itor(HashTable_* pTable, unsigned int uiPos) : table_(pTable), pos_((int)uiPos) {}

        HashTable_* table_;
        int pos_;
    };

    Itor Begin()
    {
        Itor it(*this);
        if (hashes_.GetSize() > 0 && hashes_[0] == 0)
        {
            ++it;
        }
        return it;
    }

    Itor End() { return Itor(this, (unsigned int)hashes_.GetSize()); }

    ELEMVEC elements_;
    KEYVEC  keys_;
    HASHVEC hashes_;
    int     size_;
    float   reserveMult_;
};

template <class ELEM, class KEY, class ALLOC>
class HashTable
    : public HashTable_<ELEM, KEY,
                        Vector<ELEM, BaseArray<ELEM, TypedArrayAllocator<ELEM, ALLOC> > >,
                        Vector<KEY, BaseArray<KEY, TypedArrayAllocator<KEY, ALLOC> > >,
                        Vector<unsigned int,
                               BaseArray<unsigned int, TypedArrayAllocator<unsigned int, ALLOC> > > >
{
public:

    typedef Vector<ELEM, BaseArray<ELEM, TypedArrayAllocator<ELEM, ALLOC> > > ElemVecT;
    typedef Vector<KEY, BaseArray<KEY, TypedArrayAllocator<KEY, ALLOC> > > KeyVecT;
    typedef Vector<unsigned int,
                   BaseArray<unsigned int, TypedArrayAllocator<unsigned int, ALLOC> > > HashVecT;

    ~HashTable() {}

    // FUNCTION: SWBF2 0x0052ad80
    void Reallocate(int iSize)
    {
        ElemVecT aElements;
        HashVecT aHashes;
        KeyVecT aKeys;

        aElements.SetSize(iSize);
        aElements.SetAll(ELEM());

        aHashes.SetSize(iSize);
        aHashes.SetAll(0);

        aKeys.SetSize(iSize);
        aKeys.SetAll(KEY());

        aElements.Swap(this->elements_);
        aHashes.Swap(this->hashes_);
        aKeys.Swap(this->keys_);

        for (int i = 0; i < aHashes.GetSize(); i++)
        {
            const unsigned int uiHash = aHashes[i];
            if (uiHash == 0)
            {
                continue;
            }

            const unsigned int uiIndex = this->FindIndex_(uiHash);
            this->hashes_[(int)uiIndex] = uiHash;
            this->elements_[(int)uiIndex] = aElements[i];
            this->keys_[(int)uiIndex] = aKeys[i];
        }
    }

    void CheckSpace_()
    {
        if ((float)this->size_ * this->reserveMult_ >= (float)this->elements_.GetSize())
        {
            int iGrown = (int)((float)this->elements_.GetSize() * this->reserveMult_);
            int iFloor = 8;
            Reallocate(Max_(iGrown, iFloor));
        }
    }

    // FUNCTION: SWBF2 0x0052af70
    ELEM& operator[](const KEY& rKey)
    {
        CheckSpace_();
        return this->Get(rKey);
    }
};

}
