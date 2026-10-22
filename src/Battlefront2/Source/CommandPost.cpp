#include "CommandPost.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Character.h"
#include "CommandFlyer.h"
#include "CommandHover.h"
#include "CollisionObject.h"
#include "Entity.h"
#include "EntityHologram.h"
#include "EventManager_CommandPost.h"
#include "FLEffect.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PlayerStats.h"
#include "StringDB.h"
#include "TargetManager.h"
#include "TriggerRegionManager.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Application/RedPath.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "AIUtil.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "NetComm.h"
#include "VoiceOverManager.h"
#include "SoundFL/source/sndsoundstream.h"
#include "SoundFL/source/sndvoicevirtual.h"

namespace EventManager
{
template <>
void Event<CommandPost, char>::Callback::Trigger(CommandPost& item, char& param);

template <>
void Event<CommandPost, DamageOwner>::Callback::Trigger(CommandPost& item, DamageOwner& param);
}

namespace EventManager
{

template <>
void Event<CommandPost, unsigned __int64>::Callback::Trigger(CommandPost& item, unsigned __int64& param);

namespace CommandPostEvent
{
extern Event<CommandPost, unsigned __int64> mBeginNeutralize;
extern Event<CommandPost, unsigned __int64> mAbortNeutralize;
extern Event<CommandPost, unsigned __int64> mFinishNeutralize;
extern Event<CommandPost, unsigned __int64> mBeginCapture;
extern Event<CommandPost, unsigned __int64> mAbortCapture;
extern Event<CommandPost, unsigned __int64> mFinishCapture;
}
}

extern int gSpawnInvicibility;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0047a570
PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos)
{
    PblVector3 vUp;
    D3DXVec3Normalize(&vUp, &vNormal);
    PblVector3 vSide(vUp.y * vDir.z - vUp.z * vDir.y, vUp.z * vDir.x - vUp.x * vDir.z,
                     vUp.x * vDir.y - vUp.y * vDir.x);
    PblVector3 vRight;
    D3DXVec3Normalize(&vRight, &vSide);
    PblVector3 vFront(vRight.y * vUp.z - vRight.z * vUp.y, vRight.z * vUp.x - vRight.x * vUp.z,
                      vRight.x * vUp.y - vRight.y * vUp.x);

    PblMatrix m;
    m.m[0][0] = vRight.x;
    m.m[0][1] = vRight.y;
    m.m[0][2] = vRight.z;
    m.m[0][3] = 0.0f;
    m.m[1][0] = vUp.x;
    m.m[1][1] = vUp.y;
    m.m[1][2] = vUp.z;
    m.m[1][3] = 0.0f;
    m.m[2][0] = vFront.x;
    m.m[2][1] = vFront.y;
    m.m[2][2] = vFront.z;
    m.m[2][3] = 0.0f;
    m.m[3][0] = vPos.x;
    m.m[3][1] = vPos.y;
    m.m[3][2] = vPos.z;
    m.m[3][3] = 1.0f;
    return m;
}

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

// FUNCTION: SWBF2 0x0047e3e0
CommandPostClass::CommandPostClass(unsigned int  )
{
    mNeutralizeTime = 6.0f;
    mCaptureTime = 10.0f;
    m_pHologramClass = 0;
    for (int i = 0; i < 8; i++)
    {
        m_pHoloModel[i] = 0;
    }
    m_fHoloTurnOnTime = 2.0f;
    mSoundPitchDev = 0.1f;
    mSpawnRotation = 0;
    mSpawnPosition = 0;
    mSpawnCount = 0;
    mBleedValue[0] = 0;
    mBleedValue[1] = 10;
    mBleedValue[2] = 10;
    mBleedValue[3] = 10;
    mBleedValue[4] = 10;
    mBleedValue[5] = 0;
    mBleedValue[6] = 10;
    mBleedValue[7] = 10;
    for (int i = 0; i < 8; i++)
    {
        mStrategicAttackValue[i] = 100;
    }
    for (int i = 0; i < 8; i++)
    {
        mStrategicDefendValue[i] = 100;
    }
    mDisplayHUDIndex = true;
    mStrategicBan = 0;
}

// FUNCTION: SWBF2 0x0047e8e0
CommandPostClass::~CommandPostClass()
{
}

// SYNTHETIC: SWBF2 0x0047e630
// CommandPostClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0047e670
CommandPostClass::CommandPostClass(CommandPostClass* pBase, unsigned int  )
{
    mNeutralizeTime = pBase->mNeutralizeTime;
    mCaptureTime = pBase->mCaptureTime;
    m_pHologramClass = pBase->m_pHologramClass;
    for (int i = 0; i < 8; i++)
    {
        m_pHoloModel[i] = pBase->m_pHoloModel[i];
    }
    m_fHoloTurnOnTime = pBase->m_fHoloTurnOnTime;
    mSoundPitchDev = pBase->mSoundPitchDev;
    for (int i = 0; i < 8; i++)
    {
        mSoundAmbience[i] = pBase->mSoundAmbience[i];
    }
    mSoundDischarge = pBase->mSoundDischarge;
    mSoundCharge = pBase->mSoundCharge;
    mSoundCaptured = pBase->mSoundCaptured;
    mSoundLost = pBase->mSoundLost;
    mSoundDispute = pBase->mSoundDispute;
    mSpawnRotation = pBase->mSpawnRotation;
    mSpawnPosition = pBase->mSpawnPosition;
    mSpawnCount = pBase->mSpawnCount;
    mDisplayHUDIndex = pBase->mDisplayHUDIndex;
    mBleedValue[0] = 0;
    mBleedValue[1] = 10;
    mBleedValue[2] = 10;
    mBleedValue[3] = 10;
    mBleedValue[4] = 10;
    mBleedValue[5] = 0;
    mBleedValue[6] = 10;
    mBleedValue[7] = 10;
    for (int i = 0; i < 8; i++)
    {
        mStrategicAttackValue[i] = pBase->mStrategicAttackValue[i];
    }
    for (int i = 0; i < 8; i++)
    {
        mStrategicDefendValue[i] = pBase->mStrategicDefendValue[i];
    }
    mStrategicBan = pBase->mStrategicBan;
}

static PblQuaternion* NewSpawnRotations(int iCount)
{
    return new PblQuaternion[iCount];
}

