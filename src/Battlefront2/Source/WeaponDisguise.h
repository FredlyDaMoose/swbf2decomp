#pragma once

#include <stddef.h>

#include "Weapon.h"
#include "PebbleFL/Common/PblHashTable.h"

// VTABLE: SWBF2 0x007b0abc
class WeaponDisguiseClass : public WeaponClass
{
public:
    WeaponDisguiseClass(unsigned int uHash);

    WeaponDisguiseClass(WeaponDisguiseClass* pBase, unsigned int uHash);
    virtual ~WeaponDisguiseClass();

    static PblHash rttiHashWeaponDisguiseClass;
    static char rttiNameWeaponDisguiseClass[];

    virtual WeaponClass* Derive(unsigned int uHash);
    virtual Weapon* Build(const WeaponDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    float m_fChangeRate;
};
  
 

#include "WeaponCannonInstance.h"

class Controllable;
class GameModel;
class Team;
class Trigger;

// VTABLE: SWBF2 0x007b0af4
class WeaponDisguise : public Weapon
{
public:
    enum DISGUISE_STATE
    {
        STATE_OFF = 0,
        STATE_ON = 1
    };

    WeaponDisguise(const WeaponDisguiseClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponDisguise();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual void Select(unsigned int uFlags, bool bOffhand);
    virtual void Deselect(unsigned int uFlags, bool bOffhand);
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

protected:
    virtual void EnterIdle();
    virtual bool UpdateIdle(float fDeltaTime);
    virtual void ExitIdle();
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual bool CheckFire() const;
    virtual bool FireDone() const;
    virtual bool CheckEmpty() const;
    virtual bool CheckReload() const;

    void InputToggleDisguise();
    void WhoTwiggedMe();

public:

    void StartRaisingDisguise();
    void FinishRaisingDisguise();
    void StartDroppingDisguise();
    void FinishDroppingDisguise();

    static PblHash rttiHashWeaponDisguise;
    static char rttiNameWeaponDisguise[];

    GameModel* mSavedOwnerModel;
    Team* mOtherTeam;
    DISGUISE_STATE m_eState;
};
  
 
 
 
 
 
