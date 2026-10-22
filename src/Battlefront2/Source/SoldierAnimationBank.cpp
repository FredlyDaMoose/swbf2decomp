#include <string.h>

#include "SoldierAnimationBank.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"

// GLOBAL: SWBF2 0x007e9430
int SoldierAnimationBank::s_iNumBanks = 1;

// GLOBAL: SWBF2 0x007e904c
int SoldierAnimationBank::s_iNumLowResBanks = 1;

// GLOBAL: SWBF2 0x007e9068
int SoldierAnimationBank::s_iNumWeapons = 5;

// GLOBAL: SWBF2 0x007e906c
int SoldierAnimationBank::s_iNumMaps = 5;

// GLOBAL: SWBF2 0x01eaf098
int SoldierAnimationBank::s_iNumAnimations;

// GLOBAL: SWBF2 0x01eaf4d8
int SoldierAnimationBank::s_iNumCombos;

// GLOBAL: SWBF2 0x01eaf094
int SoldierAnimationBank::s_iNumComboAnimations;

// GLOBAL: SWBF2 0x01eaf09c
int SoldierAnimationBank::s_iNumComboAnimationsInPool;

// GLOBAL: SWBF2 0x007e9440
SoldierAnimationBank::Bank SoldierAnimationBank::s_aBank[MAX_BANKS] =
{
    { "human", 0x714daa7au, HUMAN, HUMANLZ },
};

// GLOBAL: SWBF2 0x007e97f0
SoldierAnimationBank::LowResBank SoldierAnimationBank::s_aLowResBank[MAX_LOWRESBANKS] =
{
    { "humanlz", 0x4a159408u, HUMANLZ },
};

// GLOBAL: SWBF2 0x007e9070
SoldierAnimationBank::Weapon SoldierAnimationBank::s_aWeapon[MAX_WEAPONS] =
{
    { "rifle",   0x0bb9a971u, INVALID_COMBO, RIFLE, 1 },
    { "bazooka", 0xed24a788u, INVALID_COMBO, RIFLE, 1 },
    { "tool",    0x51750ec7u, INVALID_COMBO, RIFLE, 0 },
    { "pistol",  0x12019e56u, INVALID_COMBO, TOOL,  1 },
    { "melee",   0x79ca637bu, INVALID_COMBO, TOOL,  0 },
};

// GLOBAL: SWBF2 0x007e9700
SoldierAnimationBank::Map SoldierAnimationBank::s_aMap[MAX_MAPS] =
{
    { HUMAN, RIFLE },
    { HUMAN, BAZOOKA },
    { HUMAN, TOOL },
    { HUMAN, PISTOL },
    { HUMAN, MELEE },
};

// GLOBAL: SWBF2 0x01eaf4e0
SoldierAnimationBank::Animation SoldierAnimationBank::s_aAnimation[MAX_ANIMATIONS];

// GLOBAL: SWBF2 0x01eaf650
SoldierAnimationBank::Combo SoldierAnimationBank::s_aCombo[MAX_COMBOS];

// GLOBAL: SWBF2 0x01eaf0a0
SoldierAnimationBank::Animation SoldierAnimationBank::s_aComboAnimation[MAX_COMBO_ANIMATIONS];

// GLOBAL: SWBF2 0x01eaf710
SoldierAnimationBank::COMBO_ANIMATION SoldierAnimationBank::s_aeComboAnimationPool[MAX_COMBO_ANIMATION_POOL_SIZE];

static inline const char* _SkipPrefix(const char* pcName)
{
    const char* pc = strchr(pcName, '_');
    return pc ? pc + 1 : pcName;
}

// FUNCTION: SWBF2 0x0063c3c0
SoldierAnimationBank::Bank* SoldierAnimationBank::_GetBank(BANK eBank)
{
    return &s_aBank[eBank];
}

// FUNCTION: SWBF2 0x0063c3d0
SoldierAnimationBank::LowResBank* SoldierAnimationBank::_GetLowResBank(LOWRESBANK eLowResBank)
{
    return &s_aLowResBank[eLowResBank];
}

// FUNCTION: SWBF2 0x0063c3e0
SoldierAnimationBank::Weapon* SoldierAnimationBank::_GetWeapon(WEAPON eWeapon)
{
    return &s_aWeapon[eWeapon];
}

