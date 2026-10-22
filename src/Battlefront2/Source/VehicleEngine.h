#pragma once

#include <stddef.h>

#include "GameSound.h"
#include "GameSoundParameterized.h"
#include "PebbleFL/Common/PblListSingle.h"
#include "SoundFL/source/sndsoundparameterized.h"

class PblVector3;

class VehicleEngineClass
{
public:

    class Boost
    {
    protected:
        Boost(int iID, float fSpeedThreshold, bool bPositiveAcceleration,
              PblListSingle& list);

    public:
        Boost(const char* pName, float fSpeedThreshold, bool bPositiveAcceleration,
              PblListSingle& list);

        ~Boost()
        {
            if (mGlobalLink.mNext)
            {
                mGlobalLink.Remove();
            }
            if (mLink.mNext)
            {
                mLink.Remove();
            }
        }

        static void* operator new(unsigned int uiSize);
        static void operator delete(void* p);

        Boost* Duplicate(PblListSingle& list) const;
        void Update(float fSpeed, float fPrevSpeed, GameSoundControllable& controller,
                    GameSound& sound, const PblVector3* const pPos,
                    const PblVector3* const pVel) const;
        static void DestroyAll();

        PblListSingle::Node mLink;
        GameSound mBoost;
        unsigned char mPositiveAcceleration;
        float mSpeedThreshold;
        PblListSingle::Node mGlobalLink;

        static Snd::FreeList smFreeList;
        static PblListSingle sGlobalList;
    };

    VehicleEngineClass();
    VehicleEngineClass(const VehicleEngineClass* pBase);

    bool SetProperty(unsigned int uiHash, const char* pValue);
    void UpdateBoost(float fSpeed, float fPrevSpeed, GameSoundControllable& controller,
                     GameSound& sound, const PblVector3* const pPos,
                     const PblVector3* const pVel) const;

    GameSoundParameterized mEngine;
    GameSound mTurnOn;
    GameSound mWindUp;
    GameSound mWindDown;
    GameSound mTurningOff;
    GameSound mTurnOff;
    float mTurnOffTime;
    PblListSingle mBoostList;
    GameSound mCollision;
    GameSound mCockpit1stPerson;
    GameSound mCockpit3rdPerson;
    float mProximityMin;
    float mProximityMax;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class VehicleEngine
{
public:

    enum Flags
    {
        flags_HasEngine = 1,
        flags_IsOn = 2,
        flags_WindingUp = 4,
        flags_WindingDown = 8,
        flags_TurningOff = 16,
        flags_EngineTransitionMask = 28
    };

    enum Params
    {
        params_Speed = 1,
        params_Acceleration = 2,
        params_Health = 4,
        params_Boost = 8,
        params_Revs = 16,
        params_Height = 32,
        params_TurnOnTime = 64,
        params_TurnOffTime = 128,
        params_Strain = 256,
        params_WindUpTime = 512,
        params_WindDownTime = 1024,
        params_Proximity = 2048
    };

    VehicleEngine(VehicleEngineClass* pClass);
    ~VehicleEngine();

    static bool UpdateEnabled();
    static void DeactivateAll();

    void Update(const VehicleEngineClass* pClass, float fDeltaTime,
                const PblVector3& pos, const PblVector3& vel, float fSpeed,
                float fAcceleration, float fHealth, float fBoost, float fRevs,
                float fHeight, float fStrain, bool bCockpit, bool bFirstPerson,
                float fProximity);
    void TurnOn(const VehicleEngineClass* pClass, const PblVector3& pos,
                const PblVector3& vel, float fHealth);
    void WindUp(const VehicleEngineClass* pClass, const PblVector3& pos,
                const PblVector3& vel);
    void WindDown(const VehicleEngineClass* pClass, const PblVector3& pos,
                  const PblVector3& vel);
    void TurnOff(const VehicleEngineClass* pClass, const PblVector3& pos,
                 const PblVector3& vel);
    void Stop();
    bool IsOn() const;
    bool IsWindingUp() const;
    bool IsWindingDown() const;
    bool IsTurningOff() const;
    float GetMuteDistance() const;
    void DeactivateEngine();
    void Collision(const VehicleEngineClass* pClass, const PblVector3& pos,
                   const PblVector3& vel);

private:
    void StartTurningOff(const VehicleEngineClass* pClass, const PblVector3& pos,
                         const PblVector3& vel);
    void TransitionState(const GameSound* const pSound, const PblVector3& pos,
                         const PblVector3& vel, Flags flag);

public:
    PblListSingle::Node mNode;
    Snd::SoundParameterized mEngine;
    GameSoundControllable mAttached;
    GameSound mAttachedProps;
    GameSoundControllable mBoostController;
    GameSound mBoostProps;
    float mPrevSpeed;
    float mTimer;
    unsigned short mParams;
    unsigned char mFlags;

    static const unsigned int sParamIDs[12];
    static PblListSingle sEngineList;
};
 
 
 
 
 
 
 
 
 
 
