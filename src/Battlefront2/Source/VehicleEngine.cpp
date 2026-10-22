#include "VehicleEngine.h"

#include <float.h>
#include <stdio.h>

#include "GameMemory.h"
#include "GameSoundEngine.h"
#include "NetGame.h"
#include "SoundFL/source/sndfreelist.h"

extern bool netIsLocalTurn;

// GLOBAL: SWBF2 0x0078bc00
const unsigned int VehicleEngine::sParamIDs[12] = {
    0x7b80c780, 0x2fa0ba9d, 0x6b98ed8f, 0xf8e9242c, 0xd7c3db73, 0xd5bdbb42,
    0x08aab3aa, 0xc8a9e3c6, 0xc7ab9270, 0x43f6a333, 0xeecf1cca, 0xbb0169a2,
};

// GLOBAL: SWBF2 0x007ebe9c
// SYNTHETIC: SWBF2 0x00406d70
// `dynamic initializer for 'VehicleEngine::sEngineList''
// SYNTHETIC: SWBF2 0x007696d0
// `dynamic atexit destructor for 'VehicleEngine::sEngineList''
PblListSingle VehicleEngine::sEngineList;

// GLOBAL: SWBF2 0x007ebeb8
// SYNTHETIC: SWBF2 0x00406da0
// `dynamic initializer for 'VehicleEngineClass::Boost::sGlobalList''
// SYNTHETIC: SWBF2 0x007696f0
// `dynamic atexit destructor for 'VehicleEngineClass::Boost::sGlobalList''
PblListSingle VehicleEngineClass::Boost::sGlobalList;

// GLOBAL: SWBF2 0x007ec3b0
// SYNTHETIC: SWBF2 0x00406d80
// `dynamic initializer for 'VehicleEngineClass::Boost::smFreeList''
// SYNTHETIC: SWBF2 0x007696e0
// `dynamic atexit destructor for 'VehicleEngineClass::Boost::smFreeList''
Snd::FreeList VehicleEngineClass::Boost::smFreeList((sizeof(Boost) + 15) & ~15u, 16, 8, 1);

// FUNCTION: SWBF2 0x0066cac0
void VehicleEngineClass::Boost::operator delete(void* p)
{
    smFreeList.Free(p);
}

void* VehicleEngineClass::Boost::operator new(unsigned int)
{
    return smFreeList.Alloc();
}

// FUNCTION: SWBF2 0x0066cad0
bool VehicleEngine::UpdateEnabled()
{
    if (!NetGame::IsEnabled())
    {
        return true;
    }
    if (NetGame::IsDedicated())
    {
        return false;
    }
    if (NetGame::IsClient() || NetGame::IsOneWorld())
    {
        return netIsLocalTurn;
    }
    return false;
}

