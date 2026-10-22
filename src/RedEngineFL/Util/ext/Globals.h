#pragma once

#include "Allocator.h"
#include "hashtable.h"

namespace util
{

template <class ALLOC>
class VariableSet
{
public:

    static void* operator new(unsigned int uiSize, IAllocator& rAlloc)
    {
        Buffer buf(0, uiSize);
        rAlloc.Allocate(buf);
        if (!buf.data_)
        {
            *(volatile char*)buf.data_ = 4;
        }
        return buf.data_;
    }

    static void operator delete(void*, IAllocator& rAlloc)
    {
        Buffer buf(0, sizeof(VariableSet));
        rAlloc.Deallocate(buf);
    }

    int& GetInt(const char* pcName);

    void ResetAll();

    HashTable<int*, const char*, ALLOC>   int32Map_;
    HashTable<float*, const char*, ALLOC> floatMap_;
    HashTable<bool*, const char*, ALLOC>  boolMap_;
};

template <class ALLOC>
int& VariableSet<ALLOC>::GetInt(const char* pcName)
{
    int*& rpValue = int32Map_[pcName];
    if (!rpValue)
    {
        Buffer buf(0, sizeof(int));
        ALLOC::Allocate(buf);
        rpValue = (int*)buf.data_;
    }
    return *rpValue;
}

class Globals
{
public:
    Globals();
    ~Globals();
    static Globals& GetInstance();
    VariableSet<SingletonAllocator<DefaultStaticAllocator> >& GetSet(const char* pcName);

    HashTable<VariableSet<SingletonAllocator<DefaultStaticAllocator> >*, const char*,
              SingletonAllocator<DefaultStaticAllocator> > vsetMap_;
};

}

 
 

namespace util { extern util::Globals globals; }