// FUNCTION: SWBF2 0x0063c3f0
SoldierAnimationBank::Map* SoldierAnimationBank::_GetMap(MAP eMap)
{
    return &s_aMap[eMap];
}

// FUNCTION: SWBF2 0x0063c400
SoldierAnimationBank::Animation* SoldierAnimationBank::_GetAnimation(ANIMATION eAnimation)
{
    return &s_aAnimation[eAnimation];
}

// FUNCTION: SWBF2 0x0063c410
SoldierAnimationBank::Combo* SoldierAnimationBank::_GetCombo(COMBO eCombo)
{
    return &s_aCombo[eCombo];
}

// FUNCTION: SWBF2 0x0063c420
SoldierAnimationBank::Animation* SoldierAnimationBank::_GetComboAnimation(COMBO_ANIMATION eAnimation)
{
    return &s_aComboAnimation[eAnimation];
}

// FUNCTION: SWBF2 0x0063c430
SoldierAnimationBank::BANK SoldierAnimationBank::FindBank(unsigned int uiNameHash)
{
    for (int i = 0; i < s_iNumBanks; ++i)
    {
        if (s_aBank[i].uiNameHash == uiNameHash)
        {
            return (BANK)i;
        }
    }
    return INVALID_BANK;
}

// FUNCTION: SWBF2 0x0063c460
SoldierAnimationBank::BANK SoldierAnimationBank::AddBank(const char* pcName, const char* pcParent)
{
    unsigned int uiNameHash = PblHash(pcName);
    BANK eBank = FindBank(uiNameHash);
    if (eBank != INVALID_BANK)
    {
        return eBank;
    }

    BANK eParent = HUMAN;
    if (pcParent)
    {
        unsigned int uiParentHash = PblHash(pcParent);
        if (uiParentHash == uiNameHash || uiParentHash == 0x20FD0E45u  )
        {
            eParent = (BANK)s_iNumBanks;
        }
        else
        {
            eParent = FindBank(uiParentHash);
            if (eParent == INVALID_BANK)
            {
                return INVALID_BANK;
            }
        }
    }

    if (s_iNumBanks >= MAX_BANKS)
    {
        return INVALID_BANK;
    }

    eBank = (BANK)s_iNumBanks++;
    Bank* pBank = _GetBank(eBank);
    strlcpy(pBank->szName, pcName, NAME_LEN_MAX);
    pBank->uiNameHash = uiNameHash;
    pBank->eParent = eParent;
    pBank->szName[NAME_LEN_MAX - 1] = 0;
    pBank->eLowResBank = HUMANLZ;
    return eBank;
}

// FUNCTION: SWBF2 0x0063c510
SoldierAnimationBank::LOWRESBANK SoldierAnimationBank::FindLowResBank(unsigned int uiNameHash)
{
    for (int i = 0; i < s_iNumLowResBanks; ++i)
    {
        if (s_aLowResBank[i].uiNameHash == uiNameHash)
        {
            return (LOWRESBANK)i;
        }
    }
    return INVALID_LOWRESBANK;
}

// FUNCTION: SWBF2 0x0063c540
SoldierAnimationBank::LOWRESBANK SoldierAnimationBank::AddLowResBank(BANK eBank, const char* pcName, const char* pcParent)
{
    if (eBank < 0 || eBank >= s_iNumBanks)
    {
        return INVALID_LOWRESBANK;
    }

    unsigned int uiNameHash = PblHash(pcName);
    LOWRESBANK eLowResBank = FindLowResBank(uiNameHash);
    if (eLowResBank != INVALID_LOWRESBANK)
    {
        _GetBank(eBank)->eLowResBank = eLowResBank;
        return eLowResBank;
    }

    LOWRESBANK eParent = HUMANLZ;
    if (pcParent)
    {
        unsigned int uiParentHash = PblHash(pcParent);
        if (uiParentHash == uiNameHash || uiParentHash == 0x20FD0E45u  )
        {
            eParent = (LOWRESBANK)s_iNumLowResBanks;
        }
        else
        {
            eParent = FindLowResBank(uiParentHash);
            if (eParent == INVALID_LOWRESBANK)
            {
                return INVALID_LOWRESBANK;
            }
        }
    }

    if (s_iNumLowResBanks >= MAX_LOWRESBANKS)
    {
        return INVALID_LOWRESBANK;
    }

    eLowResBank = (LOWRESBANK)s_iNumLowResBanks++;
    LowResBank* pLowResBank = _GetLowResBank(eLowResBank);
    strlcpy(pLowResBank->szName, pcName, NAME_LEN_MAX);
    pLowResBank->uiNameHash = uiNameHash;
    pLowResBank->szName[NAME_LEN_MAX - 1] = 0;
    pLowResBank->eParent = eParent;
    _GetBank(eBank)->eLowResBank = eLowResBank;
    return eLowResBank;
}

