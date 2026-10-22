#include "Weapon.h"

#include <math.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Aimer.h"
#include "BonusManager.h"
#include "CameraManager.h"
#include "Controllable.h"
#include "EnergyBar.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Ordnance.h"
#include "ParticleEmitterObject.h"
#include "SoldierAnimationBank.h"
#include "Trigger.h"
#include "WeaponCannonInstance.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "SoundFL/source/sndvoicevirtual.h"
#include "FLEffect.h"
#include "StringDB.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Graphics/RedMaterial.h"

// FUNCTION: SWBF2 0x00679250
bool Weapon::CheckFire() const
{
    return false;
}

// FUNCTION: SWBF2 0x00679260
bool Weapon::FireDone() const
{
    return true;
}

// FUNCTION: SWBF2 0x0067bce0
float WeaponClass::GetHeatPerSalvo()
{
    return mHeatPerShot;
}

// FUNCTION: SWBF2 0x0067bcf0
float WeaponClass::GetShotsPerSalvo()
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x0067bc00
Weapon* WeaponClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    if (Weapon::sMemoryPool.IsFull())
    {
        return 0;
    }
    void* pMemory = Weapon::operator new(sizeof(Weapon));
    return pMemory ? ::new (pMemory) Weapon(this, desc) : 0;
}

// SYNTHETIC: SWBF2 0x00679b40
// WeaponClass::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0067c120
// Factory<Weapon,WeaponClass,WeaponDesc>::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0067a1b0
WeaponClass::~WeaponClass() {}

// FUNCTION: SWBF2 0x0067bb90
WeaponClass* WeaponClass::Derive(unsigned int uHash)
{
    return new WeaponClass(this, uHash);
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

extern bool netIsLateTurn;

class AmmoCounter
{
public:
    AmmoCounter(const AmmoCounterClass* pClass, int iNumClips);
    AmmoCounter* AddRef();

    unsigned int Release();
    void UnloadClip();
    void LoadClip();

    void UnloadClip_Unlimitted();
    void LoadClip_Unlimitted();

    static MemoryPool sMemoryPool;

    const AmmoCounterClass* m_pClass;
    float mDefaultMaxClips;
    float mMaxClips;
    float mNumClips;
    float mCurClip;
    unsigned int m_uiRefCount;
};

namespace GameLoop { extern bool gUnlimitedAmmo; }

namespace
{

    const PblMatrix& GetCollisionMatrix(GameObject* pObject)
    {
        return static_cast<CollisionObject*>(pObject)->GetMatrix();
    }

    const PblVector3& GetVelocity(GameObject* pObject)
    {
        return pObject->GetVelocity();
    }

    int GetEntityType(GameObject* pObject)
    {
        return pObject->GetEntityType();
    }

    int GetTeam(GameObject* pObject)
    {
        return pObject->GetTeam();
    }

    const PblVector3& GetTranslation(const PblMatrix& m)
    {
        return *reinterpret_cast<const PblVector3*>(m.m[3]);
    }

    const GameObject* GetHandleObject(const PblHandle<const GameObject>& handle)
    {
        PblHandle<const GameObject>& h = const_cast<PblHandle<const GameObject>&>(handle);
        if (h.mObject &&
            h.mObject->mHandleId != h.mSavedHandleId)
        {
            h.mObject = 0;
            h.mSavedHandleId = 0;
        }
        return h.mObject;
    }

    float Clamp(float f, float fMin, float fMax)
    {
        if (fMin > f)
        {
            return fMin;
        }
        if (f > fMax)
        {
            return fMax;
        }
        return f;
    }

    bool ObjectIsRtti(GameObject* pObject, unsigned int uRtti)
    {
        return pObject->IsRtti(uRtti);
    }
}

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

// FUNCTION: SWBF2 0x00679700
WeaponClass::WeaponClass(unsigned int uHash)
    : Factory<Weapon, WeaponClass, WeaponDesc>(uHash)
{
    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    mPostLoadInit = false;

    mSoldierAnimationWeapon = -1;
    for (int i = 0; i < 32; ++i)
    {
        mFilename[i] = 0;
    }

    mZoomFirstPerson = false;
    mMaxRangeDefault = true;
    mHashWeaponName = false;

    mStrengthLimit = 0.0f;
    mNextCharge = 0;
    mLabel = 0;
    mNameHash = uHash;
    mHUDNameHash = 0;
    mIconTexture = 0;
    mReticuleTexture = 0;
    mScopeTexture = 0xF6BCFA8Eu;
    mOrdnanceClass = 0;
    mModel = 0;
    mHighResModel = 0;
    mReloadTime = 0.6f;
    mExtremeRange = 0.0f;
    mOptimalRange = 30.0f;
    mMaxRange = 60.0f;
    mZoomMin = 1.0f;
    mZoomMax = 1.0f;
    mZoomRate = 0.0f;
    mZoomTurnDivisorMin = 1.0f;
    mZoomTurnDivisorMax = 1.0f;

    mFirePointOffset.x = 0.0f;
    mFirePointOffset.y = 0.0f;
    mFirePointOffset.z = 0.0f;

    mTriggerSingle = true;

    mMuzzleFlashEffect = 0;
    mRecoilStrengthHeavy = 0.0f;
    mRecoilStrengthLight = 0.0f;
    mRecoilLengthLight = 0.0f;
    mRecoilLengthHeavy = 0.0f;
    mRecoilDelayLight = 0.0f;
    mRecoilDelayHeavy = 0.0f;
    mRecoilDecayLight = 0.0f;
    mRecoilDecayHeavy = 0.0f;
    mChargeRateLight = 0.0f;
    mChargeRateHeavy = 0.0f;
    mMaxChargeStrengthHeavy = 0.0f;
    mMaxChargeStrengthLight = 0.0f;
    mChargeDelayLight = 0.0f;
    mChargeDelayHeavy = 0.0f;
    mTimeAtMaxCharge = 2.0f;
    mLockOnRange = 0.0f;
    mLockOnAngle = 0.0f;
    mLockOffAngle = 0.0f;
    mMinRange = 0.0f;
    mTargetSides = 1;
    mTargetTypes = -97;
    mAITargetTypes = -97;
    mRoundsPerShot = 1.0f;
    mHeatPerShot = 0.0f;
    mHeatRecoverRate = 0.0f;

    mEnergyBarClass.Init(1.0f, 1.0f);

    mReticuleInAimingOnly = false;
    mAIUseBubbleCircle = false;
    mSniperScope = false;
    mInstantPlayFireAnim = false;
    mIsOffhand = false;

    mFlashColor.b = 0x32;
    mFlashColor.g = 0xFF;
    mFlashColor.r = 0xFF;
    mFlashColor.a = 0xFF;
    mFlashLightColor.b = 0x19;
    mFlashLightColor.g = 0x7F;
    mFlashLightColor.r = 0x7F;
    mFlashLightColor.a = 0xFF;

    mScoreForMedalsType = -1;
    mMedalsTypeToUnlock = -1;
    mMedalsTypeToLock = -1;
    mFlashRadius = -1.0f;
    mFlashWidth = 0.25f;
    mFlashLength = 0.1f;
    mBarrageDelay = 2.0f;
    mFlashLightRadius = 0.0f;
    mFlashLightDuration = 0.0f;
    mBarrageMin = 3;
    mBarrageMax = 6;
    mEnergyDrain = 0.0f;
    mChargeUpEffect = 0;
    mAIBubbleSizeMultiplier = 1.0f;
    mAIBubbleScaleDistDivider = 20.0f;
    mAIBubbleScaleClamp = 10.0f;
    mAutoPitchScreenDist = 0.3f;
    mAutoTurnScreenDist = 0.3f;
    mTargetLockMaxDistance = 100.0f;
    mTargetLockMaxDistanceLose = 110.0f;
    mChargeSoundPitch = 1.0f;
    mOverheatSoundPitch = 1.0f;
    mName[0] = 0;

    mFireAnim = 0;
    mNoFirstPersonFireAnim = false;
    mDisplayRefire = true;
    mStopChargeSound = true;

    mMuzzleFlashModel = 0;
    mFireSoundStop = 0;
}

// FUNCTION: SWBF2 0x00679b70
WeaponClass::WeaponClass(WeaponClass* pBase, unsigned int uHash)
    : Factory<Weapon, WeaponClass, WeaponDesc>(pBase, uHash)
{

    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    mPostLoadInit = false;
    mStrengthLimit = pBase->mStrengthLimit;
    mNextCharge = 0;
    mHashWeaponName = pBase->mHashWeaponName;
    mNameHash = mHashWeaponName ? pBase->mNameHash : uHash;
    mHUDNameHash = pBase->mHUDNameHash;
    mSoldierAnimationWeapon = pBase->mSoldierAnimationWeapon;
    memcpy(mFilename, pBase->mFilename, sizeof(mFilename));
    mLabel = pBase->mLabel;
    mIconTexture = pBase->mIconTexture;
    mReticuleTexture = pBase->mReticuleTexture;
    mScopeTexture = pBase->mScopeTexture;
    mOrdnanceClass = pBase->mOrdnanceClass;
    mAmmoCounterClass = pBase->mAmmoCounterClass;
    mRoundsPerShot = pBase->mRoundsPerShot;
    mHeatPerShot = pBase->mHeatPerShot;
    mHeatRecoverRate = pBase->mHeatRecoverRate;
    mEnergyBarClass.Init(pBase->mEnergyBarClass.m_fEnergyMax, pBase->mEnergyBarClass.m_fEnergyOverheat);
    mBarrageMin = pBase->mBarrageMin;
    mBarrageMax = pBase->mBarrageMax;
    mBarrageDelay = pBase->mBarrageDelay;
    mSniperScope = pBase->mSniperScope;
    mModel = pBase->mModel;
    mHighResModel = pBase->mHighResModel;
    mZoomMin = pBase->mZoomMin;
    mZoomMax = pBase->mZoomMax;
    mZoomRate = pBase->mZoomRate;
    mZoomTurnDivisorMin = pBase->mZoomTurnDivisorMin;
    mZoomTurnDivisorMax = pBase->mZoomTurnDivisorMax;
    mZoomFirstPerson = pBase->mZoomFirstPerson;
    mReloadTime = pBase->mReloadTime;
    mMinRange = pBase->mMinRange;
    mOptimalRange = pBase->mOptimalRange;
    mMaxRange = pBase->mMaxRange;
    mMaxRangeDefault = pBase->mMaxRangeDefault;
    mTriggerSingle = pBase->mTriggerSingle;
    mExtremeRange = pBase->mExtremeRange;
    mLockOnRange = pBase->mLockOnRange;
    mLockOnAngle = pBase->mLockOnAngle;
    mLockOffAngle = pBase->mLockOffAngle;
    mTargetSides = pBase->mTargetSides;
    mTargetTypes = pBase->mTargetTypes;
    mAITargetTypes = pBase->mAITargetTypes;
    mFirePointOffset = pBase->mFirePointOffset;
    mMuzzleFlashEffect = pBase->mMuzzleFlashEffect;
    mFireSound = pBase->mFireSound;
    mFireLoopSound = pBase->mFireLoopSound;
    mFireEmptySound = pBase->mFireEmptySound;
    mReloadSound = pBase->mReloadSound;
    mChargeSound = pBase->mChargeSound;
    mChargedSound = pBase->mChargedSound;
    mChargeSoundPitch = pBase->mChargeSoundPitch;
    mOverheatSound = pBase->mOverheatSound;
    mOverheatSoundPitch = pBase->mOverheatSoundPitch;
    mFireSoundStop = pBase->mFireSoundStop;
    mRecoilStrengthHeavy = pBase->mRecoilStrengthHeavy;
    mRecoilStrengthLight = pBase->mRecoilStrengthLight;
    mRecoilLengthLight = pBase->mRecoilLengthLight;
    mRecoilLengthHeavy = pBase->mRecoilLengthHeavy;
    mRecoilDelayLight = pBase->mRecoilDelayLight;
    mRecoilDelayHeavy = pBase->mRecoilDelayHeavy;
    mRecoilDecayLight = pBase->mRecoilDecayLight;
    mRecoilDecayHeavy = pBase->mRecoilDecayHeavy;
    mChargeRateLight = pBase->mChargeRateLight;
    mChargeRateHeavy = pBase->mChargeRateHeavy;
    mMaxChargeStrengthHeavy = pBase->mMaxChargeStrengthHeavy;
    mMaxChargeStrengthLight = pBase->mMaxChargeStrengthLight;
    mTimeAtMaxCharge = pBase->mTimeAtMaxCharge;
    mWeaponChangeSound = pBase->mWeaponChangeSound;
    mFlashRadius = pBase->mFlashRadius;
    mFlashWidth = pBase->mFlashWidth;
    mFlashLength = pBase->mFlashLength;
    mFlashColor = pBase->mFlashColor;
    mFlashLightColor = pBase->mFlashLightColor;
    mFlashLightRadius = pBase->mFlashLightRadius;
    mFlashLightDuration = pBase->mFlashLightDuration;
    mFireAnim = pBase->mFireAnim;
    mEnergyDrain = pBase->mEnergyDrain;
    mInstantPlayFireAnim = pBase->mInstantPlayFireAnim;
    mIsOffhand = pBase->mIsOffhand;
    mReticuleInAimingOnly = pBase->mReticuleInAimingOnly;
    mAIUseBubbleCircle = pBase->mAIUseBubbleCircle;
    mAIBubbleSizeMultiplier = pBase->mAIBubbleSizeMultiplier;
    mAIBubbleScaleDistDivider = pBase->mAIBubbleScaleDistDivider;
    mAIBubbleScaleClamp = pBase->mAIBubbleScaleClamp;
    mAutoPitchScreenDist = pBase->mAutoPitchScreenDist;
    mAutoTurnScreenDist = pBase->mAutoTurnScreenDist;
    mTargetLockMaxDistance = pBase->mTargetLockMaxDistance;
    mTargetLockMaxDistanceLose = pBase->mTargetLockMaxDistanceLose;
    mScoreForMedalsType = pBase->mScoreForMedalsType;
    mMedalsTypeToUnlock = pBase->mMedalsTypeToUnlock;
    mMedalsTypeToLock = pBase->mMedalsTypeToLock;
    mChargeUpEffect = pBase->mChargeUpEffect;
    mMuzzleFlashModel = RedModel::_HashTable.Find(0x9DB10C98);
    mName[0] = 0;
    mDisplayRefire = pBase->mDisplayRefire;
    mStopChargeSound = pBase->mStopChargeSound;
    mNoFirstPersonFireAnim = pBase->mNoFirstPersonFireAnim;
}

// FUNCTION: SWBF2 0x0067a240
void WeaponClass::Read(PblFileChunk& chunk)
{
    char acBuffer[256];
    char acType[128];
    WeaponClass* pCharge = 0;
    WeaponClass* pClass = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x45505954:
        {
            memset(acType, 0, sizeof(acType));
            const char* pcType = child.ReadString(acType, sizeof(acType));
            const unsigned int uType = PblHash(pcType);
            if (Find(uType))
            {
                return;
            }

            pClass = pClass->Derive(uType);
            StringDB::Store(uType, pcType);
            strlcpy(pClass->mFilename, acType, sizeof(pClass->mFilename));
            _snprintf(acBuffer, sizeof(acBuffer), "weapons.%s", acType);
            char* pcUnderscore = strchr(acBuffer, '_');
            if (pcUnderscore)
            {
                *pcUnderscore = '.';
            }
            pcUnderscore = strchr(acBuffer, '_');
            if (pcUnderscore)
            {
                *pcUnderscore = '.';
            }
            pClass->mLabel = (unsigned short*)RedLocalizeDB::Find(PblHash(acBuffer), true);
            pCharge = pClass;
            break;
        }
        case 0x45534142:
            pClass = Find(PblHash(child.ReadString(acBuffer, sizeof(acBuffer))));
            break;
        case 0x504F5250:
        {
            const unsigned int uProperty = child._pFile->Read32();
            pCharge->SetProperty(uProperty, child.ReadString(acBuffer, sizeof(acBuffer)));
            if (pCharge->mNextCharge)
            {
                pCharge = pCharge->mNextCharge;
            }
            break;
        }
        }
    }

    for (WeaponClass* p = pClass; p; p = p->mNextCharge)
    {
        p->PostReadInit();
    }
    pCharge->mNextCharge = pCharge;
}

