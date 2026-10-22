#include <math.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdio.h>
#include <stdlib.h>

#include "Explosion.h"
#include "CameraManager.h"
#include "ChaseCamera.h"
#include "Chunk.h"
#include "ParticleEmitter.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "DeathCamera.h"
#include "Entity.h"
#include "EntityMine.h"
#include "FishEffect.h"
#include "GameObject.h"
#include "LightFlash.h"
#include "NetGame.h"
#include "Trackable.h"
#include "WaterEffect.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "CollisionBody.h"
#include "CollisionPrimitive.h"
#include "FLEffect.h"
#include "GameModel.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Graphics/RedModel.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x007e6660
static int sCurrentTrail = 0;
// GLOBAL: SWBF2 0x007e665c
static int sCurrentSmoke = 0;

// FUNCTION: SWBF2 0x0051c980
Ordnance* ExplosionClass::Build(const OrdnanceDesc&)
{
    return 0;
}

// FUNCTION: SWBF2 0x0051c9a0
ExplosionClass::ExplosionClass(unsigned int uHash)
    : Factory<Ordnance, ExplosionClass, OrdnanceDesc>(uHash)
{
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    mDamage.mDamageOwner = owner;
    mDamage.mBaseValue = 0.0f;
    for (int i = 0; i < 8; ++i)
    {
        mDamage.mHealthScale[i] = 1.0f;
    }
    mDamage.mShieldScale = 1.0f;

    mDamageRadius[0] = 0.0f;
    mDamageRadius[1] = 0.0f;
    mRumbleLight = 0.0f;
    mRumbleHeavy = 0.0f;
    mRumbleLength = 0.0f;
    mShake = 0.0f;
    mShakeRadius[0] = 0.0f;
    mShakeRadius[1] = 0.0f;
    mShakeLength = 0.5f;
    mPersonScale = 1.0f;
    mVehicleScale = 1.0f;
    mShieldScale = 1.0f;
    mPush = 0.0f;
    mPushRadius[0] = 0.0f;
    mPushRadius[1] = 0.0f;
    mPushDeadOnly = true;
    mNumEffects = 0;
    for (int i = 0; i < 5; ++i)
    {
        mEffects[i] = 0;
    }
    mDecalHash = 0;
    mWaterEffectHash = 0;
    mShieldEffectHash = 0x6B369C5Fu;
    mNumChunks = 0;

    mChunkDescriptor.Init();

    mInitialChunkSpeedMin = -1.0f;
    mInitialChunkSpeedMax = -1.0f;
    mChunkStartDistance = 0.0f;
    mDoDamageToOwner = true;
    mLightColor = RedColor::BLACK;
    mLightRadius = 0.0f;
    mLightDuration = 0.0f;
    mVisibleRadius = 60.0f;
}

// SYNTHETIC: SWBF2 0x0051cb70
// ExplosionClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0051cba0
ExplosionClass::ExplosionClass(ExplosionClass* pParent, unsigned int uHash)
    : Factory<Ordnance, ExplosionClass, OrdnanceDesc>(pParent, uHash)
{
    mDamage = pParent->mDamage;
    mDamageRadius[0] = pParent->mDamageRadius[0];
    mDamageRadius[1] = pParent->mDamageRadius[1];
    mRumbleLight = pParent->mRumbleLight;
    mRumbleHeavy = pParent->mRumbleHeavy;
    mRumbleLength = pParent->mRumbleLength;
    mShake = pParent->mShake;
    mShakeRadius[0] = pParent->mShakeRadius[0];
    mShakeRadius[1] = pParent->mShakeRadius[1];
    mShakeLength = pParent->mShakeLength;
    mPersonScale = pParent->mPersonScale;
    mVehicleScale = pParent->mVehicleScale;
    mShieldScale = pParent->mShieldScale;
    mPush = pParent->mPush;
    mPushRadius[0] = pParent->mPushRadius[0];
    mPushRadius[1] = pParent->mPushRadius[1];
    mPushDeadOnly = pParent->mPushDeadOnly;
    for (int i = 0; i < 5; ++i)
    {
        mEffects[i] = pParent->mEffects[i];
    }
    mNumEffects = pParent->mNumEffects;
    mDecalHash = pParent->mDecalHash;
    mWaterEffectHash = pParent->mWaterEffectHash;
    mShieldEffectHash = pParent->mShieldEffectHash;
    mSound = pParent->mSound;
    mNumChunks = pParent->mNumChunks;
    mChunkDescriptor = pParent->mChunkDescriptor;
    mInitialChunkSpeedMin = pParent->mInitialChunkSpeedMin;
    mInitialChunkSpeedMax = pParent->mInitialChunkSpeedMax;
    mChunkStartDistance = pParent->mChunkStartDistance;
    mLightColor = pParent->mLightColor;
    mLightRadius = pParent->mLightRadius;
    mLightDuration = pParent->mLightDuration;
    mVisibleRadius = pParent->mVisibleRadius;
    mDoDamageToOwner = pParent->mDoDamageToOwner;
}