// FUNCTION: SWBF2 0x0047e970
void CommandPostClass::SetProperty(unsigned int uProperty, const char* pszValue,
                                   const RedModel* pModel, const char*  )
{
    switch (uProperty)
    {
    case 0xF1A8F31B:
        sscanf(pszValue, "%f", &mNeutralizeTime);
        break;
    case 0xEB1BCBF8:
        sscanf(pszValue, "%f", &mCaptureTime);
        break;
    case 0x6A83ED9C:
        m_pHologramClass = EntityHologramClass::Find(PblHash(pszValue));
        break;
    case 0xB64210D2:
    {
        char acGeometry[80];
        char acTeam[80];
        if (sscanf(pszValue, "%s %s", acGeometry, acTeam) == 2)
        {
            int iSide = Team::TranslateTeamName(acTeam);
            m_pHoloModel[iSide] = RedModel::_HashTable.Find(PblHash(acGeometry));
        }
        break;
    }
    case 0x630C0E9E:
        sscanf(pszValue, "%f", &m_fHoloTurnOnTime);
        break;
    case 0x65A920BA:
    {
        char acTeam[80];
        char acSound[80];
        if (sscanf(pszValue, "%s %s", acTeam, acSound) == 2)
        {
            int iSide = Team::TranslateTeamName(acTeam);
            strcat(acSound, " defer");
            mSoundAmbience[iSide].SetID(acSound, false, false);
        }
        break;
    }
    case 0x16E4A90E:
        mSoundDischarge.SetID(pszValue, false, false);
        break;
    case 0x4EE58508:
        mSoundCharge.SetID(pszValue, false, false);
        break;
    case 0xF17A02E6:
        mSoundCaptured.SetID(pszValue, false, false);
        break;
    case 0x8E39006A:
        mSoundLost.SetID(pszValue, false, false);
        break;
    case 0x12CAACAA:
        mSoundDispute.SetID(pszValue, false, false);
        break;
    case 0xA33D3401:
        sscanf(pszValue, "%f", &mSoundPitchDev);
        break;
    case 0x6B9B4728:
    {
        char acTeam[80];
        char acMusic[80];
        if (sscanf(pszValue, "%s %s", acTeam, acMusic) == 2)
        {
            int iSide = Team::TranslateTeamName(acTeam);
            mMusicCapture[iSide].SetID(acMusic);
        }
        break;
    }
    case 0x42CB8C86:
    {
        char acTeam[80];
        char acMusic[80];
        if (sscanf(pszValue, "%s %s", acTeam, acMusic) == 2)
        {
            int iSide = Team::TranslateTeamName(acTeam);
            mMusicLost[iSide].SetID(acMusic);
        }
        break;
    }
    case 0x32579F4D:
    {
        int iCount = 0;
        sscanf(pszValue, "%i", &iCount);
        if (mSpawnRotation)
        {
            delete[] mSpawnRotation;
        }
        if (mSpawnPosition)
        {
            delete[] mSpawnPosition;
        }
        if (iCount > 0)
        {
            mSpawnRotation = NewSpawnRotations(iCount);
            mSpawnPosition = new PblVector3[iCount];
        }
        mSpawnCount = 0;
        break;
    }
    case 0xA067D145:
    {
        if (!mSpawnRotation || !mSpawnPosition)
        {
            break;
        }
        float fX = 0.0f;
        float fY = 0.0f;
        float fZ = 0.0f;
        float fYaw = 0.0f;
        if (!sscanf(pszValue, "%f %f %f %f", &fX, &fY, &fZ, &fYaw))
        {
            break;
        }
        mSpawnPosition[mSpawnCount] = PblVector3(fX, fY, fZ);
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, fYaw * 0.017453292f * 0.5f);
        PblQuaternion qRot;
        qRot._fX = fSin * gVectorUnitY.x;
        qRot._fY = fSin * gVectorUnitY.y;
        qRot._fZ = fSin * gVectorUnitY.z;
        qRot._fS = fCos;
        mSpawnRotation[mSpawnCount] = qRot;
        mSpawnCount++;
        break;
    }
    case 0xC5190CBA:
    {
        char acFormat[32] = "";
        int iCount = 1;
        if (!sscanf(pszValue, "%s %i", acFormat, &iCount))
        {
            break;
        }
        if (mSpawnRotation)
        {
            delete[] mSpawnRotation;
        }
        if (mSpawnPosition)
        {
            delete[] mSpawnPosition;
        }
        mSpawnCount = 0;
        if (iCount <= 0)
        {
            break;
        }
        mSpawnRotation = NewSpawnRotations(iCount);
        mSpawnPosition = new PblVector3[iCount];
        for (int i = 0; i < iCount;)
        {
            char acName[32];
            sprintf(acName, acFormat, ++i);
            PblMatrix mBone;
            if (pModel->GetHierarchicalBoneMatrix(PblTEMPHash(acName), mBone))
            {
                mSpawnRotation[mSpawnCount] = PblQuaternion(mBone);
                mSpawnPosition[mSpawnCount] = *(const PblVector3*)mBone.m[3];
                mSpawnCount++;
            }
        }
        break;
    }
    case 0xC069B070:
        mDisplayHUDIndex = atol(pszValue) != 0;
        break;

    case 0xEB13C1FE:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Neutral]);
        break;
    case 0xFDE7C9D2:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Alliance]);
        break;
    case 0x5A7EB7DB:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Empire]);
        break;
    case 0xDC392F3F:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Republic]);
        break;
    case 0x2450A89C:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_CIS]);
        break;
    case 0xD97FC017:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Locals]);
        break;
    case 0xA340D6B5:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Alliance]);
        break;
    case 0x4DB44670:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Empire]);
        break;
    case 0x9E78B410:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Republic]);
        break;
    case 0x00E73245:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_CIS]);
        break;
    case 0x573C98A4:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Locals]);
        break;
    case 0x44A2C4A6:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Alliance]);
        break;
    case 0xA8679457:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Empire]);
        break;
    case 0x8F5540CB:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Republic]);
        break;
    case 0x5BF85AE8:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_CIS]);
        break;
    case 0xE689EF93:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Locals]);
        break;
    }
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

extern PblVector3 gVectorZero;

class CommandWalker
{
public:
    static PblHash rttiHashCommandWalker;
};

static char sEventParam;

// GLOBAL: SWBF2 0x01e308e0
static CommandPost* sPostArrayHost[16];
// GLOBAL: SWBF2 0x01e30888
static int sPostCountHost;
// GLOBAL: SWBF2 0x01e30890
static int sActivePostCountHost;
// GLOBAL: SWBF2 0x01e30894
static int* sActivePostCount;

// GLOBAL: SWBF2 0x01e308a0
static CommandPost* sPostArrayClient[16];
// GLOBAL: SWBF2 0x01e3088c
static int sPostCountClient;
// GLOBAL: SWBF2 0x01e30884
static int sActivePostCountClient;

// FUNCTION: SWBF2 0x0047aac0
void CommandPost::CreateClient()
{
    memset(sPostArrayClient, 0, sizeof(sPostArrayClient));
    sPostCountClient = 0;
    sActivePostCountClient = 0;
}

// FUNCTION: SWBF2 0x0047aaf0
void CommandPost::DestroyClient()
{
    for (int i = 0; i < sPostCountClient; ++i)
    {
        if (sPostArrayClient[i])
        {
            delete sPostArrayClient[i];
        }
    }
}

// FUNCTION: SWBF2 0x0047ab20
void CommandPost::EnterClient()
{
    sPostArray = sPostArrayClient;
    sPostCount = &sPostCountClient;
    sActivePostCount = &sActivePostCountClient;
}

// FUNCTION: SWBF2 0x0047ab40
void CommandPost::LeaveClient()
{
    sPostArray = sPostArrayHost;
    sPostCount = &sPostCountHost;
    sActivePostCount = &sActivePostCountHost;
}

// FUNCTION: SWBF2 0x0047c390
void CommandPost::CopyFrom(const CommandPost* pOther)
{
    mCaptureRegion = pOther->mCaptureRegion;
    mControlRegion = pOther->mControlRegion;
    mSpawnPath = pOther->mSpawnPath;
    mAllyPath = pOther->mAllyPath;
    mTurretPath = pOther->mTurretPath;
    mName = pOther->mName;
    mLabel = pOther->mLabel;
    mLocalizeName = pOther->mLocalizeName;
    for (int i = 0; i < 8; ++i)
    {
        for (int side = 0; side < Team::side_MAX; ++side)
        {
            for (int stream = 0; stream < VoiceOver::stream_MAX; ++stream)
            {
                mVoiceOver[i].mStream[side][stream] = pOther->mVoiceOver[i].mStream[side][stream];
            }
        }
    }
}