// FUNCTION: SWBF2 0x0067a450
void WeaponClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xFC9B0358:
        mFoleyFX[foleyFX_squat].SetID(pszValue, false, false);
        break;
    case 0xFFCC84B2:
        sscanf(pszValue, "%f", &mBarrageDelay);
        break;
    case 0xFBF47DBA:
        mNameHash = PblHash(pszValue);
        mHashWeaponName = true;
        for (int i = 0; i < (int)sizeof(mName) - 1; ++i)
        {
            mName[i] = pszValue[i];
        }
        mName[sizeof(mName) - 1] = 0;
        break;
    case 0xFAE6054D:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x01;
        }
        else
        {
            mTargetTypes &= ~0x01;
        }
        break;
    case 0xFAF37909:
        mAIUseBubbleCircle = atoi(pszValue) != 0;
        break;
    case 0xFBC7C100:
        sscanf(pszValue, "%f", &mAutoPitchScreenDist);
        break;
    case 0xF8899C5E:
    case 0x399BF05D:
        mOrdnanceClass = OrdnanceClass::Find(PblHash(pszValue));
        if (mOrdnanceClass)
        {
            mExtremeRange = mOrdnanceClass->GetRange();
        }
        break;
    case 0xF2E54D72:
    {
        int r = 255, g = 255, b = 255, a = 255;
        const int iCount = sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        if (iCount >= 4)
        {
            mFlashLightColor.a = 0xFF;
            mFlashLightColor.r = (unsigned char)(r * a / 255);
            mFlashLightColor.g = (unsigned char)(g * a / 255);
            mFlashLightColor.b = (unsigned char)(b * a / 255);
        }
        else if (iCount == 3)
        {
            mFlashLightColor.r = (unsigned char)r;
            mFlashLightColor.g = (unsigned char)g;
            mFlashLightColor.b = (unsigned char)b;
            mFlashLightColor.a = 0xFF;
        }
        break;
    }
    case 0xEF6119F3:
        mReloadSound.SetID(pszValue, false, false);
        break;
    case 0xF15AAEB2:
        mSniperScope = atoi(pszValue) != 0;
        break;
    case 0xF303AC7B:
        sscanf(pszValue, "%f", &mChargeDelayHeavy);
        break;
    case 0xF4A517D4:
        mOverheatSound.SetID(pszValue, false, false);
        break;
    case 0xF5701D6D:
        sscanf(pszValue, "%f", &mChargeRateLight);
        break;
    case 0xEB4230AC:
    {
        int r = 255, g = 255, b = 255, a = 255;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        mFlashColor.r = (unsigned char)r;
        mFlashColor.g = (unsigned char)g;
        mFlashColor.b = (unsigned char)b;
        mFlashColor.a = (unsigned char)a;
        break;
    }
    case 0xE10D2224:
        if (atoi(pszValue))
        {
            mTargetSides |= 0x01;
        }
        else
        {
            mTargetSides &= ~0x01;
        }
        break;
    case 0xD211149D:
        mFoleyFX[foleyFX_roll].SetID(pszValue, false, false);
        break;
    case 0xD6EE0E46:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x02;
        }
        else
        {
            mAITargetTypes &= ~0x02;
        }
        break;
    case 0xD8468816:
        mChargedSound.SetID(pszValue, false, false);
        break;
    case 0xE4AEFAB9:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x40;
        }
        else
        {
            mTargetTypes &= ~0x40;
        }
        break;
    case 0xE60CD1C3:
        mAIBubbleSizeMultiplier = (float)atof(pszValue);
        break;
    case 0xE6C12D74:
        sscanf(pszValue, "%f", &mMaxChargeStrengthLight);
        break;
    case 0xCFDE2FC0:
        mFireSound.SetID(pszValue, false, false);
        break;
    case 0xCDFE1739:
        mAITargetTypes = atoi(pszValue) ? -1 : 0;
        break;
    case 0xCF3649E6:
        sscanf(pszValue, "%f", &mRecoilStrengthLight);
        break;
    case 0xCFCCCBCE:
    {
        RedColor flashColor = { 0xFF, 0xFF, 0x32, 0xFF };
        RedColor lightColor;
        switch ((unsigned int)PblHash(pszValue))
        {
        case 0xB92CF7A1:
        {
            mFlashRadius = 0.6f;
            mFlashWidth = 0.15f;
            mFlashLength = 0.1f;
            RedColor c = { 0x3F, 0x3F, 0x0D, 0xFF };
            lightColor = c;
            break;
        }
        case 0xAE94DED9:
        {
            mFlashRadius = 1.8f;
            mFlashWidth = 0.45f;
            mFlashLength = 0.3f;
            RedColor c = { 0xBF, 0xBF, 0x25, 0xFF };
            lightColor = c;
            break;
        }
        case 0x84674572:
        {
            mFlashRadius = 1.0f;
            mFlashWidth = 0.25f;
            mFlashLength = 0.15f;
            RedColor c = { 0x7F, 0x7F, 0x19, 0xFF };
            lightColor = c;
            break;
        }
        default:
            return;
        }
        mFlashColor = flashColor;
        mFlashLightColor = lightColor;
        mFlashLightRadius = 0.0f;
        mFlashLightDuration = 0.0f;
        break;
    }
    case 0xCADB9D74:
        mFoleyFX[foleyFX_prone].SetID(pszValue, false, false);
        break;
    case 0xC57BC595:
        mInstantPlayFireAnim = atoi(pszValue) != 0;
        break;
    case 0xC915B156:
        if (sscanf(pszValue, "%f", &mLockOffAngle))
        {
            mLockOffAngle *= 0.017453292f;
        }
        break;
    case 0xCA7F5887:
        if (atoi(pszValue))
        {
            mTargetSides |= 0x04;
        }
        else
        {
            mTargetSides &= ~0x04;
        }
        break;
    case 0xC4409FF0:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x10;
        }
        else
        {
            mTargetTypes &= ~0x10;
        }
        break;
    case 0xB7C581AD:
        sscanf(pszValue, "%f", &mRecoilDelayHeavy);
        break;
    case 0xB97F1F52:
        sscanf(pszValue, "%f", &mRecoilDecayHeavy);
        break;
    case 0xB989FC18:
        mFoleyFX[foleyFX_rightWalk].SetID(pszValue, false, false);
        break;
    case 0xB7B2D011:
        sscanf(pszValue, "%f", &mFlashLength);
        break;
    case 0xB53B908E:
        mFireLoopSound.SetID(pszValue, false, false);
        break;
    case 0xB7226D43:
        sscanf(pszValue, "%f", &mZoomTurnDivisorMin);
        if (mZoomTurnDivisorMin == 0.0f)
        {
            mZoomTurnDivisorMin = 1.0f;
        }
        break;
    case 0xB77F3981:
        mFoleyFX[foleyFX_leftWalk].SetID(pszValue, false, false);
        break;
    case 0xB52F4E1A:
        sscanf(pszValue, "%f", &mOverheatSoundPitch);
        break;
    case 0xA9B93C09:
        sscanf(pszValue, "%f", &mRecoilDecayLight);
        break;
    case 0xAD37783D:
        sscanf(pszValue, "%f", &mZoomTurnDivisorMax);
        if (mZoomTurnDivisorMax == 0.0f)
        {
            mZoomTurnDivisorMax = 1.0f;
        }
        break;
    case 0xAD3E89E3:
        mFoleyFX[foleyFX_land].SetID(pszValue, false, false);
        break;
    case 0xA7C25382:
    {
        char acBuffer[257];
        strlcpy(acBuffer, pszValue, sizeof(acBuffer));
        char* pcParent = 0;
        char* pcAlertState = 0;
        char* pcName = strtok(acBuffer, " ");
        while (pcName && !*pcName)
        {
            pcName = strtok(0, " ");
        }
        if (pcName)
        {
            do
                pcParent = strtok(0, " ");
            while (pcParent && !*pcParent);
            if (pcParent)
            {
                do
                    pcAlertState = strtok(0, " ");
                while (pcAlertState && !*pcAlertState);
            }
        }
        mSoldierAnimationWeapon = SoldierAnimationBank::AddWeapon(pcName, pcParent, pcAlertState);
        break;
    }
    case 0xA3BFD787:
        sscanf(pszValue, "%f", &mFlashLightDuration);
        break;
    case 0xA61D280F:
        sscanf(pszValue, "%f", &mMaxChargeStrengthHeavy);
        break;
    case 0xA7678F8F:
    {
        const char* pcUnderscore = strchr(pszValue, '_');
        if (pcUnderscore)
        {
            mSoldierAnimationWeapon = SoldierAnimationBank::FindWeapon(pcUnderscore + 1);
        }
        else
        {
            mSoldierAnimationWeapon = SoldierAnimationBank::FindWeapon(pszValue);
        }
        break;
    }
    case 0x9FF840D6:
        mHUDNameHash = PblHash(pszValue);
        break;
    case 0x8DDBEB4B:
        mIconTexture = PblHash(pszValue);
        break;
    case 0x86FDC077:
        sscanf(pszValue, "%f", &mFlashRadius);
        break;
    case 0x8B3185DD:
        mHighResModel = RedModel::_HashTable.Find(PblHash(pszValue));
        break;
    case 0x8474EA2C:
        sscanf(pszValue, "%f", &mExtremeRange);
        if (mLockOnRange > mExtremeRange)
        {
            mExtremeRange = mLockOnRange;
        }
        break;
    case 0x83EEA041:
        sscanf(pszValue, "%f", &mFlashLightRadius);
        break;
    case 0x8316F444:
        mNextCharge = Derive(mId);
        if (mNextCharge)
        {
            sscanf(pszValue, "%f", &mNextCharge->mStrengthLimit);
        }
        break;
    case 0x7FD5A0F0:
        sscanf(pszValue, "%f", &mTargetLockMaxDistance);
        break;
    case 0x801F7ACB:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mMedalsTypeToLock = (short)iValue;
        break;
    }
    case 0x9573FB20:
        sscanf(pszValue, "%f", &mZoomRate);
        break;
    case 0x8F2A8613:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x80;
        }
        else
        {
            mTargetTypes &= ~0x80;
        }
        break;
    case 0x903574D1:
        sscanf(pszValue, "%f", &mRecoilLengthLight);
        break;
    case 0x94650003:
        sscanf(pszValue, "%f", &mFlashWidth);
        break;
    case 0x96A29630:
        mStopChargeSound = atol(pszValue) != 0;
        break;
    case 0x9BAD5828:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x04;
        }
        else
        {
            mAITargetTypes &= ~0x04;
        }
        break;
    case 0x9FC1A1AD:
    case 0x136A7450:
        sscanf(pszValue, "%f", &mReloadTime);
        if (mReloadTime < 0.01f)
        {
            mReloadTime = 0.01f;
        }
        break;
    case 0x7F012F70:
        sscanf(pszValue, "%f", &mLockOnRange);
        if (mLockOnRange > mExtremeRange)
        {
            mExtremeRange = mLockOnRange;
        }
        break;
    case 0x6B7F9189:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x80;
        }
        else
        {
            mAITargetTypes &= ~0x80;
        }
        break;
    case 0x6667A8E0:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mMedalsTypeToUnlock = (short)iValue;
        break;
    }
    case 0x6729F8E4:
        mNoFirstPersonFireAnim = atoi(pszValue) != 0;
        break;
    case 0x67CFFD1A:
        sscanf(pszValue, "%f", &mHeatPerShot);
        break;
    case 0x64667773:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x40;
        }
        else
        {
            mAITargetTypes &= ~0x40;
        }
        break;
    case 0x60329904:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x20;
        }
        else
        {
            mAITargetTypes &= ~0x20;
        }
        break;
    case 0x60DDBF8E:
        sscanf(pszValue, "%f", &mChargeRateHeavy);
        break;
    case 0x617E2F93:
        mReticuleTexture = PblHash(pszValue);
        break;
    case 0x7CD21CEA:
        sscanf(pszValue, "%f", &mRecoilDelayLight);
        break;
    case 0x7DBE88FA:
        if (sscanf(pszValue, "%f", &mLockOnAngle))
        {
            mLockOnAngle *= 0.072f;
        }
        break;
    case 0x778F9AAE:
        sscanf(pszValue, "%f", &mOptimalRange);
        break;
    case 0x70BB9AA6:
        mAIBubbleScaleDistDivider = (float)atof(pszValue);
        break;
    case 0x72B4743E:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x08;
        }
        else
        {
            mAITargetTypes &= ~0x08;
        }
        break;
    case 0x75C58748:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x02;
        }
        else
        {
            mTargetTypes &= ~0x02;
        }
        break;
    case 0x5FAB7F1D:
        sscanf(pszValue, "%f", &mAutoTurnScreenDist);
        break;
    case 0x5136C56B:
        mTargetTypes = atoi(pszValue) ? -1 : 0;
        break;
    case 0x47C86B4A:
        if (!mName[0])
        {
            const char* pcName = pszValue;
            for (const char* pc = pszValue; *pc;)
            {
                if (*pc++ == '_')
                {
                    pcName = pc;
                }
            }
            strlcpy(mName, pcName, sizeof(mName));
        }
        if (!mHashWeaponName)
        {
            mNameHash = PblHash(pszValue);
        }
        mModel = RedModel::_HashTable.Find(PblHash(pszValue));
        if (mModel)
        {
            mModel->GetParentBoneAndOffset(0xB3EA0EE8, mFirePointOffset);
        }
        break;
    case 0x41D6CEC0:
        mAIBubbleScaleClamp = (float)atof(pszValue);
        break;
    case 0x4274FC96:
        if (mModel)
        {
            mModel->GetParentBoneAndOffset(PblTEMPHash(pszValue), mFirePointOffset);
        }
        break;
    case 0x43BCEBE6:
        sscanf(pszValue, "%f", &mMaxRange);
        mMaxRangeDefault = false;
        break;
    case 0x48E05B6F:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x01;
        }
        else
        {
            mAITargetTypes &= ~0x01;
        }
        break;
    case 0x4B92A06A:
        sscanf(pszValue, "%f", &mRecoilLengthHeavy);
        break;
    case 0x4EE58508:
        mChargeSound.SetID(pszValue, false, false);
        break;
    case 0x580DC4EC:
        mFoleyFX[foleyFX_jump].SetID(pszValue, false, false);
        break;
    case 0x516C9F08:
        sscanf(pszValue, "%f", &mChargeDelayLight);
        break;
    case 0x578AD99E:
        mReticuleInAimingOnly = atoi(pszValue) != 0;
        break;
    case 0x57E7BBA3:
        sscanf(pszValue, "%f", &mHeatRecoverRate);
        break;
    case 0x5C0246C4:
    {
        int iValue = 0;
        sscanf(pszValue, "%i", &iValue);
        mFireAnim = iValue != 0;
        break;
    }
    case 0x5CD23209:
        sscanf(pszValue, "%f", &mTargetLockMaxDistanceLose);
        break;
    case 0x5F0AE167:
        if (atoi(pszValue))
        {
            mTargetSides |= 0x02;
        }
        else
        {
            mTargetSides &= ~0x02;
        }
        break;
    case 0x3F76AE9E:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x20;
        }
        else
        {
            mTargetTypes &= ~0x20;
        }
        break;
    case 0x2A96E8B4:
        sscanf(pszValue, "%f", &mZoomMax);
        if (mZoomMax == 0.0f)
        {
            mZoomMax = 1.0f;
        }
        mZoomTurnDivisorMax = mZoomMax;
        break;
    case 0x222090F6:
        sscanf(pszValue, "%f", &mRoundsPerShot);
        break;
    case 0x22624B58:
        sscanf(pszValue, "%d", &mFireSoundStop);
        break;
    case 0x281B9338:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x08;
        }
        else
        {
            mTargetTypes &= ~0x08;
        }
        break;
    case 0x20E1E379:
        mFireEmptySound.SetID(pszValue, false, false);
        break;
    case 0x1AA5EC35:
        sscanf(pszValue, "%i", &mBarrageMin);
        break;
    case 0x1EF25B6C:
        sscanf(pszValue, "%f", &mMinRange);
        break;
    case 0x1F63537B:
        mMuzzleFlashModel = RedModel::_HashTable.Find(PblHash(pszValue));
        break;
    case 0x3935C7CA:
        if (sscanf(pszValue, "%f", &mLockOnAngle))
        {
            mLockOnAngle *= 0.017453292f;
        }
        break;
    case 0x2ECD946F:
        mIsOffhand = atoi(pszValue) != 0;
        break;
    case 0x32E2A37A:
        mScopeTexture = PblHash(pszValue);
        break;
    case 0x352A4DB8:
        mWeaponChangeSound.SetID(pszValue, false, false);
        break;
    case 0x39AAA863:
        mMuzzleFlashEffect = FLEffect::Get(PblHash(pszValue));
        break;
    case 0x19007F1A:
        if (atoi(pszValue))
        {
            mTargetTypes |= 0x04;
        }
        else
        {
            mTargetTypes &= ~0x04;
        }
        break;
    case 0x17668629:
        sscanf(pszValue, "%f", &mEnergyDrain);
        break;
    case 0x1881B1A6:
        sscanf(pszValue, "%f", &mZoomMin);
        if (mZoomMin == 0.0f)
        {
            mZoomMin = 1.0f;
        }
        mZoomTurnDivisorMin = mZoomMin;
        break;
    case 0x18C5F75D:
        mChargeUpEffect = FLEffect::Get(PblHash(pszValue));
        break;
    case 0x14EC7E1D:
        mZoomFirstPerson = atoi(pszValue) != 0;
        break;
    case 0x12586DAE:
        sscanf(pszValue, "%f", &mChargeSoundPitch);
        break;
    case 0x12BA9F01:
        sscanf(pszValue, "%f", &mRecoilStrengthHeavy);
        break;
    case 0x10A991E4:
        mFoleyFX[foleyFX_rightRun].SetID(pszValue, false, false);
        break;
    case 0x0E8B2D62:
        mFoleyFX[foleyFX_stand].SetID(pszValue, false, false);
        break;
    case 0x0EE66C41:
        sscanf(pszValue, "%f", &mTimeAtMaxCharge);

    case 0xED9D56DB:
        mTriggerSingle = atoi(pszValue) != 0;
        break;
    case 0x1064C0A7:
        mFoleyFX[foleyFX_leftRun].SetID(pszValue, false, false);
        break;
    case 0x08F174A6:
        if (atoi(pszValue))
        {
            mAITargetTypes |= 0x10;
        }
        else
        {
            mAITargetTypes &= ~0x10;
        }
        break;
    case 0x01A333E4:
        mDisplayRefire = atoi(pszValue) != 0;
        break;
    case 0x04BAE44B:
        sscanf(pszValue, "%i", &mBarrageMax);
        break;
    case 0x059DF9F0:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mScoreForMedalsType = (short)iValue;
        break;
    }
    default:
        if (!mEnergyBarClass.SetProperty(uProperty, pszValue))
        {
            mAmmoCounterClass.SetProperty(uProperty, pszValue);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0067bb50
void WeaponClass::PostReadInit()
{
    if (mAmmoCounterClass.mAmmoPerRound == 0.0f)
    {
        mRoundsPerShot = 0.0f;
    }
    for (WeaponClass* p = this; p; p = p->mNextCharge)
    {
        mEnergyBarClass.SetPropertyEnergyCost(p->mHeatPerShot);
    }
}

// FUNCTION: SWBF2 0x0067bc80
void WeaponClass::CreateMuzzleFlash(const PblMatrix& mFire) const
{
    if (!mMuzzleFlashEffect)
    {
        return;
    }
    if (!CameraManager::sInstance->IsInsideAnyCamera(*reinterpret_cast<const PblVector3*>(mFire.m[3]), 30.0f, 70.0f))
    {
        return;
    }
    if (FLEffectObject* pEffect = mMuzzleFlashEffect->BuildEffect())
    {
        pEffect->AttachEffectToMatrix(&mFire, false);
    }
}

// FUNCTION: SWBF2 0x0067bd00
int WeaponClass::GetBarrageMin()
{
    return mBarrageMin;
}

// FUNCTION: SWBF2 0x0067bd10
int WeaponClass::GetBarrageMax()
{
    return mBarrageMax;
}

// FUNCTION: SWBF2 0x0067bd20
float WeaponClass::GetBarrageDelay() const
{
    return mBarrageDelay;
}

// FUNCTION: SWBF2 0x0067bd30
void WeaponClass::RenderFlash(const PblVector3& vPos, const PblVector3& vDir, float fScale) const
{
    if (!(mFlashRadius > 0.0f) || !mMuzzleFlashModel)
    {
        return;
    }

    float fAlpha = (float)mFlashColor.a;
    float fFade = fAlpha * 2.0f * fScale;
    if (!(fAlpha > fFade))
    {
        fFade = fAlpha;
    }
    RedColor color;
    color.b = mFlashColor.b;
    color.g = mFlashColor.g;
    color.r = mFlashColor.r;
    color.a = (unsigned char)(int)fFade;

    const float r = mFlashRadius;
    PblVector3 vSide(vDir.y * 0.0f - vDir.z, vDir.z * 0.0f - vDir.x * 0.0f, vDir.x - vDir.y * 0.0f);
    PblMatrix mFlash;
    mFlash.m[0][0] = vSide.x * r;
    mFlash.m[0][1] = vSide.y * r;
    mFlash.m[0][2] = vSide.z * r;
    mFlash.m[0][3] = 0.0f;
    mFlash.m[1][0] = (vDir.y * vSide.z - vDir.z * vSide.y) * r;
    mFlash.m[1][1] = (vDir.z * vSide.x - vDir.x * vSide.z) * r;
    mFlash.m[1][2] = (vDir.x * vSide.y - vDir.y * vSide.x) * r;
    mFlash.m[1][3] = 0.0f;
    mFlash.m[2][0] = vDir.x * r;
    mFlash.m[2][1] = vDir.y * r;
    mFlash.m[2][2] = vDir.z * r;
    mFlash.m[2][3] = 0.0f;
    mFlash.m[3][0] = vPos.x;
    mFlash.m[3][1] = vPos.y;
    mFlash.m[3][2] = vPos.z;
    mFlash.m[3][3] = 1.0f;
    mMuzzleFlashModel->Render(&mFlash, 0, color, 0, 0);
}

// GLOBAL: SWBF2 0x01f9a73c
// SYNTHETIC: SWBF2 0x00407020
// `dynamic initializer for 'Weapon::rttiHashWeapon''
PblHash Weapon::rttiHashWeapon("Weapon");
char Weapon::rttiNameWeapon[] = "Weapon";

// GLOBAL: SWBF2 0x01f9a880
// SYNTHETIC: SWBF2 0x00407030
// `dynamic initializer for 'WeaponClass::rttiHashWeaponClass''
PblHash WeaponClass::rttiHashWeaponClass("WeaponClass");
char WeaponClass::rttiNameWeaponClass[] = "WeaponClass";

// GLOBAL: SWBF2 0x01f9a318
// SYNTHETIC: SWBF2 0x00407040
// `dynamic initializer for 'WeaponClass::sWeaponChangeUnableSound''
// SYNTHETIC: SWBF2 0x007697e0
// `dynamic atexit destructor for 'WeaponClass::sWeaponChangeUnableSound''
GameSound WeaponClass::sWeaponChangeUnableSound;

// GLOBAL: SWBF2 0x01f9a830
// SYNTHETIC: SWBF2 0x00407000
// `dynamic initializer for 'Weapon::sMemoryPool''
// SYNTHETIC: SWBF2 0x007697d0
// `dynamic atexit destructor for 'Weapon::sMemoryPool''
MemoryPool Weapon::sMemoryPool("Weapon", 0x110);

void Weapon::operator delete(void* p)
{
    sMemoryPool.Free(p);
}

void* Weapon::operator new(size_t uSize)
{
    return sMemoryPool.Allocate(uSize);
}

// FUNCTION: SWBF2 0x00677610
bool Weapon::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeapon;
}

