#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameSound.h"
#include "SoldierAnimationBank.h"

class PblConfig;
class WeaponMelee;
class ZephyrSkeletonShared;

class Combo
{
public:
    class DamageSample;
    class Deflect;
    class Condition;
    class Transition;
    class State;

    class Transition
    {
    public:

        Condition* AddCondition();

        void SetEnergyCost(float fCost)
        {
            int iCost = (int)(fCost * ENERGY_SCALE_INV);
            if (iCost < 0)
            {
                iCost = 0;
            }
            else if (iCost > 1023)
            {
                iCost = 1023;
            }
            m_uiEnergyCost = iCost;
        }

    private:
        friend class State;

        void _Init(unsigned int uiTargetState)
        {
            m_uiTargetState = uiTargetState;
            m_aCondition = 0;
            m_uiNumConditions = 0;
            m_uiEnergyCost = 0;
            z_uiReserved0 = 0;
        }

        void _ReadConfig(PblConfig& config, const char* pszComboName, const char* pszStateName);

    public:

        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float ENERGY_SCALE_INV;

        unsigned int m_uiTargetState;
        Condition* m_aCondition;
        unsigned int m_uiNumConditions : 6;
        unsigned int m_uiEnergyCost : 10;
        unsigned int z_uiReserved0 : 16;
    };

    class Condition
    {
    public:

        bool TestSoldierState(int iState, float fTime) const;

        bool TestEnergy(float fEnergy) const;

        bool TestInputThrust(float fThrust, float fStrafe) const;

        bool TestVelocity(float fX, float fY, float fZ) const;

        void SetTimeStart(float fTime, bool bFromEnd);
        void SetTimeEnd(float fTime, bool bFromEnd, bool b);
        void SetInputMag(float fMag, bool bLess);

        void SetBreak(float fTime);
        void SetSoldierStateTime(float fTime, bool bLess);
        void SetEnergy(float fEnergy, bool bLess);
        void SetInputDir(float fMin, float fMax);
        void SetVelocityY(float fVelocity, bool bSigned, bool bLess);
        void SetVelocityXZMag(float fMag, bool bLess);
        void SetVelocityXZDir(float fMin, float fMax);

        void SetSoldierStates(unsigned int uiStates) { m_uiSoldierStates = uiStates; }
        void SetButton(unsigned int eButton, unsigned int eAction)
        {
            m_uiButton = eButton;
            m_uiButtonAction = eAction;
        }
        void SetInputButtonsPressed(unsigned int uiButtons) { m_uiInputButtonsPressed = uiButtons; }
        void SetInputButtonsReleased(unsigned int uiButtons) { m_uiInputButtonsReleased = uiButtons; }
        bool IsBreak() const { return m_bBreak != 0; }

        void SetBreak(bool bBreak)
        {
            m_uiBreakTime = 0;
            m_bBreak = bBreak;
        }

    private:
        friend class Transition;
        friend class State;

        void _Init()
        {
            for (int i = 0; i < 5; ++i)
            {
                m_auiPackedData[i] = 0;
            }
            m_bTimeEndFromEnd = 1;
        }

        void _ReadConfig(PblConfig& config, const char* pszComboName, const char* pszStateName);

    public:

        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float TIME_SCALE_INV;
        static const float TIME_SCALE;
        static const float ENERGY_SCALE_INV;
        static const float ENERGY_SCALE;
        static const float INPUT_MAG_SCALE_INV;
        static const float INPUT_MAG_SCALE;
        static const float INPUT_DIR_SCALE_RAD;
        static const float VELOCITYY_MAG_SCALE_INV;
        static const float VELOCITYY_MAG_SCALE;
        static const float VELOCITYY_VAL_SCALE_INV;
        static const float VELOCITYY_VAL_SCALE;
        static const float VELOCITYY_VAL_OFFSET;
        static const float VELOCITYXZ_MAG_SCALE_INV;
        static const float VELOCITYXZ_MAG_SCALE;
        static const float VELOCITYXZ_DIR_SCALE_RAD;