// FUNCTION: SWBF2 0x0063c630
SoldierAnimationBank::WEAPON SoldierAnimationBank::GetWeaponRoot(WEAPON eWeapon)
{
    while (eWeapon >= NUM_STANDARD_WEAPONS)
    {
        eWeapon = _GetWeapon(eWeapon)->eParent;
    }
    return eWeapon;
}

SoldierAnimationBank::WEAPON SoldierAnimationBank::GetWeaponParent(WEAPON eWeapon)
{
    return _GetWeapon(eWeapon)->eParent;
}

SoldierAnimationBank::BANK SoldierAnimationBank::GetBankParent(BANK eBank)
{
    return _GetBank(eBank)->eParent;
}

// FUNCTION: SWBF2 0x0063c650
SoldierAnimationBank::WEAPON SoldierAnimationBank::FindWeapon(const char* pcName)
{
    return FindWeapon(PblHash(pcName));
}

// FUNCTION: SWBF2 0x0063c680
SoldierAnimationBank::WEAPON SoldierAnimationBank::FindWeapon(unsigned int uiNameHash)
{
    return FindWeaponCombo(uiNameHash, INVALID_COMBO);
}

// FUNCTION: SWBF2 0x0063c670
SoldierAnimationBank::WEAPON __cdecl ForwardFindWeaponByHash(unsigned int uiNameHash)
{
    return SoldierAnimationBank::FindWeapon(uiNameHash);
}

// FUNCTION: SWBF2 0x0063c690
SoldierAnimationBank::WEAPON SoldierAnimationBank::FindWeaponCombo(unsigned int uiNameHash, COMBO eCombo)
{
    for (int i = 0; i < s_iNumWeapons; ++i)
    {
        if (s_aWeapon[i].uiNameHash == uiNameHash && s_aWeapon[i].eCombo == eCombo)
        {
            return (WEAPON)i;
        }
    }
    return INVALID_WEAPON;
}

// FUNCTION: SWBF2 0x0063c6c0
SoldierAnimationBank::WEAPON SoldierAnimationBank::_FindWeaponAnyCombo(unsigned int uiNameHash)
{
    for (int i = 0; i < s_iNumWeapons; ++i)
    {
        if (s_aWeapon[i].uiNameHash == uiNameHash)
        {
            return (WEAPON)i;
        }
    }
    return INVALID_WEAPON;
}

// FUNCTION: SWBF2 0x0063c6f0
SoldierAnimationBank::WEAPON SoldierAnimationBank::AddWeapon(const char* pcName, const char* pcParent, const char* pcAlertState)
{
    const char* pcShortName = _SkipPrefix(pcName);
    unsigned int uiNameHash = PblHash(pcShortName);
    WEAPON eWeapon = FindWeapon(uiNameHash);
    if (eWeapon != INVALID_WEAPON)
    {
        return eWeapon;
    }

    if (!pcParent)
    {
        return INVALID_WEAPON;
    }

    WEAPON eParent;
    unsigned int uiParentHash = PblHash(_SkipPrefix(pcParent));
    if (uiParentHash == uiNameHash || uiParentHash == 0x20FD0E45u  )
    {
        eParent = (WEAPON)s_iNumWeapons;
    }
    else
    {
        eParent = FindWeapon(uiParentHash);
        if (eParent == INVALID_WEAPON)
        {
            return INVALID_WEAPON;
        }
    }

    if (s_iNumWeapons >= MAX_WEAPONS)
    {
        return INVALID_WEAPON;
    }

    bool bHasAlertState;
    if (pcAlertState && _stricmp(pcAlertState, "noalert") == 0)
    {
        bHasAlertState = false;
    }
    else if (pcAlertState && _stricmp(pcAlertState, "alert") == 0)
    {
        bHasAlertState = true;
    }
    else
    {
        bHasAlertState = _GetWeapon(eParent)->bHasAlertState != 0;
    }

    eWeapon = (WEAPON)s_iNumWeapons++;
    Weapon* pWeapon = _GetWeapon(eWeapon);
    strlcpy(pWeapon->szName, pcShortName, NAME_LEN_MAX);
    pWeapon->uiNameHash = uiNameHash;
    pWeapon->eParent = eParent;
    pWeapon->szName[NAME_LEN_MAX - 1] = 0;
    pWeapon->eCombo = INVALID_COMBO;
    pWeapon->bHasAlertState = bHasAlertState;
    return eWeapon;
}