// FUNCTION: SWBF2 0x00677630
unsigned int Weapon::GetDerivedRtti() const
{
    return rttiHashWeapon;
}

// FUNCTION: SWBF2 0x00677640
const char* Weapon::GetDerivedRttiName() const
{
    return rttiNameWeapon;
}

// FUNCTION: SWBF2 0x00677660
bool Weapon::IsLocked() const
{
    return GetHandleObject(mTarget) != 0;
}

// FUNCTION: SWBF2 0x006776a0
const GameObject* Weapon::GetLocked() const
{
    return GetHandleObject(mTarget);
}

// FUNCTION: SWBF2 0x006776d0
int Weapon::GetLockedTargetBodyId() const
{
    return mTargetBodyID;
}

// FUNCTION: SWBF2 0x006776e0
void Weapon::SetKickSpreadFactor(float)
{
}

// FUNCTION: SWBF2 0x006776f0
void Weapon::SetYawSpreadFactor(float)
{
}

// FUNCTION: SWBF2 0x00677700
float Weapon::GetYawSpread() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x00677710
float Weapon::GetPitchSpread() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x00677720
bool Weapon::Deflect(const Ordnance*, const PblVector3&, const PblVector3&)
{
    return false;
}

// FUNCTION: SWBF2 0x00677730
bool Weapon::ShouldShowReticule() const
{
    return true;
}

