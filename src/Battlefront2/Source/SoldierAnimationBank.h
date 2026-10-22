#pragma once

class SoldierAnimationBank
{
public:

    enum ANIMATION
    {
        INVALID_ANIMATION = -1,
        MAX_ANIMATIONS = 10
    };

    enum BANK
    {
        INVALID_BANK = -1,
        HUMAN = 0,
        NUM_STANDARD_BANKS = 1,
        MAX_BANKS = 16
    };

    enum COMBO_ANIMATION
    {
        INVALID_COMBO_ANIMATION = -1,
        MAX_COMBO_ANIMATIONS = 30
    };

    enum LOWRESBANK
    {
        INVALID_LOWRESBANK = -1,
        HUMANLZ = 0,
        NUM_STANDARD_LOWRESBANKS = 1,
        MAX_LOWRESBANKS = 12
    };

    enum WEAPON
    {
        INVALID_WEAPON = -1,
        RIFLE = 0,
        BAZOOKA = 1,
        TOOL = 2,
        PISTOL = 3,
        MELEE = 4,
        NUM_STANDARD_WEAPONS = 5,
        MAX_WEAPONS = 20
    };

    enum MAP
    {
        INVALID_MAP = -1,
        HUMAN_RIFLE = 0,
        HUMAN_BAZOOKA = 1,
        HUMAN_TOOL = 2,
        HUMAN_PISTOL = 3,
        HUMAN_MELEE = 4,
        NUM_STANDARD_MAPS = 5,
        MAX_MAPS = 30
    };

    enum COMBO
    {
        INVALID_COMBO = -1,
        MAX_COMBOS = 16
    };

    static const unsigned int NAME_LEN_MAX = 32;
    static const int MAX_COMBO_ANIMATION_POOL_SIZE = 256;

    struct Animation
    {
        char szName[32];
        unsigned int uiNameHash;
    };

    struct Bank
    {
        char szName[32];
        unsigned int uiNameHash;
        BANK eParent;
        LOWRESBANK eLowResBank;
    };

    struct Combo
    {
        unsigned int uiNameHash;
        unsigned int uiNumAnimations;
        COMBO_ANIMATION* aeAnimation;
    };

    struct LowResBank
    {
        char szName[32];
        unsigned int uiNameHash;
        LOWRESBANK eParent;
    };

    struct Map
    {
        BANK eBank;
        WEAPON eWeapon;
    };

    struct Weapon
    {
        char szName[32];
        unsigned int uiNameHash;
        COMBO eCombo;
        WEAPON eParent;
        unsigned int bHasAlertState;
    };

    static BANK FindBank(unsigned int uiNameHash);
    static BANK AddBank(const char* pcName, const char* pcParent);
    static LOWRESBANK FindLowResBank(unsigned int uiNameHash);
    static LOWRESBANK AddLowResBank(BANK eBank, const char* pcName, const char* pcParent);
    static WEAPON GetWeaponRoot(WEAPON eWeapon);
    static WEAPON FindWeapon(const char* pcName);
    static WEAPON FindWeapon(unsigned int uiNameHash);
    static WEAPON FindWeaponCombo(unsigned int uiNameHash, COMBO eCombo);
    static WEAPON AddWeapon(const char* pcName, const char* pcParent, const char* pcAlertState);
    static WEAPON AddWeaponCombo(const char* pcName, const char* pcParent, const char* pcCombo);
    static MAP FindMap(BANK eBank, WEAPON eWeapon);
    static MAP AddMap(BANK eBank, WEAPON eWeapon);

    static WEAPON GetMapWeapon(MAP eMap) { return _GetMap(eMap)->eWeapon; }

    static BANK GetMapBank(MAP eMap) { return _GetMap(eMap)->eBank; }
    static unsigned int GetBankNameHash(BANK eBank) { return _GetBank(eBank)->uiNameHash; }

    static COMBO GetWeaponCombo(WEAPON eWeapon) { return _GetWeapon(eWeapon)->eCombo; }
    static unsigned int GetComboNameHash(COMBO eCombo) { return _GetCombo(eCombo)->uiNameHash; }