// FUNCTION: SWBF2 0x0047cc70
float CommandPost::GetCaptureRatio() const
{
    return mCaptureTimer / GetCaptureTime();
}

// FUNCTION: SWBF2 0x0047cc90
float CommandPost::GetNeutralizeRatio() const
{
    return mNeutralizeTimer / GetNeutralizeTime();
}

// FUNCTION: SWBF2 0x0047ce00
void CommandPost::SetAICanCapture(unsigned int iTeam, bool bCanCapture)
{
    if (iTeam <= 7)
    {
        unsigned int mask = 1u << iTeam;
        if (bCanCapture)
        {
            mAICanCaptureTeamMask |= mask;
        }
        else
        {
            mAICanCaptureTeamMask &= ~mask;
        }
    }
}

// GLOBAL: SWBF2 0x007e6314
CommandPost** CommandPost::sPostArray = sPostArrayHost;
// GLOBAL: SWBF2 0x007e631c
int* CommandPost::sPostCount = &sPostCountHost;

// GLOBAL: SWBF2 0x007e6318
static int sForcedIndex = -1;

// FUNCTION: SWBF2 0x0047aa40
void CommandPost::Create()
{
    memset(sPostArrayHost, 0, sizeof(sPostArrayHost));
    sPostArray = sPostArrayHost;
    sPostCountHost = 0;
    sPostCount = &sPostCountHost;
    sActivePostCountHost = 0;
    sActivePostCount = &sActivePostCountHost;
}

// FUNCTION: SWBF2 0x0047aa90
void CommandPost::Destroy()
{
    for (int i = 0; i < sPostCountHost; ++i)
    {
        if (sPostArrayHost[i])
        {
            delete sPostArrayHost[i];
        }
    }
}

// FUNCTION: SWBF2 0x0047ab70
int CommandPost::GetCount()
{
    return *sActivePostCount;
}

// FUNCTION: SWBF2 0x0047ab80
CommandPost* CommandPost::GetActive(int iIndex)
{
    CommandPost* pPost = sPostArray[iIndex];
    if (!pPost || !pPost->IsThreadActive())
    {
        return 0;
    }
    return pPost;
}

// SYNTHETIC: SWBF2 0x0047a690
// CommandPost::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0047a6c0
CommandPost::VoiceOver::VoiceOver()
{
}

// FUNCTION: SWBF2 0x0047a6e0
CommandPost::VoiceOver::~VoiceOver()
{
}

// FUNCTION: SWBF2 0x0047a710
CommandPost::CommandPost(CommandPostClass* pClass, int iIndex)
    : mPostIndex(iIndex),
      mHUDPostIndex(iIndex),
      mHoldTeam(0),
      mHoldPlayerMask(0),
      mSkippedPlayerMask(0),
      mBiasTeam(0),
      mAICanCaptureTeamMask(0xffffffff),
      mCaptureRegion(0),
      mControlRegion(0),
      mKillRegion(0),
      mSpawnPath(0),
      mAllyPath(0),
      mTurretPath(0),
      mSpawnRegion(0),
      mAllyCount(0),
      mNeutralizeTimer(0),
      mCaptureTimer(0),
      m_pHologram(0),
      mClass(0),
      mIsAISpawnPoint(true),
      mDisplayHUDIndex(true),
      mbIsEnabled(true)
{
    sPostArray[mPostIndex] = this;
    mName = 0;
    mLabel = 0;
    mLocalizeName = 0;
    for (int i = 0; i < 8; i++)
    {
        mHoldCount[i] = 0;
    }
    mSpawnQueue[0][0] = 0;
    mSpawnQueue[0][1] = 0;
    mSpawnQueue[1][0] = 0;
    mSpawnQueue[1][1] = 0;
    for (int i = 0; i < 8; i++)
    {
        mBleedValue[i] = pClass->mBleedValue[i];
    }
    for (int i = 0; i < 8; i++)
    {
        mStrategicAttackValue[i] = pClass->mStrategicAttackValue[i];
    }
    for (int i = 0; i < 8; i++)
    {
        mStrategicDefendValue[i] = pClass->mStrategicDefendValue[i];
    }
    mClass = pClass;
    mStrategicBan = 0;
    mAISpawnWeight = 100.0f;
    mPosition = gVectorZero;
    mDisplayHUDIndex = pClass->mDisplayHUDIndex;
}

// FUNCTION: SWBF2 0x0047a990
CommandPost::~CommandPost()
{
    delete m_pHologram;
    TargetManager::RemovePost(this);
    sPostArray[mPostIndex] = 0;
}

