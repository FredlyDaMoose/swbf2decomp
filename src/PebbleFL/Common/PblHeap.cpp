#include "PblHeap.h"

#include <stdlib.h>

// FUNCTION: SWBF2 0x0072b890
template void PblHeap<int, int>::Pop();

namespace pheap
{

// FUNCTION: SWBF2 0x0072b920
void __cdecl UnitTest()
{
    PblHeap<int, int> heap(0x800, 0x2000000, 0);

    for (int i = 0; i < 0x800; ++i)
    {
        int iValue = rand() % 0x2000000;
        heap.Push(iValue, iValue);
    }

    for (int i = 0; i < 0x800; ++i)
    {
        int iValue = heap.Value(0);
        heap.Pop();
        if (rand() % 3)
        {
            heap.Push(iValue, iValue);
        }
    }
}

}

class Test_
{
public:
    Test_() { pheap::UnitTest(); }
};

// SYNTHETIC: SWBF2 0x0040b060
// `dynamic initializer for 'test''
static Test_ test;