// FUNCTION: SWBF2 0x00677740
bool Weapon::IsMelee() const
{
    return false;
}

// FUNCTION: SWBF2 0x00677750
bool Weapon::IsMeleeThrow() const
{
    return false;
}

// FUNCTION: SWBF2 0x00677760
bool Weapon::OverrideSoldierVelocity(float&, float&, float&, float&) const
{
    return false;
}

// FUNCTION: SWBF2 0x00677770
bool Weapon::OverrideSoldierEnergyRestore(float&) const
{
    return false;
}

// FUNCTION: SWBF2 0x00677780
bool Weapon::OverrideAimer()
{
    return false;
}

// FUNCTION: SWBF2 0x00677790
bool Weapon::IsBusy() const
{
    return false;
}

// SYNTHETIC: SWBF2 0x006777a0
// Weapon::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006777d0
bool WeaponClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponClass;
}

// FUNCTION: SWBF2 0x006777f0
unsigned int WeaponClass::GetDerivedRtti()
{
    return rttiHashWeaponClass;
}

// FUNCTION: SWBF2 0x00677800
const char* WeaponClass::GetDerivedRttiName()
{
    return rttiNameWeaponClass;
}

// FUNCTION: SWBF2 0x00677810
bool WeaponClass::PostLoadInit()
{
    if (mPostLoadInit)
    {
        return false;
    }
    mPostLoadInit = true;
    if (mNextCharge)
    {
        mNextCharge->PostLoadInit();
    }
    return true;
}