        union
        {
            struct
            {
                unsigned int m_uiBreakTime : 9;
                unsigned int m_bBreak : 1;
                int m_iTimeStart : 10;
                unsigned int m_bTimeStartFromEnd : 1;
                int m_iTimeEnd : 10;
                unsigned int m_bTimeEndFromEnd : 1;

                unsigned int m_uiSoldierStates : 8;
                unsigned int m_uiSoldierStateTimeRef : 11;
                unsigned int m_bSoldierStateTimeLess : 1;
                unsigned int m_uiEnergyRef : 10;
                unsigned int m_bEnergyLess : 1;
                unsigned int m_bSyncIfOvertime : 1;

                unsigned int m_uiButtonAction : 4;
                unsigned int m_uiButton : 3;
                unsigned int z_uiReserved2 : 5;
                unsigned int m_uiInputButtonsPressed : 6;
                unsigned int m_uiInputButtonsReleased : 6;
                unsigned int m_uiVelocityYRef : 6;
                unsigned int m_bVelocityYLess : 1;
                unsigned int m_bVelocityYSigned : 1;

                unsigned int m_uiInputMagRef : 7;
                unsigned int m_bInputMagLess : 1;
                unsigned int z_uiReserved3 : 6;
                int m_iInputDirMin : 9;
                int m_iInputDirMax : 9;

                unsigned int m_uiVelocityXZMagRef : 11;
                unsigned int m_bVelocityXZMagLess : 1;
                unsigned int z_uiReserved4 : 2;
                int m_iVelocityXZDirMin : 9;
                int m_iVelocityXZDirMax : 9;
            };
            struct
            {
                unsigned int zz_0 : 32;
                unsigned int zz_1a : 8;
                unsigned int m_uiSoldierStateTimeCnd : 12;
                unsigned int m_uiEnergyCnd : 11;
                unsigned int zz_1b : 1;
                unsigned int zz_2 : 12;
                unsigned int m_uiInputButtonCnd : 12;
                unsigned int m_uiVelocityYCnd : 8;
                unsigned int m_uiInputMagCnd : 8;
                unsigned int zz_3 : 24;
                unsigned int m_uiVelocityXZMagCnd : 12;
                unsigned int zz_4 : 20;
            };
            unsigned int m_auiPackedData[5];
        };
    };

    class Attack
    {
    public:
        void AddDamageSample(float fTime, const PblVector3& vBegin, const PblVector3& vEnd);
        bool _ResolveDamageData(const Combo* pCombo, const State* pState,
                                unsigned int uiAttach, const PblVector3& vBegin,
                                const PblVector3& vEnd);

        void _Init();

        void SetDamageTimeStart(float fTime, bool bFromAnim);
        void SetDamageTimeEnd(float fTime, bool bFromAnim);
        void SetDamageLength(float fLength, bool bFromEdge);
        void SetDamage(float fDamage);

        void SetDamage() { m_uiDamage = 0xfff; }
        void SetDamageEdge(unsigned int uiEdge)
        {
            int iEdge = (int)uiEdge;
            if (iEdge < 0)
            {
                iEdge = 0;
            }
            else if (iEdge > 15)
            {
                iEdge = 15;
            }
            m_uiDamageEdge = iEdge;
        }
        void SetDamageWidth(float fWidth, bool bFromEdge)
        {
            int iWidth = (int)(fWidth * WIDTH_SCALE_INV);
            if (iWidth < 0)
            {
                iWidth = 0;
            }
            else if (iWidth > 255)
            {
                iWidth = 255;
            }
            m_uiDamageWidth = iWidth;
            m_bDamageWidthFromEdge = bFromEdge;
        }

        void SetPushStrength(float fStrength)
        {
            int iStrength = (int)(fStrength * PUSH_SCALE_INV);
            if (iStrength < 0)
            {
                iStrength = 0;
            }
            else if (iStrength > 255)
            {
                iStrength = 255;
            }
            m_uiSpecialType = 0;
            m_uiSpecial = iStrength;
        }
        void SetAttackId(unsigned int uiAttackId) { m_uiAttackId = uiAttackId; }