// SYNTHETIC: SWBF2 0x0051ce10
// DamageDesc::operator=

// FUNCTION: SWBF2 0x0051ce80
ExplosionClass::~ExplosionClass()
{
}

// FUNCTION: SWBF2 0x0051cec0
void ExplosionClass::Read(PblFileChunk& chunk)
{
    char acBuffer[128];
    ExplosionClass* pClass = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x45505954:
        {
            const unsigned int uType = PblHash(child.ReadString(acBuffer, sizeof(acBuffer)));
            if (Find(uType))
            {
                return;
            }
            pClass = pClass->Derive(uType);
            break;
        }
        case 0x45534142:
            pClass = Find(PblHash(child.ReadString(acBuffer, sizeof(acBuffer))));
            break;
        case 0x504F5250:
        {
            const unsigned int uProperty = child._pFile->Read32();
            pClass->SetProperty(uProperty, child.ReadString(acBuffer, sizeof(acBuffer)));
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x0051cff0
void ExplosionClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x59e94c40:
    case 0x47d0e794:
        sscanf(pszValue, "%f", &mDamage.mBaseValue);
        break;
    case 0x0bb94602:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[0]);
        break;
    case 0x9aa17a49:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[1]);
        break;
    case 0x338a07cd:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[2]);
        break;
    case 0x6ccdc5ef:
        sscanf(pszValue, "%f", &mDamage.mHealthScale[3]);
        break;
    case 0xc37970cf:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        mDamage.mHealthScale[0] = fScale;
        mDamage.mHealthScale[1] = fScale;
        mDamage.mHealthScale[2] = fScale;
        break;
    }
    case 0xa0bb60ce:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        mDamage.mHealthScale[3] = fScale;
        mDamage.mHealthScale[4] = fScale;
        mDamage.mHealthScale[5] = fScale;
        mDamage.mHealthScale[6] = fScale;
        break;
    }
    case 0x02168f61:
    {
        float fScale;
        sscanf(pszValue, "%f", &fScale);
        mDamage.mHealthScale[4] = fScale;
        mDamage.mHealthScale[5] = fScale;
        mDamage.mHealthScale[6] = fScale;
        break;
    }
    case 0xd0545c72:
        sscanf(pszValue, "%f", &mDamage.mShieldScale);
        break;
    case 0xded40df6:
        sscanf(pszValue, "%f", &mDamageRadius[0]);
        break;
    case 0xb68798a6:
    case 0xa23b939d:
        sscanf(pszValue, "%f", &mDamageRadius[1]);
        break;
    case 0xbd8bb2f5:
        sscanf(pszValue, "%f", &mShake);
        break;
    case 0x4954490d:
        sscanf(pszValue, "%f", &mShakeRadius[0]);
        break;
    case 0xd634b283:
    case 0x125399a6:
        sscanf(pszValue, "%f", &mShakeRadius[1]);
        break;
    case 0x25b787e5:
        sscanf(pszValue, "%f", &mShakeLength);
        break;
    case 0x876fffdd:
        sscanf(pszValue, "%f", &mPush);
        break;
    case 0x96301ec5:
        sscanf(pszValue, "%f", &mPushRadius[0]);
        break;
    case 0x19b5e2bb:
    case 0x71262ebe:
        sscanf(pszValue, "%f", &mPushRadius[1]);
        break;
    case 0x26d8bec1:
        mPushDeadOnly = atoi(pszValue) != 0;
        break;
    case 0x0be13b2f:
        mDoDamageToOwner = atoi(pszValue) != 0;
        break;
    case 0x6e6e8d54:
        if (*pszValue && mNumEffects < 5)
        {
            mEffects[mNumEffects++] = FLEffect::Get(PblHash(pszValue));
        }
        break;
    case 0xde15f6ae:
        mDecalHash = *pszValue ? (unsigned int)PblHash(pszValue) : 0;
        break;
    case 0x87733801:
        mWaterEffectHash = *pszValue ? (unsigned int)PblHash(pszValue) : 0;
        break;
    case 0xddb924a1:
        mShieldEffectHash = *pszValue ? (unsigned int)PblHash(pszValue) : 0;
        break;
    case 0xf350b4e5:
        mSound.SetID(pszValue, false, false);
        break;
    case 0xf54f2869:
    {
        unsigned short uValue;
        sscanf(pszValue, "%hu", &uValue);
        mNumChunks = (unsigned char)uValue;
        break;
    }
    case 0xda328455:
    {
        mChunkDescriptor.Init();
        mChunkDescriptor.mModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));

        char acName[128];
        _snprintf(acName, 127, "%s0", pszValue);
        acName[127] = 0;
        mChunkDescriptor.mPrimitive = CollisionBodyPrimitive::Find(PblHash(acName));
        _snprintf(acName, 127, "%s1", pszValue);
        acName[127] = 0;
        if (!mChunkDescriptor.mPrimitive || CollisionBodyPrimitive::Find(PblHash(acName)))
        {
            const PblBox& box = mChunkDescriptor.mModel->_mModel->_TotalBoundingBox;
            float fCenterX = (box._vMin.x + box._vMax.x) * 0.5f;
            float fCenterY = (box._vMin.y + box._vMax.y) * 0.5f;
            float fCenterZ = (box._vMin.z + box._vMax.z) * 0.5f;
            CollisionPrimitive primitive;
            PblMatrix matrix = _IdentityMatrix;
            float fHalfX = (float)fabs(box._vMax.x - box._vMin.x) * 0.5f;
            float fHalfY = (float)fabs(box._vMax.y - box._vMin.y) * 0.5f;
            float fHalfZ = (float)fabs(box._vMax.z - box._vMin.z) * 0.5f;
            primitive.SetSphere(sqrtf(fHalfX * fHalfX + fHalfY * fHalfY + fHalfZ * fHalfZ));
            matrix.m[3][0] = fCenterX;
            matrix.m[3][1] = fCenterY;
            matrix.m[3][2] = fCenterZ;
            matrix.m[3][3] = 1.0f;
            mChunkDescriptor.mPrimitive = new CollisionBodyPrimitive(primitive, "", matrix, 0);
        }
        sCurrentSmoke = 0;
        sCurrentTrail = 0;
        break;
    }
    case 0x0a157be2:
    {
        unsigned short uValue;
        sscanf(pszValue, "%hu", &uValue);
        mChunkDescriptor.mMaxTerrainCollisions = (unsigned char)uValue;
        break;
    }
    case 0x048f1226:
        mChunkDescriptor.mTerrainImpactEffect = PblHash(pszValue);
        break;
    case 0xc8e32dc3:
    {
        const unsigned int uType = PblHash(pszValue);
        if (uType == 0x2648cd55)
        {
            mChunkDescriptor.mPhysicsType = 3;
        }
        else if (uType == 0xd290c23b)
        {
            mChunkDescriptor.mPhysicsType = 2;
        }
        else
        {
            mChunkDescriptor.mPhysicsType = uType != 0xff79b33c;
        }
        break;
    }
    case 0x166f6939:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mChunkDescriptor.mOmega.x = x;
        mChunkDescriptor.mOmega.y = y;
        mChunkDescriptor.mOmega.z = z;
        break;
    }
    case 0xd0259d88:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mChunkDescriptor.mGravity.x = x;
        mChunkDescriptor.mGravity.y = y;
        mChunkDescriptor.mGravity.z = z;
        break;
    }
    case 0x94f0544b:
        if (sCurrentTrail < sizeof(mChunkDescriptor.mTrailEffect) / sizeof(mChunkDescriptor.mTrailEffect[0]))
        {
            mChunkDescriptor.mTrailEffect[sCurrentTrail] = PblHash(pszValue);
            ++sCurrentTrail;
        }
        break;
    case 0xd350c3de:
        if (sCurrentSmoke < sizeof(mChunkDescriptor.mSmokeEffect) / sizeof(mChunkDescriptor.mSmokeEffect[0]))
        {
            mChunkDescriptor.mSmokeEffect[sCurrentSmoke] = PblHash(pszValue);
            ++sCurrentSmoke;
        }
        break;
    case 0x945d1d15:
        sscanf(pszValue, "%f %f", &mInitialChunkSpeedMin, &mInitialChunkSpeedMax);
        break;
    case 0xcd61e401:
        sscanf(pszValue, "%f", &mChunkStartDistance);
        break;
    case 0x746ce73e:
    {
        int r, g, b, a;
        int iCount = sscanf(pszValue, "%d %d %d %d", &r, &g, &b, &a);
        RedColor color;
        if (iCount >= 4)
        {
            color.b = (unsigned char)(b * a / 255);
            color.g = (unsigned char)(g * a / 255);
            color.r = (unsigned char)(r * a / 255);
        }
        else if (iCount == 3)
        {
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
        }
        else
        {
            break;
        }
        color.a = 255;
        mLightColor = color;
        break;
    }
    case 0x2776564d:
        sscanf(pszValue, "%f", &mLightRadius);
        break;
    case 0x6370a66b:
        sscanf(pszValue, "%f", &mLightDuration);
        break;
    case 0x53a81b7f:
        sscanf(pszValue, "%f", &mVisibleRadius);
        break;
    }
}

