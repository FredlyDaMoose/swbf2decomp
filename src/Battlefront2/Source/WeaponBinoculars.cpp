#include "WeaponBinoculars.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <new>

// FUNCTION: SWBF2 0x0067e350
Weapon* WeaponBinocularsClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    void* pMemory = Weapon::sMemoryPool.Allocate(0x110);
    return pMemory ? reinterpret_cast<Weapon*>(::new (pMemory) WeaponBinoculars(this, desc)) : 0;
}

// GLOBAL: SWBF2 0x01f9a88c
// SYNTHETIC: SWBF2 0x00407080
// `dynamic initializer for 'WeaponBinoculars::rttiHashWeaponBinoculars''
PblHash WeaponBinoculars::rttiHashWeaponBinoculars("WeaponBinoculars");
char WeaponBinoculars::rttiNameWeaponBinoculars[] = "WeaponBinoculars";

// FUNCTION: SWBF2 0x0067e120
bool WeaponBinoculars::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponBinoculars || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0067e1c0
WeaponBinoculars::WeaponBinoculars(const WeaponBinocularsClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
}

// FUNCTION: SWBF2 0x0067e210
WeaponBinoculars::~WeaponBinoculars()
{
}

// SYNTHETIC: SWBF2 0x0067e1e0
// WeaponBinoculars::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0067e150
unsigned int WeaponBinoculars::GetDerivedRtti() const
{
    return rttiHashWeaponBinoculars;
}

// FUNCTION: SWBF2 0x0067e160
const char* WeaponBinoculars::GetDerivedRttiName() const
{
    return rttiNameWeaponBinoculars;
}

// GLOBAL: SWBF2 0x01f9a890
// SYNTHETIC: SWBF2 0x00407090
// `dynamic initializer for 'WeaponBinocularsClass::rttiHashWeaponBinocularsClass''
PblHash WeaponBinocularsClass::rttiHashWeaponBinocularsClass("WeaponBinocularsClass");
char WeaponBinocularsClass::rttiNameWeaponBinocularsClass[] = "WeaponBinocularsClass";

// FUNCTION: SWBF2 0x0067e220
WeaponBinocularsClass::WeaponBinocularsClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mLockOnAngle = 0.0f;
    mTargetSides = 0;
    mTargetTypes = 0;
    mAITargetTypes = 0;
}

// FUNCTION: SWBF2 0x0067e2a0
WeaponBinocularsClass::WeaponBinocularsClass(WeaponBinocularsClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x0067e2c0
WeaponBinocularsClass::~WeaponBinocularsClass()
{
}

// SYNTHETIC: SWBF2 0x0067e270
// WeaponBinocularsClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0067e170
bool WeaponBinocularsClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponBinocularsClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x0067e1a0
unsigned int WeaponBinocularsClass::GetDerivedRtti()
{
    return rttiHashWeaponBinocularsClass;
}

// FUNCTION: SWBF2 0x0067e1b0
const char* WeaponBinocularsClass::GetDerivedRttiName()
{
    return rttiNameWeaponBinocularsClass;
}

// FUNCTION: SWBF2 0x0067e2d0
void WeaponBinocularsClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    WeaponClass::SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x0067e2e0
WeaponClass* WeaponBinocularsClass::Derive(unsigned int uHash)
{
    return new WeaponBinocularsClass(this, uHash);
}
