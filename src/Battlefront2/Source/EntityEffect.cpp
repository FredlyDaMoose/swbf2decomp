#include "EntityEffect.h"
#include "FLEffect.h"

// GLOBAL: SWBF2 0x01ebbc60
// SYNTHETIC: SWBF2 0x00402aa0
// `dynamic initializer for 'EntityEffectClass::rttiHashEntityEffectClass''
PblHash EntityEffectClass::rttiHashEntityEffectClass("EntityEffectClass");
char EntityEffectClass::rttiNameEntityEffectClass[] = "EntityEffectClass";

// FUNCTION: SWBF2 0x004a9a00
EntityEffectClass::EntityEffectClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mEffectClass = 0;
}

// FUNCTION: SWBF2 0x004a99b0
bool EntityEffectClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityEffectClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004a99e0
unsigned int EntityEffectClass::GetDerivedRtti()
{
    return rttiHashEntityEffectClass;
}

// FUNCTION: SWBF2 0x004a99f0
const char* EntityEffectClass::GetDerivedRttiName()
{
    return rttiNameEntityEffectClass;
}

// SYNTHETIC: SWBF2 0x004a9a30
// EntityEffectClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004a9a60
EntityEffectClass::~EntityEffectClass()
{
}

// FUNCTION: SWBF2 0x004a9a70
void EntityEffectClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x6E6E8D54)
    {
        mEffectClass = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
    }
    else
    {
        EntityClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x004a9ac0
EntityClass* EntityEffectClass::Derive(unsigned int uHash)
{
    return new EntityEffectClass(uHash);
}

// FUNCTION: SWBF2 0x004a9b20
Entity* EntityEffectClass::Build(const EntityDesc& desc)
{
    if (mEffectClass)
    {
        FLEffectObject* pEffect = mEffectClass->BuildEffect();
        if (pEffect)
        {
            pEffect->AttachEffectToMatrix((const PblMatrix*)&desc, false);
        }
    }
    return 0;
}
