#pragma once

#include <stddef.h>

#include "Aimer.h"
#include "Character.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "NetPacket.h"
#include "ParticleEmitterObject.h"
#include "WeaponCannonInstance.h"
#include "WeaponMeleeInstance.h"

class GameModel;
class Team;
class EntityBuildingArmed;
class EntityHologram;
class CommandBuilding;
class CommandWalker;
class NetObjState;
class CannonStrengthConvert;

template <class CONVERT> void WriteFixed(CONVERT& convert, NetPktGroup& group, float value);
template <class CONVERT> void ReadFixed(CONVERT& convert, NetPktGroup& group, float& value);
class WeaponMelee;

void BeginNetLOD();
Team* ReadTeam(NetPktGroup& group);
void WriteString(NetPktGroup* group, const char* text, unsigned int capacity);
void ReadString(NetPktGroup& group, char* text, unsigned int capacity);

void __fastcall PropagateTeam(Character* pCharacter, int iTeam);
void WriteEntityClass(NetPktGroup& group, const void* pValue);
void WriteWeaponClass(NetPktGroup& group, const void* pValue);

class NetTrigger : public Trigger
{
public:
    void Write(NetPktGroup& group);
    void Read(NetPktGroup& group);
};
 

void WriteTrigger(NetPktGroup& group, Trigger* pTrigger);
void ReadTrigger(NetPktGroup& group, Trigger* pTrigger);
void WriteSwitch(NetPktGroup& group, float value);
void ReadSwitch(NetPktGroup& group, float& value);
void WriteJetFuel(NetPktGroup& group, float value);
void ReadJetFuel(NetPktGroup& group, float& value);

#include "GameObject.h"

unsigned int GetUniqObjectId(const GameObject* pObject);
int GetUniqObjectBits();
GameObject* FindClientObject(unsigned int uId);

class AmmoCounter
{
public:
    void Write(NetPktGroup& group) const;
    void Read(NetPktGroup& group);

    AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
    unsigned int m_uiRefCount;
};
 

class WeaponDisguise : public Weapon
{
public:
    enum DISGUISE_STATE
    {
        STATE_OFF = 0,
        STATE_ON = 1
    };

    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    void StartRaisingDisguise();
    void FinishRaisingDisguise();
    void StartDroppingDisguise();
    void FinishDroppingDisguise();

    GameModel* mSavedOwnerModel;
    Team* mOtherTeam;
    DISGUISE_STATE m_eState;

    static PblHash rttiHashWeaponDisguise;
};
 

class WeaponMeleeThrow : public Weapon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    WeaponMelee* m_pPrimaryWeapon;
    unsigned int m_bFired : 1;
    unsigned int m_bThrow : 1;
    unsigned int m_bReturnedThisFrame : 1;
    unsigned int z_uiReserved0 : 29;
};
 

class WeaponDispenser : public Weapon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    bool mNetFiredFlag;
    bool mFired;
    float mStrength;
    PblHandle<GameObject>** mItemArray;
    int* mItemCount;
};
 
 

class WeaponDetonator : public WeaponDispenser
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
};

class WeaponRemote : public WeaponDispenser
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    PblHandle<GameObject> mItem;
};
 

class WeaponGrenade : public Weapon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    bool mNetFiredFlag;
    bool mFired;
    char m_iFirePos;
    float mStrength;
};
 

class WeaponInvisibility : public Weapon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    float m_fInvisibilityFactor;
    float m_fInvisibilityPersistTimer;
    float m_fInvisibilityFlickerTimer;
    bool m_bInvisibilityOn;
    bool m_bInvisibilityActive;
    bool mFired;
    float mOwnerHealth;

    static PblHash rttiHashWeaponInvisibility;
};
 

class WeaponLaser : public Weapon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    float mSalvoTimer;
};

class WeaponLauncher : public WeaponCannon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    static PblHash rttiHashWeaponLauncher;

    PblHandle<const GameObject> mCurTarget;
    int mCurTargetBodyID;
    bool mLocked;
    float mTrackingSoundInterval;
    float mTrackingSoundTimer;
};
 

class WeaponTowCable : public WeaponCannon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);
};

class WeaponAreaEffect : public Weapon
{
public:
    virtual void Write(NetPktGroup& group);
    virtual void Read(NetPktGroup& group);

    PblHandle<GameObject> m_apTarget[4];
    float m_afTargetRange[4];
    int m_iNumTargets;
    float m_fStrength;
    bool m_bFired;
};
 
 

class WeaponGrenadeClass : public WeaponClass
{
public:
    float mShotDelay;
    float mInitialSalvoDelay[3];
    float mShotElevate;
    float mMaxPressedTime;
};
 

class WeaponLaserClass : public WeaponClass
{
public:
    float mShotDelay;
    float mSalvoTime;
};
 

class WeaponInvisibilityClass : public WeaponClass
{
public:

    char mInvisibilityStateStorage[0x258 - 0x22c];
    float mInitialSalvoDelay;
};
 

class WeaponLauncherClass : public WeaponCannonClass
{
public:
    float mLockTime;
};
 

class WeaponAreaEffectClass : public WeaponClass
{
public:
    int m_eFireType;
};
 