// FUNCTION: SWBF2 0x0063c840
SoldierAnimationBank::WEAPON SoldierAnimationBank::AddWeaponCombo(const char* pcName, const char* pcParent, const char* pcCombo)
{
    if (!pcName || !pcCombo)
    {
        return INVALID_WEAPON;
    }

    const char* pcShortName = _SkipPrefix(pcName);
    unsigned int uiNameHash = PblHash(pcShortName);
    COMBO eCombo = AddCombo(PblHash(pcCombo));
    if (eCombo == INVALID_COMBO)
    {
        return INVALID_WEAPON;
    }

    WEAPON eWeapon = FindWeaponCombo(uiNameHash, eCombo);
    if (eWeapon != INVALID_WEAPON)
    {
        return GetWeaponRoot(eWeapon) == MELEE ? eWeapon : INVALID_WEAPON;
    }

    if (!pcParent)
    {
        return INVALID_WEAPON;
    }

    WEAPON eParent = _FindWeaponAnyCombo(PblHash(_SkipPrefix(pcParent)));
    if (eParent == INVALID_WEAPON || GetWeaponRoot(eParent) != MELEE)
    {
        return INVALID_WEAPON;
    }

    if (s_iNumWeapons >= MAX_WEAPONS)
    {
        return INVALID_WEAPON;
    }

    eWeapon = (WEAPON)s_iNumWeapons++;
    Weapon* pWeapon = _GetWeapon(eWeapon);
    strlcpy(pWeapon->szName, pcShortName, NAME_LEN_MAX);
    pWeapon->uiNameHash = uiNameHash;
    pWeapon->eCombo = eCombo;
    pWeapon->eParent = eParent;
    pWeapon->szName[NAME_LEN_MAX - 1] = 0;
    pWeapon->bHasAlertState = 0;
    return eWeapon;
}

// FUNCTION: SWBF2 0x0063c970
SoldierAnimationBank::MAP SoldierAnimationBank::FindMap(BANK eBank, WEAPON eWeapon)
{
    for (int i = 0; i < s_iNumMaps; ++i)
    {
        if (s_aMap[i].eBank == eBank && s_aMap[i].eWeapon == eWeapon)
        {
            return (MAP)i;
        }
    }
    return INVALID_MAP;
}

// FUNCTION: SWBF2 0x0063c9a0
SoldierAnimationBank::MAP SoldierAnimationBank::AddMap(BANK eBank, WEAPON eWeapon)
{
    MAP eMap = FindMap(eBank, eWeapon);
    if (eMap != INVALID_MAP)
    {
        return eMap;
    }

    if (s_iNumMaps >= MAX_MAPS)
    {
        return INVALID_MAP;
    }

    eMap = (MAP)s_iNumMaps++;
    Map* pMap = _GetMap(eMap);
    pMap->eBank = eBank;
    pMap->eWeapon = eWeapon;
    return eMap;
}

// FUNCTION: SWBF2 0x0063c9e0
SoldierAnimationBank::BANK SoldierAnimationBank::GetFirstBank()
{
    return HUMAN;
}