    private:
        friend class State;

        void _ReadConfig(PblConfig& config, const char* pszStateName);

    public:
        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float WIDTH_SCALE_INV;
        static const float PUSH_SCALE_INV;
        static const float TIME_SCALE_INV;
        static const float LENGTH_SCALE_INV;

        union
        {
            struct
            {
                unsigned int m_uiNumDamageSamples : 8;
                unsigned int m_bDamageTimeStartFromAnim : 1;
                unsigned int m_bDamageTimeEndFromAnim : 1;
                unsigned int m_uiDamageTimeStart : 10;
                unsigned int m_uiDamageTimeEnd : 10;
                unsigned int m_uiSpecialType : 2;

                unsigned int m_uiDamage : 12;
                unsigned int m_uiAttackId : 20;

                unsigned int m_uiDamageEdge : 4;
                unsigned int m_bDamageLengthFromEdge : 1;
                unsigned int m_bDamageWidthFromEdge : 1;
                unsigned int m_uiDamageLength : 10;
                unsigned int m_uiDamageWidth : 8;
                unsigned int m_uiSpecial : 8;
            };
            unsigned int m_auiPackedData[3];
        };
        DamageSample* m_aDamageSample;
    };

    class State
    {
    public:

        unsigned char GetAnimIndex() const { return (unsigned char)m_uiAnimIndex; }
        const Deflect* GetDeflect() const { return m_pDeflect; }
        unsigned int GetNumAttacks() const { return m_uiNumAttacks; }

        void SetDuration(float fDuration);

        void SetSound(const char* pName, float fTime);

        void SetVelocityZ(float fVelocity, bool bFromAnim);

        void SetVelocityX(float fVelocity, bool bFromAnim);

        void SetInputLock(unsigned int uiInputLock, float fDuration);

        void SetEnergyRestoreRate(float fRate, bool b);

        void SetThrustFactor(float fFactor);

        void SetStrafeFactor(float fFactor);

        void SetPitchFactor(float fFactor);

        void SetGravity(float fGravity);

        void SetGravityVelocityYTarget(float fVelocity, bool bZeroAtTarget);

        void SetVelocityYImpulse(float fVelocity, bool bZeroAtTarget);

        Transition* AddTransition(unsigned int uiTargetState);
        Attack* AddAttack();

        const State& operator=(const State& other);
        Deflect* AddDeflect();

        Condition* AddAnimatedMoveCondition();

        void SetEnergyRestoreRate()
        {
            m_iEnergyRestoreRate = 0;
            m_bEnergyRestoreRateOverride = 0;
        }
        void SetSoldierStates(unsigned int uiStates) { m_uiSoldierStates = uiStates; }
        void SetTurnFactor(float fFactor)
        {
            int iFactor = (int)(fFactor * INPUT_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > INPUT_MAX)
            {
                iFactor = INPUT_MAX;
            }
            m_uiTurnFactor = iFactor;
        }
        void SetVelocityFromThrust(float fFactor)
        {
            int iFactor = (int)(fFactor * VELOCITYDELTA_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > 255)
            {
                iFactor = 255;
            }
            m_uiVelocityFromThrust = iFactor;
        }
        void SetVelocityFromStrafe(float fFactor)
        {
            int iFactor = (int)(fFactor * VELOCITYDELTA_SCALE_INV);
            if (iFactor < 0)
            {
                iFactor = 0;
            }
            else if (iFactor > 255)
            {
                iFactor = 255;
            }
            m_uiVelocityFromStrafe = iFactor;
        }
        void SetAnimatedMove(bool bAnimatedMove) { m_bAnimatedMove = bAnimatedMove; }
        void SetAlignedToThrust(bool bAligned, bool bInitial)
        {
            m_bAlignToThrust = bAligned;
            m_bAlignToInitialThrust = bInitial;
        }
        void SetMustShowOneFrame(bool b) { m_bMustShowOneFrame = b; }
        void SetPlayExplosionOnEnter(bool b) { m_bPlayExplosion = b; }
        void SetRestartAnimationOnEnter(bool b) { m_bRestartAnimation = b; }
        void SetLightsaberOff(bool bOff) { m_uiLightsaberOffMask = bOff ? 0xf : 0; }
        void SetLightsaberOff(unsigned int uiBlade, bool bOff)
        {
            if (uiBlade < 4)
            {
                if (bOff)
                {
                    m_uiLightsaberOffMask |= 1 << uiBlade;
                }
                else
                {
                    m_uiLightsaberOffMask &= ~(1 << uiBlade);
                }
            }
        }

