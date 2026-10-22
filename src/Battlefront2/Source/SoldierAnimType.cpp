#include <stdio.h>

#include "SoldierAnimType.h"

namespace SoldierAnimType
{

static const int kFirstMovementAnimIndex = NUM_ACTION_ANIMATIONS;
static const int kFirstWeaponAnimIndex =
    kFirstMovementAnimIndex + NUM_MOVEMENT_ANIMATIONS * NUM_POSTURES;
static const int kFirstMeleeAnimIndex =
    kFirstWeaponAnimIndex + NUM_WEAPON_ANIMATIONS * NUM_STANDARD_POSTURES;
static const int kEndAnimIndex =
    kFirstMeleeAnimIndex + SoldierAnimationBank::MAX_COMBO_ANIMATIONS;

// GLOBAL: SWBF2 0x007aed08
static const char* postureTable[NUM_POSTURES] = {
    "stand", "crouch", "prone", "standalert", "crouchalert", "pronealert",
};

// GLOBAL: SWBF2 0x007aed20
static const char* weaponAnimationTable[NUM_WEAPON_ANIMATIONS] = {
    "shoot", "shoot2", "shoot_secondary", "shoot_secondary2", "charge", "reload",
};

// GLOBAL: SWBF2 0x007aed38
static const char* actionAnimationTable[NUM_ACTION_ANIMATIONS] = {
    "sprint",
    "jump",
    "jump_forward",
    "jump_backward",
    "jump_left",
    "jump_right",
    "landsoft",
    "landhard",
    "fall",
    "thrown_flail",
    "thrown_flyingfront",
    "thrown_flyingback",
    "thrown_flyingleft",
    "thrown_flyingright",
    "thrown_tumblefront",
    "thrown_tumbleback",
    "thrown_bouncefrontsoft",
    "thrown_bouncebacksoft",
    "thrown_landfrontsoft",
    "thrown_landbacksoft",
    "stand_getupfront",
    "stand_getupback",
    "thrown_restfrontsoft",
    "thrown_restbacksoft",
    "diveforward",
    "jetpack_hover",
    "stand_deadhero",
    "prone_getup_stand",
    "crouch_getdown_prone",
    "prone_getup_crouch",
    "stand_death_forward",
    "stand_death_backward",
    "stand_death_left",
    "stand_death_right",
    "stand_hitfront",
    "stand_hitback",
    "stand_hitleft",
    "stand_hitright",
};

// GLOBAL: SWBF2 0x007aedd0
static const char* movementAnimationTable[NUM_MOVEMENT_ANIMATIONS] = {
    "idle_emote",
    "idle_checkweapon",
    "idle_lookaround",
    "turnleft",
    "turnright",
    "walkforward",
    "runforward",
    "walkbackward",
    "runbackward",
    "walkright",
    "runright",
    "walkleft",
    "runleft",
};

// GLOBAL: SWBF2 0x0078b4d4
static const char* s_asz_ScopeName[NUM_SCOPES] = {
    "_upper",
    "_lower",
};

// FUNCTION: SWBF2 0x0064a3d0
unsigned char GetAnimIndex(ActionAnimation anim)
{
    return (unsigned char)anim;
}

// FUNCTION: SWBF2 0x0064a3e0
unsigned char GetAnimIndex(Posture posture, MovementAnimation anim)
{
    return (unsigned char)(kFirstMovementAnimIndex + posture * NUM_MOVEMENT_ANIMATIONS + anim);
}

// FUNCTION: SWBF2 0x0064a3f0
unsigned char GetAnimIndex(Posture posture, WeaponAnimation anim)
{
    return (unsigned char)(kFirstWeaponAnimIndex + posture * NUM_WEAPON_ANIMATIONS + anim);
}

// FUNCTION: SWBF2 0x0064a400
unsigned char GetAnimIndex(SoldierAnimationBank::COMBO_ANIMATION anim)
{
    return (unsigned char)(kFirstMeleeAnimIndex + anim);
}

// FUNCTION: SWBF2 0x0064a410
bool IsActionAnimIndex(unsigned char animIndex)
{
    return animIndex < kFirstMovementAnimIndex;
}

// FUNCTION: SWBF2 0x0064a420
bool IsMovementAnimIndex(unsigned char animIndex)
{
    return animIndex >= kFirstMovementAnimIndex &&
           animIndex - kFirstMovementAnimIndex < NUM_MOVEMENT_ANIMATIONS * NUM_POSTURES;
}

// FUNCTION: SWBF2 0x0064a440
bool IsWeaponAnimIndex(unsigned char animIndex)
{
    return animIndex >= kFirstWeaponAnimIndex &&
           animIndex - kFirstWeaponAnimIndex < NUM_WEAPON_ANIMATIONS * NUM_STANDARD_POSTURES;
}

// FUNCTION: SWBF2 0x0064a460
bool IsWeaponMeleeAnimIndex(unsigned char animIndex)
{
    return animIndex >= kFirstMeleeAnimIndex &&
           animIndex - kFirstMeleeAnimIndex < SoldierAnimationBank::MAX_COMBO_ANIMATIONS;
}

// FUNCTION: SWBF2 0x0064a480
Posture GetPostureFromAnimIndex(unsigned char animIndex)
{
    if (IsMovementAnimIndex(animIndex))
    {
        return (Posture)((animIndex - kFirstMovementAnimIndex) / NUM_MOVEMENT_ANIMATIONS);
    }
    if (IsWeaponAnimIndex(animIndex))
    {
        return (Posture)((animIndex - kFirstWeaponAnimIndex) / NUM_WEAPON_ANIMATIONS);
    }
    return STAND;
}

// FUNCTION: SWBF2 0x0064a4d0
int GetAnimFromAnimIndex(unsigned char animIndex)
{
    if (animIndex < kFirstMovementAnimIndex)
    {
        return animIndex;
    }
    if (animIndex < kFirstWeaponAnimIndex)
    {
        return (animIndex - kFirstMovementAnimIndex) % NUM_MOVEMENT_ANIMATIONS;
    }
    if (animIndex < kFirstMeleeAnimIndex)
    {
        return (animIndex - kFirstWeaponAnimIndex) % NUM_WEAPON_ANIMATIONS;
    }
    if (animIndex < kEndAnimIndex)
    {
        return animIndex - kFirstMeleeAnimIndex;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0064a520
const char* GetActionAnimationName(ActionAnimation anim)
{
    return actionAnimationTable[anim];
}

// FUNCTION: SWBF2 0x0064a530
const char* GetMovementAnimationName(MovementAnimation anim, Posture posture)
{
    if (anim == IDLE_TAKEKNEE && (posture == CROUCH || posture == CROUCH_ALERT))
    {
        return "idle_takeknee";
    }
    return movementAnimationTable[anim];
}

// FUNCTION: SWBF2 0x0064a550
const char* GetWeaponAnimationName(WeaponAnimation anim)
{
    return weaponAnimationTable[anim];
}

// FUNCTION: SWBF2 0x0064a560
const char* GetWeaponMeleeAnimationName(SoldierAnimationBank::COMBO_ANIMATION anim)
{
    if (anim < SoldierAnimationBank::GetNumComboAnimations())
    {
        return SoldierAnimationBank::GetComboAnimationName(anim);
    }
    return "[INVALID_COMBO_ANIMATION]";
}

// FUNCTION: SWBF2 0x0064a580
const char* GetPostureName(Posture posture)
{
    return postureTable[posture];
}

// FUNCTION: SWBF2 0x0064a590
const char* GetScopeExtension(Scope scope)
{
    return s_asz_ScopeName[scope];
}

// FUNCTION: SWBF2 0x0064a5a0
const char* AnimIndexToName(unsigned char animIndex, Scope scope)
{
    static char s_szName[256];

    int length;
    if (IsActionAnimIndex(animIndex))
    {
        length = _snprintf(s_szName, 255,
                           GetActionAnimationName((ActionAnimation)GetAnimFromAnimIndex(animIndex)));
    }
    else if (IsMovementAnimIndex(animIndex))
    {
        Posture posture = GetPostureFromAnimIndex(animIndex);
        length = _snprintf(s_szName, 255, "%s_%s", GetPostureName(posture),
                           GetMovementAnimationName(
                               (MovementAnimation)GetAnimFromAnimIndex(animIndex), posture));
    }
    else if (IsWeaponAnimIndex(animIndex))
    {
        length = _snprintf(s_szName, 255, "%s_%s",
                           GetPostureName(GetPostureFromAnimIndex(animIndex)),
                           GetWeaponAnimationName((WeaponAnimation)GetAnimFromAnimIndex(animIndex)));
    }
    else if (IsWeaponMeleeAnimIndex(animIndex))
    {
        length = _snprintf(s_szName, 255, "%s",
                           GetWeaponMeleeAnimationName(
                               (SoldierAnimationBank::COMBO_ANIMATION)GetAnimFromAnimIndex(animIndex)));
    }
    else
    {
        _snprintf(s_szName, 255, "[INVALID]");
        return s_szName;
    }

    s_szName[255] = 0;
    if (scope < NUM_SCOPES)
    {
        sprintf(&s_szName[length], GetScopeExtension(scope));
    }
    return s_szName;
}

}