// FUNCTION: SWBF2 0x0063c9f0
SoldierAnimationBank::BANK SoldierAnimationBank::GetNextBank(BANK eBank)
{
    BANK eParent = eBank;
    for (;;)
    {
        for (int i = eBank + 1; i < s_iNumBanks; ++i)
        {
            if (_GetBank((BANK)i)->eParent == eParent)
            {
                return (BANK)i;
            }
        }
        eBank = eParent;
        eParent = _GetBank(eBank)->eParent;
        if (eParent == eBank)
        {
            break;
        }
    }

    for (int i = eBank + 1; i < s_iNumBanks; ++i)
    {
        if (_GetBank((BANK)i)->eParent == i)
        {
            return (BANK)i;
        }
    }
    return INVALID_BANK;
}

// FUNCTION: SWBF2 0x0063ca50
SoldierAnimationBank::WEAPON SoldierAnimationBank::GetFirstWeapon()
{
    return RIFLE;
}

// FUNCTION: SWBF2 0x0063ca60
SoldierAnimationBank::WEAPON SoldierAnimationBank::GetNextWeapon(WEAPON eWeapon)
{
    WEAPON eParent = eWeapon;
    for (;;)
    {
        for (int i = eWeapon + 1; i < s_iNumWeapons; ++i)
        {
            if (_GetWeapon((WEAPON)i)->eParent == eParent)
            {
                return (WEAPON)i;
            }
        }
        eWeapon = eParent;
        eParent = _GetWeapon(eWeapon)->eParent;
        if (eParent == eWeapon)
        {
            break;
        }
    }

    for (int i = eWeapon + 1; i < s_iNumWeapons; ++i)
    {
        if (_GetWeapon((WEAPON)i)->eParent == i)
        {
            return (WEAPON)i;
        }
    }
    return INVALID_WEAPON;
}

// FUNCTION: SWBF2 0x0063cac0
SoldierAnimationBank::ANIMATION SoldierAnimationBank::FindAnimation(unsigned int uiNameHash)
{
    for (int i = 0; i < s_iNumAnimations; ++i)
    {
        if (s_aAnimation[i].uiNameHash == uiNameHash)
        {
            return (ANIMATION)i;
        }
    }
    return INVALID_ANIMATION;
}

// FUNCTION: SWBF2 0x0063caf0
SoldierAnimationBank::ANIMATION SoldierAnimationBank::AddAnimation(const char* pcName)
{
    unsigned int uiNameHash = PblHash(pcName);
    ANIMATION eAnimation = FindAnimation(uiNameHash);
    if (eAnimation != INVALID_ANIMATION)
    {
        return eAnimation;
    }

    if (s_iNumAnimations >= MAX_ANIMATIONS)
    {
        return INVALID_ANIMATION;
    }

    eAnimation = (ANIMATION)s_iNumAnimations++;
    Animation* pAnimation = _GetAnimation(eAnimation);
    strlcpy(pAnimation->szName, pcName, NAME_LEN_MAX);
    pAnimation->uiNameHash = uiNameHash;
    pAnimation->szName[NAME_LEN_MAX - 1] = 0;
    return eAnimation;
}

// FUNCTION: SWBF2 0x0063cb60
SoldierAnimationBank::COMBO SoldierAnimationBank::FindCombo(unsigned int uiNameHash)
{
    for (int i = 0; i < s_iNumCombos; ++i)
    {
        if (s_aCombo[i].uiNameHash == uiNameHash)
        {
            return (COMBO)i;
        }
    }
    return INVALID_COMBO;
}

// FUNCTION: SWBF2 0x0063cb90
unsigned int SoldierAnimationBank::GetComboAnimationIndex(COMBO eCombo, COMBO_ANIMATION eAnimation)
{
    Combo* pCombo = _GetCombo(eCombo);
    unsigned int i;
    for (i = 0; i < pCombo->uiNumAnimations; ++i)
    {
        if (pCombo->aeAnimation[i] == eAnimation)
        {
            break;
        }
    }
    return i;
}

// FUNCTION: SWBF2 0x0063cbc0
SoldierAnimationBank::COMBO SoldierAnimationBank::AddCombo(unsigned int uiNameHash)
{
    COMBO eCombo = FindCombo(uiNameHash);
    if (eCombo != INVALID_COMBO)
    {
        return eCombo;
    }

    if (s_iNumCombos >= MAX_COMBOS)
    {
        return INVALID_COMBO;
    }

    eCombo = (COMBO)s_iNumCombos++;
    Combo* pCombo = _GetCombo(eCombo);
    pCombo->uiNameHash = uiNameHash;
    pCombo->uiNumAnimations = 0;
    pCombo->aeAnimation = 0;
    return eCombo;
}