// FUNCTION: SWBF2 0x0066cb20
VehicleEngine::VehicleEngine(VehicleEngineClass* pClass)
{
    mPrevSpeed = 0.0f;
    mParams = 0;
    mFlags = 0;
    mTimer = FLT_MAX;
    const Snd::SoundParameterized::Properties* pProps =
        (const Snd::SoundParameterized::Properties*)pClass->mEngine.mProps;
    if (pProps)
    {
        int iHeap = GameSoundEngine::GetHeap();
        for (unsigned int i = 0; i < 12; ++i)
        {
            mParams |= pProps->QueryParameter(sParamIDs[i]) ? (unsigned short)(1 << i) : 0;
        }
        GameSoundEngine::SetHeap(GameMemory::RunTimeHeap);
        mEngine.SetProperties(pProps);
        GameSoundEngine::SetHeap(iHeap);
        mFlags |= flags_HasEngine;
    }
    sEngineList.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0066cc30
VehicleEngine::~VehicleEngine()
{
    DeactivateEngine();
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0066ccb0
void VehicleEngine::Update(const VehicleEngineClass* pClass, float fDeltaTime,
                           const PblVector3& pos, const PblVector3& vel, float fSpeed,
                           float fAcceleration, float fHealth, float fBoost, float fRevs,
                           float fHeight, float fStrain, bool bCockpit, bool bFirstPerson,
                           float fProximity)
{
    if (!UpdateEnabled())
    {
        return;
    }

    if (bCockpit)
    {
        (bFirstPerson ? pClass->mCockpit1stPerson : pClass->mCockpit3rdPerson)
            .Play(&pos, &vel, false, true);
    }

    mAttached.Update(&mAttachedProps, &pos, &vel);

    if (!IsOn() || !(mFlags & flags_HasEngine) || !mEngine.mProperties)
    {
        return;
    }

    mEngine.ResetParameters();
    if (mParams & params_Speed)
    {
        mEngine.SetParameter(sParamIDs[0], fSpeed, false);
    }
    if (mParams & params_Acceleration)
    {
        mEngine.SetParameter(sParamIDs[1], fAcceleration, false);
    }
    if (mParams & params_Health)
    {
        mEngine.SetParameter(sParamIDs[2], fHealth, false);
    }
    if (mParams & params_Boost)
    {
        mEngine.SetParameter(sParamIDs[3], fBoost, false);
    }
    if (mParams & params_Revs)
    {
        mEngine.SetParameter(sParamIDs[4], fRevs, false);
    }
    if (mParams & params_Height)
    {
        mEngine.SetParameter(sParamIDs[5], fHeight, false);
    }
    if (mParams & params_Strain)
    {
        mEngine.SetParameter(sParamIDs[8], fStrain, false);
    }
    if (mParams & params_Proximity)
    {
        float fMin = pClass->mProximityMin;
        float fMax = pClass->mProximityMax;
        float fDist = fProximity;
        if (fMin > fDist)
        {
            fDist = fMin;
        }
        else if (fDist > fMax)
        {
            fDist = fMax;
        }
        float fRange = fMax - fMin;
        float fValue = 1.0f;
        if (fRange != 0.0f)
        {
            fValue = 1.0f - (fDist - fMin) / fRange;
        }
        mEngine.SetParameter(sParamIDs[11], fValue, false);
    }

    mEngine.SetPosition(&pos);
    mEngine.SetVelocity(&vel);
    pClass->UpdateBoost(fSpeed, mPrevSpeed, mBoostController, mBoostProps, &pos, &vel);
    mPrevSpeed = fSpeed;

    if (IsWindingUp())
    {
        if (mParams & params_TurnOnTime)
        {
            mEngine.SetParameter(sParamIDs[6], 1.0f, true);
        }
        if (mParams & params_TurnOffTime)
        {
            mEngine.SetParameter(sParamIDs[7], 1.0f, true);
        }
        if (mParams & params_WindUpTime)
        {
            mEngine.SetParameter(sParamIDs[9], mTimer, false);
        }
        if (mParams & params_WindDownTime)
        {
            mEngine.SetParameter(sParamIDs[10], 1.0f, true);
        }
    }
    else if (IsTurningOff())
    {
        if (mParams & params_TurnOnTime)
        {
            mEngine.SetParameter(sParamIDs[6], 1.0f, true);
        }
        if (mParams & params_TurnOffTime)
        {
            mEngine.SetParameter(sParamIDs[7], mTimer, false);
        }
        if (mParams & params_WindUpTime)
        {
            mEngine.SetParameter(sParamIDs[9], 1.0f, true);
        }
        if (mParams & params_WindDownTime)
        {
            mEngine.SetParameter(sParamIDs[10], 1.0f, true);
        }
        TurnOff(pClass, pos, vel);
    }
    else if (IsWindingDown())
    {
        if (mParams & params_TurnOnTime)
        {
            mEngine.SetParameter(sParamIDs[6], 1.0f, true);
        }
        if (mParams & params_TurnOffTime)
        {
            mEngine.SetParameter(sParamIDs[7], 1.0f, true);
        }
        if (mParams & params_WindUpTime)
        {
            mEngine.SetParameter(sParamIDs[9], 1.0f, true);
        }
        if (mParams & params_WindDownTime)
        {
            mEngine.SetParameter(sParamIDs[10], mTimer, false);
        }
    }
    else
    {
        if (mParams & params_TurnOnTime)
        {
            mEngine.SetParameter(sParamIDs[6], mTimer, false);
        }
        if (mParams & params_TurnOffTime)
        {
            mEngine.SetParameter(sParamIDs[7], 1.0f, true);
        }
        if (mParams & params_WindUpTime)
        {
            mEngine.SetParameter(sParamIDs[9], 1.0f, true);
        }
        if (mParams & params_WindDownTime)
        {
            mEngine.SetParameter(sParamIDs[10], 1.0f, true);
        }
    }

    mTimer += fDeltaTime;
}

// FUNCTION: SWBF2 0x0066d0e0
void VehicleEngine::TurnOn(const VehicleEngineClass* pClass, const PblVector3& pos,
                           const PblVector3& vel, float fHealth)
{
    if (!UpdateEnabled())
    {
        return;
    }
    if (IsOn() && !IsTurningOff())
    {
        return;
    }
    TransitionState(&pClass->mTurnOn, pos, vel, flags_IsOn);
    if (mFlags & flags_HasEngine)
    {
        mEngine.Play(true);
    }
    Update(pClass, 0.0f, pos, vel, 0.0f, 0.0f, fHealth, 0.0f, 0.0f, 0.0f, 0.0f,
           false, false, 0.0f);
}

// FUNCTION: SWBF2 0x0066d1c0
void VehicleEngine::WindUp(const VehicleEngineClass* pClass, const PblVector3& pos,
                           const PblVector3& vel)
{
    if (UpdateEnabled() && IsOn())
    {
        TransitionState(&pClass->mWindUp, pos, vel, flags_WindingUp);
    }
}

// FUNCTION: SWBF2 0x0066d200
void VehicleEngine::WindDown(const VehicleEngineClass* pClass, const PblVector3& pos,
                             const PblVector3& vel)
{
    if (UpdateEnabled() && IsOn())
    {
        TransitionState(&pClass->mWindDown, pos, vel, flags_WindingDown);
    }
}

// FUNCTION: SWBF2 0x0066d240
void VehicleEngine::StartTurningOff(const VehicleEngineClass* pClass, const PblVector3& pos,
                                    const PblVector3& vel)
{
    if (UpdateEnabled() && IsOn())
    {
        TransitionState(&pClass->mTurningOff, pos, vel, flags_TurningOff);
    }
}

// FUNCTION: SWBF2 0x0066d280
void VehicleEngine::TurnOff(const VehicleEngineClass* pClass, const PblVector3& pos,
                            const PblVector3& vel)
{
    if (!UpdateEnabled())
    {
        return;
    }
    if (IsOn() && !IsTurningOff())
    {
        StartTurningOff(pClass, pos, vel);
    }
    if (IsOn() && mTimer >= pClass->mTurnOffTime)
    {
        mAttached.Stop(true);
        mAttachedProps = pClass->mTurnOff;
        mAttached.Play(&mAttachedProps, &pos, &vel, true);
        if (mFlags & flags_HasEngine)
        {
            mEngine.Stop();
        }
        mFlags &= ~(flags_IsOn | flags_TurningOff);
    }
}

// FUNCTION: SWBF2 0x0066d330
void VehicleEngine::TransitionState(const GameSound* const pSound, const PblVector3& pos,
                                    const PblVector3& vel, Flags flag)
{
    if (!UpdateEnabled())
    {
        return;
    }
    mAttached.Stop(true);
    mAttachedProps = *pSound;
    mAttached.Play(&mAttachedProps, &pos, &vel, true);
    mTimer = 0.0f;
    mFlags = (unsigned char)((mFlags & ~flags_EngineTransitionMask) | flag);
}

// FUNCTION: SWBF2 0x0066d390
void VehicleEngine::Stop()
{
    if (!UpdateEnabled())
    {
        return;
    }
    mAttached.Stop(true);
    mEngine.Stop();
    mEngine.Update(0.1f);
    mFlags &= ~(flags_IsOn | flags_EngineTransitionMask);
}

// FUNCTION: SWBF2 0x0066d3d0
bool VehicleEngine::IsOn() const
{
    return (mFlags & flags_IsOn) != 0;
}

// FUNCTION: SWBF2 0x0066d3e0
bool VehicleEngine::IsWindingUp() const
{
    return (mFlags & flags_WindingUp) != 0;
}

// FUNCTION: SWBF2 0x0066d3f0
bool VehicleEngine::IsWindingDown() const
{
    return (mFlags & flags_WindingDown) != 0;
}

// FUNCTION: SWBF2 0x0066d400
bool VehicleEngine::IsTurningOff() const
{
    return (mFlags & flags_TurningOff) != 0;
}

// FUNCTION: SWBF2 0x0066d410
float VehicleEngine::GetMuteDistance() const
{
    float fMuteDistance = 0.0f;
    if (mAttached.GetPlaying())
    {
        fMuteDistance = mAttachedProps.GetMuteDistance();
    }

    if (IsOn() && (mFlags & flags_HasEngine))
    {
        const PblListSingle& rLayers = mEngine.mProperties->mSoundLayeredProps->mLayerList;
        unsigned int uNumLayers = 0;
        for (const PblListSingle::Node* pNode = rLayers.mHead.mNext; pNode != &rLayers.mHead;
             pNode = pNode->mNext)
        {
            ++uNumLayers;
        }

        for (unsigned int i = 0; i < uNumLayers; ++i)
        {
            const Snd::Sound* pLayer =
                const_cast<Snd::SoundParameterized&>(mEngine).GetLayerByIndex(i);
            if (pLayer->GetGain() > 0.0f && pLayer->mMuteDistance > fMuteDistance)
            {
                fMuteDistance = pLayer->mMuteDistance;
            }
        }
    }
    return fMuteDistance;
}

// FUNCTION: SWBF2 0x0066d4e0
void VehicleEngine::DeactivateEngine()
{
    if (!UpdateEnabled())
    {
        return;
    }
    int iHeap = GameSoundEngine::GetHeap();
    Stop();
    mEngine.Update(0.0f);
    GameSoundEngine::SetHeap(GameMemory::RunTimeHeap);
    mEngine.SetProperties(0);
    GameSoundEngine::SetHeap(iHeap);
    mFlags = 0;
    mParams = 0;
}

// FUNCTION: SWBF2 0x0066d540
void VehicleEngine::DeactivateAll()
{
    while (!sEngineList.IsEmpty())
    {
        VehicleEngine* pEngine = (VehicleEngine*)sEngineList.mHead.mNext;
        pEngine->DeactivateEngine();
        pEngine->mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0066d590
void VehicleEngine::Collision(const VehicleEngineClass* pClass, const PblVector3& pos,
                              const PblVector3& vel)
{
    if (UpdateEnabled())
    {
        pClass->mCollision.Play(&pos, &vel, false, true);
    }
}

// FUNCTION: SWBF2 0x0066d5c0
VehicleEngineClass::Boost::Boost(int iID, float fSpeedThreshold, bool bPositiveAcceleration,
                                 PblListSingle& list)
{
    mSpeedThreshold = fSpeedThreshold;
    mPositiveAcceleration = bPositiveAcceleration;
    mBoost.SetID((unsigned int)iID, false, false);
    list.AddLast(&mLink);
    sGlobalList.AddLast(&mGlobalLink);
}

// FUNCTION: SWBF2 0x0066d660
VehicleEngineClass::Boost::Boost(const char* pName, float fSpeedThreshold,
                                 bool bPositiveAcceleration, PblListSingle& list)
{
    mSpeedThreshold = fSpeedThreshold;
    mPositiveAcceleration = bPositiveAcceleration;
    mBoost.SetID(pName, false, false);
    list.AddLast(&mLink);
    sGlobalList.AddLast(&mGlobalLink);
}

// FUNCTION: SWBF2 0x0066d700
VehicleEngineClass::Boost* VehicleEngineClass::Boost::Duplicate(PblListSingle& list) const
{
    return new Boost(mBoost.GetPropertiesHashID(), mSpeedThreshold,
                     mPositiveAcceleration != 0, list);
}

// FUNCTION: SWBF2 0x0066d780
void VehicleEngineClass::Boost::Update(float fSpeed, float fPrevSpeed,
                                       GameSoundControllable& controller, GameSound& sound,
                                       const PblVector3* const pPos,
                                       const PblVector3* const pVel) const
{
    if (mPositiveAcceleration)
    {
        if (!(fSpeed > mSpeedThreshold) || !(mSpeedThreshold >= fPrevSpeed))
        {
            return;
        }
    }
    else
    {
        if (!(mSpeedThreshold > fSpeed) || !(fPrevSpeed >= mSpeedThreshold))
        {
            return;
        }
    }
    if ((controller.mFlags & GameSoundControllable::flags_playing) && !mPositiveAcceleration)
    {
        return;
    }
    sound = mBoost;
    controller.Play(&sound, pPos, pVel, true);
}

// FUNCTION: SWBF2 0x0066d7e0
void VehicleEngineClass::Boost::DestroyAll()
{
    while (!sGlobalList.IsEmpty())
    {
        delete (Boost*)((char*)sGlobalList.mHead.mNext - offsetof(Boost, mGlobalLink));
    }
}

// FUNCTION: SWBF2 0x0066d8c0
VehicleEngineClass::VehicleEngineClass(const VehicleEngineClass* pBase)
{
    mEngine = pBase->mEngine;
    mTurnOn = pBase->mTurnOn;
    mWindUp = pBase->mWindUp;
    mWindDown = pBase->mWindDown;
    mTurningOff = pBase->mTurningOff;
    mTurnOff = pBase->mTurnOff;
    mTurnOffTime = pBase->mTurnOffTime;
    mCockpit1stPerson = pBase->mCockpit1stPerson;
    mCockpit3rdPerson = pBase->mCockpit3rdPerson;
    mProximityMin = pBase->mProximityMin;
    mProximityMax = pBase->mProximityMax;
    for (PblListSingle::Node* pNode = pBase->mBoostList.mHead.mNext;
         pNode != &pBase->mBoostList.mHead; pNode = pNode->mNext)
    {
        ((Boost*)pNode)->Duplicate(mBoostList);
    }
}

// FUNCTION: SWBF2 0x0066da20
bool VehicleEngineClass::SetProperty(unsigned int uiHash, const char* pValue)
{
    switch (uiHash)
    {
    case 0xea603e52:
        mTurnOn.SetID(pValue, false, false);
        return true;
    case 0x205b8d59:
    case 0xc093b232:
        mWindUp.SetID(pValue, false, false);
        return true;
    case 0xfc64a1f2:
    case 0xad3e89e3:
        mWindDown.SetID(pValue, false, false);
        return true;
    case 0x7d2c2fa2:
        mTurningOff.SetID(pValue, false, false);
        return true;
    case 0x042dabae:
        mTurnOff.SetID(pValue, false, false);
        return true;
    case 0x84824518:
        mCollision.SetID(pValue, false, false);
        return true;
    case 0x1fe0c822:
        mCockpit1stPerson.SetID(pValue, false, false);
        return true;
    case 0x421337df:
        mCockpit3rdPerson.SetID(pValue, false, false);
        return true;
    case 0x84bbef6a:
        mEngine.SetID(pValue);
        return true;
    case 0xc8a9e3c6:
        sscanf(pValue, "%f", &mTurnOffTime);
        return true;
    case 0xf3a06dca:
        sscanf(pValue, "%f", &mProximityMin);
        return true;
    case 0xac4226d4:
        sscanf(pValue, "%f", &mProximityMax);
        return true;
    case 0x79e2d013:
    {
        char name[256];
        float fSpeedThreshold = 0.0f;
        int iPositive = 1;
        sscanf(pValue, "%s %f %d", name, &fSpeedThreshold, &iPositive);
        new Boost(name, fSpeedThreshold, iPositive == 1, mBoostList);
        return true;
    }
    }
    return false;
}

// FUNCTION: SWBF2 0x0066dc20
void VehicleEngineClass::UpdateBoost(float fSpeed, float fPrevSpeed,
                                     GameSoundControllable& controller, GameSound& sound,
                                     const PblVector3* const pPos,
                                     const PblVector3* const pVel) const
{
    for (const PblListSingle::Node* pNode = mBoostList.mHead.mNext; pNode != &mBoostList.mHead;
         pNode = pNode->mNext)
    {
        ((const Boost*)pNode)->Update(fSpeed, fPrevSpeed, controller, sound, pPos, pVel);
    }
    controller.Update(&sound, pPos, pVel);
}

// FUNCTION: SWBF2 0x0066d850
VehicleEngineClass::VehicleEngineClass()
{
    mTurnOffTime = 3.0f;
    mProximityMin = 60.0f;
    mProximityMax = 500.0f;
}
