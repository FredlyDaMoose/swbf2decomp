#pragma once

#include "SoldierAnimationBank.h"

namespace SoldierAnimType
{
    enum ActionAnimation
    {
        SPRINT = 0,
        JUMP_UP = 1,
        JUMP_FORWARD = 2,
        JUMP_BACKWARD = 3,
        JUMP_LEFT = 4,
        JUMP_RIGHT = 5,
        LAND_SOFT = 6,
        LAND_HARD = 7,
        JUMP_FLY = 8,
        FLY_FLAIL = 9,
        FLY_FORWARD = 10,
        FLY_BACKWARD = 11,
        FLY_LEFT = 12,
        FLY_RIGHT = 13,
        TUMBLE_FORWARD = 14,
        TUMBLE_BACKWARD = 15,
        BOUNCE_FRONT = 16,
        BOUNCE_BACK = 17,
        LAND_FRONT = 18,
        LAND_BACK = 19,
        GETUP_FRONT = 20,
        GETUP_BACK = 21,
        DEAD_BELLY = 22,
        DEAD_ASS = 23,
        DIVE = 24,
        JET = 25,
        DEAD_HERO_STUN = 26,
        PRONE_TO_STAND = 27,
        CROUCH_TO_PRONE = 28,
        PRONE_TO_CROUCH = 29,
        DIE_FORWARD = 30,
        DIE_BACKWARD = 31,
        DIE_LEFT = 32,
        DIE_RIGHT = 33,
        HIT_FRONT = 34,
        HIT_BACK = 35,
        HIT_LEFT = 36,
        HIT_RIGHT = 37,
        NUM_ACTION_ANIMATIONS = 38,
        MOVE = 39,
        WEAPON_ANIMATION = 40
    };

    enum MovementAnimation
    {
        IDLE = 0,
        IDLE_CHECK_WEAPON = 1,
        IDLE_LOOK_AROUND = 2,
        IDLE_TAKEKNEE = 2,
        TURN_LEFT = 3,
        TURN_RIGHT = 4,
        WALK_FORWARD = 5,
        RUN_FORWARD = 6,
        WALK_BACKWARD = 7,
        RUN_BACKWARD = 8,
        WALK_LEFT = 9,
        RUN_LEFT = 10,
        WALK_RIGHT = 11,
        RUN_RIGHT = 12,
        NUM_MOVEMENT_ANIMATIONS = 13,
        SLIDE = 14
    };

    enum WeaponAnimation
    {
        SHOOT = 0,
        SHOOT2 = 1,
        SHOOT_SECONDARY = 2,
        SHOOT_SECONDARY2 = 3,
        CHARGE = 4,
        RELOAD = 5,
        NUM_WEAPON_ANIMATIONS = 6,
        WEAPON_IDLE = 6,
        WEAPON_MELEE = 7
    };

    enum Posture
    {
        STAND = 0,
        CROUCH = 1,
        PRONE = 2,
        NUM_STANDARD_POSTURES = 3,
        STAND_ALERT = 3,
        NUM_ALERT_POSTURES = 3,
        CROUCH_ALERT = 4,
        PRONE_ALERT = 5,
        NUM_POSTURES = 6
    };

    enum Scope
    {
        UPPER = 0,
        LOWER = 1,
        NUM_SCOPES = 2
    };

    unsigned char GetAnimIndex(ActionAnimation anim);
    unsigned char GetAnimIndex(Posture posture, MovementAnimation anim);
    unsigned char GetAnimIndex(Posture posture, WeaponAnimation anim);
    unsigned char GetAnimIndex(SoldierAnimationBank::COMBO_ANIMATION anim);

    bool IsActionAnimIndex(unsigned char animIndex);
    bool IsMovementAnimIndex(unsigned char animIndex);
    bool IsWeaponAnimIndex(unsigned char animIndex);
    bool IsWeaponMeleeAnimIndex(unsigned char animIndex);
    Posture GetPostureFromAnimIndex(unsigned char animIndex);
    int GetAnimFromAnimIndex(unsigned char animIndex);

    const char* GetActionAnimationName(ActionAnimation anim);
    const char* GetMovementAnimationName(MovementAnimation anim, Posture posture);
    const char* GetWeaponAnimationName(WeaponAnimation anim);

    const char* GetWeaponMeleeAnimationName(SoldierAnimationBank::COMBO_ANIMATION anim);
    const char* GetPostureName(Posture posture);
    const char* GetScopeExtension(Scope scope);

    const char* AnimIndexToName(unsigned char animIndex, Scope scope);
}