// FUNCTION: SWBF2 0x00677840
Weapon::Weapon(const WeaponClass* pClass, const WeaponDesc& desc)
{
    mStart = const_cast<WeaponClass*>(pClass);
    mClass = const_cast<WeaponClass*>(pClass);
    mRenderClass = const_cast<WeaponClass*>(pClass);
    mOwner = desc.mOwner;
    mAimer = desc.mAimer;
    mTrigger = desc.mTrigger;
    mReload = desc.mReload;

    if (desc.m_pAmmoCounter)
    {
        m_pAmmoCounter = desc.m_pAmmoCounter->AddRef();
    }
    else
    {
        void* p = AmmoCounter::sMemoryPool.Allocate(sizeof(AmmoCounter));
        m_pAmmoCounter = p ? new (p) AmmoCounter(&pClass->mAmmoCounterClass, desc.mNumClips) : 0;
    }

    EnergyBar* pEnergyBar;
    if (desc.m_pEnergyBar)
    {
        pEnergyBar = desc.m_pEnergyBar->AddRef();
    }
    else
    {
        pEnergyBar = static_cast<EnergyBar*>(EnergyBar::sMemoryPool.Allocate(sizeof(EnergyBar)));
        if (pEnergyBar)
        {
            pEnergyBar->m_pClass = &pClass->mEnergyBarClass;
            pEnergyBar->m_fEnergy = pClass->mEnergyBarClass.m_fEnergyMax;
            pEnergyBar->m_bOverheat = 0;
            pEnergyBar->m_uiRefCount = 1;
        }
    }
    mHideWeapon = 0;
    mFiredFlag = 0;
    mSelectedFlag = 0;
    m_pEnergyBar = pEnergyBar;

    mState = IDLE;
    mStateTimer = 0.0f;
    mStateLimit = 1.0f;
    mZoom = pClass->mZoomMin;
    mZoomTurnScale = pClass->mZoomTurnDivisorMin;
    mSoldierAnimationMap = -1;

    mChargeUpEmitter.mObject = 0;
    mChargeUpEmitter.mSavedHandleId = 0;
    mSkip = false;
    mCharged = false;
    mSkipTime = 0.0f;
    mDeactivateScheduled = 0;
    mTarget.mObject = 0;
    mTarget.mSavedHandleId = 0;
    mTargetBodyID = -1;

    mAimer->SetWeapon(this);

    if (pClass->mSoldierAnimationWeapon != -1 && mOwner &&
        mOwner->GetSoldierAnimationBank() != SoldierAnimationBank::INVALID_BANK)
    {
        mSoldierAnimationMap = SoldierAnimationBank::FindMap(
            mOwner->GetSoldierAnimationBank(),
            (SoldierAnimationBank::WEAPON)pClass->mSoldierAnimationWeapon);
    }

    Aimer* pFirst = mAimer;
    while (mAimer->mClass->mHierarchyLevel > 0 && mAimer->mNextAimer != pFirst)
    {
        mAimer = mAimer->mNextAimer;
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        ActivatePhysics();
    }

    m_iSoldierState = 0;
    mLastFireTime = 0.0f;
}

// FUNCTION: SWBF2 0x00677ab0
Weapon::~Weapon()
{
    mSoundControl.Stop(true);
    mSoundControlFire.Stop(true);
    mFoleyFXControl.Stop(true);
    m_pEnergyBar->Release();
    m_pAmmoCounter->Release();
}

// FUNCTION: SWBF2 0x00677b70
void Weapon::Init()
{
    if (BonusManager::sInstance.IsTeamBonusActive(GetTeam(mOwner->GetGameObject()), 0xc81956e6))
    {
        float fDefault = m_pAmmoCounter->mDefaultMaxClips;
        if (fDefault < 1.7014117e38f)
        {
            m_pAmmoCounter->mMaxClips = fDefault * 2.0f;
            m_pAmmoCounter->mNumClips = m_pAmmoCounter->mMaxClips;
        }
    }
    mCurChargeStrengthHeavy = 0.0f;
    mCurChargeStrengthLight = 0.0f;
}

// FUNCTION: SWBF2 0x00677bf0
void Weapon::UpdateZoom(float fDeltaTime, float fInput)
{
    const WeaponClass* pClass = mClass;
    float fZoom = mZoom * fInput * pClass->mZoomRate * fDeltaTime + mZoom;
    if (pClass->mZoomMin > fZoom)
    {
        fZoom = pClass->mZoomMin;
    }
    else if (fZoom > pClass->mZoomMax)
    {
        fZoom = pClass->mZoomMax;
    }
    mZoom = fZoom;

    float fRange = pClass->mZoomMax - pClass->mZoomMin;
    if (fRange > 0.0001f)
    {
        float t = (fZoom - pClass->mZoomMin) / fRange;
        mZoomTurnScale = (1.0f - t) * pClass->mZoomTurnDivisorMin + pClass->mZoomTurnDivisorMax * t;
    }
    else
    {
        mZoomTurnScale = pClass->mZoomTurnDivisorMax;
    }
}

