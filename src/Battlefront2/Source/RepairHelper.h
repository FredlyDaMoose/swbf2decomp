#pragma once

#include <stddef.h>

#include "GameObject.h"
#include "PebbleFL/Common/PblHandle.h"

class GameObject;
class UnitAgent;
class UnitController;
class Controllable;

struct AidType
{
    AidType() {}
    AidType(HealthType t, bool bAmmo)
        : type(t)
        , ammo(bAmmo)
    {
    }

    HealthType type;
    unsigned int ammo;
};

// VTABLE: SWBF2 0x007adfec
class RepairHelper
{
public:
    RepairHelper(UnitAgent* pAgent);
    virtual ~RepairHelper();

    bool CheckForRecipientAndInit();
    bool Update(float dt);
    const char* NFINAL_GetStateString(int iState) const;

protected:
    Controllable* GetMe();
    UnitController* GetCtrl();
    bool SetAidWpn(bool* pbChanged);
    void CalcMyAidTypes(unsigned int* pHealth, unsigned int* pAmmo);
    void FindPowerupsNearby(const GameObject* pObj, unsigned int* pHealth, unsigned int* pAmmo);
    void FindAllAidNeeds(const GameObject* pObj, unsigned int health, unsigned int ammo,
                         bool b, bool* pbNeedHealth, bool* pbNeedAmmo);
    bool FindAidNeed(const GameObject* pObj, unsigned int health, unsigned int ammo,
                     AidType* pAid, bool b);
    bool FireRepairWeapon(float dt, const GameObject* pTarget);

public:
    UnitAgent* mAgent;
    AidType mAidType;
    float mContactTime;
    bool mPathing;
    float mPathTime;
    float mHealth;
    float mSearchTime;
    PblHandle<GameObject> mAidTarget;
    float mCheckForRecipientTime;
};

 
 
 
 