// FUNCTION: SWBF2 0x0047abb0
CommandPost* CommandPost::FindPostByName(const char* pszName)
{
    for (int i = 0; i < *sPostCount; i++)
    {
        if (sPostArray[i]->mName && !_stricmp(sPostArray[i]->mName, pszName))
        {
            return sPostArray[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0047ac10
CommandPost* CommandPost::FindPostByObject(GameObject* pObject)
{
    for (int i = 0; i < *sPostCount; i++)
    {
        if (sPostArray[i]->mObject == pObject)
        {
            return sPostArray[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0047ac80
CommandPost* CommandPost::BuildPost(GameObject* pObject, CommandPostClass* pClass)
{
    int iIndex = sForcedIndex;
    sForcedIndex = -1;
    if (iIndex < 0)
    {
        for (int i = 0; i < *sPostCount; i++)
        {
            if (sPostArray[i] && !sPostArray[i]->mObject && sPostArray[i]->mClass == pClass)
            {
                const PblMatrix& mMatrix = pObject->GetMatrix();
                const PblVector3& vPos = sPostArray[i]->mPosition;
                float fX = vPos.x - mMatrix.m[3][0];
                float fY = vPos.y - mMatrix.m[3][1];
                float fZ = vPos.z - mMatrix.m[3][2];
                if (fX * fX + fY * fY + fZ * fZ < 1.0f)
                {
                    iIndex = i;
                    break;
                }
            }
        }
        if (iIndex < 0)
        {
            iIndex = *sPostCount;
            *sPostCount = iIndex + 1;
        }
    }
    else if (iIndex >= *sPostCount)
    {
        *sPostCount = iIndex + 1;
    }

    if (!sPostArray[iIndex])
    {
        new CommandPost(pClass, iIndex);
    }
    sPostArray[iIndex]->Attach(pObject);
    return sPostArray[iIndex];
}

// FUNCTION: SWBF2 0x0047ae00
void CommandPost::Attach(GameObject* pObject)
{
    mObject = pObject;
    mPosition = *(const PblVector3*)pObject->GetMatrix().m[3];
    if (!mName)
    {
        mName = const_cast<char*>(StringDB::Find(pObject->mId));
        if (!mName)
        {
            mName = mLabel ? mLabel : const_cast<char*>("");
        }
    }
    if (!mLabel)
    {
        mLabel = mName;
    }
    if (!mLocalizeName)
    {
        mLocalizeName = pObject->mEntityClass->mLabel;
    }

    if (!m_pHologram && (!NetGame::IsEnabled() || NetGame::IsClient() || NetGame::IsOneWorld()) &&
        mClass->m_pHologramClass)
    {
        EntityDesc desc;
        PblMatrix mBone;
        if (mObject->GetModel()->_mLod[0]->GetBoneMatrix(0xD1E4CC7A, mBone))
        {
            PblMatrix mWorld;
            D3DXMatrixMultiply(&mWorld, &mBone, &mObject->GetMatrix());
            desc.mMatrix = mWorld;
        }
        else
        {
            desc.mMatrix = mObject->GetMatrix();
        }
        m_pHologram = (EntityHologram*)mClass->m_pHologramClass->Build(desc);
    }
}

// FUNCTION: SWBF2 0x0047b020
void CommandPost::Kill()
{
    if (!IsThreadActive())
    {
        return;
    }
    DeactivateThread();

    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        NetGame::CreateEvent_Message(NetGame::MSG_LOST, 0, 0, 0, 0, Team::sTeams[mObject->mTeam], this);
        if (!mObject->IsRtti(CommandFlyer::rttiHashCommandFlyer) &&
            !mObject->IsRtti(CommandHover::rttiHashCommandHover) &&
            !mObject->IsRtti(CommandWalker::rttiHashCommandWalker))
        {
            Character* pKiller = GetGameObject()->mDamageOwner.mCharacter;
            if (pKiller && pKiller->mTeamPtr->GetAffiliation(GetGameObject()->mTeam) <= 0)
            {
                Character::GetPlayerStats(pKiller).AddCommandPostPoints(0, false);
            }
        }
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            EventManager::CommandPostEvent::mKill.Trigger(*this, mObject->mDamageOwner);
        }
    }

    TurnHologramOff();

    if (mKillRegion)
    {
        for (int i = 0; i < 15; i++)
        {
            PblVector3 vOffset = PblRandom::GetGlobalVector3();
            vOffset.x *= mKillRegion->mSize.x;
            vOffset.y *= mKillRegion->mSize.y;
            vOffset.z *= mKillRegion->mSize.z;
            PblVector3 vPos;
            D3DXVec3TransformCoord(&vPos, &vOffset, &mKillRegion->mMatrix);

            FLEffectClass* pEffectClass;
            switch (PblRandom::GetGlobalInt() % 3)
            {
            case 0:
                pEffectClass = FLEffect::Get(0x02BB1FE0);
                break;
            case 1:
                pEffectClass = FLEffect::Get(0xE07F39B5);
                break;
            case 2:
                pEffectClass = FLEffect::Get(0x9DB69EE4);
                break;
            default:
                continue;
            }
            if (pEffectClass)
            {
                FLEffectObject* pEffect = pEffectClass->BuildEffect();
                if (pEffect)
                {
                    pEffect->AttachEffectToVector(&vPos, false);
                }
            }
        }

        TriggerRegionManager::sInstance->ApplyTempDeathRegion(mKillRegion);
    }
    --*sActivePostCount;
}

// FUNCTION: SWBF2 0x0047b330
void CommandPost::Respawn()
{
    if (IsThreadActive())
    {
        return;
    }
    TargetManager::RemoveTarget(mObject);
    TargetManager::AddPost(this);
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        EventManager::CommandPostEvent::mRespawn.Trigger(*this, sEventParam);
    }
    ActivateThread(-20, PER_TURN);
    ++*sActivePostCount;
}

void CommandPost::TurnHologramOff()
{
    if (m_pHologram)
    {
        m_pHologram->TurnOff();
    }
}

// FUNCTION: SWBF2 0x0047b3d0
void CommandPost::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xC6C161F9:
        mCaptureRegion = RedRegion::Find(PblHash(pszValue));
        break;
    case 0x805C1F8A:
        mControlRegion = RedRegion::Find(PblHash(pszValue));
        break;
    case 0x76ED2F81:
        mKillRegion = RedRegion::Find(PblHash(pszValue));
        break;
    case 0x9B4AAD0C:
        mSpawnRegion = RedRegion::Find(PblHash(pszValue));
        break;
    case 0x0DBA4CB3:
    {
        float fRadius = 0.0f;
        sscanf(pszValue, "%f", &fRadius);
        if (fRadius > 0.0f)
        {
            RedRegionDesc desc;
            desc.mType = PblRegion::type_invalid;
            desc.mMatrix = GetMatrix();
            desc.mSize.w = fRadius;
            desc.mNameId = GetGameObject()->mId;
            desc.mTypeId = 0xAFD98518;
            if (!mCaptureRegion)
            {
                mCaptureRegion = new RedRegion(desc);
            }
            if (!mControlRegion)
            {
                mControlRegion = new RedRegion(desc);
            }
        }
        break;
    }
    case 0xDDE0B14F:
        if (!strncmp("", pszValue, 1))
        {
            mSpawnPath = 0;
        }
        else
        {
            mSpawnPath = const_cast<RedPath*>(RedPath::Find(PblHash(pszValue)));
        }
        break;
    case 0xDAD215B0:
        mAllyPath = const_cast<RedPath*>(RedPath::Find(PblHash(pszValue)));
        break;
    case 0x73466220:
        mTurretPath = const_cast<RedPath*>(RedPath::Find(PblHash(pszValue)));
        break;
    case 0x7136FD3E:
        sscanf(pszValue, "%i", &mAllyCount);
        break;
    case 0xF69717FD:
    {
        mLabel = const_cast<char*>(StringDB::Store(PblHash(pszValue), pszValue));
        char acKey[256];
        _snprintf(acKey, sizeof(acKey), "level.%s.%s", GameLoop::mWorldFileName, pszValue);
        mLocalizeName = (unsigned short*)RedLocalizeDB::Find(PblHash(acKey), true);
        if (!mLocalizeName)
        {
            _snprintf(acKey, sizeof(acKey), "level.%s.%s", GameLoop::mTerrainFileName, pszValue);
            mLocalizeName = (unsigned short*)RedLocalizeDB::Find(PblHash(acKey), true);
        }
        break;
    }
    case 0x66BCDAEA:
        mHUDPostIndex = atol(pszValue);
        break;
    case 0xC069B070:
        mDisplayHUDIndex = atol(pszValue) != 0;
        break;
    case 0xE07282C2:
    {
        float fWeight = (float)atof(pszValue);
        mAISpawnWeight = fWeight > 0.0f ? fWeight : 0.0f;
        break;
    }

    case 0xEB13C1FE:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Neutral]);
        break;
    case 0xFDE7C9D2:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Alliance]);
        break;
    case 0x5A7EB7DB:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Empire]);
        break;
    case 0xDC392F3F:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Republic]);
        break;
    case 0x2450A89C:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_CIS]);
        break;
    case 0xD97FC017:
        sscanf(pszValue, "%d", &mBleedValue[Team::side_Locals]);
        break;
    case 0xA340D6B5:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Alliance]);
        break;
    case 0x4DB44670:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Empire]);
        break;
    case 0x9E78B410:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Republic]);
        break;
    case 0x00E73245:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_CIS]);
        break;
    case 0x573C98A4:
        sscanf(pszValue, "%d", &mStrategicAttackValue[Team::side_Locals]);
        break;
    case 0x44A2C4A6:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Alliance]);
        break;
    case 0xA8679457:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Empire]);
        break;
    case 0x8F5540CB:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Republic]);
        break;
    case 0x5BF85AE8:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_CIS]);
        break;
    case 0xE689EF93:
        sscanf(pszValue, "%d", &mStrategicDefendValue[Team::side_Locals]);
        break;

    case 0x3994A056:
        if (atoi(pszValue) > 0)
        {
            mStrategicBan |= 0x01;
        }
        break;
    case 0x58FC17EE:
        if (atoi(pszValue) > 0)
        {
            mStrategicBan |= 0x02;
        }
        break;
    case 0x6D1CA06D:
        if (atoi(pszValue) > 0)
        {
            mStrategicBan |= 0x04;
        }
        break;
    case 0xC8F3C76B:
        if (atoi(pszValue) > 0)
        {
            mStrategicBan |= 0x08;
        }
        break;
    case 0xB08090E7:
        if (atoi(pszValue) > 0)
        {
            mStrategicBan |= 0x10;
        }
        break;
    case 0x1CD984F6:
        if (atoi(pszValue) > 0)
        {
            mStrategicBan |= 0x20;
        }
        break;

    case 0x13AD1EA0:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Alliance][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0x9E71F320:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Alliance][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0x22BD3453:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Alliance][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0xFD4B3AA1:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Alliance][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0xBA31CB0A:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Alliance][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0x64340317:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Empire][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0xC0FC4645:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Empire][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0xE018B5A4:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Empire][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0x1ADDABEA:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Empire][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0xF7C6ECCB:
        mVoiceOver[Team::side_Alliance].mStream[Team::side_Empire][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0xD74A7FCB:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Alliance][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0x2446D141:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Alliance][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0x87834218:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Alliance][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0x23121DDE:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Alliance][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0x05F2BEC7:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Alliance][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0xBD2190B4:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Empire][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0x1816200C:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Empire][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0xDED807E7:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Empire][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0xC2871425:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Empire][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0x6F900A76:
        mVoiceOver[Team::side_Empire].mStream[Team::side_Empire][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0x951D8F40:
        mVoiceOver[Team::side_Republic].mStream[Team::side_Republic][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0xA4E3CE40:
        mVoiceOver[Team::side_Republic].mStream[Team::side_Republic][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0xF8D18CF3:
        mVoiceOver[Team::side_Republic].mStream[Team::side_Republic][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0x1BD37E41:
        mVoiceOver[Team::side_Republic].mStream[Team::side_Republic][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0xBE2AD02A:
        mVoiceOver[Team::side_Republic].mStream[Team::side_Republic][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0x7B21958E:
        mVoiceOver[Team::side_Republic].mStream[Team::side_CIS][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0xB2BA322A:
        mVoiceOver[Team::side_Republic].mStream[Team::side_CIS][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0x8653E871:
        mVoiceOver[Team::side_Republic].mStream[Team::side_CIS][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0x6D015027:
        mVoiceOver[Team::side_Republic].mStream[Team::side_CIS][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0x1F5E25A0:
        mVoiceOver[Team::side_Republic].mStream[Team::side_CIS][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0x1934685E:
        mVoiceOver[Team::side_CIS].mStream[Team::side_Republic][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0x9F6648DA:
        mVoiceOver[Team::side_CIS].mStream[Team::side_Republic][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0x0D03A541:
        mVoiceOver[Team::side_CIS].mStream[Team::side_Republic][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0x6EB55FF7:
        mVoiceOver[Team::side_CIS].mStream[Team::side_Republic][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0x66A3FA90:
        mVoiceOver[Team::side_CIS].mStream[Team::side_Republic][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    case 0x8F270798:
        mVoiceOver[Team::side_CIS].mStream[Team::side_CIS][VoiceOver::stream_Capture].SetID(pszValue, false, false);
        break;
    case 0xD4D671E8:
        mVoiceOver[Team::side_CIS].mStream[Team::side_CIS][VoiceOver::stream_Lost].SetID(pszValue, false, false);
        break;
    case 0x5BA686BB:
        mVoiceOver[Team::side_CIS].mStream[Team::side_CIS][VoiceOver::stream_Dispute].SetID(pszValue, false, false);
        break;
    case 0x3EE46599:
        mVoiceOver[Team::side_CIS].mStream[Team::side_CIS][VoiceOver::stream_Saved].SetID(pszValue, false, false);
        break;
    case 0xB708CE82:
        mVoiceOver[Team::side_CIS].mStream[Team::side_CIS][VoiceOver::stream_Info].SetID(pszValue, false, false);
        break;
    }
}

// FUNCTION: SWBF2 0x0047c440
int CommandPost::PushSpawnQueue(bool bPlayer, bool bTeam)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }
    int iSlot = ++mSpawnQueue[bTeam][bPlayer];
    if (!bPlayer)
    {
        iSlot += mSpawnQueue[bTeam][1];
    }
    return iSlot;
}

// FUNCTION: SWBF2 0x0047c4b0
int CommandPost::PopSpawnQueue(bool bPlayer, bool bTeam)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return 0;
    }
    int iCount = mSpawnQueue[bTeam][bPlayer] - 1;
    if (iCount < 0)
    {
        iCount = 0;
    }
    mSpawnQueue[bTeam][bPlayer] = iCount;
    if (!bPlayer)
    {
        iCount += mSpawnQueue[bTeam][1];
    }
    return iCount;
}

// FUNCTION: SWBF2 0x0047c530
int CommandPost::GetBleedValue(int iTeam) const
{
    return mBleedValue[Team::sTeams[iTeam]->GetSide()];
}

// FUNCTION: SWBF2 0x0047c550
bool CommandPost::CanTeamSpawnHere(int iTeam) const
{
    if (!mIsAISpawnPoint || !GetGameObject())
    {
        return false;
    }
    if (GetGameObject()->mTeam == iTeam)
    {
        return mSpawnPath || mClass->mSpawnCount > 0;
    }
    return Team::sTeams[GetGameObject()->mTeam]->GetAffiliation(iTeam) > 0 && mAllyPath;
}

// FUNCTION: SWBF2 0x0047c640
PblMatrix CommandPost::GetMatrixOnSpawnPath(bool bAlly) const
{
    PblMatrix mMatrix = GetMatrix();
    const RedPath* pPath = bAlly ? mAllyPath : mSpawnPath;
    if (pPath && pPath->GetNumPoints())
    {
        const unsigned int uPoint = PblRandom::GetSim().GetInt() % pPath->GetNumPoints();
        const PblQuaternion qRotation = pPath->GetPointRotation(uPoint).Inverse();
        const PblVector3& vPosition = pPath->GetPointPosition(uPoint);
        PblQuaternion qUnit;
        D3DXMatrixRotationQuaternion(&mMatrix, D3DXQuaternionNormalize(&qUnit, &qRotation));
        mMatrix.m[3][0] = vPosition.x;
        mMatrix.m[3][1] = vPosition.y;
        mMatrix.m[3][2] = vPosition.z;
        mMatrix.m[3][3] = 1.0f;
        mMatrix.m[0][0] = -mMatrix.m[0][0];
        mMatrix.m[0][1] = -mMatrix.m[0][1];
        mMatrix.m[0][2] = -mMatrix.m[0][2];
        mMatrix.m[0][3] = 0.0f;
        mMatrix.m[2][0] = -mMatrix.m[2][0];
        mMatrix.m[2][1] = -mMatrix.m[2][1];
        mMatrix.m[2][2] = -mMatrix.m[2][2];
        mMatrix.m[2][3] = 0.0f;

        if (AIUtil::IsUberMode() && mSpawnRegion)
        {
            const PblVector3 vPoint = AIUtil::GetRandomPtInRegion(mSpawnRegion);
            mMatrix.m[3][0] = vPoint.x;
            mMatrix.m[3][1] = vPoint.y;
            mMatrix.m[3][2] = vPoint.z;
            mMatrix.m[3][3] = 1.0f;
        }

        PblVector3 vStart(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
        CollisionObject* pHitObject;
        const float fUp = CollisionManager::RayHit(vStart, gVectorUnitY, 10.0f, pHitObject, 0, 0, 0,
                                                   COLL_STATIC | COLL_TERRAIN, true);
        vStart.y += fUp * 10.0f - 0.1f;
        const PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
        const float fDown = CollisionManager::RayHit(vStart, vDown, 20.0f, pHitObject, 0, 0, 0,
                                                     COLL_STATIC | COLL_TERRAIN, true);
        if (fDown < 1.0f)
        {
            mMatrix.m[3][0] = vStart.x - fDown * 20.0f * gVectorUnitY.x;
            mMatrix.m[3][1] = vStart.y - fDown * 20.0f * gVectorUnitY.y;
            mMatrix.m[3][2] = vStart.z - fDown * 20.0f * gVectorUnitY.z;
            mMatrix.m[3][3] = 1.0f;
        }
    }
    else if (!bAlly && mClass->mSpawnCount)
    {
        const unsigned int uPoint = PblRandom::GetSim().GetInt() % mClass->mSpawnCount;
        const PblVector3& vPosition = mClass->mSpawnPosition[uPoint];
        PblQuaternion qUnit;
        PblMatrix mSpawn;
        D3DXMatrixRotationQuaternion(&mSpawn, D3DXQuaternionNormalize(&qUnit, &mClass->mSpawnRotation[uPoint]));
        mSpawn.m[3][0] = vPosition.x;
        mSpawn.m[3][1] = vPosition.y;
        mSpawn.m[3][2] = vPosition.z;
        mSpawn.m[3][3] = 1.0f;
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, &mSpawn, &mMatrix);
        mMatrix = mWorld;

        if (GetGameObject())
        {
            const PblVector3 vCenter = GetGameObject()->GetCollisionObject()->GetCollisionCenter();
            const PblVector3 vPoint(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
            const GameObject* pIgnore = GetGameObject();
            CollisionObject* pHitObject;
            const float fHit = CollisionManager::RayHit(vCenter, vPoint, pHitObject, 0, &pIgnore, 1,
                                                        COLL_STATIC | COLL_TERRAIN, true);
            if (fHit < 1.0f)
            {
                mMatrix.m[3][0] = (vPoint.x - vCenter.x) * fHit + vCenter.x;
                mMatrix.m[3][1] = (vPoint.y - vCenter.y) * fHit + vCenter.y;
                mMatrix.m[3][2] = (vPoint.z - vCenter.z) * fHit + vCenter.z;
                mMatrix.m[3][3] = 1.0f;
            }
        }
    }
    return NormalMatrix(*(const PblVector3*)mMatrix.m[2], gVectorUnitY, *(const PblVector3*)mMatrix.m[3]);
}

// FUNCTION: SWBF2 0x0047ccd0
void CommandPost::PlayVoiceOver(Team::Side eSide, VoiceOver::Stream eStream, bool bPlayMusic)
{
    if (GameLoop::IsGameOver() || !VoiceOverManager::sInstance)
    {
        return;
    }
    const Team::Side eListener = Team::sTeams[VoiceOverManager::sInstance->GetPlayerTeamNumber() + 1]->GetSide();
    const bool bPlay = VoiceOverManager::sInstance->mPlay;
    bPlayMusic = bPlayMusic && bPlay;
    if (CommandPost::sEnableVO && bPlay)
    {
        GameSoundStream& rStream = mVoiceOver[eListener].mStream[eSide][eStream];
        const PblVector3* pPos = 0;
        if (rStream.GetProperties(false) && rStream.GetProperties(false)->Get3DMode())
        {
            pPos = (const PblVector3*)GetMatrix().m[3];
        }
        rStream.Play(pPos, 0, false, true);
    }

    GameMusic* pMusic = 0;
    switch (eStream)
    {
    case VoiceOver::stream_Capture:
        mClass->mSoundCaptured.Play((const PblVector3*)GetMatrix().m[3], 0, false, true);
        pMusic = &mClass->mMusicCapture[eListener];
        break;
    case VoiceOver::stream_Lost:
        mClass->mSoundLost.Play((const PblVector3*)GetMatrix().m[3], 0, false, true);
        pMusic = &mClass->mMusicLost[eListener];
        break;
    }
    if (bPlayMusic && pMusic && pMusic->ReadyToPlay())
    {
        pMusic->Play();
    }
}

// FUNCTION: SWBF2 0x0047ce30
bool CommandPost::Update(float fDeltaTime)
{
    const int iOldTeam = GetGameObject()->mTeam;
    const int iOldHoldTeam = mHoldTeam;
    mHoldTeam = 0;
    memset(mHoldCount, 0, sizeof(mHoldCount));
    mHoldPlayerMask = 0;
    mSkippedPlayerMask = 0;
    bool bHologramOn = true;
    int iCapturePlayer = 0;

    mSoundAmbience.Update(&mSoundAmbienceProps, (const PblVector3*)GetMatrix().m[3], 0);
    if (!mSoundAmbience.GetPlaying())
    {
        mSoundAmbience.Play(&mSoundAmbienceProps, (const PblVector3*)GetMatrix().m[3], 0, true);
    }
    mSoundTransition.Update(&mSoundTransitionProps, (const PblVector3*)GetMatrix().m[3], 0);

    Character* pLongestHolder = 0;
    float fLongestHoldTime = 0.0f;
    GameObject* apObjects[64];
    int iCount = 0;
    if (!NetGame::IsEnabled() || !NetGame::IsClient() || NetGame::IsNearLocalPlayer(*(const PblVector3*)GetMatrix().m[3]))
    {
        iCount = GetObjectsInCaptureRegion(apObjects, 64);
        for (int i = 0; i < iCount; ++i)
        {
            GameObject* pObject = apObjects[i];
            if (!pObject->IsAlive() || !pObject->GetControllable())
            {
                continue;
            }
            if (pObject->mTeam != pObject->mPerceivedTeam)
            {
                continue;
            }
            const int iCaptureTeam = Team::sTeams[pObject->mTeam]->mCaptureTeam;
            if (!iCaptureTeam)
            {
                continue;
            }

            Controllable* pControllable = pObject->GetControllable();
            bool bSpawnProtected = false;
            const Character* pCharacter = pControllable->mCharacter;
            if (pCharacter)
            {
                if (pCharacter->mPlayerId < 0 && !((1 << pCharacter->mTeamNumber) & mAICanCaptureTeamMask))
                {
                    continue;
                }
                if (pCharacter->mPlayerId >= 0 && NetGame::IsEnabled())
                {
                    bSpawnProtected = (float)gSpawnInvicibility > NetComm::GetTime() - pCharacter->GetSpawnTime();
                }
            }

            if (pControllable->GetControllableClass()->mCanCapturePosts &&
                !(NetGame::IsEnabled() && bSpawnProtected))
            {
                if (!mHoldTeam)
                {
                    mHoldTeam = iCaptureTeam;
                }
                else if (mHoldTeam != iCaptureTeam)
                {
                    mHoldTeam = -1;
                }
                ++mHoldCount[iCaptureTeam];
            }

            const PblList<Controllable>& rControllables = pObject->GetControllableList();
            for (PblList<Controllable>::Itor it = rControllables.Begin(); (Controllable*)it; ++it)
            {
                const int iPlayerId = it->GetPlayerId();
                if (iPlayerId >= 0)
                {
                    if (pControllable->GetControllableClass()->mCanCapturePosts &&
                        !(NetGame::IsEnabled() && bSpawnProtected))
                    {
                        mHoldPlayerMask |= (unsigned __int64)1 << iPlayerId;
                    }
                    else
                    {
                        mSkippedPlayerMask |= (unsigned __int64)1 << iPlayerId;
                    }
                }

                Character* pHolder = it->mCharacter;
                if (pHolder && pHolder->GetCPHold() != this)
                {
                    pHolder->SetCPHold(this);
                    pHolder->SetCPHoldTimestamp(GameLoop::GetMissionTime());
                }
                const float fHoldTime = pHolder ? GameLoop::GetMissionTime() - pHolder->GetCPHoldTimestamp() : 0.0f;
                if (fHoldTime > fLongestHoldTime)
                {
                    pLongestHolder = pHolder;
                    fLongestHoldTime = fHoldTime;
                }
            }
        }
    }

    unsigned __int64 uMask = mHoldPlayerMask;
    while (uMask >>= 1)
    {
        ++iCapturePlayer;
    }

    if (NetGame::IsPreGame())
    {
        return true;
    }

    if (!GetGameObject()->mTeam)
    {

        const float fCaptureTime = GetCaptureTime();
        if (mHoldTeam > 0)
        {
            if (mHoldTeam != iOldHoldTeam && (!NetGame::IsEnabled() || !NetGame::IsClient()))
            {
                EventManager::CommandPostEvent::mBeginCapture.Trigger(*this, mHoldPlayerMask);
            }

            float fRate = 0.0f;
            float fWeight = 1.0f;
            for (int i = mHoldCount[mHoldTeam]; i > 0; --i)
            {
                fRate += fWeight;
                fWeight *= 0.5f;
            }
            fRate *= fDeltaTime;
            if (mBiasTeam == mHoldTeam)
            {
                mCaptureTimer += fRate;
            }
            else
            {
                mCaptureTimer -= fRate;
                if (mCaptureTimer <= 0.0f)
                {
                    mCaptureTimer = -mCaptureTimer;
                    mBiasTeam = mHoldTeam;
                }
            }

            if (mClass->mSoundCharge.GetPropertiesHashID() != mSoundTransitionProps.GetPropertiesHashID() ||
                !mSoundTransition.GetPlaying())
            {
                mSoundTransition.Stop(true);
                mSoundTransitionProps = mClass->mSoundCharge;
                mSoundTransition.Play(&mSoundTransitionProps, (const PblVector3*)GetMatrix().m[3], 0, true);
            }
            if (Snd::VoiceVirtual* pVoice = mSoundTransition.GetVoiceVirtual())
            {
                const float fPitchDev = mClass->mSoundPitchDev;
                pVoice->SetPitch(mCaptureTimer / fCaptureTime * fPitchDev + (1.0f - fPitchDev));
            }

            if (mCaptureTimer > fCaptureTime)
            {
                mCaptureTimer = fCaptureTime;
                if (!NetGame::IsEnabled() || !NetGame::IsClient())
                {
                    GetGameObject()->SetTeam(mHoldTeam);
                    GetGameObject()->mPerceivedTeam = mHoldTeam;
                }
                if (!NetGame::IsEnabled() || !NetGame::IsClient())
                {
                    EventManager::CommandPostEvent::mFinishCapture.Trigger(*this, mHoldPlayerMask);
                }
                mSoundTransition.Stop(true);

                Team* pTeam = Team::sTeams[GetGameObject()->mTeam];
                const Character* pCapturer = iCapturePlayer ? Character::FindPlayer(iCapturePlayer - 1) : 0;
                if (!NetGame::IsEnabled() || !NetGame::IsClient())
                {
                    NetGame::CreateEvent_Message(NetGame::MSG_CAPTURED, pCapturer, 0, 0, 0, pTeam, this);
                }
                if (!NetGame::IsEnabled() || !NetGame::IsClient())
                {
                    for (int i = 0; i < iCount; ++i)
                    {
                        GameObject* pObject = apObjects[i];
                        if (!pObject->IsAlive())
                        {
                            continue;
                        }
                        Controllable* pControllable = pObject->GetControllable();
                        if (pControllable && pControllable->mCharacter)
                        {
                            Character::GetPlayerStats(pControllable->mCharacter)
                                .AddCommandPostPoints(0, pControllable->mCharacter == pLongestHolder);
                        }
                    }
                }
            }
            if (mCaptureTimer >= fCaptureTime - mClass->m_fHoloTurnOnTime)
            {
                bHologramOn = false;
            }
        }
        else if (mHoldTeam == -1)
        {
            if (iOldHoldTeam != -1 && (!NetGame::IsEnabled() || !NetGame::IsClient()))
            {
                EventManager::CommandPostEvent::mAbortCapture.Trigger(*this, mHoldPlayerMask);
            }
            if (mClass->mSoundDispute.GetPropertiesHashID() != mSoundTransitionProps.GetPropertiesHashID() ||
                !mSoundTransition.GetPlaying())
            {
                mSoundTransition.Stop(true);
                mSoundTransitionProps = mClass->mSoundDispute;
                mSoundTransition.Play(&mSoundTransitionProps, (const PblVector3*)GetMatrix().m[3], 0, true);
            }
        }
        else
        {
            if (mHoldTeam != iOldHoldTeam && (!NetGame::IsEnabled() || !NetGame::IsClient()))
            {
                EventManager::CommandPostEvent::mAbortCapture.Trigger(*this, mHoldPlayerMask);
            }
            mSoundTransition.Stop(true);
        }
    }
    else if (mHoldTeam && mHoldTeam != GetGameObject()->mTeam)
    {

        const float fNeutralizeTime = GetNeutralizeTime();
        const bool bDisputed = mNeutralizeTimer == 0.0f && mHoldTeam == -1;
        if (mNeutralizeTimer == 0.0f && (!NetGame::IsEnabled() || !NetGame::IsClient()))
        {
            EventManager::CommandPostEvent::mBeginNeutralize.Trigger(*this, mHoldPlayerMask);
        }

        float fRate = 0.0f;
        float fWeight = 1.0f;
        for (int iTeam = 1; iTeam < 8; ++iTeam)
        {
            if (iTeam == GetGameObject()->mTeam)
            {
                continue;
            }
            for (int i = mHoldCount[iTeam]; i > 0; --i)
            {
                fRate += fWeight;
                fWeight *= 0.5f;
            }
        }
        mNeutralizeTimer += fRate * fDeltaTime;

        if (mClass->mSoundDischarge.GetPropertiesHashID() != mSoundTransitionProps.GetPropertiesHashID() ||
            !mSoundTransition.GetPlaying())
        {
            mSoundTransition.Stop(true);
            mSoundTransitionProps = mClass->mSoundDischarge;
            mSoundTransition.Play(&mSoundTransitionProps, (const PblVector3*)GetMatrix().m[3], 0, true);
        }
        if (Snd::VoiceVirtual* pVoice = mSoundTransition.GetVoiceVirtual())
        {
            const float fPitchDev = mClass->mSoundPitchDev;
            pVoice->SetPitch((1.0f - mNeutralizeTimer / fNeutralizeTime) * fPitchDev + (1.0f - fPitchDev));
        }

        if (bDisputed)
        {
            Team* pTeam = Team::sTeams[GetGameObject()->mTeam];
            if (!NetGame::IsEnabled() || !NetGame::IsClient())
            {
                NetGame::CreateEvent_Message(NetGame::MSG_DISPUTED, 0, 0, 0, 0, pTeam, this);
            }
        }
        else if (mNeutralizeTimer >= fNeutralizeTime)
        {
            Team* pTeam = Team::sTeams[GetGameObject()->mTeam];
            const Character* pCapturer = iCapturePlayer ? Character::FindPlayer(iCapturePlayer - 1) : 0;
            if (!NetGame::IsEnabled() || !NetGame::IsClient())
            {
                NetGame::CreateEvent_Message(NetGame::MSG_LOST, pCapturer, 0, 0, 0, pTeam, this);
            }
            mNeutralizeTimer = fNeutralizeTime;
            mCaptureTimer = 0.0f;
            mBiasTeam = mHoldTeam > 0 ? mHoldTeam : 0;
            if (!NetGame::IsEnabled() || !NetGame::IsClient())
            {
                GetGameObject()->SetTeam(0);
                GetGameObject()->mPerceivedTeam = 0;
            }
            mSoundTransition.Stop(true);
            if (!NetGame::IsEnabled() || !NetGame::IsClient())
            {
                EventManager::CommandPostEvent::mFinishNeutralize.Trigger(*this, mHoldPlayerMask);
            }
        }
        if (mNeutralizeTimer >= fNeutralizeTime - mClass->m_fHoloTurnOnTime)
        {
            bHologramOn = false;
        }
    }
    else if (mNeutralizeTimer > 0.0f)
    {

        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            EventManager::CommandPostEvent::mAbortNeutralize.Trigger(*this, mHoldPlayerMask);
        }
        if (mNeutralizeTimer > 0.0f)
        {
            float fRate = 0.0f;
            float fWeight = 1.0f;
            for (int i = mHoldCount[GetGameObject()->mTeam]; i > 0; --i)
            {
                fRate += fWeight;
                fWeight *= 0.5f;
            }
            mNeutralizeTimer -= fRate * fDeltaTime;
            if (mNeutralizeTimer < 0.0f)
            {
                mNeutralizeTimer = 0.0f;
            }
        }
        if (mNeutralizeTimer == 0.0f && mHoldTeam == GetGameObject()->mTeam &&
            mNeutralizeTimer != GetNeutralizeTime())
        {
            if (mClass->mSoundCharge.GetPropertiesHashID() != mSoundTransitionProps.GetPropertiesHashID() ||
                !mSoundTransition.GetPlaying())
            {
                mSoundTransition.Stop(true);
                mSoundTransitionProps = mClass->mSoundCharge;
                mSoundTransition.Play(&mSoundTransitionProps, (const PblVector3*)GetMatrix().m[3], 0, true);
            }
            Team* pTeam = Team::sTeams[GetGameObject()->mTeam];
            if (!NetGame::IsEnabled() || !NetGame::IsClient())
            {
                NetGame::CreateEvent_Message(NetGame::MSG_SAVED, 0, 0, 0, 0, pTeam, this);
            }
        }
    }
    else
    {
        mSoundTransition.Stop(true);
    }

    if (GetGameObject()->IsAlive() && (!NetGame::IsEnabled() || NetGame::IsClient()))
    {
        PlayVoiceOver(Team::sTeams[GetGameObject()->mTeam]->GetSide(), VoiceOver::stream_Info, false);
    }

    const int iTeam = GetGameObject()->mTeam;
    if (iTeam != iOldTeam && NetGame::IsEnabled() && !NetGame::IsClient())
    {
        NetGame::CreateEvent_ChangeTeam(this, (unsigned __int64)-1);
    }

    if (m_pHologram)
    {
        if (bHologramOn)
        {
            m_pHologram->TurnOn();
        }
        else
        {
            m_pHologram->TurnOff();
        }
        if (iTeam != m_pHologram->GetTeam())
        {
            m_pHologram->SetModel(mClass->m_pHoloModel[Team::sTeams[iTeam]->GetSide()]);
            m_pHologram->SetTeam(iTeam);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0047ccb0
float CommandPost::GetCaptureTime() const
{
    return mClass->mCaptureTime;
}

// FUNCTION: SWBF2 0x0047ccc0
float CommandPost::GetNeutralizeTime() const
{
    return mClass->mNeutralizeTime;
}

// FUNCTION: SWBF2 0x0047e250
int CommandPost::GetObjectsInRegion(const RedRegion* pRegion, GameObject** const ppObjects, int iMaxObjects) const
{
    if (!pRegion)
    {
        return 0;
    }
    return CollisionManager::GetObjectsInRegion(*pRegion, ppObjects, iMaxObjects, 0, COLL_SOFT | COLL_RIGID);
}

// FUNCTION: SWBF2 0x0047e280
bool CommandPost::IsInsideRegion(const RedRegion* pRegion, const GameObject* pObject) const
{
    if (!pRegion)
    {
        return false;
    }
    return pRegion->IsPointInside(pObject->GetMatrix().GetTranslation());
}

// FUNCTION: SWBF2 0x0047e2b0
void CommandPost::SetTeam(int iOldTeam, int iNewTeam)
{
    if (iOldTeam == iNewTeam)
    {
        return;
    }
    if (NetGame::IsEnabled() && NetGame::IsClient() && m_pHologram)
    {
        m_pHologram->SetModel(mClass->m_pHoloModel[Team::sTeams[iNewTeam]->GetSide()]);
        m_pHologram->SetTeam(iNewTeam);
        m_pHologram->TurnOn();
    }
    mSoundAmbience.Stop(true);
    if (iNewTeam > 0)
    {
        mSoundAmbienceProps = mClass->mSoundAmbience[Team::sTeams[iNewTeam]->GetSide()];
        mSoundAmbience.Play(&mSoundAmbienceProps, (const PblVector3*)GetMatrix().m[3], 0, true);
    }
}

// FUNCTION: SWBF2 0x0047e390
int CommandPost::GetStrategicAttackValue(Team::Side eSide) const
{
    return mStrategicAttackValue[eSide];
}

// FUNCTION: SWBF2 0x0047e3b0
int CommandPost::GetStrategicDefendValue(Team::Side eSide) const
{
    return mStrategicDefendValue[eSide];
}

// FUNCTION: SWBF2 0x0047e3d0
int CommandPost::GetStrategicBan() const
{
    return mStrategicBan;
}

namespace EventManager
{

// FUNCTION: SWBF2 0x0047f460
template <>
void Event<CommandPost, char>::Callback::Trigger(CommandPost& item, char& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

// FUNCTION: SWBF2 0x0047a670
unsigned int __fastcall GetDoorEntityId(const unsigned int* pRecord)
{
    return pRecord[1];
}

// FUNCTION: SWBF2 0x0047ab60
void CommandPost::ForceIndex(int iIndex)
{
    sForcedIndex = iIndex;
}

namespace EventManager
{

// FUNCTION: SWBF2 0x0047f220
template <>
void Event<CommandPost, unsigned __int64>::Callback::Trigger(CommandPost& item, unsigned __int64& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}

namespace EventManager
{

// FUNCTION: SWBF2 0x0047f340
template <>
void Event<CommandPost, DamageOwner>::Callback::Trigger(CommandPost& item, DamageOwner& param)
{
    AddRef();
    if ((*this)(item, param))
    {
        Callback* pChild = (Callback*)mChild;
        if (pChild)
        {
            pChild->AddRef();
            while (pChild->IsAlive())
            {
                Callback* pNext = (Callback*)pChild->mSibling;
                if (pNext)
                {
                    pNext->AddRef();
                }
                pChild->Trigger(item, param);
                pChild->Release();
                pChild = pNext;
                if (!pChild)
                {
                    break;
                }
            }
            if (pChild)
            {
                pChild->Release();
            }
        }
    }
    Release();
}
}