// FUNCTION: SWBF2 0x00677ca0
bool Weapon::CycleZoom(bool bCycle)
{
    const WeaponClass* pClass = mClass;
    if (!bCycle)
    {
        mZoom = pClass->mZoomMin;
        mZoomTurnScale = pClass->mZoomTurnDivisorMin;
        return true;
    }
    if (mZoom == pClass->mZoomMin && pClass->mZoomMin != pClass->mZoomMax && pClass->mZoomRate == 0.0f)
    {
        mZoom = pClass->mZoomMax;
        mZoomTurnScale = pClass->mZoomTurnDivisorMax;
        return true;
    }
    mZoom = pClass->mZoomMin;
    mZoomTurnScale = pClass->mZoomTurnDivisorMin;
    return false;
}

// FUNCTION: SWBF2 0x00677d40
bool Weapon::ZoomFirstPerson() const
{
    if (mClass->mZoomFirstPerson)
    {
        return true;
    }
    if (!mOwner || !mOwner->GetGameObject())
    {
        return false;
    }
    return GetEntityType(mOwner->GetGameObject()) != ENT_SOLDIER &&
           GetEntityType(mOwner->GetGameObject()) != ENT_DROIDEKA;
}

// FUNCTION: SWBF2 0x00677db0
void Weapon::NotifySoldierState(int iState, float, bool, bool, const PblVector3&)
{
    if (iState == 8)
    {
        m_iSoldierState = 4;
    }
    else if (iState == 7)
    {
        m_iSoldierState = 6;
    }
    else
    {
        m_iSoldierState = iState;
    }

    if (!SoldierCanOperate() && mState > IDLE)
    {
        if (mState <= FIRE2)
        {
            ExitFire();
        }
        else if (mState == CHARGE)
        {
            ExitCharge();
        }
        else
        {
            return;
        }
        mState = IDLE;
        EnterIdle();
    }
}

// FUNCTION: SWBF2 0x00677e60
bool Weapon::SoldierCanOperate() const
{
    return m_iSoldierState <= 3 || m_iSoldierState == 6;
}

// FUNCTION: SWBF2 0x00677e80
bool Weapon::OverrideSoldierControls()
{
    if ((mState == IDLE || mState == CHARGE || mState == RELOAD || mState >= OVERHEAT) &&
        mStateTimer >= 0.4f)
    {
        return false;
    }

    mOwner->mControlSprint.Clear();
    if (m_iSoldierState == 3 && mOwner->EndSprint())
    {
        m_iSoldierState = 0;
    }
    return true;
}

// FUNCTION: SWBF2 0x00677ef0
float Weapon::GetOrdnanceVelocity() const
{
    OrdnanceClass* pOrdnance = mClass->mOrdnanceClass;
    if (!pOrdnance)
    {
        return 0.0f;
    }
    if (mState == CHARGE)
    {
        float fCharge = mStateTimer / mStateLimit;
        while (fCharge > pOrdnance->mNextCharge->mStrengthLimit && pOrdnance->mNextCharge != pOrdnance)
        {
            pOrdnance = pOrdnance->mNextCharge;
        }
        if (pOrdnance->mStrengthLimit <= fCharge &&
            pOrdnance->mStrengthLimit < pOrdnance->mNextCharge->mStrengthLimit)
        {
            float t = (fCharge - pOrdnance->mStrengthLimit) /
                      (pOrdnance->mNextCharge->mStrengthLimit - pOrdnance->mStrengthLimit);
            float fLow = pOrdnance->GetVelocity();
            float fHigh = pOrdnance->mNextCharge->GetVelocity();
            return (fHigh - fLow) * t + fLow;
        }
    }
    return pOrdnance->GetVelocity();
}

// FUNCTION: SWBF2 0x00677fb0
float Weapon::GetOrdnanceGravity() const
{
    OrdnanceClass* pOrdnance = mClass->mOrdnanceClass;
    if (!pOrdnance)
    {
        return 0.0f;
    }
    if (mState == CHARGE)
    {
        float fCharge = mStateTimer / mStateLimit;
        while (fCharge > pOrdnance->mNextCharge->mStrengthLimit && pOrdnance->mNextCharge != pOrdnance)
        {
            pOrdnance = pOrdnance->mNextCharge;
        }
        if (pOrdnance->mStrengthLimit <= fCharge &&
            pOrdnance->mStrengthLimit < pOrdnance->mNextCharge->mStrengthLimit)
        {
            float t = (fCharge - pOrdnance->mStrengthLimit) /
                      (pOrdnance->mNextCharge->mStrengthLimit - pOrdnance->mStrengthLimit);
            float fLow = pOrdnance->GetGravity();
            float fHigh = pOrdnance->mNextCharge->GetGravity();
            return (fHigh - fLow) * t + fLow;
        }
    }
    return pOrdnance->GetGravity();
}

// FUNCTION: SWBF2 0x00678070
int Weapon::GetSoldierAnimationWeapon() const
{
    return mClass->mSoldierAnimationWeapon;
}

// FUNCTION: SWBF2 0x00678080
void Weapon::Select(unsigned int, bool bOffhand)
{
    GameObject* pObject = mOwner->GetGameObject();
    m_iSoldierState = 0;
    mSelectedFlag = 1;
    mMuzzleFlashStartTime = GameLoop::GetMissionTime() - 1.0f;
    if (!bOffhand)
    {
        mClass->mWeaponChangeSound.Play(&GetTranslation(GetCollisionMatrix(pObject)),
                                        &GetVelocity(pObject), false, true);
    }
}

// FUNCTION: SWBF2 0x00678100
void Weapon::Deselect(unsigned int, bool)
{
    if (mChargeUpEmitter)
    {
        mChargeUpEmitter->StopAndFinish();
        mChargeUpEmitter = 0;
    }
    mSoundControl.Stop(true);
    mSoundControlFire.Stop(true);
    mSelectedFlag = 0;
    m_iSoldierState = 0;
    if (mState == CHARGE)
    {
        ExitCharge();
        mState = IDLE;
        EnterIdle();
    }
}

// FUNCTION: SWBF2 0x006781b0
bool Weapon::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    if (mOwner->mPlayerId < 0)
    {
        if (mSkip)
        {
            mSkipTime = fDeltaTime;
            return true;
        }
        fDeltaTime = mSkipTime + fDeltaTime;
        mSkipTime = 0.0f;
    }

    if (mState != CHARGE)
    {
        if (mChargeUpEmitter)
        {
            mChargeUpEmitter->StopAndFinish();
            mChargeUpEmitter = 0;
        }
        if (!mOwner)
        {
            mSoundControl.Stop(true);
            mSoundControlFire.Stop(true);
        }
    }

    GameObject* pObject = mOwner->GetGameObject();
    const PblVector3& vPos = GetTranslation(GetCollisionMatrix(pObject));
    const PblVector3& vVel = GetVelocity(pObject);
    if (mSoundControl.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControl.Update(&mSoundProps, &vPos, &vVel);
    }
    if (mSoundControlFire.mFlags & GameSoundControllable::flags_playing)
    {
        mSoundControlFire.Update(&mSoundPropsFire, &vPos, &vVel);
    }
    if (mFoleyFXControl.mFlags & GameSoundControllable::flags_playing)
    {
        mFoleyFXControl.Update(&mFoleyFXProps, &vPos, &vVel);
    }

    m_pEnergyBar->AddEnergy(mClass->mHeatRecoverRate * fDeltaTime);

    bool bExit = false;
    for (;;)
    {
        switch (mState)
        {
        case IDLE:
            if (mSelectedFlag && CheckFire() && SoldierCanOperate())
            {
                ExitIdle();
                mState = (WeaponState)(mClass->mFireAnim + 1);
                EnterFire();
            }
            else if (mSelectedFlag && CheckCharge())
            {
                ExitIdle();
                mState = CHARGE;
                EnterCharge();
            }
            else if (mSelectedFlag && (CheckReload() || ForceReload()))
            {
                ExitIdle();
                if (CheckEmpty())
                {
                    mState = EMPTY;
                    EnterEmpty();
                }
                else
                {
                    mState = RELOAD;
                    EnterReload();
                }
            }
            else if (UpdateIdle(fDeltaTime))
            {
                return true;
            }
            break;

        case FIRE:
        case FIRE2:
            if (FireDone())
            {
                ExitFire();
                if (CheckReload())
                {
                    if (CheckEmpty())
                    {
                        mState = EMPTY;
                        EnterEmpty();
                    }
                    else
                    {
                        mState = RELOAD;
                        EnterReload();
                    }
                }
                else
                {
                    mState = IDLE;
                    EnterIdle();
                }
            }
            else if (UpdateFire(fDeltaTime))
            {
                if (mOwner && mOwner->mPlayerId >= 0 && mSelectedFlag && mStateTimer > 1.0f &&
                    mTrigger->IsPressed())
                {
                    mSoundPropsFire = mClass->mFireEmptySound;
                    mSoundControlFire.Play(&mSoundPropsFire, &GetTranslation(GetCollisionMatrix(pObject)),
                                           &GetVelocity(pObject), true);
                }
                return true;
            }
            break;

        case CHARGE:
            if (mSelectedFlag && CheckFire() && SoldierCanOperate())
            {
                ExitCharge();
                mState = (WeaponState)(mClass->mFireAnim + 1);
                EnterFire();
            }
            else if (mSelectedFlag && CheckCharge())
            {
                if (UpdateCharge(fDeltaTime))
                {
                    return true;
                }
            }
            else
            {
                ExitCharge();
                mState = IDLE;
                EnterIdle();
            }
            break;

        case RELOAD:
            if (!mSelectedFlag)
            {
                bExit = true;
            }
            if (ReloadDone())
            {
                ExitReload();
                mState = IDLE;
                EnterIdle();
            }
            else if (UpdateReload(fDeltaTime))
            {
                if (mOwner && mOwner->mPlayerId >= 0 && mSelectedFlag && mTrigger->IsPressed())
                {
                    mSoundPropsFire = mClass->mFireEmptySound;
                    mSoundControlFire.Play(&mSoundPropsFire, &GetTranslation(GetCollisionMatrix(pObject)),
                                           &GetVelocity(pObject), true);
                }
                return true;
            }
            if (bExit)
            {
                return true;
            }
            break;

        case OVERHEAT:
            if (UpdateIdle(fDeltaTime))
            {
                if (mSelectedFlag && mTrigger->IsPressed() && mOwner && mOwner->mPlayerId >= 0)
                {
                    mSoundPropsFire = mClass->mFireEmptySound;
                    mSoundControlFire.Play(&mSoundPropsFire, &GetTranslation(GetCollisionMatrix(pObject)),
                                           &GetVelocity(pObject), true);
                }
                return true;
            }
            break;

        case EMPTY:
            if (!CheckEmpty())
            {
                ExitEmpty();
                mState = RELOAD;
                EnterReload();
            }
            else if (UpdateEmpty(fDeltaTime))
            {
                if (mOwner && mOwner->mPlayerId >= 0 && mSelectedFlag && mTrigger->IsPressed())
                {
                    mSoundPropsFire = mClass->mFireEmptySound;
                    mSoundControlFire.Play(&mSoundPropsFire, &GetTranslation(GetCollisionMatrix(pObject)),
                                           &GetVelocity(pObject), true);
                }
                return true;
            }
            break;

        default:
            return true;
        }
    }
}

