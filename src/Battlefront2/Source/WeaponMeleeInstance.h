#pragma once

#include <stddef.h>

#include "Combo.h"
#include "GameSound.h"
#include "LightsaberTrailEffect.h"
#include "WeaponCannonInstance.h"
#include "WeaponMelee.h"

class WeaponMeleeThrow;

// VTABLE: SWBF2 0x007b1578
class WeaponMelee : public Weapon
{
public:

    enum ComboState
    {
        COMBO_IDLE = 0
    };

    enum WeaponMeleeState
    {
        MELEE_IDLE = 0,
        MELEE_ATTACK = 1,
        MELEE_ATTACK2 = 2,
        MELEE_CHARGE = 3,
        MELEE_DEFLECT = 4,
        MELEE_RECOVER = 5,
        MELEE_EMPTY = 6,
        NUM_WEAPON_MELEE_STATES = 7
    };

    struct DamageData
    {
        enum { MAX_OBJECTS_PER_ATTACK = 8 };

        float fTime;
        void* pAttack;
        int iNumObjects;
        GameObject* apObject[8];
        DamageData* pNext;
    };

    class InputQueue
    {
    public:
        struct Event
        {
            float fTime;
            unsigned char uiButton;
            unsigned char uiButtonAction;
            unsigned short z_uiReserved;
        };

        InputQueue() : m_uiCount(0), m_uiStart(0), m_uiButtonDown(0) {}

        void Update(float fTime, unsigned char uiButtons);

        Event m_aEventQueue[8];
        unsigned char m_uiCount;
        unsigned char m_uiStart;
        unsigned char m_uiButtonDown;
        unsigned char z_uiReserved;
    };

    class TrailData
    {
    public:
        TrailData();
        ~TrailData();
        void Init(unsigned int uiEdge, const PblVector3& vOffsetPos, const PblVector3& vOffsetDir,
                  float fLength, float fWidth, const RedColor& colorTrail, const RedColor& colorHead);

        TrailData* pNext;
        unsigned char m_uiEdge;
        unsigned char m_uiFrameLastUpdated;
        LightsaberTrailClass m_TrailClass;
        LightsaberTrail* m_pTrail;
    };

    WeaponMelee(const WeaponMeleeClass* pClass, const WeaponDesc& desc);
    virtual ~WeaponMelee();
    virtual bool Update(float fDeltaTime);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual bool Deflect(const Ordnance* pOrdnance, const PblVector3& pos,
                         const PblVector3& dir);
    virtual bool ShouldShowReticule() const;
    virtual bool IsMelee() const;
    virtual void NotifySoldierState(int iState, float fTime, bool b0, bool b1,
                                    const PblVector3& vec);
    virtual bool SoldierCanOperate() const;
    virtual bool OverrideSoldierVelocity(float& f0, float& f1, float& f2,
                                         float& f3) const;
    virtual bool OverrideSoldierControls();
    virtual bool OverrideSoldierEnergyRestore(float& fRestore) const;
    virtual bool OverrideAimer();
    virtual void Select(unsigned int uFlags, bool bOffhand);
    virtual void Deselect(unsigned int uFlags, bool bOffhand);
    virtual bool IsBusy() const;
    virtual bool WillBecomeBusy() const;
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
    virtual void Render(const PblMatrix& mat, const RedPose* pPose,
                        const RedColor& color, unsigned int uFlags,
                        bool bArg) const;
    virtual void EnterIdle();
    virtual bool UpdateIdle(float fDeltaTime);
    virtual void ExitIdle();
    virtual void EnterFire();
    virtual bool UpdateFire(float fDeltaTime);
    virtual void ExitFire();
    virtual void EnterRecover();
    virtual void ExitRecover();
    virtual bool UpdateRecover(float fDeltaTime);
    virtual void EnterDeflect();
    virtual void ExitDeflect();
    virtual bool UpdateDeflect(float fDeltaTime);