// FUNCTION: SWBF2 0x0063cc00
SoldierAnimationBank::COMBO_ANIMATION SoldierAnimationBank::FindComboAnimation(unsigned int uiNameHash)
{
    for (int i = 0; i < s_iNumComboAnimations; ++i)
    {
        if (s_aComboAnimation[i].uiNameHash == uiNameHash)
        {
            return (COMBO_ANIMATION)i;
        }
    }
    return INVALID_COMBO_ANIMATION;
}

// FUNCTION: SWBF2 0x0063cc30
SoldierAnimationBank::COMBO_ANIMATION SoldierAnimationBank::AddComboAnimation(COMBO eCombo, const char* pcName)
{
    unsigned int uiNameHash = PblHash(pcName);
    Combo* pCombo = _GetCombo(eCombo);
    for (unsigned int i = 0; i < pCombo->uiNumAnimations; ++i)
    {
        if (_GetComboAnimation(pCombo->aeAnimation[i])->uiNameHash == uiNameHash)
        {
            return pCombo->aeAnimation[i];
        }
    }

    if (s_iNumComboAnimationsInPool >= MAX_COMBO_ANIMATION_POOL_SIZE)
    {
        return INVALID_COMBO_ANIMATION;
    }

    COMBO_ANIMATION* peSlot = &s_aeComboAnimationPool[s_iNumComboAnimationsInPool];
    if (pCombo->uiNumAnimations == 0)
    {
        pCombo->aeAnimation = peSlot;
    }
    else if (peSlot != &pCombo->aeAnimation[pCombo->uiNumAnimations])
    {
        return INVALID_COMBO_ANIMATION;
    }

    COMBO_ANIMATION eAnimation = FindComboAnimation(uiNameHash);
    if (eAnimation == INVALID_COMBO_ANIMATION)
    {
        if (s_iNumComboAnimations >= MAX_COMBO_ANIMATIONS)
        {
            return INVALID_COMBO_ANIMATION;
        }

        eAnimation = (COMBO_ANIMATION)s_iNumComboAnimations++;
        Animation* pAnimation = _GetComboAnimation(eAnimation);
        strlcpy(pAnimation->szName, pcName, NAME_LEN_MAX);
        pAnimation->szName[NAME_LEN_MAX - 1] = 0;
        pAnimation->uiNameHash = uiNameHash;
    }

    ++s_iNumComboAnimationsInPool;
    ++pCombo->uiNumAnimations;
    *peSlot = eAnimation;
    return eAnimation;
}

// FUNCTION: SWBF2 0x0063cd20
unsigned int SoldierAnimationBank::AddComboAnimation(COMBO eCombo, COMBO_ANIMATION eAnimation)
{
    Combo* pCombo = _GetCombo(eCombo);
    unsigned int uiIndex = GetComboAnimationIndex(eCombo, eAnimation);
    if (uiIndex < pCombo->uiNumAnimations)
    {
        return uiIndex;
    }

    if (s_iNumComboAnimationsInPool >= MAX_COMBO_ANIMATION_POOL_SIZE)
    {
        return pCombo->uiNumAnimations;
    }

    COMBO_ANIMATION* peSlot = &s_aeComboAnimationPool[s_iNumComboAnimationsInPool];
    if (pCombo->uiNumAnimations == 0)
    {
        pCombo->aeAnimation = peSlot;
    }
    else if (peSlot != &pCombo->aeAnimation[pCombo->uiNumAnimations])
    {
        return pCombo->uiNumAnimations;
    }

    ++s_iNumComboAnimationsInPool;
    pCombo->uiNumAnimations = uiIndex + 1;
    *peSlot = eAnimation;
    return uiIndex;
}

// FUNCTION: SWBF2 0x0063cdb0
void SoldierAnimationBank::Reset()
{
    s_iNumBanks = 1;
    s_iNumWeapons = 5;
    s_iNumMaps = 5;
    s_iNumAnimations = 0;
    s_iNumCombos = 0;
    s_iNumComboAnimations = 0;
    s_iNumComboAnimationsInPool = 0;
}