    private:
        friend class Combo;

        void _Init(unsigned int uiNameHash);

        void _ReadConfig(PblConfig& config, const Combo& combo, const char* pszComboName);

        void _ReadAnimatedMoveScope(PblConfig& config, const char* pszComboName);

    public:

        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float TIME_SCALE_INV;
        static const float ENERGY_RATE_SCALE_INV;
        static const float INPUT_SCALE_INV;
        static const float VELOCITY_SCALE_INV;
        static const float GRAVITY_SCALE_INV;
        static const float VELOCITYY_SCALE_INV;
        static const float VELOCITYDELTA_SCALE_INV;
        static const int DURATION_MAX = 1022;
        static const unsigned int DURATION_INVALID = 0x3ff;
        static const int INPUTLOCK_DURATION_MAX = 1023;
        static const int SOUND_TIME_MAX = 1023;
        static const int ENERGY_RATE_MIN = -512;
        static const int ENERGY_RATE_MAX = 511;
        static const int INPUT_MAX = 255;
        static const int VELOCITY_MIN = -2048;
        static const int VELOCITY_MAX = 2047;
        static const int GRAVITY_MIN = -127;
        static const int GRAVITY_MAX = 127;
        static const int GRAVITY_IMPULSE = -128;
        static const int VELOCITYY_MIN = -2047;
        static const int VELOCITYY_MAX = 2047;

        unsigned int m_uiNameHash;
        GameSound m_sound;
        Transition* m_aTransition;
        Attack* m_aAttack;
        Deflect* m_pDeflect;
        Condition* m_aAnimatedMoveCondition;
        union
        {
            struct
            {
                unsigned int m_uiAnimIndex : 8;
                unsigned int m_uiSoldierStates : 8;
                unsigned int m_uiNumTransitions : 6;
                unsigned int m_uiNumAnimatedMoveConditions : 6;
                unsigned int m_uiNumAttacks : 4;

                unsigned int m_uiDuration : 10;
                unsigned int m_uiSoundTime : 10;
                int m_iEnergyRestoreRate : 10;
                unsigned int m_bEnergyRestoreRateOverride : 1;
                unsigned int m_bMustShowOneFrame : 1;

                unsigned int m_uiThrustFactor : 8;
                unsigned int m_uiStrafeFactor : 8;
                unsigned int m_uiTurnFactor : 8;
                unsigned int m_uiPitchFactor : 8;

                unsigned int m_uiInputLock : 8;
                unsigned int m_uiInputLockDuration : 10;
                unsigned int z_uiReserved3 : 1;
                unsigned int m_bAnimatedMove : 1;
                unsigned int m_bAlignToThrust : 1;
                unsigned int m_bAlignToInitialThrust : 1;
                unsigned int m_bVelocityZFromAnim : 1;
                unsigned int m_bVelocityXFromAnim : 1;
                unsigned int m_uiVelocityFromThrust : 8;

                unsigned int m_uiVelocityFromStrafe : 8;
                int m_iVelocityZ : 12;
                int m_iVelocityX : 12;

                int m_iGravity : 8;
                int m_iGravityVelocityYTarget : 12;
                unsigned int m_bGravityZeroAtTarget : 1;
                unsigned int z_uiReserved5 : 5;
                unsigned int m_bRestartAnimation : 1;
                unsigned int m_bPlayExplosion : 1;
                unsigned int m_uiLightsaberOffMask : 4;
            };
            unsigned int m_auiPackedData[6];
        };
    };
    class DamageSample { public: static MemoryPool sMemoryPool; };