    static LOWRESBANK GetBankLowResBank(BANK eBank) { return _GetBank(eBank)->eLowResBank; }
    static unsigned int GetLowResBankNameHash(LOWRESBANK eLowResBank)
    {
        return _GetLowResBank(eLowResBank)->uiNameHash;
    }

    static unsigned int GetComboNumAnimations(COMBO eCombo) { return _GetCombo(eCombo)->uiNumAnimations; }

    static const char* GetBankName(BANK eBank) { return _GetBank(eBank)->szName; }
    static const char* GetWeaponName(WEAPON eWeapon) { return _GetWeapon(eWeapon)->szName; }
    static unsigned int GetWeaponNameHash(WEAPON eWeapon) { return _GetWeapon(eWeapon)->uiNameHash; }
    static int GetNumWeapons() { return s_iNumWeapons; }
    static bool HasAlertState(WEAPON eWeapon) { return _GetWeapon(eWeapon)->bHasAlertState != 0; }

    static WEAPON GetWeaponParent(WEAPON eWeapon);

    static int GetNumBanks() { return s_iNumBanks; }
    static int GetNumMaps() { return s_iNumMaps; }
    static int GetNumAnimations() { return s_iNumAnimations; }
    static const char* GetAnimationName(ANIMATION eAnimation) { return _GetAnimation(eAnimation)->szName; }
    static BANK GetBankParent(BANK eBank);
    static BANK GetFirstBank();
    static BANK GetNextBank(BANK eBank);
    static WEAPON GetFirstWeapon();
    static WEAPON GetNextWeapon(WEAPON eWeapon);
    static ANIMATION FindAnimation(unsigned int uiNameHash);
    static ANIMATION AddAnimation(const char* pcName);
    static COMBO FindCombo(unsigned int uiNameHash);
    static COMBO AddCombo(unsigned int uiNameHash);
    static unsigned int GetComboAnimationIndex(COMBO eCombo, COMBO_ANIMATION eAnimation);
    static COMBO_ANIMATION FindComboAnimation(unsigned int uiNameHash);
    static COMBO_ANIMATION AddComboAnimation(COMBO eCombo, const char* pcName);
    static unsigned int AddComboAnimation(COMBO eCombo, COMBO_ANIMATION eAnimation);

    static int GetNumComboAnimations() { return s_iNumComboAnimations; }
    static unsigned int GetComboAnimationNameHash(COMBO_ANIMATION eAnimation)
    {
        return _GetComboAnimation(eAnimation)->uiNameHash;
    }

    static const char* GetComboAnimationName(COMBO_ANIMATION eAnimation)
    {
        return _GetComboAnimation(eAnimation)->szName;
    }

private:
    static Bank* _GetBank(BANK eBank);
    static LowResBank* _GetLowResBank(LOWRESBANK eLowResBank);
    static Weapon* _GetWeapon(WEAPON eWeapon);
    static Map* _GetMap(MAP eMap);
    static Animation* _GetAnimation(ANIMATION eAnimation);
    static Combo* _GetCombo(COMBO eCombo);
    static Animation* _GetComboAnimation(COMBO_ANIMATION eAnimation);
    static WEAPON _FindWeaponAnyCombo(unsigned int uiNameHash);

public:

    static void Reset();

    static int s_iNumBanks;
    static int s_iNumLowResBanks;
    static int s_iNumWeapons;
    static int s_iNumMaps;
    static int s_iNumAnimations;
    static int s_iNumCombos;
    static int s_iNumComboAnimations;
    static int s_iNumComboAnimationsInPool;
    static Bank s_aBank[MAX_BANKS];
    static LowResBank s_aLowResBank[MAX_LOWRESBANKS];
    static Weapon s_aWeapon[MAX_WEAPONS];
    static Map s_aMap[MAX_MAPS];
    static Animation s_aAnimation[MAX_ANIMATIONS];
    static Combo s_aCombo[MAX_COMBOS];
    static Animation s_aComboAnimation[MAX_COMBO_ANIMATIONS];
    static COMBO_ANIMATION s_aeComboAnimationPool[MAX_COMBO_ANIMATION_POOL_SIZE];
};

 
 
 
 