// FUNCTION: SWBF2 0x0051dba0
ExplosionClass* ExplosionClass::Derive(unsigned int uHash)
{
    return new ExplosionClass(this, uHash);
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

static CollisionObject* GameObject_GetCollisionBase(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject);
}
static Damageable* GameObject_GetDamageable(GameObject* pObject)
{
    return static_cast<Damageable*>(pObject);
}
static bool GameObject_IsRtti(GameObject* pObject, unsigned int uRtti)
{
    return reinterpret_cast<Entity*>(pObject)->IsRtti(uRtti);
}

static GameObject* GetHandleObject(const PblHandle<GameObject>& handle)
{
    PblHandle<GameObject>& h = const_cast<PblHandle<GameObject>&>(handle);
    if (h.mObject &&
        h.mObject->mHandleId !=
            h.mSavedHandleId)
    {
        h.mObject = 0;
        h.mSavedHandleId = 0;
    }
    return h.mObject;
}

static const GameSound& ParticleEmitterClass_GetAmbientSound(const FLEffectClass* pClass)
{
    return static_cast<const ParticleEmitterClass*>(pClass)->mAmbientSound;
}

// FUNCTION: SWBF2 0x0051dc10
void ExplosionClass::CreateExplosion(const PblVector3& aPos, const DamageOwner& aOwner, bool aFlying,
                                     CollisionObjectType aCollType)
{
    (void)aCollType;
    bool doClient;
    bool bCreateEffects;
    if (NetGame::IsEnabled() && !NetGame::IsClient() && !(NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        doClient = false;
        bCreateEffects = false;
    }
    else
    {
        doClient = true;
        bCreateEffects = CameraManager::sInstance->IsInsideAnyCamera(aPos, mVisibleRadius, 400.0f);
    }

    mSound.Play(&aPos, 0, false, true);

    float fWaterHeight;
    bool bUnderWater = RedWater::GetWaterHeight(aPos, fWaterHeight) && fWaterHeight + 0.3f >= aPos.y;

    int iEffect = (int)(mNumEffects * PblRandom::GetGlobalFloat());
    if (bCreateEffects)
    {
        if (bUnderWater)
        {
            if (mDamageRadius[1] > 0.0f)
            {
                float fSplashRadius = mDamageRadius[1] * 0.5f;
                if (fSplashRadius >= 0.4f)
                {
                    fSplashRadius = 0.4f;
                }
                RedWater::AddWaterRing(aPos, fSplashRadius + 3.0f, fSplashRadius);
                if (mWaterEffectHash)
                {
                    WaterEffect::AddWaterEffect(mWaterEffectHash, aPos);
                }
                float fFish = mDamageRadius[1] * 0.5f;
                FishParticleManager::SpawnFloatingFish(aPos, fFish < 4.0f ? fFish : 4.0f,
                                                       (int)(fFish < 8.0f ? fFish : 8.0f) + 1);
            }
        }
        else
        {
            FLEffectClass* pEffectClass = mEffects[iEffect];
            if (pEffectClass)
            {
                FLEffectObject* pEffect = pEffectClass->BuildEffect();
                if (pEffect)
                {
                    pEffect->AttachEffectToVector(&aPos, false);
                }
            }
            if (mLightRadius > 0.0f && mLightDuration > 0.0f)
            {

                RedColorValue newcolor;
                newcolor.r = mLightColor.r * (1.0f / 255.0f) * 20.0f;
                newcolor.g = mLightColor.g * (1.0f / 255.0f) * 20.0f;
                newcolor.b = mLightColor.b * (1.0f / 255.0f) * 20.0f;
                newcolor.a = mLightColor.a * (1.0f / 255.0f) * 20.0f;
                if (newcolor.r > 1.0f)
                {
                    newcolor.r = 1.0f;
                }
                if (newcolor.g > 1.0f)
                {
                    newcolor.g = 1.0f;
                }
                if (newcolor.b > 1.0f)
                {
                    newcolor.b = 1.0f;
                }
                if (newcolor.a > 1.0f)
                {
                    newcolor.a = 1.0f;
                }
                RedColor color;
                color.b = (unsigned char)(int)(newcolor.b * 255.0f);
                color.g = (unsigned char)(int)(newcolor.g * 255.0f);
                color.r = (unsigned char)(int)(newcolor.r * 255.0f);
                color.a = (unsigned char)(int)(newcolor.a * 255.0f);
                PblSphere sphere;
                sphere.x = aPos.x + 0.0f;
                sphere.y = aPos.y + 3.0f;
                sphere.z = aPos.z + 0.0f;
                sphere.w = mLightRadius * 3.0f;
                new LightFlash(sphere, color, mLightDuration);
            }
        }
    }
    else if (doClient)
    {
        FLEffectClass* pEffectClass = mEffects[iEffect];
        if (pEffectClass && pEffectClass->mType == 1)
        {
            ParticleEmitterClass_GetAmbientSound(pEffectClass).Play(&aPos, 0, false, true);
        }
    }

    if (mNumChunks > 0)
    {
        ChunkCluster* chunkCluster = ChunkClusterManager::GetFreeCluster(false);
        if (chunkCluster)
        {
            chunkCluster->mParentClass = 0;
            PblRandom& rand = PblRandom::GetSim();
            for (int i = 0; i < mNumChunks; ++i)
            {
                if (i >= 10)
                {
                    break;
                }
                if (!mChunkDescriptor.mModel)
                {
                    continue;
                }
                mChunkDescriptor.mFullCollisionModel = 0;

                float sinPitch, cosPitch, sinYaw, cosYaw;
                PblMath::SinCos(&sinPitch, &cosPitch, rand.GetFloat() * PblMath::fTwoPi);
                PblMath::SinCos(&sinYaw, &cosYaw, rand.GetFloat() * PblMath::fTwoPi);
                PblVector3 vAxis(sinYaw * cosPitch, sinPitch, cosYaw * cosPitch);
                float fSinHalf, fCosHalf;
                PblMath::SinCos(&fSinHalf, &fCosHalf, rand.GetFloat() * PblMath::fTwoPi * 0.5f);
                PblQuaternion q;
                q._fX = fSinHalf * vAxis.x;
                q._fY = fSinHalf * vAxis.y;
                q._fZ = fSinHalf * vAxis.z;
                q._fS = fCosHalf;
                PblQuaternion qNorm;
                PblMatrix mRot;
                D3DXMatrixRotationQuaternion(&mRot, D3DXQuaternionNormalize(&qNorm, &q));
                mChunkDescriptor.mMatrix = mRot;
                mChunkDescriptor.mMatrix.m[3][0] = aPos.x;
                mChunkDescriptor.mMatrix.m[3][1] = aPos.y;
                mChunkDescriptor.mMatrix.m[3][2] = aPos.z;
                mChunkDescriptor.mMatrix.m[3][3] = 1.0f;

                PblVector3 vRand;
                if (aFlying)
                {
                    vRand.x = (0.5f - rand.GetFloat()) * 2.0f;
                    vRand.y = (0.5f - rand.GetFloat()) * 2.0f;
                    vRand.z = (0.5f - rand.GetFloat()) * 2.0f;
                }
                else if (mInitialChunkSpeedMin < 0.0f)
                {
                    vRand.x = (0.5f - rand.GetFloat()) * 2.0f;
                    vRand.y = (rand.GetFloat() + 1.0f) * 4.0f;
                    vRand.z = (0.5f - rand.GetFloat()) * 2.0f;
                }
                else
                {
                    vRand.x = (0.5f - rand.GetFloat()) * 2.0f;
                    vRand.y = rand.GetFloat() + 0.5f;
                    vRand.z = (0.5f - rand.GetFloat()) * 2.0f;
                }
                PblVector3 vDir;
                D3DXVec3Normalize(&vDir, &vRand);
                float fSpeed;
                if (mInitialChunkSpeedMin < 0.0f)
                {
                    fSpeed = rand.GetFloat() * mPushRadius[1] + mPushRadius[1];
                }
                else
                {
                    fSpeed = rand.GetFloat() * (mInitialChunkSpeedMax - mInitialChunkSpeedMin) +
                             mInitialChunkSpeedMin;
                }
                mChunkDescriptor.mVelocity.x = vDir.x * fSpeed;
                mChunkDescriptor.mVelocity.y = fSpeed * vDir.y;
                mChunkDescriptor.mVelocity.z = fSpeed * vDir.z;

                PblVector3 vOut;
                D3DXVec3Normalize(&vOut, &mChunkDescriptor.mVelocity);
                mChunkDescriptor.mMatrix.m[3][0] = vOut.x * mChunkStartDistance + aPos.x;
                mChunkDescriptor.mMatrix.m[3][1] = aPos.y + mChunkStartDistance * vOut.y;
                mChunkDescriptor.mMatrix.m[3][2] = aPos.z + mChunkStartDistance * vOut.z;
                mChunkDescriptor.mMatrix.m[3][3] = 1.0f;

                ChunkDescriptor tmpDesc(mChunkDescriptor);

                tmpDesc.mOmega = mChunkDescriptor.mOmega;
                tmpDesc.mNoBuildingCollisions = true;
                chunkCluster->AddChunk(tmpDesc);
            }
        }
    }

    if (mDamageRadius[1] > 0.0f || mPushRadius[1] > 0.0f)
    {
        float maxRadius = mDamageRadius[1] > mPushRadius[1] ? mDamageRadius[1] : mPushRadius[1];
        GameObject* array[20];
        int count = CollisionManager::GetObjectsInRange(
            aPos, maxRadius, array, 20, 0,
            COLL_ITEM | COLL_SOFT | COLL_RIGID | COLL_STATIC | COLL_ATTACHED | COLL_SHIELD);

        DamageDesc damage = mDamage;
        damage.mDamageOwner = aOwner;

        while (count--)
        {
            GameObject* pObject = array[count];
            if (!mDoDamageToOwner && GetHandleObject(aOwner.mGameObject) &&
                pObject == GetHandleObject(aOwner.mGameObject))
            {
                continue;
            }

            CollisionObject* pBase = GameObject_GetCollisionBase(pObject);
            if (pBase->GetCollisionType() == COLL_ITEM &&
                !GameObject_IsRtti(pObject, EntityMine::rttiHashEntityMine))
            {
                continue;
            }

            PblVector3 targetPos = pBase->GetMatrix().GetTranslation();
            const CollisionObject* pCollision = pObject->GetCollisionObject();
            if (pCollision)
            {
                targetPos = pCollision->GetCollisionCenter();
            }
            PblVector3 dir(targetPos.x - aPos.x, targetPos.y - aPos.y, targetPos.z - aPos.z);
            targetPos = pObject->GetTargetPoint(aPos, dir, -1);

            const GameObject* ignore[1] = { pObject };
            if (CollisionManager::RayTest(aPos, targetPos, ignore, 1, COLL_STATIC | COLL_TERRAIN))
            {
                continue;
            }

            if (mDamageRadius[1] > 0.0f)
            {
                pObject->ApplyRadiusDamage(aPos, mDamageRadius[0], mDamageRadius[1], damage);
            }
            if (!mPushDeadOnly || !GameObject_GetDamageable(pObject)->mIsAlive)
            {
                if (mPushRadius[1] > 0.0f)
                {
                    pObject->ApplyRadiusPush(aPos, mPushRadius[0], mPushRadius[1], mPush, false);
                }
            }
        }

        if (bCreateEffects && mShakeRadius[1] > 0.0f)
        {

            CameraManager* pManager = CameraManager::sInstance;
            Trackable* pTrackable = pManager->mCurGameCamera[0] == pManager->mDeathCamera[0]
                                        ? pManager->mDeathCamera[0]->mOwner
                                        : pManager->mChaseCamera[0]->mOwner;
            if (pTrackable)
            {
                GameObject* pTarget = pTrackable->GetGameObject();
                if (pTarget)
                {
                    const PblVector3& vTarget = pTarget->GetCollisionObject()->GetPosition();
                    float dx = vTarget.x - aPos.x;
                    float dy = vTarget.y - aPos.y;
                    float dz = vTarget.z - aPos.z;
                    float length = sqrtf(dx * dx + dy * dy + dz * dz);
                    if (length > 0.0001f)
                    {
                        float scale = pTarget->CalcRadiusEffect(length, mShakeRadius[0], mShakeRadius[1]);
                        if (scale > 0.0f)
                        {

                            scale = pTrackable->GetControllable()->GetControllableClass()->mShakeScale * scale;
                            float scaledShake = mShake * scale;
                            float scaledLength = mShakeLength * scale;
                            CameraManager::sInstance->ApplyShake(scaledShake, scaledLength);

                            pTrackable->GetControllable();
                        }
                    }
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0051eba0
template Factory<Ordnance, ExplosionClass, OrdnanceDesc>::Factory(ExplosionClass*, unsigned int);

// SYNTHETIC: SWBF2 0x0051ec60
// Factory<Ordnance,ExplosionClass,OrdnanceDesc>::`scalar deleting destructor'