    class Deflect
    {
    public:

        bool TestDeflect(const float* pMatrix, const float* pRight, const float* pAttack, int iUnused,
                         unsigned char uiFallback, unsigned char* pResult) const;
        struct DEFLECT_ANIM
        {
            unsigned char uiAnimIndex;
            unsigned char uiDirections : 6;
            unsigned char uiWeapon : 2;
        };

        unsigned int GetNumDeflectAnimations() const { return m_uiNumDeflectAnimations; }
        unsigned char GetDeflectAnimIndex(unsigned int i) const { return m_aDeflectAnim[i].uiAnimIndex; }

        unsigned int AddDeflectAnimation(unsigned char uiAnimIndex, unsigned char uiDirections,
                                         unsigned char uiWeapon);

        void SetDeflectAngle(float fMin, float fMax);

        void SetEnergyCost(float fCost)
        {
            int iCost = (int)(fCost * ENERGY_SCALE_INV);
            if (iCost < 0)
            {
                iCost = 0;
            }
            else if (iCost > 1023)
            {
                iCost = 1023;
            }
            m_uiEnergyCost = iCost;
        }

    private:
        friend class State;

        void _Init()
        {
            for (int i = 0; i < 9; ++i)
            {
                m_auiPackedData[i] = 0;
            }
        }

        void _ReadConfig(PblConfig& config, const Combo& combo, const char* pszStateName);

    public:
        static MemoryPool sMemoryPool;

        static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

        static const float ENERGY_SCALE_INV;

        union
        {
            struct
            {
                unsigned int m_uiNumDeflectAnimations : 4;
                int m_iDeflectAngleMin : 9;
                int m_iDeflectAngleMax : 9;
                unsigned int m_uiEnergyCost : 10;
                DEFLECT_ANIM m_aDeflectAnim[15];
                unsigned short z_uiReserved;
            };
            unsigned int m_auiPackedData[9];
        };
    };

    static void Init();

    Combo();
    Combo* CopyConstructor(const Combo& other);
    void ReadConfig(PblConfig& config);
    static void PostLoad();
    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }

    static const Combo* Find(unsigned int uiNameHash) { return s_ComboTable.Find(uiNameHash); }
    unsigned int GetNumStates() const { return m_uiNumStates; }
    const State* GetState(unsigned int i) const { return &m_aState[i]; }

    static MemoryPool sMemoryPool;

    static int s_ComboTableNumEntries;
    static PblHashTable<const Combo, 32> s_ComboTable;

    void InitDefault(unsigned int uiNameHash);
    void AddDefaultAttackState();
    void AddDefaultDeflect();
    State* AddState(unsigned int uiNameHash);
    unsigned char GetOffhandAnimIndex(int eState) const;
    const Combo* ResolveForWeapon(const WeaponMelee* pWeapon, const ZephyrSkeletonShared* pSkeleton) const;

    void SetOffhandAnimIndex(int eState, unsigned char uiAnimIndex);

    void SetThrowAnimIndex(unsigned char uiAnimIndex) { m_uiThrowAnimIndex = uiAnimIndex; }

private:

    static Transition* _AllocTransition();
    static Condition* _AllocCondition();
    static Attack* _AllocAttack();
    static Deflect* _AllocDeflect();

    static State* _AllocState();

    void _PostLoad();
    unsigned int _GetDeflectButton() const;

public:

    static void* _AllocDamageSample();

    static unsigned int __fastcall GetNameHashForMap(unsigned int uComboHash, SoldierAnimationBank::MAP eMap);

    unsigned int m_uiNameHash;
    int m_eCombo;
    int m_eMap;
    ZephyrSkeletonShared* m_pSkeleton;
    State* m_aState;
    State* m_pCatchState;
    State* m_pTakeKneeState;
    unsigned char m_auiOffhandAnimIndex[4];
    unsigned int m_uiThrowAnimIndex : 8;
    unsigned int m_uiNumStates : 8;
    unsigned int m_uiDeflectButton : 8;
    unsigned int m_uiReserved0 : 8;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
