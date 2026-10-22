#include "Globals.h"

// FUNCTION: SWBF2 0x00715800
template bool util::BaseArray<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*,
    util::TypedArrayAllocator<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x00715700
template bool util::BaseArray<float*,
    util::TypedArrayAllocator<float*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x00715900
template bool util::BaseArray<bool*,
    util::TypedArrayAllocator<bool*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x00715a00
template bool util::Vector<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, util::BaseArray<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*,
    util::TypedArrayAllocator<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, util::SingletonAllocator<util::DefaultStaticAllocator> > > >::SetSize_(int, bool);
// FUNCTION: SWBF2 0x00715280
template unsigned int util::HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::FindIndex_(unsigned int) const;

// GLOBAL: SWBF2 0x009caacc
// SYNTHETIC: SWBF2 0x0040a8e0
// `dynamic initializer for 'util::globals''
// SYNTHETIC: SWBF2 0x0076a080
// `dynamic atexit destructor for 'util::globals''
namespace util { util::Globals globals; }

// FUNCTION: SWBF2 0x00714b90
template util::HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::HashTable_();

// FUNCTION: SWBF2 0x007149d0
template util::HashTable_<int*, char const*, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::HashTable_();
// FUNCTION: SWBF2 0x00714ab0
template util::HashTable_<float*, char const*, util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::HashTable_();
// FUNCTION: SWBF2 0x00714c70
template util::HashTable_<bool*, char const*, util::HashTable<bool*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<bool*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<bool*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::HashTable_();

// FUNCTION: SWBF2 0x00714d50
util::Globals::Globals()
{
}

// TEMPLATE: SWBF2 0x00714d60
// util::HashTable<int *,char const *,util::SingletonAllocator<util::DefaultStaticAllocator> >::~HashTable<int *,char const *,util::SingletonAllocator<util::DefaultStaticAllocator> >
template util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::~HashTable();
// TEMPLATE: SWBF2 0x00714d70
// util::HashTable<float *,char const *,util::SingletonAllocator<util::DefaultStaticAllocator> >::~HashTable<float *,char const *,util::SingletonAllocator<util::DefaultStaticAllocator> >
template util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::~HashTable();

// TEMPLATE: SWBF2 0x00714d80
// util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,char const *,util::SingletonAllocator<util::DefaultStaticAllocator> >::~HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,char const *,util::SingletonAllocator<util::DefaultStaticAllocator> >
template util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::~HashTable();

// TEMPLATE: SWBF2 0x00714d90
// util::HashTable_<int *,char const *,util::Vector<int *,util::BaseArray<int *,util::TypedArrayAllocator<int *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::SingletonAllocator<util::DefaultStaticAllocator> > > > >::~HashTable_<int *,char const *,util::Vector<int *,util::BaseArray<int *,util::TypedArrayAllocator<int *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::SingletonAllocator<util::DefaultStaticAllocator> > > > >
template util::HashTable_<int*, char const*, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::~HashTable_();
// TEMPLATE: SWBF2 0x00714e90
// util::HashTable_<float *,char const *,util::Vector<float *,util::BaseArray<float *,util::TypedArrayAllocator<float *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::SingletonAllocator<util::DefaultStaticAllocator> > > > >::~HashTable_<float *,char const *,util::Vector<float *,util::BaseArray<float *,util::TypedArrayAllocator<float *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::SingletonAllocator<util::DefaultStaticAllocator> > > > >
template util::HashTable_<float*, char const*, util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<float*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::~HashTable_();
// TEMPLATE: SWBF2 0x00714f90
// util::HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,char const *,util::Vector<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,util::BaseArray<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,util::TypedArrayAllocator<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::SingletonAllocator<util::DefaultStaticAllocator> > > > >::~HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,char const *,util::Vector<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,util::BaseArray<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,util::TypedArrayAllocator<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> > *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::SingletonAllocator<util::DefaultStaticAllocator> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::SingletonAllocator<util::DefaultStaticAllocator> > > > >
template util::HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::~HashTable_();

// FUNCTION: SWBF2 0x00715180
util::Globals::~Globals()
{
    for (util::HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::Itor it = vsetMap_.Begin(); it != vsetMap_.End(); ++it)
    {
        util::Buffer buf(*it, sizeof(util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >));
        util::SingletonAllocator<util::DefaultStaticAllocator>::Deallocate(buf);
    }
}

// FUNCTION: SWBF2 0x00715090
template util::BaseArray<float*,
    util::TypedArrayAllocator<float*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::~BaseArray();
// FUNCTION: SWBF2 0x007150e0
template util::BaseArray<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*,
    util::TypedArrayAllocator<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::~BaseArray();
// FUNCTION: SWBF2 0x00715130
template util::BaseArray<bool*,
    util::TypedArrayAllocator<bool*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::~BaseArray();

// FUNCTION: SWBF2 0x00715250
template void util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >::operator delete(void*, util::IAllocator&);

// FUNCTION: SWBF2 0x007152d0
template util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*& util::HashTable_<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::Get(char const* const&);

// FUNCTION: SWBF2 0x00715350
util::Globals& util::Globals::GetInstance()
{
    return util::globals;
}

// FUNCTION: SWBF2 0x00715360
util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >&
util::Globals::GetSet(const char* pcName)
{
    util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*& rpSet = vsetMap_[pcName];
    if (!rpSet)
    {
        rpSet = new (DefaultStaticAllocator::GetInstance()) util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >;
    }
    return *rpSet;
}

// FUNCTION: SWBF2 0x007154a0
template void util::HashTable<util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::Reallocate(int);
