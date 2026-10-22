#pragma once

#include "PebbleFL/Common/PblList.h"

class Entity;
class EntityClass;
struct EntityDesc;
class Ordnance;
class OrdnanceClass;
class ExplosionClass;
struct OrdnanceDesc;
class Weapon;
class WeaponClass;
struct WeaponDesc;

template <class TYPE, class CLASS, class DESC>
class Factory
{
public:
    Factory(unsigned int uHash);
    Factory(CLASS* pParent, unsigned int uHash);

    virtual ~Factory()
    {
        if (mNode._pList)
        {
            sList.Remove(&mNode);
        }
    }

    virtual CLASS* Derive(unsigned int uHash) = 0;
    virtual TYPE* Build(const DESC& desc) = 0;

    static CLASS* Find(unsigned int uHash);
    static void Reset();

    static CLASS* FindNetIndex(int iNetIndex);
    unsigned int GetId() const;

    typename PblList<Factory<TYPE, CLASS, DESC> >::Node mNode;
    CLASS* mParent;
    unsigned int mId;
    int mNetIndex;

    static PblList<Factory<TYPE, CLASS, DESC> > sList;
    static unsigned int sCounter;
};

template <class TYPE, class CLASS, class DESC>
Factory<TYPE, CLASS, DESC>::Factory(unsigned int uHash)
{
    mParent = 0;
    mId = uHash;
    mNetIndex = sCounter++;
    sList.AddLast(&mNode, this);
}

template <class TYPE, class CLASS, class DESC>
PblList<Factory<TYPE, CLASS, DESC> > Factory<TYPE, CLASS, DESC>::sList;

template <class TYPE, class CLASS, class DESC>
unsigned int Factory<TYPE, CLASS, DESC>::sCounter;

template <class TYPE, class CLASS, class DESC>
void Factory<TYPE, CLASS, DESC>::Reset()
{
    sCounter = 0;
    sList.RemoveAll();
}

template <class TYPE, class CLASS, class DESC>
Factory<TYPE, CLASS, DESC>::Factory(CLASS* pParent, unsigned int uHash)
{
    mParent = pParent;
    mId = uHash;
    mNetIndex = sCounter++;
    sList.AddLast(&mNode, this);
}

template <class TYPE, class CLASS, class DESC>
CLASS* Factory<TYPE, CLASS, DESC>::Find(unsigned int uHash)
{
    for (typename PblList<Factory<TYPE, CLASS, DESC> >::Itor it(sList); it; ++it)
    {
        if (it->mId == uHash)
        {
            return static_cast<CLASS*>((Factory<TYPE, CLASS, DESC>*)it);
        }
    }
    return 0;
}

// VTABLE: SWBF2 0x00799318

// GLOBAL: SWBF2 0x007ec55c
// GLOBAL: SWBF2 0x01faa458
// FUNCTION: SWBF2 0x00491e70
template Factory<Entity, EntityClass, EntityDesc>::Factory(unsigned int);

// FUNCTION: SWBF2 0x00491ef0
template Factory<Entity, EntityClass, EntityDesc>::Factory(EntityClass*, unsigned int);

// VTABLE: SWBF2 0x007ac47c

// GLOBAL: SWBF2 0x007ec590
// GLOBAL: SWBF2 0x01faa728
// FUNCTION: SWBF2 0x005f8940
template Factory<Ordnance, OrdnanceClass, OrdnanceDesc>::Factory(unsigned int);

// VTABLE: SWBF2 0x007b0298

// GLOBAL: SWBF2 0x007ec5a4
// GLOBAL: SWBF2 0x01faa758
// FUNCTION: SWBF2 0x0067bfe0
template Factory<Weapon, WeaponClass, WeaponDesc>::Factory(unsigned int);

// FUNCTION: SWBF2 0x0067c060
template Factory<Weapon, WeaponClass, WeaponDesc>::Factory(WeaponClass*, unsigned int);

// VTABLE: SWBF2 0x0079e8ac

// GLOBAL: SWBF2 0x007ec570
// GLOBAL: SWBF2 0x01faa470
// FUNCTION: SWBF2 0x0051eb20
template Factory<Ordnance, ExplosionClass, OrdnanceDesc>::Factory(unsigned int);