// FUNCTION: SWBF2 0x006787c0
bool Weapon::WillBecomeBusy() const
{
    switch (mState)
    {
    case EMPTY:
        if (CheckEmpty())
        {
            return false;
        }

    case RELOAD:
        if (!ReloadDone())
        {
            return false;
        }

    case IDLE:
        return CheckFire() || CheckCharge();
    case FIRE:
    case FIRE2:
        return !FireDone();
    case CHARGE:
        return CheckFire() || CheckCharge();
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x00678880
float Weapon::GetSoundRadius() const
{
    switch (mState)
    {
    case IDLE:
    case EMPTY:
        if (CheckFire())
        {
            return mSoundPropsFire.GetMuteDistance();
        }
        return 0.0f;
    case FIRE:
    case FIRE2:
    case CHARGE:
    case RELOAD:
        return mSoundProps.GetMuteDistance();
    default:
        return 0.0f;
    }
}

// FUNCTION: SWBF2 0x006788e0
void Weapon::EnterIdle()
{
    WeaponState eOld = mState;
    mClass = mStart;
    mStateTimer = 0.0f;
    mState = CheckOverheat() ? OVERHEAT : IDLE;
    if (eOld != OVERHEAT && mState == OVERHEAT)
    {
        mSoundProps = mClass->mOverheatSound;
        GameObject* pObject = mOwner->GetGameObject();
        mSoundControl.Play(&mSoundProps, &GetTranslation(GetCollisionMatrix(mOwner->GetGameObject())),
                           &GetVelocity(pObject), true);
        if (Snd::VoiceVirtual* pVoice = mSoundControl.GetVoiceVirtual())
        {
            pVoice->SetPitch(mSoundProps.GetPitch() * mClass->mOverheatSoundPitch);
        }
    }
}

// FUNCTION: SWBF2 0x006789c0
bool Weapon::UpdateIdle(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    WeaponState eOld = mState;
    mState = CheckOverheat() ? OVERHEAT : IDLE;
    if (eOld == OVERHEAT)
    {
        if (mState == eOld)
        {
            if (Snd::VoiceVirtual* pVoice = mSoundControl.GetVoiceVirtual())
            {
                const EnergyBarClass* pBarClass = m_pEnergyBar->m_pClass;
                float t = (pBarClass->m_fEnergyMax - pBarClass->m_fEnergyOverheat) /
                          (pBarClass->m_fEnergyMax - m_pEnergyBar->m_fEnergy);
                pVoice->SetPitch(mSoundProps.GetPitch() *
                                 ((1.0f - t) * mClass->mOverheatSoundPitch + t));
            }
        }
        else
        {
            mSoundControl.Stop(true);
        }
    }
    if (mDeactivateScheduled && mState == IDLE)
    {
        Thread::DeactivateThread();
    }
    return true;
}

// FUNCTION: SWBF2 0x00678a90
void Weapon::ExitIdle()
{
}

// FUNCTION: SWBF2 0x00678aa0
void Weapon::PlayFireSound(bool bNoLoop)
{
    GameObject* pObject = mOwner->GetGameObject();
    const PblVector3* pPos = mAimer ? &mAimer->mFirePos : &GetTranslation(GetCollisionMatrix(pObject));
    const PblVector3& vVel = GetVelocity(pObject);
    if (mClass->mFireSoundStop && (mSoundControlFire.mFlags & GameSoundControllable::flags_playing))
    {
        mSoundControlFire.Stop(true);
    }
    mSoundPropsFire = mClass->mFireSound;
    mSoundProps = mClass->mFireLoopSound;
    mSoundControlFire.Play(&mSoundPropsFire, pPos, &vVel, true);
    if (!bNoLoop)
    {
        mSoundControl.Play(&mSoundProps, pPos, &vVel, true);
    }
}

// FUNCTION: SWBF2 0x00678b70
void Weapon::StopFireSound()
{
    mSoundControl.Stop(true);
}

// FUNCTION: SWBF2 0x00678b80
void Weapon::EnterFire()
{
    if (!NetGame::IsEnabled())
    {
        PlayFireSound(false);
    }
}

// FUNCTION: SWBF2 0x00678bb0
unsigned int Weapon::GetNameHash() const
{
    if (!mClass)
    {
        return 0;
    }
    return mClass->mHUDNameHash ? mClass->mHUDNameHash : mClass->mNameHash;
}

// FUNCTION: SWBF2 0x00678bd0
void Weapon::PlayFoleyFX(unsigned int uIndex)
{
    if (mOwner)
    {
        GameObject* pObject = mOwner->GetGameObject();
        mFoleyFXProps = mClass->mFoleyFX[uIndex];
        mFoleyFXControl.Play(&mFoleyFXProps, &GetTranslation(mOwner->GetMatrix()),
                             pObject ? &GetVelocity(pObject) : 0, true);
    }
}

// FUNCTION: SWBF2 0x00678c40
void Weapon::ActivatePhysics()
{
    mDeactivateScheduled = 0;
    if (!mThreadListNode._pList)
    {
        Thread::ActivateThread(-30, PER_TURN);
    }
}

// FUNCTION: SWBF2 0x00678c60
void Weapon::DeactivatePhysics()
{
    mDeactivateScheduled = 1;
}

// FUNCTION: SWBF2 0x00678c70
bool Weapon::UpdateFire(float)
{
    return true;
}

// FUNCTION: SWBF2 0x00678c80
void Weapon::ExitFire()
{
    StopFireSound();
}

// FUNCTION: SWBF2 0x00678c90
void Weapon::EnterCharge()
{
    mCharged = false;
    mClass = mStart;
    GameObject* pObject = mOwner->GetGameObject();
    if (!IsRtti(0xe33ded9d) || NetGame::GetJoystickIndex(mOwner->mPlayerId) >= 0)
    {
        mSoundProps = mClass->mChargeSound;
        const PblVector3* pPos = mAimer ? &mAimer->mFirePos : &GetTranslation(GetCollisionMatrix(pObject));
        mSoundControl.Play(&mSoundProps, pPos, &GetVelocity(pObject), true);
    }
    if (Snd::VoiceVirtual* pVoice = mSoundControl.GetVoiceVirtual())
    {
        pVoice->SetPitch(Clamp(mSoundProps.GetPitch() * mClass->mChargeSoundPitch, 0.2f, 4.0f));
    }

    const WeaponClass* pClass = mClass;
    mCurChargeStrengthHeavy = 0.0f;
    mCurChargeStrengthLight = 0.0f;
    mCurChargeRateLight = 1.0f;
    mCurChargeRateHeavy = 1.0f;
    mCurTimeAtMaxCharge = pClass->mTimeAtMaxCharge;
    mCurChargeDelayHeavy = pClass->mChargeDelayHeavy;
    mCurChargeDelayLight = pClass->mChargeDelayLight;
    if (0.0f > pClass->mChargeRateHeavy)
    {
        mCurChargeStrengthHeavy = pClass->mMaxChargeStrengthHeavy;
    }
    if (0.0f > pClass->mChargeRateLight)
    {
        mCurChargeStrengthLight = pClass->mMaxChargeStrengthLight;
    }
}

// FUNCTION: SWBF2 0x00678e00
bool Weapon::UpdateCharge(float fDeltaTime)
{
    float fCharge = mStateTimer / mStateLimit;
    while (fCharge > mClass->mNextCharge->mStrengthLimit && mClass->mNextCharge != mClass)
    {
        mClass = mClass->mNextCharge;
    }

    if (mOwner->mPlayerId >= 0)
    {
        bool bHeavyFull = false;
        mCurChargeDelayHeavy -= fDeltaTime;
        mCurChargeDelayLight -= fDeltaTime;
        if (mCurChargeDelayHeavy <= 0.0f)
        {
            const WeaponClass* pClass = mClass;
            mCurChargeStrengthHeavy += pClass->mChargeRateHeavy * fDeltaTime * mCurChargeRateHeavy;
            mCurChargeStrengthHeavy = mCurChargeStrengthHeavy > pClass->mMaxChargeStrengthHeavy
                                          ? pClass->mMaxChargeStrengthHeavy
                                          : mCurChargeStrengthHeavy;
            if (mCurChargeStrengthHeavy == pClass->mMaxChargeStrengthHeavy)
            {
                bHeavyFull = true;
            }
        }
        if (mCurChargeDelayLight <= 0.0f)
        {
            const WeaponClass* pClass = mClass;
            mCurChargeStrengthLight += pClass->mChargeRateLight * fDeltaTime * mCurChargeRateLight;
            mCurChargeStrengthLight = mCurChargeStrengthLight > pClass->mMaxChargeStrengthLight
                                          ? pClass->mMaxChargeStrengthLight
                                          : mCurChargeStrengthLight;
        }
        if (bHeavyFull)
        {
            mCurTimeAtMaxCharge -= fDeltaTime;
            if (mCurTimeAtMaxCharge <= 0.0f)
            {
                mCurChargeRateLight = -1.0f;
                mCurChargeRateHeavy = -1.0f;
            }
        }
    }

    Snd::VoiceVirtual* pVoice = mSoundControl.GetVoiceVirtual();
    if (pVoice && mStateLimit > 0.0f)
    {
        float t = mStateTimer / mStateLimit;
        pVoice->SetPitch(Clamp(mSoundProps.GetPitch() * ((1.0f - t) * mClass->mChargeSoundPitch + t),
                               0.2f, 0.4f));
    }

    if (mStateTimer > mStateLimit && !mCharged)
    {
        GameObject* pObject = mOwner->GetGameObject();
        const PblVector3* pPos = mAimer ? &mAimer->mFirePos : &GetTranslation(GetCollisionMatrix(pObject));
        mClass->mChargedSound.Play(pPos, &GetVelocity(pObject), false, true);
        mCharged = true;
        if (mClass->mStopChargeSound)
        {
            mSoundControl.Stop(true);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00679050
void Weapon::ExitCharge()
{
    if (mClass->mStopChargeSound)
    {
        mSoundControl.Stop(true);
    }
}

// FUNCTION: SWBF2 0x00679070
void Weapon::EnterReload()
{
    GameObject* pObject = mOwner->GetGameObject();
    mStateTimer = 0.0f;
    mStateLimit = mClass->mReloadTime;
    mSoundProps = mClass->mReloadSound;
    const PblVector3* pPos = mAimer ? &mAimer->mFirePos : &GetTranslation(GetCollisionMatrix(pObject));
    mSoundControl.Play(&mSoundProps, pPos, &GetVelocity(pObject), true);
    if (mOwner->mPlayerId >= 0 && GameLoop::gUnlimitedAmmo && !NetGame::IsEnabled())
    {
        m_pAmmoCounter->UnloadClip_Unlimitted();
    }
    else
    {
        m_pAmmoCounter->UnloadClip();
    }
}

// FUNCTION: SWBF2 0x00679140
bool Weapon::UpdateReload(float fDeltaTime)
{
    if (mSelectedFlag)
    {
        mStateTimer += fDeltaTime;
    }
    if (mStateTimer >= mStateLimit)
    {
        mStateTimer = mStateLimit;
    }
    return true;
}

// FUNCTION: SWBF2 0x00679190
void Weapon::ExitReload()
{
    if (mOwner->mPlayerId >= 0 && GameLoop::gUnlimitedAmmo && !NetGame::IsEnabled())
    {
        m_pAmmoCounter->LoadClip_Unlimitted();
    }
    else
    {
        m_pAmmoCounter->LoadClip();
    }
    mSoundControl.Stop(true);
}

// FUNCTION: SWBF2 0x00679200
void Weapon::EnterEmpty()
{
    mStateTimer = 0.0f;
}

// FUNCTION: SWBF2 0x00679210
bool Weapon::UpdateEmpty(float fDeltaTime)
{
    mStateTimer += fDeltaTime;
    return true;
}

// FUNCTION: SWBF2 0x00679230
void Weapon::ExitEmpty()
{
}

// FUNCTION: SWBF2 0x00679240
bool Weapon::CheckCharge() const
{
    return false;
}

// FUNCTION: SWBF2 0x00679270
bool Weapon::CheckEmpty() const
{
    const AmmoCounter* pAmmo = m_pAmmoCounter;
    float fPerRound = pAmmo->m_pClass->mAmmoPerRound;
    return fPerRound > pAmmo->mCurClip && fPerRound > pAmmo->mNumClips;
}

// FUNCTION: SWBF2 0x00679290
bool Weapon::ForceReload() const
{
    return mReload && mReload->IsPressed() && m_pAmmoCounter->mCurClip < 1.0f;
}

// FUNCTION: SWBF2 0x006792c0
bool Weapon::CheckReload() const
{
    return m_pAmmoCounter->m_pClass->mAmmoPerRound > m_pAmmoCounter->mCurClip;
}

// FUNCTION: SWBF2 0x006792e0
bool Weapon::ReloadDone() const
{
    return mStateTimer >= mStateLimit;
}

// FUNCTION: SWBF2 0x00679300
bool Weapon::CheckOverheat() const
{
    if (mOwner->mPlayerId < 0 && mOwner->GetGameObject() &&
        ObjectIsRtti(mOwner->GetGameObject(), EntityFlyer::rttiHashEntityFlyer))
    {
        return false;
    }
    return m_pEnergyBar->m_bOverheat;
}

// FUNCTION: SWBF2 0x00679350
void Weapon::Render(const PblMatrix& mat, const RedPose* pPose, const RedColor& color,
                    unsigned int uFlags, bool bArg) const
{
    Weapon* pThis = const_cast<Weapon*>(this);
    if (mHideWeapon)
    {
        return;
    }

    RedModel* pModel;
    if (!bArg || (pModel = mRenderClass->mHighResModel) == 0)
    {
        pModel = mRenderClass->mModel;
        if (!pModel)
        {
            return;
        }
    }

    const PblMatrix* pMatrix = &mat;
    PblMatrix mBoneMatrix;
    if (pPose)
    {
        const PblMatrix* pBone = pPose->_Table.Find(mRenderClass->mIsOffhand ? 0xB7EC1D31 : 0x2B960099);
        if (!pBone)
        {
            return;
        }
        D3DXMatrixMultiply(&mBoneMatrix, pBone, &mat);
        pMatrix = &mBoneMatrix;
    }

    pModel->Render(pMatrix, 0, color, uFlags, 0);
    if (!color.a)
    {
        return;
    }

    pThis->mFirePointMatrix = *pMatrix;
    PblVector3 vFirePos;
    D3DXVec3TransformCoord(&vFirePos, &mRenderClass->mFirePointOffset, pMatrix);
    *reinterpret_cast<PblVector3*>(pThis->mFirePointMatrix.m[3]) = vFirePos;
    pThis->mFirePointMatrix.m[3][3] = 1.0f;

    float fFlash = mMuzzleFlashStartTime - GameLoop::GetMissionTime();
    if (fFlash > 0.0f)
    {
        mRenderClass->RenderFlash(GetTranslation(mFirePointMatrix), mAimer->mDirection,
                                  (1.0f / mClass->mFlashLength) * fFlash);
    }

    if (mState == CHARGE && mRenderClass->mChargeUpEffect)
    {
        if (!mChargeUpEmitter)
        {
            pThis->mChargeUpEmitter =
                static_cast<ParticleEmitterObject*>(mRenderClass->mChargeUpEffect->BuildEffect());
            if (mChargeUpEmitter)
            {
                mChargeUpEmitter->PermaRemoveParticleEffect();
            }
        }
        if (mChargeUpEmitter)
        {
            mChargeUpEmitter->AttachEffectToMatrix(&mFirePointMatrix, false);

            RedColorValue chargeColor;
            chargeColor.r = chargeColor.g = chargeColor.b = chargeColor.a = mStateTimer / mStateLimit;
            mChargeUpEmitter->SetEmitterColor(chargeColor);
            mChargeUpEmitter->Update(0.0f);
            mChargeUpEmitter->Render(1, 1.0f, 0);
        }
    }
    else if (mChargeUpEmitter)
    {
        mChargeUpEmitter->StopNow();
    }
}

// FUNCTION: SWBF2 0x00679610
void Weapon::SignalFire()
{
    float fTime = GameLoop::GetMissionTime();
    if (NetGame::IsEnabled() && (NetGame::IsClient() || NetGame::IsOneWorld()))
    {
        float fDelta = (float)fabs(fTime - mLastFireTime);
        if (fDelta >= 0.05f)
        {
            PlayFireSound(true);
        }
    }
    mFiredFlag = 1;
    mLastFireTime = fTime;
    mMuzzleFlashStartTime = mClass->mFlashLength + fTime;
}

// FUNCTION: SWBF2 0x006796c0
void Weapon::SetSkip(bool bSkip)
{
    mSkip = bSkip;
}

// FUNCTION: SWBF2 0x006796e0
void Weapon::ResetSkip()
{
    mSkip = false;
    mSkipTime = 0.0f;
}

// FUNCTION: SWBF2 0x0067bf00
void WeaponClass::Render(const PblMatrix& mMatrix, const RedPose* pPose, const RedColor& color,
                         unsigned int uiFlags, bool) const
{
    if (!color.a || !mModel)
    {
        return;
    }

    const PblMatrix* pMatrix = &mMatrix;
    PblMatrix mBoneMatrix;
    if (pPose)
    {
        const PblMatrix* pBone = pPose->_Table.Find(mIsOffhand ? 0xB7EC1D31 : 0x2B960099);
        if (!pBone)
        {
            return;
        }
        D3DXMatrixMultiply(&mBoneMatrix, pBone, &mMatrix);
        pMatrix = &mBoneMatrix;
    }
    mModel->Render(pMatrix, 0, color, uiFlags, 0);
}

// FUNCTION: SWBF2 0x005b41b0
Controllable* Weapon::GetOwner() const
{
    return mOwner;
}