    WeaponMeleeState GetMeleeState() const { return (WeaponMeleeState)mState; }
    unsigned char GetWeaponMeleeAnimation() const;
    unsigned char GetNextWeaponMeleeAnimation() const;
    unsigned char GetOffhandWeaponMeleeAnimation(int eState) const;
    unsigned char GetWeaponMeleeThrowAnimation() const;
    bool IsAnimationVelocityAligned() const;
    bool IsFullBodyAnimation() const;
    void UpdateEffects(float fDeltaTime, const PblVector3& vPos, const PblVector3& vVel,
                       const GameSound* pSound);

    static DamageData* AllocDamageData();
    static void FreeDamageData(DamageData* pData);

    void TurnOn(float fDuration);
    void TurnOff(float fDuration);

    float GetLightSaberLengthFactor() const;
    bool IsOn(unsigned int uiBlade) const;
    float GetLightSaberLengthFactor(unsigned int uiBlade) const;
    void RenderLightsaberTrail(unsigned int uiEdge, const PblMatrix* pMatrix, bool bTrailOn,
                               unsigned int uiFlags);
    bool TestInputQueueForButtons(int iPressedMask, int iReleasedMask) const;
    bool TestConditionExceptInputQueue(float fPreviousTime, float fCurrentTime,
                                       const Combo::Condition* pCondition);
    bool TestCondition(float fPreviousTime, float fCurrentTime, const Combo::Condition* pCondition,
                       float* pfMatchTime);

protected:
    void ClearInput();
    Trigger* GetButton(unsigned char uiButton);
    void ReadButtons();
    bool AlignVectorToThrust(PblVector3& v, bool bInitial) const;
    void ReleaseDamageData();
    void EnterState(ComboState eState);
    bool UpdateState(float fDeltaTime);

    void SetNetworkState(ComboState eState, bool bAnimatedMove);
    void ExitState();

    friend class WeaponMeleeClass;

    static void _InitDamageDataPool();
    static void _InitTrailDataPool();

    static void PostLoad() { WeaponMeleeClass::PostLoad(); }
    static TrailData* _AllocTrailData();
    static void _FreeTrailData(TrailData* pData);

public:
    static PblHash rttiHashWeaponMelee;

    static int _ClearInputQueueToTime(InputQueue* pQueue, float fTime);
    static bool _TestInputQueueForButtonEvent(int iButton, InputQueue* pQueue, float fTime,
                                              int iAction, float fBegin, float fEnd, float* pfMatch);
    static char rttiNameWeaponMelee[];

    static TrailData* s_aTrailDataPool;
    static TrailData* s_pTrailDataFreeList;
    static DamageData* s_aDamageDataPool;
    static DamageData* s_pDamageDataFreeList;

    WeaponMeleeThrow* m_pMeleeThrow;
    const Combo* m_pCombo;
    float m_fSoldierStateTimer;
    float m_fInputThrustZ;
    float m_fInputThrustX;
    PblVector3 m_vVelocityLocal;
    float m_fInputTimer;
    InputQueue m_InputQueue;
    unsigned char m_uiButtonDown;
    unsigned char m_bAnimatedMove : 1;
    unsigned char m_bLightsaberOn : 1;
    unsigned char m_bLightsaberSoundOn : 1;
    unsigned char m_bSoldierInputLock : 1;
    unsigned char m_bUnreachableState_WeaponUpdate : 1;
    unsigned char m_bUnreachableState_SoldierUpdate : 1;
    unsigned char m_bGravityDone : 1;
    unsigned char m_bAI_Deflect : 1;
    unsigned char m_bRestartAnimation : 1;
    unsigned char m_bInHeroFlee : 1;
    unsigned char m_uiAnimIndex;
    unsigned char m_uiTrailOnMask : 4;
    unsigned char m_uiLightsaberOnMask : 4;
    unsigned char m_uiLightsaberWasOnMask : 4;
    unsigned char m_uiLightsaberSoundOnMask : 4;
    ComboState m_eComboState;
    ComboState m_eComboStatePrev;
    float m_fComboStateTimer;
    float m_fComboStateDurationPrev;
    float m_fAlignZ;
    float m_fAlignX;
    float m_fDeflectTimer;
    float m_fVelocityOverrideFactor;
    float m_fTurnOnTimer;
    float m_fTurnOnDuration;
    DamageData* m_pDamageData;
    TrailData* m_pTrailData;
    GameSoundControllable m_OnOffSound;
    GameSound m_OnOffSoundProps;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
