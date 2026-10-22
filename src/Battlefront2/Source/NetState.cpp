#include "NetState.h"

#include "EntityDroid.h"
#include "EntityBuildingArmed.h"
#include "EntityBuildingArmedDynamic.h"
#include "FlagItem.h"
#include "GameModel.h"
#include "PowerupStation.h"
#include "CommandBuilding.h"
#include "CommandPost.h"
#include "CommandBuildingArmed.h"
#include "CommandBuildingAnimatedArmed.h"
#include "CommandHover.h"
#include "CommandWalker.h"
#include "Asteroid.h"
#include "EntityBuildingAnimatedArmed.h"
#include "EntityHologram.h"
#include "NetStateViews.h"
#include "EntityLight.h"
#include "LightBeam.h"
#include "FLEffect.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "EntityDroideka.h"
#include "EntityCarrier.h"
#include "EntityHover.h"
#include "EntitySoldier.h"
#include "EntityWalker.h"
#include "WalkerLegPair.h"
#include "MountedTurret.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "PowerupItem.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "CollisionManager.h"
#include "TeamManager.h"
#include "Team.h"
#include "AILowLevel.h"
#include "PlayerController.h"
#include "UnitController.h"
#include "FirstPerson.h"
#include "MedalsMgr.h"
#include "TargetManager.h"
#include "Tracker.h"
#include "EntityMine.h"
#include "StickInfo.h"
#include "EntityTrap.h"
#include "EntityRemoteTerminal.h"
#include "ProceduralAnimation.h"
#include "EntityPath.h"
#include "EntityPathFollower.h"
#include "WeaponGrapplingHook.h"
#include "WeaponCannonInstance.h"

#include <float.h>
#include <math.h>
#include <string.h>
#include "EntityProp.h"

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut,
                                                              const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut,
                                                               const PblQuaternion* pQ);
extern "C" PblQuaternion* __stdcall D3DXQuaternionRotationMatrix(PblQuaternion* pOut,
                                                                    const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut,
    const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut,
    const PblVector3* pV, const PblMatrix* pM);
PblMatrix DirectionMatrix(const PblVector3& direction, const PblVector3& up,
                          const PblVector3& position);

// GLOBAL: SWBF2 0x01fa8c30
int netLOD;

static int NetVectorBits()
{
    return netLOD == 1 ? 32 : 11;
}

class ConvertValues
{
public:

    // FUNCTION: SWBF2 0x005b4000
    void TrackValue(float fValue) {}
};

class QuatConvert : public ConvertValues
{
public:
    float GetMin() const { return -1.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return 13; }
};
class UnitConvert : public ConvertValues
{
public:
    float GetMin() const { return -1.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return NetVectorBits(); }
};
class PositionXZConvert : public ConvertValues
{
public:
    float GetMin() const { return -10240.0f; }
    float GetMax() const { return 10240.0f; }
    int GetBits() const { return netLOD == 1 ? 32 : 21; }
};
class PositionYConvert : public ConvertValues
{
public:
    float GetMin() const { return -1024.0f; }
    float GetMax() const { return 1024.0f; }
    int GetBits() const { return netLOD == 1 ? 32 : 21; }
};

class SwitchConvert : public ConvertValues
{
public:
    float GetMin() const { return -1.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return 4; }
};
class ThrustConvert : public ConvertValues
{
public:
    float GetMin() const { return -1.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return netLOD == 1 ? 32 : 7; }
};
class AngleConvert : public ConvertValues
{
public:
    float GetMin() const { return -3.1415927410125732f; }
    float GetMax() const { return 3.1415927410125732f; }
    int GetBits() const { return netLOD == 1 ? 32 : 9; }
};
class JetFuelConvert : public ConvertValues
{
public:
    float GetMin() const { return 0.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return 11; }
};
 
 
 
 

static QuatConvert quatConvert;
static UnitConvert unitConvert;
static PositionXZConvert positionXZConvert;
static PositionYConvert positionYConvert;

static SwitchConvert switchConvert;
static ThrustConvert thrustConvert;
static AngleConvert angleConvert;
static JetFuelConvert jetFuelConvert;

template <class CONVERT> void WriteFixed(CONVERT&, NetPktGroup&, float);
template <class CONVERT> void ReadFixed(CONVERT&, NetPktGroup&, float&);

// GLOBAL: SWBF2 0x01fa8c34
int lastNetLOD;

void WriteMatrix(NetPktGroup& group, const PblMatrix& matrix);
void ReadMatrix(NetPktGroup& group, PblMatrix& matrix);
void AddNetCollision(CollisionObject* object);
void WriteVelocity(NetPktGroup& group, const PblVector3& value, float maxSpeed, int bits);
void ReadVelocity(NetPktGroup& group, PblVector3& value, float maxSpeed, int bits);
void WriteUnitVector(NetPktGroup& group, const PblVector3& value);
void ReadUnitVector(NetPktGroup& group, PblVector3& value);
void WriteCharacter(NetPktGroup& group, Character* pCharacter);
Character* ReadCharacter(NetPktGroup& group);
void WritePilots(NetPktGroup& group, GameObject& owner);
int ComputeNetLOD(int position, int playerId);
void ReadPilots(NetPktGroup& group, GameObject& owner);
bool IsNeverDies(EntityType type);
void CheckReadAlign(NetPktGroup& group);
extern int gDebugCount;

// FUNCTION: SWBF2 0x005f5860
void BeginNetLOD()
{
    netLOD = 0;
    lastNetLOD = 0;
}

// FUNCTION: SWBF2 0x005f6060
void WriteTeam(NetPktGroup& group, Team* pTeam)
{
    group.WriteBits(pTeam ? pTeam->GetNumber() : 7, 3);
}

// FUNCTION: SWBF2 0x005f6080
Team* ReadTeam(NetPktGroup& group)
{
    unsigned int index = 0;
    group.ReadBits(index, 3);
    return index == 7 ? 0 : Team::sTeams[index];
}

// FUNCTION: SWBF2 0x005f61c0
void WriteString(NetPktGroup* group, const char* text, unsigned int capacity)
{
    unsigned int length = (unsigned int)strlen(text);
    if (length >= capacity)
    {
        length = capacity - 1;
    }
    group->WriteBits(length, 8);
    for (unsigned int i = 0; i < length; ++i)
    {
        group->WriteBits(text[i], 8);
    }
}

// FUNCTION: SWBF2 0x005f6220
void ReadString(NetPktGroup& group, char* text, unsigned int capacity)
{
    unsigned int length = 0;
    group.ReadBits(length, 8);
    for (unsigned int i = 0; i < length; ++i)
    {
        unsigned int value = 0;
        group.ReadBits(value, 8);
        if (i < capacity)
        {
            text[i] = (char)value;
        }
    }
    text[capacity - 1] = 0;
    if (length < capacity - 1)
    {
        text[length] = 0;
    }
}

// FUNCTION: SWBF2 0x005e7e40
int __fastcall EncodeTriggerAction(unsigned int type)
{
    switch (type)
    {
    case 1: return 2;
    case 3: return 1;
    case 4: return 4;
    case 6: return 3;
    case 8: return 7;
    case 9: return 5;
    case 10: return 6;
    case 12:
    case 28: return 15;
    case 17: return 9;
    case 19: return 8;
    case 20: return 11;
    case 22: return 10;
    case 24: return 14;
    case 25: return 12;
    case 26: return 13;
    default: return 0;
    }
}

// FUNCTION: SWBF2 0x005e7f20
int __fastcall DecodeTriggerAction(unsigned int type)
{
    switch (type)
    {
    case 1: return 3;
    case 2: return 1;
    case 3: return 6;
    case 4: return 4;
    case 5: return 9;
    case 6: return 10;
    case 7: return 8;
    case 8: return 19;
    case 9: return 17;
    case 10: return 22;
    case 11: return 20;
    case 12: return 25;
    case 13: return 26;
    case 14: return 24;
    case 15: return 12;
    default: return 0;
    }
}

// FUNCTION: SWBF2 0x005f5850
void SetNetLOD(bool precision)
{
    netLOD = precision;
}

// FUNCTION: SWBF2 0x005f5880
void SetNetPrecision(NetPktGroup& group)
{
    if (netLOD == lastNetLOD)
    {
        group.WriteBits(1, 1);
        return;
    }
    group.WriteBits(0, 1);
    group.WriteBits(netLOD, 2);
    lastNetLOD = netLOD;
}

// FUNCTION: SWBF2 0x005f58c0
void ReadNetLOD(NetPktGroup& group)
{
    unsigned int unchanged = 0;
    group.ReadBits(unchanged, 1);
    if (!unchanged)
    {
        unsigned int precision = 0;
        group.ReadBits(precision, 2);
        lastNetLOD = precision;
    }
    netLOD = lastNetLOD;
}

// FUNCTION: SWBF2 0x005f60b0
void NetTrigger::Write(NetPktGroup& group)
{
    const unsigned int uiActionState = m_uiActionState;
    group.WriteBits(EncodeTriggerAction(uiActionState), 4);
    if (uiActionState == ACTIONSTATE_CLEAR && !m_uiTimerNet)
    {
        group.WriteBits(1, 5);
    }
    else
    {
        group.WriteBits(m_uiTimerNet, 5);
    }
}

// FUNCTION: SWBF2 0x005f6110
void NetTrigger::Read(NetPktGroup& group)
{
    unsigned int uiCode = 0;
    group.ReadBits(uiCode, 4);
    m_uiActionState = (unsigned int)DecodeTriggerAction(uiCode);
    unsigned int uiTimerNet = 0;
    group.ReadBits(uiTimerNet, 5);
    m_uiTimerNet = uiTimerNet;
    if (m_uiTimerNet)
    {
        m_uiTimer += 0x8000;
    }
    else if (m_uiActionState == ACTIONSTATE_CLEAR)
    {
        m_uiActionState = ACTIONSTATE_CLEAR_UNTIL_PRESS;
    }
}

// FUNCTION: SWBF2 0x005f61a0
void WriteTrigger(NetPktGroup& group, Trigger* pTrigger)
{
    static_cast<NetTrigger*>(pTrigger)->Write(group);
}

// FUNCTION: SWBF2 0x005f61b0
void ReadTrigger(NetPktGroup& group, Trigger* pTrigger)
{
    static_cast<NetTrigger*>(pTrigger)->Read(group);
}

// FUNCTION: SWBF2 0x005f5900
void WriteUnitScalar(NetPktGroup& group, float value)
{
    WriteFixed(unitConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5910
void ReadUnitScalar(NetPktGroup& group, float& value)
{
    ReadFixed(unitConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5be0
void WriteCosSin(NetPktGroup& group, const float* values)
{
    WriteFixed(unitConvert, group, values[0]);
    WriteFixed(unitConvert, group, values[1]);
}

// FUNCTION: SWBF2 0x005f5c00
void ReadCosSin(NetPktGroup& group, float* values)
{
    float x;
    float y;
    ReadFixed(unitConvert, group, x);
    ReadFixed(unitConvert, group, y);
    const float lengthSquared = x * x + y * y;
    float half = (3.0f - lengthSquared) * 0.5f;
    const float scale = (3.0f - half * lengthSquared * half) * (half * 0.5f);
    values[0] = scale * x;
    values[1] = scale * y;
}

// FUNCTION: SWBF2 0x005f6000
void WriteWeapon(NetPktGroup& group, Weapon* pObject)
{
    if (!pObject)
    {
        group.WriteBits(0, 1);
        return;
    }
    group.WriteBits(1, 1);
    pObject->Write(group);
}

// FUNCTION: SWBF2 0x005f6030
void ReadWeapon(NetPktGroup& group, Weapon* pObject)
{
    unsigned int present = 0;
    group.ReadBits(present, 1);
    if (!present)
    {
        return;
    }
    pObject->Read(group);
}

// FUNCTION: SWBF2 0x005f5e90
void WriteThrust(NetPktGroup& group, float value)
{
    WriteFixed(thrustConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5ea0
void ReadThrust(NetPktGroup& group, float& value)
{
    ReadFixed(thrustConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5ec0
void WriteAngle(NetPktGroup& group, float value)
{
    WriteFixed(angleConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5ed0
void ReadAngle(NetPktGroup& group, float& value)
{
    ReadFixed(angleConvert, group, value);
}

// FUNCTION: SWBF2 0x005ed430
void WalkerLegPair::Write(NetPktGroup& group)
{
    group.WriteBits(mFootContact[0] ? 1u : 0u, 1);
    group.WriteBits(mFootContact[1] ? 1u : 0u, 1);
    group.WriteBits(mIdleBlendFoot, 2);
    WriteUnitScalar(group, mLastIdleBlend);
}

// FUNCTION: SWBF2 0x005ed480
void WalkerLegPair::Read(NetPktGroup& group)
{
    unsigned int value = 0;
    group.ReadBits(value, 1);
    mFootContact[0] = value != 0;
    group.ReadBits(value, 1);
    mFootContact[1] = value != 0;
    group.ReadBits(value, 2);
    mIdleBlendFoot = (WalkerLegPair::eFoot)value;
    ReadUnitScalar(group, mLastIdleBlend);
}

// FUNCTION: SWBF2 0x005ed4e0
void WalkerBlend::Write(NetPktGroup& group)
{
    group.WriteFixed(mTime, 0.0f, 10.0f, 24);
    group.WriteBits(mCount, 2);
    for (int i = 0; i < mCount; ++i)
    {
        WriteUnitScalar(group, mUnits[i]->mWeight);
    }
}

// FUNCTION: SWBF2 0x005ed640
void WalkerBlend::Skip(NetPktGroup& group)
{
    float time;
    group.ReadFixed(time, 0.0f, 10.0f, 24);
    unsigned int count = 0;
    group.ReadBits(count, 2);
    mCount = count;
    for (int i = 0; i < mCount; ++i)
    {
        ReadUnitScalar(group, mUnits[i]->mWeight);
    }
}

// FUNCTION: SWBF2 0x005ed550
void WalkerBlend::Read(NetPktGroup& group)
{
    float time;
    group.ReadFixed(time, 0.0f, 10.0f, 24);
    unsigned int count = 0;
    group.ReadBits(count, 2);
    mCount = count;
    for (int i = 0; i < mCount; ++i)
    {
        ReadUnitScalar(group, mUnits[i]->mWeight);
    }
    Update(m_bReversed ? mTimeTotal - time : time);
    for (int i = 0; i < mCount; ++i)
    {
        ZephyrPoseDyn<64>& pose = mUnits[i]->mPose;
        ZephyrTransform transform;
        pose.GetJointTransform(transform, 0);
        pose.m_kRootTrans = transform.kTrans;
    }
}

// FUNCTION: SWBF2 0x005ed3f0
void EntityWalker::Move(const PblMatrix& matrix)
{
    GameObject& object = *(GameObject*)this;
    object.SetMatrix(matrix);
    RedSceneObject* scene = &object;
    scene->Deactivate();
    scene->Activate();
}

// FUNCTION: SWBF2 0x005eedb0
void EntityDroid::Move(const PblMatrix& matrix)
{
    GameObject& object = *(GameObject*)this;
    object.SetMatrix(matrix);
    RedSceneObject* scene = &object;
    scene->Deactivate();
    scene->Activate();
}

// FUNCTION: SWBF2 0x005ef790
void EntityHover::Move(const PblMatrix& matrix)
{
    GameObject& object = *(GameObject*)this;
    object.SetMatrix(matrix);
    RedSceneObject* scene = &object;
    scene->Deactivate();
    scene->Activate();
}

// FUNCTION: SWBF2 0x005efea0
void Aimer::Read(NetPktGroup&)
{
    Aimer& aimer = *this;
    aimer.DeactivateThread();
    aimer.ActivateThread(-20 - aimer.mClass->mHierarchyLevel, PER_TURN);
}

// FUNCTION: SWBF2 0x005f0c40
void AddSendPilots(void (*send)(GameObject*, int), GameObject& owner, int depth)
{
    for (PblList<Controllable>::Itor it = owner.mControllableList.Begin();
         it != owner.mControllableList.End(); ++it)
    {
        Controllable* pControl = it;
        if (pControl->mPilotType < PILOT_VEHICLE)
        {
            continue;
        }
        if (pControl->mPilotType != PILOT_VEHICLESELF || pControl->mPilot != pControl)
        {
            Controllable* pPilot = pControl->mPilot;
            if (pPilot)
            {
                Trackable* pTrackable = static_cast<Trackable*>(pPilot);
                send(pTrackable->GetGameObject(), depth);
            }
        }
        if (pControl->mPlayerId == NetGame::GetCurDst() && pControl->mCtrl)
        {
            const GameObject* pTarget = pControl->mReticuleTarget[0];
            if (pTarget)
            {
                send(const_cast<GameObject*>(pTarget), depth + 1);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005eb4e0
void Asteroid::WriteObject(NetPktGroup& group, NetObjState*)
{
    WriteMatrix(group, static_cast<const CollisionObject*>(this)->GetMatrix());
}

// FUNCTION: SWBF2 0x005eb550
void Asteroid::Move(const PblMatrix& matrix)
{
    static_cast<CollisionObject*>(this)->SetMatrix(matrix);
    RedSceneObject* scene = this;
    scene->Deactivate();
    scene->Activate();
}

// FUNCTION: SWBF2 0x005eb500
void Asteroid::ReadObject(NetPktGroup& group, NetObjState*)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = static_cast<const CollisionObject*>(this)->GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
}

// FUNCTION: SWBF2 0x005f1d00
void CommandBuilding::WriteObject(NetPktGroup& group, NetObjState*)
{
    if (mMaxHealth < FLT_MAX)
    {
        group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    }
    group.WriteBits(mIsAlive, 1);
    if (mHealthTypeForLockOn == 8)
    {
        group.WriteBits(0, 1);
    }
    else
    {
        group.WriteBits(1, 1);
        group.WriteBits(mHealthTypeForLockOn & 7, 3);
    }
    group.WriteBits(mIsShielded, 1);
    group.WriteBits(mState, 2);
}

// FUNCTION: SWBF2 0x005f04e0
void PowerupStation::WriteObject(NetPktGroup& group, NetObjState*)
{
    group.WriteBits(mState, 2);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive, 1);
    if (mState == STATE_ALIVE)
    {
        group.WriteFixed(mPowerupTimer, -0.1f, mClass->mPowerupDelay, 8);
    }
}

// FUNCTION: SWBF2 0x005f0570
void PowerupStation::ReadObject(NetPktGroup& group, NetObjState*)
{
    unsigned int value;
    group.ReadBits(value, 2);
    const int state = value;
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (state != mState)
    {
        if (state == STATE_ALIVE)
        {
            static_cast<Damageable*>(this)->Respawn();
        }
        else if (state == STATE_DEAD)
        {
            static_cast<Damageable*>(this)->Kill();
        }
    }
    if (mState == STATE_ALIVE)
    {
        group.ReadFixed(mPowerupTimer, -0.1f, mClass->mPowerupDelay, 8);
    }
}

// FUNCTION: SWBF2 0x005f0820
void FlagItem::WriteCreate(NetPktGroup& group)
{
    group.WriteBits(mDroppedModel ? mDroppedModel->_uiNameHash : 0, 32);
    group.WriteBits(mCarriedModel ? mCarriedModel->_uiNameHash : 0, 32);
    group.WriteBits(mAllowTeamPickup != 0, 1);
    group.WriteBits(mAllowAIPickup != 0, 1);
    group.WriteBits(mHomeRegionId, 32);
}

// FUNCTION: SWBF2 0x005f08a0
void FlagItem::ReadCreate(NetPktGroup& group)
{
    unsigned int value;
    group.ReadBits(value, 32);
    mDroppedModel = value ? const_cast<GameModel*>(GameModel::Find(value)) : 0;
    group.ReadBits(value, 32);
    mCarriedModel = value ? const_cast<GameModel*>(GameModel::Find(value)) : 0;
    group.ReadBits(value, 1);
    mAllowTeamPickup = value != 0;
    group.ReadBits(value, 1);
    mAllowAIPickup = value != 0;
    group.ReadBits(mHomeRegionId, 32);
    mHomeRegion = RedRegion::Find(mHomeRegionId);
}

// FUNCTION: SWBF2 0x005f0970
void FlagItem::WriteObject(NetPktGroup& group, NetObjState*)
{
    WriteMatrix(group, static_cast<const CollisionObject*>(this)->GetMatrix());
    group.WriteBits(mStationary != 0, 1);
    if (!mStationary)
    {
        WriteVelocity(group, mVelocity, 5120.0f, -1);
    }
    if (mLifeTimer < FLT_MAX)
    {
        group.WriteBits(1, 1);
        group.WriteFixed(mLifeTimer, -1.0f, mClass->mLifeSpan, 8);
    }
    else
    {
        group.WriteBits(0, 1);
    }
    WriteCharacter(group, mOwner);
    group.WriteBits(mMoved != 0, 1);
}

// FUNCTION: SWBF2 0x005f0a40
void FlagItem::ReadObject(NetPktGroup& group, NetObjState*)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    CollisionObject* collision = this;
    collision->SetMatrix(matrix);
    memcpy(&collision->mLastPosition, &collision->GetPosition(), sizeof(PblVector3));
    unsigned int value;
    group.ReadBits(value, 1);
    mStationary = value != 0;
    if (mStationary)
    {
        mVelocity = PblVector3(0.0f, 0.0f, 0.0f);
        CollisionManager::DeactivateItem(collision);
    }
    else
    {
        ReadVelocity(group, mVelocity, 5120.0f, -1);
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mLifeTimer, -1.0f, mClass->mLifeSpan, 8);
    }
    else if (mLifeTimer != FLT_MAX)
    {
        ResetFlag();
    }
    Character* carrier = ReadCharacter(group);
    if (carrier != mOwner)
    {
        if (!carrier || mOwner)
        {
            ClearCarrierFlagItem();
        }
        mOwner = carrier;
        if (carrier)
        {
            carrier->mCarriedFlagPtr = this;
            if (mCarriedModel)
            {
                SetModel(mCarriedModel);
            }
            else
            {
                static_cast<RedSceneObject*>(this)->Deactivate();
            }
            if (collision->IsInCollisionList())
            {
                CollisionManager::RemoveBody(collision);
            }
            if (m_pLight)
            {
                m_pLight->TurnOff();
            }
        }
        else
        {
            if (!collision->IsInCollisionList())
            {
                CollisionManager::RemoveBody(collision);
                collision->AddItemToCollisionManager();
            }
            if (mCarriedModel)
            {
                SetModel(mDroppedModel);
            }
            else
            {
                static_cast<RedSceneObject*>(this)->Activate();
            }
            if (m_pLight)
            {
                m_pLight->TurnOn();
            }
        }
    }
    group.ReadBits(value, 1);
    mMoved = value != 0;
}

// FUNCTION: SWBF2 0x005f14b0
void EntityBuildingArmed::WriteObject(NetPktGroup& group, NetObjState*)
{
    group.WriteBits(mState, 2);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive, 1);
    if (mHealthTypeForLockOn == 8)
    {
        group.WriteBits(0, 1);
    }
    else
    {
        group.WriteBits(1, 1);
        group.WriteBits(mHealthTypeForLockOn & 7, 3);
    }
    if (mState == EntityBuilding::STATE_UNBUILT)
    {
        return;
    }
    WritePilots(group, *this);
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Write(group);
    }
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded, 1);
        const bool writeAddShield = (PblRandom::_Global.GetInt() & 3) == 0;
        group.WriteBits(writeAddShield, 1);
        if (writeAddShield)
        {
            const bool zero = mAddShield == 0.0f;
            group.WriteBits(zero, 1);
            if (!zero)
            {
                group.WriteFixed(mAddShield, 0.0f, mMaxShield, 11);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005f1650
void EntityBuildingArmed::ReadObject(NetPktGroup& group, NetObjState*)
{
    unsigned int value;
    group.ReadBits(value, 2);
    const int state = value;
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadBits(value, 3);
        mHealthTypeForLockOn = value;
    }
    else
    {
        mHealthTypeForLockOn = 8;
    }
    if (state == EntityBuilding::STATE_UNBUILT)
    {
        if (mState != EntityBuilding::STATE_UNBUILT)
        {
            static_cast<Damageable*>(this)->Kill();
        }
        return;
    }
    ReadPilots(group, *this);
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Read(group);
    }
    if (state != mState)
    {
        if (state == EntityBuilding::STATE_ALIVE)
        {
            static_cast<Damageable*>(this)->Respawn();
        }
        else if (state == EntityBuilding::STATE_DEAD || state == EntityBuilding::STATE_UNBUILT)
        {
            static_cast<Damageable*>(this)->Kill();
        }
    }
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
        group.ReadBits(value, 1);
        if (value)
        {
            group.ReadBits(value, 1);
            if (value)
            {
                mAddShield = 0.0f;
            }
            else
            {
                group.ReadFixed(mAddShield, 0.0f, mMaxShield, 11);
            }
        }
    }
}

// FUNCTION: SWBF2 0x005f19a0
void EntityBuildingAnimatedArmed::WriteObject(NetPktGroup& group, NetObjState*)
{
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive, 1);
    if (mHealthTypeForLockOn == 8)
    {
        group.WriteBits(0, 1);
    }
    else
    {
        group.WriteBits(1, 1);
        group.WriteBits(mHealthTypeForLockOn & 7, 3);
    }
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded, 1);
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        MountedTurret* turret = mTurret[i];
        Controllable* control = turret;
        Character* character = 0;
        if (control->mPilotType != PILOT_SELF &&
            (control->mPilotType != PILOT_VEHICLESELF || control->mPilot != control) &&
            control->mPilot)
        {
            Controllable* pilot = control->mPilot;
            static_cast<Trackable*>(pilot)->GetGameObject();
            character = pilot->mCharacter;
            AddSendLate();
        }
        WriteCharacter(group, character);
        turret->Write(group);
    }
}

// FUNCTION: SWBF2 0x005f1af0
void EntityBuildingAnimatedArmed::ReadObject(NetPktGroup& group, NetObjState*)
{
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    unsigned int value;
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadBits(value, 3);
        mHealthTypeForLockOn = value;
    }
    else
    {
        mHealthTypeForLockOn = 8;
    }
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        MountedTurret* turret = mTurret[i];
        Controllable* control = turret;
        Character* character = ReadCharacter(group);
        if (character)
        {
            Controllable* oldPilot = control->mPilotType == PILOT_SELF ||
                (control->mPilotType == PILOT_VEHICLESELF && control->mPilot == control)
                ? 0 : control->mPilot;
            Controllable* newPilot = character->mUnit;
            if (oldPilot != newPilot)
            {
                if (oldPilot)
                {
                    control->SetPilot(0);
                }
                control->SetPilot(newPilot);
                if (control->mPilotType == PILOT_REMOTE)
                {
                    character->SetRemote(control);
                }
                else if (control->mPilotType == PILOT_VEHICLE)
                {
                    character->SetVehicle(control);
                }
            }
        }
        else if (control->mPilotType != PILOT_SELF &&
                 (control->mPilotType != PILOT_VEHICLESELF || control->mPilot != control) &&
                 control->mPilot)
        {
            control->SetPilot(0);
        }
        turret->Read(group);
    }
}

// FUNCTION: SWBF2 0x005eb590
void EntityFlyer::AddSends(void (*send)(GameObject*, int), int depth)
{
    if (mPilot)
    {
        send(static_cast<Trackable*>(mPilot)->GetGameObject(), depth);
    }
    AddSendPilots(send, *this, depth);
    if (mPlayerId != NetGame::GetCurDst() || !mCtrl)
    {
        return;
    }
    const GameObject* reticuleTarget = mReticuleTarget[0];
    if (reticuleTarget)
    {
        send(const_cast<GameObject*>(reticuleTarget), depth + 1);
    }
    GameObject* lockedTarget = mTargetLockedObj;
    if (lockedTarget)
    {
        send(lockedTarget, depth + 1);
    }
}

// FUNCTION: SWBF2 0x005f1980
void EntityBuildingAnimatedArmed::AddSends(void (*send)(GameObject*, int), int depth)
{
    AddSendPilots(send, *this, depth);
}

// FUNCTION: SWBF2 0x005f1490
void EntityBuildingArmed::AddSends(void (*send)(GameObject*, int), int depth)
{
    AddSendPilots(send, *this, depth);
}

// FUNCTION: SWBF2 0x005f27d0
void EntityBuildingArmedDynamic::AddSends(void (*send)(GameObject*, int), int depth)
{
    EntityBuildingArmed::AddSends(send, depth);
}

// FUNCTION: SWBF2 0x005ec820
void EntityWalker::AddSends(void (*send)(GameObject*, int), int depth)
{
    if (mPilot)
    {
        Trackable* pTrackable = static_cast<Trackable*>(mPilot);
        send(pTrackable->GetGameObject(), depth);
    }
    AddSendPilots(send, *this, depth);
}

// FUNCTION: SWBF2 0x005ec860
void EntityWalker::WriteObject(NetPktGroup& group, NetObjState*)
{
    const int clientId = NetGame::GetCurDst();
    if (clientId == 1)
    {
        ++gDebugCount;
    }
    const PblMatrix& matrix = GetMatrix();
    if (!NetGame::IsFullState())
    {
        SetNetLOD(ComputeNetLOD((int)&matrix.m[3][0], mPlayerId) != 0);
        SetNetPrecision(group);
    }
    WriteMatrix(group, matrix);
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    ((EnergyBar*)mEnergyBar)->Write(group);
    MarkWriteAlign(group);
    group.WriteBits(mDirection, 2);
    group.WriteBits(mState, 4);
    mBlend->Write(group);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->mBlend->Write(group);
    }
    WriteAngle(group, mTotalTurn);
    WriteCosSin(group, (const float*)&mTurnOffset);
    WriteCosSin(group, (const float*)&mPitchOffset);
    WriteCosSin(group, (const float*)&mRollOffset);
    MarkWriteAlign(group);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->Write(group);
    }
    MarkWriteAlign(group);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive ? 1u : 0u, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded ? 1u : 0u, 1);
    }
    group.WriteBits(mIsDead ? 1u : 0u, 1);
    group.WriteBits(reinterpret_cast<const unsigned int&>(mPitchValue), 32);
    group.WriteBits(reinterpret_cast<const unsigned int&>(mYawValue), 32);
    group.WriteBits(reinterpret_cast<const unsigned int&>(mLegAngle), 32);
    WritePilots(group, *this);
    if (mPlayerId >= 0)
    {
        WriteThrust(group, mTurnBuildup);
        group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
        GameObject* target = mTargetLockedObj;
        bool hasTarget = false;
        if (target)
        {
            hasTarget = IsNeverDies(target->GetEntityType()) ||
                        GetUniqObjectId(target) != 63;
        }
        group.WriteBits(hasTarget ? 1u : 0u, 1);
        if (hasTarget)
        {
            WriteObjectPtr(group, mTargetLockedObj);
        }
        if (NetGame::IsFullState() || NetGame::IsLocalTo(clientId, mPlayerId))
        {
            WriteTrigger(group, &mControlSprint);
            WriteTrigger(group, &mControlThrustFwd);
            if (mWeapon[0])
            {
                WriteWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                WriteWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Write(group);
            }
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Write(group);
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005ecca0
void EntityWalker::ReadObject(NetPktGroup& group, NetObjState*)
{
    if (!NetGame::IsFullState())
    {
        ReadNetLOD(group);
    }
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    mVelocityNoGrav = mVelocity;
    mVelocityNoGrav.y = 0.0f;
    ((EnergyBar*)mEnergyBar)->Read(group);
    CheckReadAlign(group);
    unsigned int direction = 0, state = 0;
    group.ReadBits(direction, 2);
    mDirection = (eDirection)direction;
    group.ReadBits(state, 4);
    const bool reversed = mDirection == (eDirection)2;
    if ((int)state != mState)
    {
        const float blendDuration = mClass->GetAnimationBlendDuration((eState)mState, (eState)state);
        mState = state;
        const int animIndex = state == 4 ? 3 : state;
        mClass->mAnimations[animIndex]->Setup(*mBlend, 0.0f,
                                               blendDuration, false, reversed);
        if (state == 4)
        {
            mBlend->Update(mBlend->mTimeTotal);
            mBlend->Skip(group);
        }
        else
        {
            mBlend->Read(group);
        }
        for (int i = 0; i < mLegPairCount; ++i)
        {
            WalkerBlend& legBlend = *mLegPairs[i]->mBlend;
            mClass->mAnimations[animIndex]->Setup(legBlend, 0.0f,
                                                   0.0f, false, reversed);
            if (state == 4)
            {
                legBlend.Update(legBlend.mTimeTotal);
                legBlend.Skip(group);
            }
            else
            {
                legBlend.Read(group);
            }
        }
    }
    else
    {
        const int animIndex = mState == 4 ? 3 : mState;
        mClass->mAnimations[animIndex]->Setup(*mBlend, 0.0f, 0.0f,
                                               false, reversed);
        mBlend->Read(group);
        for (int i = 0; i < mLegPairCount; ++i)
        {
            WalkerBlend& legBlend = *mLegPairs[i]->mBlend;
            mClass->mAnimations[animIndex]->Setup(legBlend, 0.0f, 0.0f,
                                                   false, reversed);
            legBlend.Read(group);
        }
    }
    ReadAngle(group, mTotalTurn);
    ReadCosSin(group, (float*)&mTurnOffset);
    ReadCosSin(group, (float*)&mPitchOffset);
    ReadCosSin(group, (float*)&mRollOffset);
    CheckReadAlign(group);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->Read(group);
    }
    BuildWorldMatrices();
    CheckReadAlign(group);
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    unsigned int value = 0;
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    group.ReadBits(value, 1);
    const bool dead = value != 0;
    if (dead != (mIsDead != 0))
    {
        if (mIsDead)
        {
            CollisionManager::RemoveBody(this);
            CollisionManager::AddRigidBody(this);
            for (int i = 0; i < mClass->mWeaponCount; ++i)
            {
                if (mWeapon[i])
                {
                    mWeapon[i]->ActivateThread(0, PER_FRAME);
                }
            }
        }
        else
        {
            Die();
        }
        mIsDead = dead;
    }
    group.ReadBits(*(unsigned int*)&mPitchValue, 32);
    group.ReadBits(*(unsigned int*)&mYawValue, 32);
    group.ReadBits(*(unsigned int*)&mLegAngle, 32);
    ReadPilots(group, *this);
    if (mPlayerId >= 0)
    {
        ReadThrust(group, mTurnBuildup);
        group.ReadBits(value, 1);
        mTargetInfo.mIsAiming = value != 0;
        group.ReadBits(value, 1);
        SetTargetLockedObj(value ? ReadObjectPtr(group) : 0);
        if (NetGame::IsFullState() || NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            ReadTrigger(group, &mControlSprint);
            ReadTrigger(group, &mControlThrustFwd);
            if (mWeapon[0])
            {
                ReadWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                ReadWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Read(group);
            }
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Read(group);
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005ee870
void EntityDroid::AddSends(void (*send)(GameObject*, int), int depth)
{
    if (mPilot)
    {
        Trackable* pTrackable = static_cast<Trackable*>(mPilot);
        send(pTrackable->GetGameObject(), depth);
    }
    AddSendPilots(send, *this, depth);
}

// FUNCTION: SWBF2 0x005ee8b0
void EntityDroid::WriteObject(NetPktGroup& group, NetObjState*)
{
    const int clientId = NetGame::GetCurDst();
    const PblMatrix& matrix = GetMatrix();
    if (!NetGame::IsFullState())
    {
        SetNetLOD(ComputeNetLOD((int)&matrix.m[3][0], mPlayerId) != 0);
        SetNetPrecision(group);
    }
    WriteMatrix(group, matrix);
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    WriteAngle(group, mPitchValue);
    group.WriteBits(IsThreadActive() ? 1u : 0u, 1);
    group.WriteBits(mTeam, 2);
    group.WriteBits(mPerceivedTeam, 2);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive ? 1u : 0u, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded ? 1u : 0u, 1);
    }
    WritePilots(group, *this);
    if (mPlayerId >= 0)
    {
        WriteThrust(group, mTurnBuildup);
        group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
        if (NetGame::IsFullState() || NetGame::IsLocalTo(clientId, mPlayerId))
        {
            if (mWeapon[0])
            {
                WriteWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                WriteWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Write(group);
            }
        }
    }
    ((EnergyBar*)mEnergyBar)->Write(group);
    group.WriteBits(mState, 2);
    group.WriteFixed(mKillTimer, 0.0f, 5.0f, 8);
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005eeaf0
void EntityDroid::ReadObject(NetPktGroup& group, NetObjState*)
{
    if (!NetGame::IsFullState())
    {
        ReadNetLOD(group);
    }
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    ReadAngle(group, mPitchValue);
    unsigned int value = 0;
    group.ReadBits(value, 1);
    if (value && !IsThreadActive())
    {
        ActivatePhysics();
    }
    else if (!value && IsThreadActive())
    {
        DeactivatePhysics();
    }
    group.ReadBits(value, 2);
    SetTeam(value);
    group.ReadBits(value, 2);
    mPerceivedTeam = value;
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    group.ReadFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    ReadPilots(group, *this);
    if (mPlayerId >= 0)
    {
        ReadThrust(group, mTurnBuildup);
        group.ReadBits(value, 1);
        mTargetInfo.mIsAiming = value != 0;
        if (NetGame::IsFullState() || NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            if (mWeapon[0])
            {
                ReadWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                ReadWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Read(group);
            }
        }
    }
    ((EnergyBar*)mEnergyBar)->Read(group);
    group.ReadBits(value, 2);
    mState = (State)value;
    group.ReadFixed(mKillTimer, 0.0f, 5.0f, 8);
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005eeef0
void EntityHover::AddSends(void (*send)(GameObject*, int), int depth)
{
    if (mPilot)
    {
        Trackable* pTrackable = static_cast<Trackable*>(mPilot);
        send(pTrackable->GetGameObject(), depth);
    }
    AddSendPilots(send, *this, depth);
}

// FUNCTION: SWBF2 0x005eef30
void EntityHover::WriteObject(NetPktGroup& group, NetObjState*)
{
    const int clientId = NetGame::GetCurDst();
    const PblMatrix& matrix = GetMatrix();
    if (!NetGame::IsFullState())
    {
        SetNetLOD(ComputeNetLOD((int)&matrix.m[3][0], mPlayerId) != 0);
        SetNetPrecision(group);
    }
    WriteMatrix(group, matrix);
    group.WriteBits(mBoost ? 1u : 0u, 1);
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    WriteVelocity(group, mOmega, 5120.0f, -1);
    ((EnergyBar*)mEnergyBar)->Write(group);
    WriteAngle(group, mBankPitchAngle);
    WriteAngle(group, mBankRollAngle);
    WriteAngle(group, mPitchValue);
    WriteAngle(group, mYawValue);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    if (mClass->mJumpForce > 0.0f)
    {
        group.WriteFixed(mJumpTimer, 0.0f, mClass->mJumpTimeMax, 11);
        group.WriteBits(mJumpMode ? 1u : 0u, 1);
    }
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive ? 1u : 0u, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded ? 1u : 0u, 1);
    }
    WritePilots(group, *this);
    if (mPlayerId >= 0)
    {
        WriteThrust(group, mTurnBuildup);
        group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
        GameObject* target = mTargetLockedObj;
        bool hasTarget = false;
        if (target)
        {
            hasTarget = IsNeverDies(target->GetEntityType()) ||
                        GetUniqObjectId(target) != 63;
        }
        group.WriteBits(hasTarget ? 1u : 0u, 1);
        if (hasTarget)
        {
            WriteObjectPtr(group, mTargetLockedObj);
        }
        if (NetGame::IsFullState() || NetGame::IsLocalTo(clientId, mPlayerId))
        {
            WriteTrigger(group, &mControlSprint);
            WriteTrigger(group, &mControlThrustFwd);
            if (mWeapon[0])
            {
                WriteWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                WriteWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Write(group);
            }
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Write(group);
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005ef2f0
void EntityHover::ReadObject(NetPktGroup& group, NetObjState*)
{
    if (!NetGame::IsFullState())
    {
        ReadNetLOD(group);
    }
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    unsigned int value = 0;
    group.ReadBits(value, 1);
    mBoost = value != 0;
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    ReadVelocity(group, mOmega, 5120.0f, -1);
    ((EnergyBar*)mEnergyBar)->Read(group);
    ReadAngle(group, mBankPitchAngle);
    ReadAngle(group, mBankRollAngle);
    ReadAngle(group, mPitchValue);
    ReadAngle(group, mYawValue);
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    if (mClass->mJumpForce > 0.0f)
    {
        group.ReadFixed(mJumpTimer, 0.0f, mClass->mJumpTimeMax, 11);
        group.ReadBits(value, 1);
        mJumpMode = value != 0;
    }
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    ReadPilots(group, *this);
    if (mPlayerId >= 0)
    {
        ReadThrust(group, mTurnBuildup);
        group.ReadBits(value, 1);
        mTargetInfo.mIsAiming = value != 0;
        group.ReadBits(value, 1);
        SetTargetLockedObj(value ? ReadObjectPtr(group) : 0);
        if (NetGame::IsFullState() || NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            ReadTrigger(group, &mControlSprint);
            ReadTrigger(group, &mControlThrustFwd);
            if (mWeapon[0])
            {
                ReadWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                ReadWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Read(group);
            }
        }
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Read(group);
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005f51d0
void EntityDroid::WriteLateState(NetPktGroup& group)
{
    WriteMatrix(group, GetMatrix());
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    WriteAngle(group, mPitchValue);
    group.WriteBits(IsThreadActive() ? 1u : 0u, 1);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    WriteThrust(group, mTurnBuildup);
    group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
    group.WriteBits(mState, 2);
    group.WriteFixed(mKillTimer, 0.0f, 5.0f, 8);
}

// FUNCTION: SWBF2 0x005f52d0
void EntityDroid::ReadLateState(NetPktGroup& group)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    ReadAngle(group, mPitchValue);
    unsigned int active = 0;
    group.ReadBits(active, 1);
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    ReadThrust(group, mTurnBuildup);
    unsigned int aiming = 0;
    group.ReadBits(aiming, 1);
    mTargetInfo.mIsAiming = aiming != 0;
    unsigned int state = 0;
    group.ReadBits(state, 2);
    mState = (State)state;
    group.ReadFixed(mKillTimer, 0.0f, 5.0f, 8);
}

// FUNCTION: SWBF2 0x005f3de0
void EntityWalker::WriteLateState(NetPktGroup& group)
{
    WriteMatrix(group, GetMatrix());
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    group.WriteBits(mDirection, 2);
    group.WriteBits(mState, 4);
    mBlend->Write(group);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->mBlend->Write(group);
    }
    WriteAngle(group, mTotalTurn);
    WriteCosSin(group, (const float*)&mTurnOffset);
    WriteCosSin(group, (const float*)&mPitchOffset);
    WriteCosSin(group, (const float*)&mRollOffset);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->Write(group);
    }
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    group.WriteBits(reinterpret_cast<const unsigned int&>(mPitchValue), 32);
    group.WriteBits(reinterpret_cast<const unsigned int&>(mYawValue), 32);
    group.WriteBits(reinterpret_cast<const unsigned int&>(mLegAngle), 32);
    WriteThrust(group, mTurnBuildup);
    group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
    for (int i = 0; i < mNumTurrets; ++i)
    {
        ((MountedTurret&)(*mTurret[i])).WriteLateState(group);
    }
}

// FUNCTION: SWBF2 0x005f3fb0
void EntityWalker::ReadLateState(NetPktGroup& group)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    unsigned int direction = 0, state = 0;
    group.ReadBits(direction, 2);
    mDirection = (eDirection)direction;
    group.ReadBits(state, 4);
    const bool reversed = mDirection == (eDirection)2;
    if ((int)state != mState)
    {
        const float blendDuration = mClass->GetAnimationBlendDuration((eState)mState, (eState)state);
        mState = state;
        const int animIndex = state == 4 ? 3 : state;
        mClass->mAnimations[animIndex]->Setup(*mBlend, 0.0f,
                                               blendDuration, false, reversed);
        if (state == 4)
        {
            mBlend->Update(mBlend->mTimeTotal);
            mBlend->Skip(group);
        }
        else
        {
            mBlend->Read(group);
        }
        for (int i = 0; i < mLegPairCount; ++i)
        {
            WalkerBlend& legBlend = *mLegPairs[i]->mBlend;
            mClass->mAnimations[animIndex]->Setup(legBlend, 0.0f,
                                                   0.0f, false, reversed);
            if (state == 4)
            {
                legBlend.Update(legBlend.mTimeTotal);
                legBlend.Skip(group);
            }
            else
            {
                legBlend.Read(group);
            }
        }
    }
    else
    {
        const int animIndex = mState == 4 ? 3 : mState;
        mClass->mAnimations[animIndex]->Setup(*mBlend, 0.0f, 0.0f,
                                               false, reversed);
        mBlend->Read(group);
        for (int i = 0; i < mLegPairCount; ++i)
        {
            WalkerBlend& legBlend = *mLegPairs[i]->mBlend;
            mClass->mAnimations[animIndex]->Setup(legBlend, 0.0f, 0.0f,
                                                   false, reversed);
            legBlend.Read(group);
        }
    }
    ReadAngle(group, mTotalTurn);
    ReadCosSin(group, (float*)&mTurnOffset);
    ReadCosSin(group, (float*)&mPitchOffset);
    ReadCosSin(group, (float*)&mRollOffset);
    for (int i = 0; i < mLegPairCount; ++i)
    {
        mLegPairs[i]->Read(group);
    }
    BuildWorldMatrices();
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    group.ReadBits(*(unsigned int*)&mPitchValue, 32);
    group.ReadBits(*(unsigned int*)&mYawValue, 32);
    group.ReadBits(*(unsigned int*)&mLegAngle, 32);
    ReadThrust(group, mTurnBuildup);
    unsigned int value = 0;
    group.ReadBits(value, 1);
    mTargetInfo.mIsAiming = value != 0;
    for (int i = 0; i < mNumTurrets; ++i)
    {
        ((MountedTurret&)(*mTurret[i])).ReadLateState(group);
    }
}

// FUNCTION: SWBF2 0x005f5410
void MountedTurret::WriteLateState(NetPktGroup& group)
{
    const MountedTurret& turret = *(const MountedTurret*)this;
    WriteThrust(group, turret.mControlPitch);
    WriteThrust(group, turret.mControlTurn);
    const PblVector3 aim(turret.mAimMatrix.m[2][0], turret.mAimMatrix.m[2][1],
                         turret.mAimMatrix.m[2][2]);
    WriteUnitVector(group, aim);
    WriteAngle(group, turret.mPitch);
    group.WriteFixed(turret.mOmegaPitch, -turret.mClass->mPitchRate,
                     turret.mClass->mPitchRate, 16);
    WriteAngle(group, turret.mYaw);
    group.WriteFixed(turret.mOmegaYaw, -turret.mClass->mTurnRate,
                     turret.mClass->mTurnRate, 16);
    WriteThrust(group, turret.mTurnBuildup);
    group.WriteBits(turret.mTargetInfo.mIsAiming ? 1u : 0u, 1);
    group.WriteFixed(turret.mTurnAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(turret.mPitchAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(turret.mTurnAuto, -1.0f, 1.0f, 16);
    group.WriteFixed(turret.mPitchAuto, -1.0f, 1.0f, 16);
    group.WriteBits(turret.mUsingTurnAdjusted ? 1u : 0u, 1);
}

// FUNCTION: SWBF2 0x005ef7d0
void MountedTurret::Write(NetPktGroup& group)
{
    const int clientId = NetGame::GetCurDst();
    const int oldPrecision = netLOD;
    netLOD = NetGame::IsLocalTo(clientId, mPlayerId) ? 1 : oldPrecision;
    WriteThrust(group, mControlPitch);
    WriteThrust(group, mControlTurn);
    const PblVector3 aim(mAimMatrix.m[2][0], mAimMatrix.m[2][1],
                         mAimMatrix.m[2][2]);
    WriteUnitVector(group, aim);
    WriteAngle(group, mPitch);
    group.WriteFixed(mOmegaPitch, -mClass->mPitchRate,
                     mClass->mPitchRate, 16);
    WriteAngle(group, mYaw);
    group.WriteFixed(mOmegaYaw, -mClass->mTurnRate,
                     mClass->mTurnRate, 16);
    if (mPlayerId < 0)
    {
        netLOD = oldPrecision;
        return;
    }
    WriteThrust(group, mTurnBuildup);
        group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
        if (NetGame::IsFullState() || NetGame::IsLocalTo(clientId, mPlayerId))
        {
            if (mWeapon[0])
            {
                WriteWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                WriteWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Write(group);
            }
        }
    group.WriteFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(mTurnAuto, -1.0f, 1.0f, 16);
    group.WriteFixed(mPitchAuto, -1.0f, 1.0f, 16);
    group.WriteBits(mUsingTurnAdjusted ? 1u : 0u, 1);
    GameObject* target = mTargetLockedObj;
    bool hasTarget = false;
    if (target)
    {
        hasTarget = IsNeverDies(target->GetEntityType()) ||
                    GetUniqObjectId(target) != 63;
    }
    group.WriteBits(hasTarget ? 1u : 0u, 1);
    if (hasTarget)
    {
        WriteObjectPtr(group, mTargetLockedObj);
    }
    netLOD = oldPrecision;
}

// FUNCTION: SWBF2 0x005efaf0
void MountedTurret::Read(NetPktGroup& group)
{
    const int oldPrecision = netLOD;
    netLOD = NetGame::GetJoystickIndex(mPlayerId) >= 0 ? 1 : oldPrecision;
    ReadThrust(group, mControlPitch);
    ReadThrust(group, mControlTurn);
    PblVector3 direction;
    ReadUnitVector(group, direction);
    ReadAngle(group, mPitch);
    group.ReadFixed(mOmegaPitch, -mClass->mPitchRate, mClass->mPitchRate, 16);
    ReadAngle(group, mYaw);
    group.ReadFixed(mOmegaYaw, -mClass->mTurnRate, mClass->mTurnRate, 16);

    const PblMatrix& parentMatrix = mParent->GetMatrix();
    PblVector3 mountPos = mClass->mMountPos;
    PblVector3 mountUp = mClass->mMountUp;
    if (mPose && mClass->mParentHash)
    {
        const PblMatrix* bone = mPose->Find(mClass->mParentHash);
        if (bone)
        {
            PblVector3 transformed;
            D3DXVec3TransformCoord(&transformed, &mountPos, bone);
            mountPos = transformed;
            D3DXVec3TransformNormal(&transformed, &mountUp, bone);
            mountUp = transformed;
        }
    }
    PblVector3 worldPos, worldUp;
    D3DXVec3TransformCoord(&worldPos, &mountPos, &parentMatrix);
    D3DXVec3TransformNormal(&worldUp, &mountUp, &parentMatrix);
    mAimMatrix = DirectionMatrix(direction, worldUp, worldPos);

    if (mPlayerId >= 0)
    {
        ReadThrust(group, mTurnBuildup);
        unsigned int value = 0;
        group.ReadBits(value, 1);
        mTargetInfo.mIsAiming = value != 0;
        if (NetGame::IsFullState() || NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            if (mWeapon[0])
            {
                ReadWeapon(group, mWeapon[0]);
            }
            if (mWeapon[1])
            {
                ReadWeapon(group, mWeapon[1]);
            }
            if (mCharacter)
            {
                mCharacter->Read(group);
            }
        }
        group.ReadFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
        group.ReadFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
        group.ReadFixed(mTurnAuto, -1.0f, 1.0f, 16);
        group.ReadFixed(mPitchAuto, -1.0f, 1.0f, 16);
        group.ReadBits(value, 1);
        mUsingTurnAdjusted = value != 0;
        group.ReadBits(value, 1);
        SetTargetLockedObj(value ? ReadObjectPtr(group) : 0);
    }
    if (NetGame::GetJoystickIndex(mPlayerId) >= 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (mAimer[i])
            {
                mAimer[i]->Read(group);
            }
        }
    }
    netLOD = oldPrecision;
}

// FUNCTION: SWBF2 0x005f0d50
void WritePilots(NetPktGroup& group, GameObject& owner)
{
    for (PblList<Controllable>::Itor it = owner.mControllableList.Begin();
         it != owner.mControllableList.End(); ++it)
    {
        Controllable* pControl = it;
        if (pControl->mPilotType < PILOT_VEHICLE)
        {
            continue;
        }
        Controllable* pPilot = pControl->mPilot;
        if (pControl->mPilotType == PILOT_VEHICLESELF && pPilot == pControl)
        {
            pPilot = 0;
        }
        Character* pCharacter = 0;
        if (pPilot)
        {
            Trackable* pTrackable = static_cast<Trackable*>(pPilot);
            pTrackable->GetGameObject();
            pCharacter = pPilot->mCharacter;
        }
        WriteCharacter(group, pCharacter);
    }
}

// FUNCTION: SWBF2 0x005f0de0
void ReadPilots(NetPktGroup& group, GameObject& owner)
{
    for (PblList<Controllable>::Itor it = owner.mControllableList.Begin();
         it != owner.mControllableList.End(); ++it)
    {
        Controllable* control = it;
        if (control->mPilotType < PILOT_VEHICLE)
        {
            continue;
        }

        Controllable* oldPilot = control->mPilot;
        if (control->mPilotType == PILOT_VEHICLESELF && oldPilot == control)
        {
            oldPilot = 0;
        }
        Character* character = ReadCharacter(group);
        Controllable* newPilot = character ? character->mUnit : 0;
        if (oldPilot == newPilot)
        {
            continue;
        }

        if (oldPilot)
        {
            Character* oldCharacter = oldPilot->mCharacter;
            if (oldCharacter)
            {
                if (oldCharacter->mRemote)
                {
                    Trackable* trackable = static_cast<Trackable*>(oldCharacter->mRemote);
                    if (trackable->GetGameObject() == &owner)
                    {
                        oldCharacter->SetRemote(0);
                    }
                }
                if (oldCharacter->mVehicle)
                {
                    Trackable* trackable = static_cast<Trackable*>(oldCharacter->mVehicle);
                    if (trackable->GetGameObject() == &owner)
                    {
                        oldCharacter->SetVehicle(0);
                    }
                }
            }
            control->SetPilot(0);
        }
        if (!character)
        {
            continue;
        }

        if (newPilot)
        {
            if (character->mRemote && character->mRemote->GetPilot() == newPilot)
            {
                character->mRemote->SetPilot(0);
                character->SetRemote(0);
            }
            if (character->mVehicle && character->mVehicle->GetPilot() == newPilot)
            {
                character->mVehicle->SetPilot(0);
                character->SetVehicle(0);
            }
        }
        control->SetPilot(newPilot);
        if (control->mPilotType == PILOT_REMOTE)
        {
            character->SetRemote(control);
        }
        else if (control->mPilotType == PILOT_VEHICLE)
        {
            character->SetVehicle(control);
            if (newPilot)
            {
                Entity* entity = newPilot->GetEntity();
                if (entity && entity->IsRtti(EntitySoldier::rttiHashEntitySoldier))
                {
                    static_cast<EntitySoldier*>(entity)->TurnOffJetEffect();
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005f5580
void MountedTurret::ReadLateState(NetPktGroup& group)
{
    MountedTurret& turret = *(MountedTurret*)this;
    ReadThrust(group, turret.mControlPitch);
    ReadThrust(group, turret.mControlTurn);
    PblVector3 direction;
    ReadUnitVector(group, direction);
    ReadAngle(group, turret.mPitch);
    group.ReadFixed(turret.mOmegaPitch, -turret.mClass->mPitchRate,
                    turret.mClass->mPitchRate, 16);
    ReadAngle(group, turret.mYaw);
    group.ReadFixed(turret.mOmegaYaw, -turret.mClass->mTurnRate,
                    turret.mClass->mTurnRate, 16);

    const PblMatrix& parentMatrix = turret.mParent->GetMatrix();
    PblVector3 mountPos = turret.mClass->mMountPos;
    PblVector3 mountUp = turret.mClass->mMountUp;
    if (turret.mPose && turret.mClass->mParentHash)
    {
        const PblMatrix* bone = turret.mPose->Find(turret.mClass->mParentHash);
        if (bone)
        {
            PblVector3 transformed;
            D3DXVec3TransformCoord(&transformed, &mountPos, bone);
            mountPos = transformed;
            D3DXVec3TransformNormal(&transformed, &mountUp, bone);
            mountUp = transformed;
        }
    }
    PblVector3 worldPos;
    PblVector3 worldUp;
    D3DXVec3TransformCoord(&worldPos, &mountPos, &parentMatrix);
    D3DXVec3TransformNormal(&worldUp, &mountUp, &parentMatrix);
    turret.mAimMatrix = DirectionMatrix(direction, worldUp, worldPos);

    ReadThrust(group, turret.mTurnBuildup);
    unsigned int aiming = 0;
    group.ReadBits(aiming, 1);
    turret.mTargetInfo.mIsAiming = aiming != 0;
    group.ReadFixed(turret.mTurnAdjusted, -1.0f, 2.0f, 16);
    group.ReadFixed(turret.mPitchAdjusted, -1.0f, 2.0f, 16);
    group.ReadFixed(turret.mTurnAuto, -1.0f, 1.0f, 16);
    group.ReadFixed(turret.mPitchAuto, -1.0f, 1.0f, 16);
    unsigned int adjusted = 0;
    group.ReadBits(adjusted, 1);
    turret.mUsingTurnAdjusted = adjusted != 0;
}

// FUNCTION: SWBF2 0x005f4490
void EntityHover::WriteLateState(NetPktGroup& group)
{
    WriteMatrix(group, GetMatrix());
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    WriteVelocity(group, mOmega, 5120.0f, -1);
    WriteAngle(group, mBankPitchAngle);
    WriteAngle(group, mBankRollAngle);
    WriteAngle(group, mPitchValue);
    WriteAngle(group, mYawValue);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    if (mClass->mJumpForce > 0.0f)
    {
        group.WriteFixed(mJumpTimer, 0.0f, mClass->mJumpTimeMax, 11);
        group.WriteBits(mJumpMode ? 1u : 0u, 1);
    }
    WriteThrust(group, mTurnBuildup);
    group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
    for (int i = 0; i < mNumTurrets; ++i)
    {
        ((MountedTurret&)(*mTurret[i])).WriteLateState(group);
    }
}

// FUNCTION: SWBF2 0x005f45f0
void EntityHover::ReadLateState(NetPktGroup& group)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    ReadVelocity(group, mOmega, 5120.0f, -1);
    ReadAngle(group, mBankPitchAngle);
    ReadAngle(group, mBankRollAngle);
    ReadAngle(group, mPitchValue);
    ReadAngle(group, mYawValue);
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    if (mClass->mJumpForce > 0.0f)
    {
        group.ReadFixed(mJumpTimer, 0.0f, mClass->mJumpTimeMax, 11);
        unsigned int jump = 0;
        group.ReadBits(jump, 1);
        mJumpMode = jump != 0;
    }
    ReadThrust(group, mTurnBuildup);
    unsigned int aiming = 0;
    group.ReadBits(aiming, 1);
    mTargetInfo.mIsAiming = aiming != 0;
    for (int i = 0; i < mNumTurrets; ++i)
    {
        ((MountedTurret&)(*mTurret[i])).ReadLateState(group);
    }
}

void ReadQuaternion(NetPktGroup& group, PblQuaternion& value);
void WriteQuaternion(NetPktGroup& group, const PblQuaternion& value);

// FUNCTION: SWBF2 0x005f5920
void WriteUnitVector(NetPktGroup& group, const PblVector3& value)
{
    WriteFixed(unitConvert, group, value.x);
    WriteFixed(unitConvert, group, value.y);
    WriteFixed(unitConvert, group, value.z);
}

// FUNCTION: SWBF2 0x005f5950
void ReadUnitVector(NetPktGroup& group, PblVector3& value)
{
    ReadFixed(unitConvert, group, value.x);
    ReadFixed(unitConvert, group, value.y);
    ReadFixed(unitConvert, group, value.z);
}

// FUNCTION: SWBF2 0x005f59b0
void WritePositionVector(NetPktGroup& group, const PblVector3& value)
{
    WriteFixed(positionXZConvert, group, value.x);
    WriteFixed(positionYConvert, group, value.y);
    WriteFixed(positionXZConvert, group, value.z);
}

// FUNCTION: SWBF2 0x005f59e0
void ReadPositionVector(NetPktGroup& group, PblVector3& value)
{
    ReadFixed(positionXZConvert, group, value.x);
    ReadFixed(positionYConvert, group, value.y);
    ReadFixed(positionXZConvert, group, value.z);
}

// FUNCTION: SWBF2 0x005f5a40
void WriteMatrix(NetPktGroup& group, const PblMatrix& value)
{
    PblQuaternion q(value);
    WriteQuaternion(group, q);
    const PblVector3 position(value.m[3][0], value.m[3][1], value.m[3][2]);
    WritePositionVector(group, position);
}

// FUNCTION: SWBF2 0x005f5a70
void ReadMatrix(NetPktGroup& group, PblMatrix& matrix)
{
    PblQuaternion q;
    ReadQuaternion(group, q);
    PblVector3 position;
    ReadPositionVector(group, position);
    PblQuaternion unit;
    PblMatrix rotation;
    D3DXQuaternionNormalize(&unit, &q);
    D3DXMatrixRotationQuaternion(&rotation, &unit);
    matrix = rotation;
    matrix.m[3][0] = position.x;
    matrix.m[3][1] = position.y;
    matrix.m[3][2] = position.z;
    matrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x005f5b30
void WriteQuaternion(NetPktGroup& group, const PblQuaternion& value)
{
    WriteFixed(quatConvert, group, value._fX);
    WriteFixed(quatConvert, group, value._fY);
    WriteFixed(quatConvert, group, value._fZ);
    WriteFixed(quatConvert, group, value._fS);
}

// FUNCTION: SWBF2 0x005f5b70
void ReadQuaternion(NetPktGroup& group, PblQuaternion& value)
{
    ReadFixed(quatConvert, group, value._fX);
    ReadFixed(quatConvert, group, value._fY);
    ReadFixed(quatConvert, group, value._fZ);
    ReadFixed(quatConvert, group, value._fS);
}

// FUNCTION: SWBF2 0x005f5c90
void WriteVelocity(NetPktGroup& group, const PblVector3& value, float fMaxSpeed,
                int iBits)
{
    const float length = sqrtf(value.x * value.x + value.y * value.y + value.z * value.z);
    const bool present = length > 0.0001f;
    group.WriteBits(present ? 1u : 0u, 1);
    if (!present)
    {
        return;
    }
    if (iBits == -1)
    {
        iBits = 1;
        const int units = (int)(fMaxSpeed * 20.0f);
        while (iBits < 32 && (1 << (iBits - 1)) < units)
        {
            ++iBits;
        }
    }
    PblVector3 direction(value.x / length, value.y / length, value.z / length);
    WriteUnitVector(group, direction);
    group.WriteFixed(length, 0.0f, fMaxSpeed, iBits);
}

// FUNCTION: SWBF2 0x005f5da0
void ReadVelocity(NetPktGroup& group, PblVector3& value, float fMaxSpeed, int iBits)
{
    unsigned int present = 0;
    group.ReadBits(present, 1);
    if (!present)
    {
        value = PblVector3(0.0f, 0.0f, 0.0f);
        return;
    }
    if (iBits == -1)
    {
        iBits = 1;
        const int units = (int)(fMaxSpeed * 20.0f);
        while (iBits < 32 && (1 << (iBits - 1)) < units)
        {
            ++iBits;
        }
    }
    PblVector3 direction;
    ReadUnitVector(group, direction);
    float length;
    group.ReadFixed(length, 0.0f, fMaxSpeed, iBits);
    value = PblVector3(direction.x * length, direction.y * length, direction.z * length);
}

// FUNCTION: SWBF2 0x005f5f00
void WriteCharacter(NetPktGroup& group, Character* pCharacter)
{
    int index = 0;
    unsigned int kind = 0;
    int bits = 0;
    if (pCharacter)
    {
        Character* pBase = Character::sCharacters && Character::sCharacterLimit > 0
            ? Character::sCharacters : 0;
        index = (int)(pCharacter - pBase);
        if (index < 16) { kind = 1; bits = 4; }
        else if (index < 64) { kind = 2; bits = 6; }
        else { kind = 3; bits = 8; }
    }
    group.WriteBits(kind, 2);
    if (kind)
    {
        group.WriteBits((unsigned int)index, bits);
    }
}

// FUNCTION: SWBF2 0x005f5f90
Character* ReadCharacter(NetPktGroup& group)
{
    unsigned int kind = 0;
    group.ReadBits(kind, 2);
    if (!kind)
    {
        return 0;
    }
    const int bits = kind == 1 ? 4 : (kind == 2 ? 6 : 8);
    unsigned int index = 0;
    group.ReadBits(index, bits);
    if (!Character::sCharacters || index >= (unsigned int)Character::sCharacterLimit)
    {
        return 0;
    }
    return Character::sCharacters + index;
}

// FUNCTION: SWBF2 0x005f0220
void PowerupItem::WriteObject(NetPktGroup& group, NetObjState*)
{
    WriteMatrix(group, GetMatrix());
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    group.WriteFixed(mLifeTimer, -1.0f, mClass->mLifeSpan, 8);
    group.WriteBits(mStationary ? 1u : 0u, 1);
}

// FUNCTION: SWBF2 0x005f02a0
void PowerupItem::ReadObject(NetPktGroup& group, NetObjState*)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    SetMatrix(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    RedSceneObject* pScene = this;
    pScene->Deactivate();
    pScene->Activate();
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    group.ReadFixed(mLifeTimer, -1.0f, mClass->mLifeSpan, 8);
    unsigned int stationary = 0;
    group.ReadBits(stationary, 1);
    mStationary = stationary != 0;
}

// FUNCTION: SWBF2 0x005eb1c0
void EntitySoldier::ResetObject()
{
    mVelocity = PblVector3(0.0f, 0.0f, 0.0f);
    mFlags |= flags_footprintLeft | flags_isGrounded;
    mGroundNormal = PblVector3(0.0f, 1.0f, 0.0f);
    mPitchValue = 0.0f;
    mPitchOffset = 0.0f;
    mPitchVelocity = 0.0f;
    mKickScale = 0.0f;
    mKickBuild = 0.0f;
    ClearControllable();
    mStanceIndex = 0;
    mState = SoldierState::STAND;
    mOldState = SoldierState::STAND;
    mStateTimer = 0.0f;
    mSoldierAnimator = 0;
    mEyePoint = (&mClass->mCameraSettings[0][0])[mStanceIndex * 2 + mTargetInfo.mIsAiming].CalcEyePoint(mMatrix);
    mEyeDir = mMatrix.GetZAxis();
    mSoundState = 0;
    mFlags = 0;
    mSpeedPrev = 0.0f;
    mDistanceMoved = 0.0f;
    mInputLockTime = 0.0f;
    mLeftFootElevation = 0.0f;
    mRightFootElevation = 0.0f;
    mDrownTime = mClass->mDrownTime;
    mFlyingInFirstPerson = 0;
    mAILastCollisionNormal = PblVector3(0.0f, 0.0f, 0.0f);
    mFoliageCollisionTime = 0.0f;
    mForceFrameNum1 = -1.0f;
    mForceFrameNum2 = -1.0f;
    mForceFrameBlend = -1.0f;
}

// FUNCTION: SWBF2 0x005e9360
void EntityBuilding::WriteObject(NetPktGroup& group, NetObjState*)
{
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
    group.WriteBits(mIsShielded, 1);
    group.WriteBits(mState, 3);
}

// FUNCTION: SWBF2 0x005e93d0
void EntityBuilding::ReadObject(NetPktGroup& group, NetObjState*)
{
    float fHealth;
    group.ReadFixed(fHealth, 0.0f, mMaxHealth, 11);
    group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
    unsigned int value;
    group.ReadBits(value, 1);
    mIsShielded = value;
    group.ReadBits(value, 3);
    const int state = (int)value;
    if (!(0.0f >= fHealth) && state != 1)
    {
        SetCurHealth(fHealth, true, 0);
    }
    else
    {
        SetCurHealth(0.0f, true, 0);
        EntityBuilding::Kill();
        mCurHealth = fHealth;
    }
    if (mState != 0 && state == 0 && fHealth >= mMaxHealth)
    {
        EntityBuilding::Respawn();
        Damageable::HealthChangeCallback(false, 0.0f);
    }
    mState = (STATE)state;
}

// FUNCTION: SWBF2 0x005f07b0
void EntityTrap::WriteObject(NetPktGroup& group, NetObjState* pState)
{
    EntityBuilding::WriteObject(group, pState);
    group.WriteBits(mTriggered, 1);
}

// FUNCTION: SWBF2 0x005f07e0
void EntityTrap::ReadObject(NetPktGroup& group, NetObjState* pState)
{
    EntityBuilding::ReadObject(group, pState);
    unsigned int value;
    group.ReadBits(value, 1);
    mTriggered = value != 0;
}

// FUNCTION: SWBF2 0x005f0650
GameObject* EntityTrap::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            Thread* pThread = this;
            pThread->DeactivateThread();
            Thread::EnterClient();
            pThread->ActivateThread(0, PER_LOCAL_TURN);
            Thread::LeaveClient();
        }
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<CollisionObject*>(this)->GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    GameObject* pClient = (GameObject*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, pClient);
    pClient->Init();
    pClient->BuildEffects();
    NetGame::LeaveClient();
    return pClient->GetGameObject();
}

// FUNCTION: SWBF2 0x005f11a0
void EntityRemoteTerminal::WriteObject(NetPktGroup& group, NetObjState*)
{
    WritePilots(group, *this);
}

// FUNCTION: SWBF2 0x005f11c0
void EntityRemoteTerminal::ReadObject(NetPktGroup& group, NetObjState*)
{
    ReadPilots(group, *this);
}

// FUNCTION: SWBF2 0x005f1180
void EntityRemoteTerminal::AddSends(void (*pfnSend)(GameObject*, int), int iArg)
{
    AddSendPilots(pfnSend, *this, iArg);
}

// FUNCTION: SWBF2 0x005f1010
GameObject* EntityRemoteTerminal::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            DeactivatePhysics();
            const PblList<Controllable>& list = GetControllableList();
            for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
            {
                Controllable* pControl = it;
                pControl->ReleaseController();
            }
            NetGame::EnterClient();
            ActivatePhysics();
            for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
            {
                Controllable* pControl = it;
                pControl->SetupController();
            }
            NetGame::LeaveClient();
        }
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<CollisionObject*>(this)->GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    Entity* pClient = GetEntityClass()->Build(desc);
    NetGame::LeaveClient();
    return pClient->GetGameObject();
}

// FUNCTION: SWBF2 0x005efed0
void EntityMine::WriteObject(NetPktGroup& group, NetObjState*)
{
    WriteMatrix(group, GetMatrix());
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    group.WriteFixed(mLifeTimer, -1.0f, mClass->mLifeSpan, 8);
    group.WriteBits(mStationary ? 1u : 0u, 1);
    group.WriteBits(mTrigger ? 1u : 0u, 1);
    WriteCharacter(group, mOwner);
    const bool bStuck = mStickInfo != 0;
    group.WriteBits(bStuck ? 1u : 0u, 1);
    if (!bStuck)
    {
        return;
    }
    const CollisionObject* pColl = mStickInfo->mCollObject;
    const bool bObject = pColl && pColl->GetGameObject();
    group.WriteBits(bObject ? 1u : 0u, 1);
    if (!bObject)
    {
        return;
    }
    pColl = mStickInfo->mCollObject;
    WriteObjectPtr(group, pColl->GetGameObject());
    group.WriteBits(mStickInfo->mCollBodyID, 4);
    WritePositionVector(group, mStickInfo->mLocalPosition);
    WriteQuaternion(group, mStickInfo->mLocalRotation);
}

// FUNCTION: SWBF2 0x005f0010
void EntityMine::ReadObject(NetPktGroup& group, NetObjState*)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    SetMatrix(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    RedSceneObject* pScene = this;
    pScene->Deactivate();
    pScene->Activate();
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    group.ReadFixed(mLifeTimer, -1.0f, mClass->mLifeSpan, 8);
    unsigned int bit = 0;
    group.ReadBits(bit, 1);
    mStationary = bit != 0;
    group.ReadBits(bit, 1);
    mTrigger = bit != 0;
    mOwner = ReadCharacter(group);
    group.ReadBits(bit, 1);
    if (bit)
    {
        group.ReadBits(bit, 1);
        if (bit)
        {
            GameObject* pObject = ReadObjectPtr(group);
            unsigned int body;
            PblVector3 position;
            PblQuaternion rotation;
            group.ReadBits(body, 4);
            ReadPositionVector(group, position);
            ReadQuaternion(group, rotation);
            if (!mStickInfo)
            {
                mStickInfo = new StickInfo;
                mStickInfo->mCollObject = pObject->GetCollisionObject();
                mStickInfo->mCollHandle = pObject;
                mStickInfo->mLocalPosition = position;
                mStickInfo->mLocalRotation = rotation;
                mStickInfo->mCollBodyID = body;
            }
        }
    }
    mNetDead = false;
}

class CannonStrengthConvert : public ConvertValues
{
public:
    float GetMin() const { return 0.0f; }
    float GetMax() const { return 1.0f; }
    int GetBits() const { return 8; }
};

static CannonStrengthConvert cannonStrengthConvert;

// FUNCTION: SWBF2 0x005f6280
template <class CONVERT>
void WriteFixed(CONVERT& convert, NetPktGroup& group, float fValue)
{
    convert.TrackValue(fValue);
    group.WriteFixed(fValue, convert.GetMin(), convert.GetMax(), convert.GetBits());
}

// FUNCTION: SWBF2 0x005f62c0
template <class CONVERT>
void ReadFixed(CONVERT& convert, NetPktGroup& group, float& fValue)
{
    group.ReadFixed(fValue, convert.GetMin(), convert.GetMax(), convert.GetBits());
}

template void WriteFixed<CannonStrengthConvert>(CannonStrengthConvert&, NetPktGroup&, float);
template void ReadFixed<CannonStrengthConvert>(CannonStrengthConvert&, NetPktGroup&, float&);

// TEMPLATE: SWBF2 0x005f62e0
// WriteFixed<QuatConvert>
// TEMPLATE: SWBF2 0x005f6330
// ReadFixed<QuatConvert>
// TEMPLATE: SWBF2 0x005f6360
// WriteFixed<UnitConvert>
// TEMPLATE: SWBF2 0x005f63c0
// ReadFixed<UnitConvert>
// TEMPLATE: SWBF2 0x005f6400
// WriteFixed<PositionXZConvert>
// TEMPLATE: SWBF2 0x005f6460
// ReadFixed<PositionXZConvert>
// TEMPLATE: SWBF2 0x005f64a0
// WriteFixed<PositionYConvert>
// TEMPLATE: SWBF2 0x005f6500
// ReadFixed<PositionYConvert>
template void WriteFixed<QuatConvert>(QuatConvert&, NetPktGroup&, float);
template void ReadFixed<QuatConvert>(QuatConvert&, NetPktGroup&, float&);
template void WriteFixed<UnitConvert>(UnitConvert&, NetPktGroup&, float);
template void ReadFixed<UnitConvert>(UnitConvert&, NetPktGroup&, float&);
template void WriteFixed<PositionXZConvert>(PositionXZConvert&, NetPktGroup&, float);
template void ReadFixed<PositionXZConvert>(PositionXZConvert&, NetPktGroup&, float&);
template void WriteFixed<PositionYConvert>(PositionYConvert&, NetPktGroup&, float);
template void ReadFixed<PositionYConvert>(PositionYConvert&, NetPktGroup&, float&);

// TEMPLATE: SWBF2 0x005f6540
// WriteFixed<SwitchConvert>
// TEMPLATE: SWBF2 0x005f6590
// WriteFixed<ThrustConvert>
// TEMPLATE: SWBF2 0x005f65f0
// ReadFixed<ThrustConvert>
// TEMPLATE: SWBF2 0x005f6630
// ReadFixed<SwitchConvert>
// TEMPLATE: SWBF2 0x005f6660
// WriteFixed<AngleConvert>
// TEMPLATE: SWBF2 0x005f66c0
// ReadFixed<AngleConvert>
// TEMPLATE: SWBF2 0x005f6700
// WriteFixed<JetFuelConvert>
// TEMPLATE: SWBF2 0x005f6740
// ReadFixed<JetFuelConvert>
template void WriteFixed<SwitchConvert>(SwitchConvert&, NetPktGroup&, float);
template void WriteFixed<ThrustConvert>(ThrustConvert&, NetPktGroup&, float);
template void ReadFixed<ThrustConvert>(ThrustConvert&, NetPktGroup&, float&);
template void ReadFixed<SwitchConvert>(SwitchConvert&, NetPktGroup&, float&);
template void WriteFixed<AngleConvert>(AngleConvert&, NetPktGroup&, float);
template void ReadFixed<AngleConvert>(AngleConvert&, NetPktGroup&, float&);
template void WriteFixed<JetFuelConvert>(JetFuelConvert&, NetPktGroup&, float);
template void ReadFixed<JetFuelConvert>(JetFuelConvert&, NetPktGroup&, float&);

// FUNCTION: SWBF2 0x005e7fd0
void EnergyBar::Write(NetPktGroup& group) const
{
    group.WriteBits(m_bOverheat, 1);
    if (m_bOverheat)
    {
        group.WriteFixed(m_fEnergy, m_pClass->m_fEnergyMin, m_pClass->m_fEnergyOverheat, 12);
    }
    else
    {
        group.WriteFixed(m_fEnergy, 0.0f, m_pClass->m_fEnergyMax, 12);
    }
}

// FUNCTION: SWBF2 0x005e8030
void EnergyBar::Read(NetPktGroup& group)
{
    unsigned int uOverheat;
    group.ReadBits(uOverheat, 1);
    m_bOverheat = uOverheat != 0;
    if (m_bOverheat)
    {
        group.ReadFixed(m_fEnergy, m_pClass->m_fEnergyMin, m_pClass->m_fEnergyOverheat, 12);
    }
    else
    {
        group.ReadFixed(m_fEnergy, 0.0f, m_pClass->m_fEnergyMax, 12);
    }
}

// FUNCTION: SWBF2 0x005e80a0
void AmmoCounter::Write(NetPktGroup& group) const
{
    int iRoundsPerClip = m_pClass->mRoundsPerClip;
    if (iRoundsPerClip == 0x7fffffff)
    {
        return;
    }

    int iBits = 0;
    for (int i = iRoundsPerClip; i > 0; i >>= 1)
    {
        ++iBits;
    }
    group.WriteBits((int)(mCurClip * (float)iRoundsPerClip), iBits);

    if (mMaxClips != FLT_MAX)
    {
        int iClipBits = 0;
        for (int i = (int)(mMaxClips * (float)iRoundsPerClip); i > 0; i >>= 1)
        {
            ++iClipBits;
        }
        group.WriteBits((int)(mNumClips * (float)iRoundsPerClip), iClipBits);
    }
}

// FUNCTION: SWBF2 0x005e8140
void AmmoCounter::Read(NetPktGroup& group)
{
    int iRoundsPerClip = m_pClass->mRoundsPerClip;
    if (iRoundsPerClip == 0x7fffffff)
    {
        return;
    }

    int iBits = 0;
    for (int i = iRoundsPerClip; i > 0; i >>= 1)
    {
        ++iBits;
    }
    unsigned int uRounds;
    group.ReadBits(uRounds, iBits);

    float fRoundsPerClip = (float)iRoundsPerClip;
    float fCurClip = ((float)uRounds + 0.0001f) / fRoundsPerClip;
    mCurClip = (fCurClip < 0.0f) ? 0.0f : ((fCurClip > 1.0f) ? 1.0f : fCurClip);

    if (mMaxClips != FLT_MAX)
    {
        int iClipBits = 0;
        for (int i = (int)(mMaxClips * fRoundsPerClip); i > 0; i >>= 1)
        {
            ++iClipBits;
        }
        bool bInClip = uRounds != 0;
        group.ReadBits(uRounds, iClipBits);
        float fNumClips = ((float)uRounds + 0.0001f) / fRoundsPerClip;
        float fMaxClips = mMaxClips - (float)bInClip;
        mNumClips = (fNumClips < 0.0f) ? 0.0f : ((fNumClips > fMaxClips) ? fMaxClips : fNumClips);
    }
}

// FUNCTION: SWBF2 0x005e8270
void Character::Write(NetPktGroup& group)
{
    for (int i = 0; i < 3; ++i)
    {
        if (mObjectHitTimer[i] > 0.0f)
        {
            group.WriteBits(1, 1);
            group.WriteFixed(mObjectHitTimer[i], 0.0f, 0.5f, 7);
        }
        else
        {
            group.WriteBits(0, 1);
        }
    }
}

// FUNCTION: SWBF2 0x005e82d0
void Character::Read(NetPktGroup& group)
{
    for (int i = 0; i < 3; ++i)
    {
        unsigned int uHit;
        group.ReadBits(uHit, 1);
        if (uHit)
        {
            group.ReadFixed(mObjectHitTimer[i], 0.0f, 0.5f, 7);
        }
        else
        {
            mObjectHitTimer[i] = 0.0f;
        }
    }
}

// FUNCTION: SWBF2 0x005e8330
void Weapon::Write(NetPktGroup& group)
{
    group.WriteBits(mState, 4);
    if (mClass->mRoundsPerShot > 0.0f)
    {
        m_pAmmoCounter->Write(group);
    }
    if (mClass->mHeatPerShot > 0.0f)
    {
        m_pEnergyBar->Write(group);
    }
    if (mState != IDLE)
    {
        group.WriteBits(*(const unsigned int*)&mStateTimer, 32);
    }

    if (mOwner->mTargetInfo.mIsAiming)
    {
        const WeaponClass* pClass = mClass;
        if (pClass->mZoomMin != pClass->mZoomMax)
        {
            if (pClass->mZoomRate != 0.0f)
            {
                group.WriteFixed(mZoom, pClass->mZoomMin, pClass->mZoomMax, 5);
            }
            else
            {
                group.WriteBits(mZoom == pClass->mZoomMax, 1);
            }
        }
    }

    if (mAimer->mNextAimer != mAimer)
    {
        for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
        {
            if (mOwner->GetWeaponPtr(i) == this)
            {
                const Aimer* pAimer = mOwner->GetAimer(i);
                if (pAimer)
                {
                    unsigned int uNumAimers = 0;
                    for (const AimerClass* pAimerClass = pAimer->mClass; pAimerClass;
                         pAimerClass = pAimerClass->mNextAimerClass)
                    {
                        ++uNumAimers;
                    }
                    int iBits = 0;
                    for (unsigned int u = uNumAimers - 1; u; u >>= 1)
                    {
                        ++iBits;
                    }
                    unsigned int uIndex = 0;
                    for (; pAimer != mAimer; pAimer = pAimer->mNextAimer)
                    {
                        ++uIndex;
                    }
                    group.WriteBits(uIndex, iBits);
                }
                break;
            }
        }
    }

    int iNumBarrels = mAimer->mClass->mNumBarrels;
    if (iNumBarrels > 1)
    {
        int iBits = 0;
        for (unsigned int u = iNumBarrels - 1; u; u >>= 1)
        {
            ++iBits;
        }
        group.WriteBits(mAimer->mCurrentBarrel, iBits);
    }
}

// FUNCTION: SWBF2 0x005e84d0
void Weapon::Read(NetPktGroup& group)
{
    group.ReadBits((unsigned int&)mState, 4);
    if (mState != CHARGE && mChargeUpEmitter)
    {
        mChargeUpEmitter->StopNow();
    }
    if (mClass->mRoundsPerShot > 0.0f)
    {
        m_pAmmoCounter->Read(group);
    }
    if (mClass->mHeatPerShot > 0.0f)
    {
        m_pEnergyBar->Read(group);
    }
    if (mState != IDLE)
    {
        group.ReadBits((unsigned int&)mStateTimer, 32);
    }

    if (mOwner->mTargetInfo.mIsAiming)
    {
        const WeaponClass* pClass = mClass;
        if (pClass->mZoomMin != pClass->mZoomMax)
        {
            if (pClass->mZoomRate != 0.0f)
            {
                group.ReadFixed(mZoom, pClass->mZoomMin, pClass->mZoomMax, 5);
                UpdateZoom(0.0f, 0.0f);
            }
            else
            {
                unsigned int uZoomed;
                group.ReadBits(uZoomed, 1);
                if (uZoomed)
                {
                    mZoom = mClass->mZoomMax;
                    mZoomTurnScale = mClass->mZoomTurnDivisorMax;
                }
                else
                {
                    mZoom = mClass->mZoomMin;
                    mZoomTurnScale = mClass->mZoomTurnDivisorMin;
                }
            }
        }
    }

    if (mAimer->mNextAimer != mAimer)
    {
        for (int i = 0; i < mOwner->GetWeaponCount(); ++i)
        {
            if (mOwner->GetWeaponPtr(i) == this)
            {
                Aimer* pAimer = const_cast<Aimer*>(mOwner->GetAimer(i));
                if (pAimer)
                {
                    unsigned int uNumAimers = 0;
                    for (const AimerClass* pAimerClass = pAimer->mClass; pAimerClass;
                         pAimerClass = pAimerClass->mNextAimerClass)
                    {
                        ++uNumAimers;
                    }
                    int iBits = 0;
                    for (unsigned int u = uNumAimers - 1; u; u >>= 1)
                    {
                        ++iBits;
                    }
                    unsigned int uIndex;
                    group.ReadBits(uIndex, iBits);
                    for (mAimer = pAimer; uIndex; --uIndex)
                    {
                        mAimer = mAimer->mNextAimer;
                    }
                }
                break;
            }
        }
    }

    int iNumBarrels = mAimer->mClass->mNumBarrels;
    if (iNumBarrels > 1)
    {
        int iBits = 0;
        for (unsigned int u = iNumBarrels - 1; u; u >>= 1)
        {
            ++iBits;
        }
        group.ReadBits((unsigned int&)mAimer->mCurrentBarrel, iBits);
    }
}

// FUNCTION: SWBF2 0x005e86f0
void WeaponCannon::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    const WeaponCannonClass* pClass = (const WeaponCannonClass*)mClass;
    WriteFixed(cannonStrengthConvert, group, mStrength);
    group.WriteFixed(mKickSpread, -pClass->mSpreadThreshold, pClass->mSpreadLimit, 13);
    if (pClass->mSecondaryOrdnancePeriod && pClass->mSecondaryOrdnanceClass)
    {
        group.WriteBits(mOrdCounter, 32);
    }
    float fWarmCoolTime = (pClass->mWarmUpTime > pClass->mCoolDownTime) ? pClass->mWarmUpTime : pClass->mCoolDownTime;
    if (fWarmCoolTime > 0.0f)
    {
        group.WriteFixed(mWarmCoolTimer, 0.0f, fWarmCoolTime, 8);
    }
    group.WriteBits(mWarmUp, 1);
    group.WriteBits(mSalvoCount, 6);
    float fSalvoDelay = (pClass->mSalvoDelayMax > pClass->mInitialSalvoDelay) ? pClass->mSalvoDelayMax : pClass->mInitialSalvoDelay;
    group.WriteFixed(mSalvoDelay, -1.0f, fSalvoDelay, 15);
    if (mState == FIRE || mState == FIRE2)
    {
        group.WriteFixed(mStateLimit, pClass->mShotDelayMin, pClass->mShotDelayMax, 6);
    }
}

// FUNCTION: SWBF2 0x005e8830
void WeaponCannon::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    ReadFixed(cannonStrengthConvert, group, mStrength);
    const WeaponCannonClass* pClass = (const WeaponCannonClass*)mClass;
    group.ReadFixed(mKickSpread, -pClass->mSpreadThreshold, pClass->mSpreadLimit, 13);
    if (pClass->mSecondaryOrdnancePeriod && pClass->mSecondaryOrdnanceClass)
    {
        group.ReadBits((unsigned int&)mOrdCounter, 32);
    }
    float fWarmCoolTime = (pClass->mWarmUpTime > pClass->mCoolDownTime) ? pClass->mWarmUpTime : pClass->mCoolDownTime;
    if (fWarmCoolTime > 0.0f)
    {
        group.ReadFixed(mWarmCoolTimer, 0.0f, fWarmCoolTime, 8);
    }
    unsigned int uWarmUp;
    group.ReadBits(uWarmUp, 1);
    mWarmUp = uWarmUp != 0;
    group.ReadBits((unsigned int&)mSalvoCount, 6);
    float fSalvoDelay = (pClass->mSalvoDelayMax > pClass->mInitialSalvoDelay) ? pClass->mSalvoDelayMax : pClass->mInitialSalvoDelay;
    group.ReadFixed(mSalvoDelay, -1.0f, fSalvoDelay, 15);
    switch (mState)
    {
    case CHARGE:
        mStateLimit = pClass->mMaxPressedTime;
        break;
    case FIRE:
    case FIRE2:
        group.ReadFixed(mStateLimit, pClass->mShotDelayMin, pClass->mShotDelayMax, 6);
        break;
    case RELOAD:
        mStateLimit = pClass->mReloadTime;
        break;
    }
}

// FUNCTION: SWBF2 0x005e89b0
void WeaponDisguise::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteBits(m_eState, 1);
}

// FUNCTION: SWBF2 0x005e89e0
void WeaponDisguise::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    unsigned int uState = 0;
    group.ReadBits(uState, 1);
    DISGUISE_STATE eState = (DISGUISE_STATE)uState;
    if (eState == m_eState)
    {
        return;
    }
    if (m_eState == STATE_ON)
    {
        if (eState != STATE_ON)
        {
            StartDroppingDisguise();
        }
        if (eState == STATE_OFF)
        {
            FinishDroppingDisguise();
            m_eState = eState;
            return;
        }
    }
    else if (m_eState == STATE_OFF)
    {
        if (eState != STATE_OFF)
        {
            StartRaisingDisguise();
        }
        if (eState == STATE_ON)
        {
            FinishRaisingDisguise();
        }
    }
    m_eState = eState;
}

// FUNCTION: SWBF2 0x005e8a60
void WeaponMelee::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteBits(m_eComboState, 5);
    group.WriteBits(m_bAnimatedMove, 1);
    group.WriteBits(m_bLightsaberOn, 1);
    group.WriteBits(m_bLightsaberSoundOn, 1);
    group.WriteBits(m_uiTrailOnMask, 4);
    group.WriteBits(m_uiLightsaberOnMask, 4);
    group.WriteBits(m_uiLightsaberWasOnMask, 4);
    group.WriteBits(m_uiLightsaberSoundOnMask, 4);
}

// FUNCTION: SWBF2 0x005e8b20
void WeaponMelee::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    unsigned int uComboState = 0;
    unsigned int uValue = 0;
    group.ReadBits(uComboState, 5);
    group.ReadBits(uValue, 1);
    SetNetworkState((ComboState)uComboState, uValue != 0);
    group.ReadBits(uValue, 1);
    m_bLightsaberOn = uValue;
    group.ReadBits(uValue, 1);
    m_bLightsaberSoundOn = uValue;
    group.ReadBits(uValue, 4);
    m_uiTrailOnMask = uValue;
    group.ReadBits(uValue, 4);
    m_uiLightsaberOnMask = uValue;
    group.ReadBits(uValue, 4);
    m_uiLightsaberWasOnMask = uValue;
    group.ReadBits(uValue, 4);
    m_uiLightsaberSoundOnMask = uValue;
}

// FUNCTION: SWBF2 0x005e8c50
void WeaponMeleeThrow::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteBits(m_bFired, 1);
    group.WriteBits(m_bThrow, 1);
    group.WriteBits(m_bReturnedThisFrame, 1);
}

// FUNCTION: SWBF2 0x005e8cb0
void WeaponMeleeThrow::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    unsigned int uValue;
    group.ReadBits(uValue, 1);
    m_bFired = uValue;
    group.ReadBits(uValue, 1);
    m_bThrow = uValue;
    group.ReadBits(uValue, 1);
    m_bReturnedThisFrame = uValue;
}

// FUNCTION: SWBF2 0x005e8d30
void WeaponDispenser::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteBits(mFired != 0, 1);
    group.WriteBits(mNetFiredFlag != 0, 1);
    group.WriteFixed(mStrength, 0.0f, 1.0f, 8);
}

// FUNCTION: SWBF2 0x005e8d90
void WeaponDispenser::Read(NetPktGroup& group)
{
    bool bBusy = IsBusy();
    Weapon::Read(group);
    unsigned int uValue;
    group.ReadBits(uValue, 1);
    mFired = uValue != 0;
    group.ReadBits(uValue, 1);
    if (!mFiredFlag && uValue && !bBusy)
    {
        mFiredFlag = 1;
    }
    group.ReadFixed(mStrength, 0.0f, 1.0f, 8);
}

// FUNCTION: SWBF2 0x005e8e20
void WeaponGrenade::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteBits(mFired != 0, 1);
    group.WriteBits(mNetFiredFlag != 0, 1);
    group.WriteFixed(mStrength, 0.0f, 1.0f, 8);
}

// FUNCTION: SWBF2 0x005e8e80
void WeaponGrenade::Read(NetPktGroup& group)
{
    bool bBusy = IsBusy();
    Weapon::Read(group);
    unsigned int uValue;
    group.ReadBits(uValue, 1);
    mFired = uValue != 0;
    group.ReadBits(uValue, 1);
    if (!mFiredFlag && uValue && !bBusy)
    {
        mFiredFlag = 1;
    }
    group.ReadFixed(mStrength, 0.0f, 1.0f, 8);

    const WeaponGrenadeClass* pClass = (const WeaponGrenadeClass*)mClass;
    switch (mState)
    {
    case CHARGE:
        mStateLimit = pClass->mMaxPressedTime;
        break;
    case FIRE:
    case FIRE2:
    {
        int iIndex = (m_iSoldierState == 2) ? 2 : mState - 1;
        mStateLimit = pClass->mInitialSalvoDelay[iIndex] + pClass->mShotDelay;
        break;
    }
    case RELOAD:
        mStateLimit = pClass->mReloadTime;
        break;
    }
}

// FUNCTION: SWBF2 0x005e8f80
void WeaponInvisibility::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteBits(mFired != 0, 1);
    group.WriteBits(m_bInvisibilityActive != 0, 1);
}

// FUNCTION: SWBF2 0x005e8fc0
void WeaponInvisibility::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    unsigned int uValue;
    group.ReadBits(uValue, 1);
    mFired = uValue != 0;
    group.ReadBits(uValue, 1);
    m_bInvisibilityActive = uValue != 0;
    if (mState == FIRE)
    {
        mStateLimit = ((const WeaponInvisibilityClass*)mClass)->mInitialSalvoDelay;
    }
}

// FUNCTION: SWBF2 0x005e9030
void WeaponLaser::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    group.WriteFixed(mSalvoTimer, 0.0f, ((const WeaponLaserClass*)mClass)->mSalvoTime, 6);
}

// FUNCTION: SWBF2 0x005e9070
void WeaponLaser::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    group.ReadFixed(mSalvoTimer, 0.0f, ((const WeaponLaserClass*)mClass)->mSalvoTime, 6);
}

// FUNCTION: SWBF2 0x005e90b0
void WeaponLauncher::Write(NetPktGroup& group)
{
    WeaponCannon::Write(group);
    group.WriteBits(mLocked != 0, 1);
}

// FUNCTION: SWBF2 0x005e90e0
void WeaponLauncher::Read(NetPktGroup& group)
{
    WeaponCannon::Read(group);
    unsigned int uLocked;
    group.ReadBits(uLocked, 1);
    mLocked = uLocked != 0;
    if (mState == CHARGE)
    {
        mStateLimit = ((const WeaponLauncherClass*)mClass)->mLockTime;
    }
}

// FUNCTION: SWBF2 0x005e9130
void WeaponTowCable::Write(NetPktGroup& group)
{
    WeaponCannon::Write(group);
}

// FUNCTION: SWBF2 0x005e9140
void WeaponTowCable::Read(NetPktGroup& group)
{
    WeaponCannon::Read(group);
}

// FUNCTION: SWBF2 0x005e9150
void WeaponGrapplingHook::Write(NetPktGroup& group)
{
    WeaponCannon::Write(group);
}

// FUNCTION: SWBF2 0x005e9160
void WeaponGrapplingHook::Read(NetPktGroup& group)
{
    WeaponCannon::Read(group);
}

// FUNCTION: SWBF2 0x005e9170
void WeaponDetonator::Write(NetPktGroup& group)
{
    WeaponDispenser::Write(group);
}

// FUNCTION: SWBF2 0x005e9180
void WeaponDetonator::Read(NetPktGroup& group)
{
    WeaponDispenser::Read(group);
}

// FUNCTION: SWBF2 0x005e9190
void WeaponRemote::Write(NetPktGroup& group)
{
    WeaponDispenser::Write(group);
    if (GameObject* pItem = mItem)
    {
        group.WriteBits(1, 1);
        group.WriteBits(GetUniqObjectId(pItem), GetUniqObjectBits());
        return;
    }
    group.WriteBits(0, 1);
}

// FUNCTION: SWBF2 0x005e9210
void WeaponRemote::Read(NetPktGroup& group)
{
    WeaponDispenser::Read(group);
    unsigned int uHasItem = 0;
    group.ReadBits(uHasItem, 1);
    mItem = 0;
    if (uHasItem)
    {
        unsigned int uId = 0;
        group.ReadBits(uId, GetUniqObjectBits());
        if (GameObject* pItem = FindClientObject(uId))
        {
            mItem = pItem;
        }
    }
}

// FUNCTION: SWBF2 0x005e9290
void WeaponAreaEffect::Write(NetPktGroup& group)
{
    Weapon::Write(group);
    if (mState == FIRE)
    {
        group.WriteBits(m_bFired != 0, 1);
        if (((const WeaponAreaEffectClass*)mClass)->m_eFireType == 1 && m_bFired)
        {
            group.WriteBits(m_iNumTargets > 0, 1);
        }
    }
}

// FUNCTION: SWBF2 0x005e92f0
void WeaponAreaEffect::Read(NetPktGroup& group)
{
    Weapon::Read(group);
    if (mState == FIRE)
    {
        unsigned int uValue;
        group.ReadBits(uValue, 1);
        m_bFired = uValue != 0;
        if (((const WeaponAreaEffectClass*)mClass)->m_eFireType == 1 && m_bFired)
        {
            group.ReadBits(uValue, 1);
            m_iNumTargets = uValue != 0;
        }
    }
}

extern bool netOnTurn;
extern bool netFrameLock;
extern bool netIsLastLocalTurn;

static const float EPSILON = 0.0001f;

// GLOBAL: SWBF2 0x01ef2580
static PblMatrix sTempMatrix;

// FUNCTION: SWBF2 0x005eb3a0
const PblMatrix& EntitySoldier::GetSmoothedMatrix() const
{
    if (NetGame::IsEnabled() && mNetUniqueId != -1 && NetGame::IsClient())
    {
        if (netOnTurn || mNetUniqueId < 0 || netFrameLock)
        {
            return NetPredict::GetSmoothInfo(mNetUniqueId).GetRenderMatrix();
        }
        gameObjectInterpolator.GetMatrix(mNetUniqueId, sTempMatrix);
        return sTempMatrix;
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x005eb420
bool EntitySoldier::Smooth(float fDeltaTime) const
{
    if (!NetGame::IsEnabled() || (NetGame::IsHost() && NetGame::IsOneWorld()))
    {
        return true;
    }
    if (!NetGame::IsClient() || mNetUniqueId == -1)
    {
        return false;
    }
    if (fDeltaTime == EPSILON)
    {
        return true;
    }

    SmoothInfo& info = NetPredict::GetSmoothInfo(mNetUniqueId);

    if (const_cast<EntitySoldier*>(this)->IsThreadActive())
    {
        info.SetMat(mMatrix, 10.0f, 200.0f);
    }
    else
    {
        info.SetMat(mMatrix, 0.0f, 0.0f);
    }
    return netIsLastLocalTurn;
}

// FUNCTION: SWBF2 0x005eb6d0
void EntityPathFollower::WriteObject(NetPktGroup& group, NetObjState* pState, float fMinSpeed, float fMaxSpeed)
{
    group.WriteBits(mPath != 0, 1);
    if (mPath)
    {
        unsigned int uPath = 0;
        for (PblList<EntityPath>::Itor it(EntityPath::GetPathList()); (EntityPath*)it && (EntityPath*)it != mPath; ++it)
        {
            ++uPath;
        }
        group.WriteBits(uPath, 8);
        group.WriteBits(mIndex, 8);
    }
    group.WriteFixed(mT, 0.0f, 1.01f, 16);
    group.WriteBits(mSetPoints, 4);
    for (int i = 0; i < 4; ++i)
    {
        if (mSetPoints & (1 << i))
        {
            WritePositionVector(group, mPathSpline.GetControlPoints()[i]);
        }
    }
    group.WriteBits(mDirection == 1, 1);
    group.WriteFixed(mCurSpeed, fMinSpeed, fMaxSpeed, 16);
}

// FUNCTION: SWBF2 0x005eb7e0
void EntityPathFollower::ReadObject(NetPktGroup& group, NetObjState* pState, float fMinSpeed, float fMaxSpeed)
{
    unsigned int uValue;
    group.ReadBits(uValue, 1);
    if (uValue)
    {
        uValue = 0;
        group.ReadBits(uValue, 8);
        unsigned int uPath = 0;
        for (PblList<EntityPath>::Itor it(EntityPath::GetPathList()); (EntityPath*)it; ++it, ++uPath)
        {
            if (uPath == uValue)
            {
                mPath = it;
                break;
            }
        }
        group.ReadBits(uValue, 8);
        mIndex = uValue;
    }
    else
    {
        mPath = 0;
    }
    group.ReadFixed(mT, 0.0f, 1.01f, 16);
    group.ReadBits(uValue, 4);
    mSetPoints = (unsigned char)uValue;
    for (int i = 0; i < 4; ++i)
    {
        if (mSetPoints & (1 << i))
        {
            ReadPositionVector(group, mPathSpline.GetControlPoints()[i]);
        }
    }
    group.ReadBits(uValue, 1);
    mDirection = uValue != 0 ? 1 : -1;
    group.ReadFixed(mCurSpeed, fMinSpeed, fMaxSpeed, 16);
}

// FUNCTION: SWBF2 0x005ed6b0
const PblMatrix& EntityWalker::GetSmoothedMatrix() const
{
    if (NetGame::IsEnabled() && mNetUniqueId != -1 && NetGame::IsClient())
    {
        return NetPredict::GetSmoothInfo(mNetUniqueId).GetRenderMatrix();
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x005ed700
bool EntityWalker::Smooth(float fDeltaTime) const
{
    if (!NetGame::IsEnabled() || (NetGame::IsHost() && NetGame::IsOneWorld()))
    {
        return true;
    }
    if (!NetGame::IsClient() || mNetUniqueId == -1)
    {
        return false;
    }
    if (fDeltaTime == EPSILON)
    {
        return true;
    }

    SmoothInfo& info = NetPredict::GetSmoothInfo(mNetUniqueId);
    info.mPlayerId = GetPlayerId();
    info.SetMat(mMatrix, 10.0f, 100.0f);
    return netIsLastLocalTurn;
}

// FUNCTION: SWBF2 0x005ee770
const PblMatrix& EntityDroideka::GetSmoothedMatrix() const
{
    if (NetGame::IsEnabled() && mNetUniqueId != -1 && NetGame::IsClient())
    {
        return NetPredict::GetSmoothInfo(mNetUniqueId).GetRenderMatrix();
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x005ee7c0
bool EntityDroideka::Smooth(float fDeltaTime) const
{
    if (!NetGame::IsEnabled() || (NetGame::IsHost() && NetGame::IsOneWorld()))
    {
        return true;
    }
    if (!NetGame::IsClient() || mNetUniqueId == -1)
    {
        return false;
    }
    if (fDeltaTime == EPSILON)
    {
        return true;
    }

    SmoothInfo& info = NetPredict::GetSmoothInfo(mNetUniqueId);
    info.mPlayerId = GetPlayerId();
    info.SetMat(mMatrix, 10.0f, 200.0f);
    return netIsLastLocalTurn;
}

// FUNCTION: SWBF2 0x005eee40
bool EntityDroid::Smooth(float fDeltaTime) const
{
    const EntityDroid* pDroid = (const EntityDroid*)this;
    if (!NetGame::IsEnabled() || (NetGame::IsHost() && NetGame::IsOneWorld()))
    {
        return true;
    }
    if (!NetGame::IsClient() || pDroid->mNetUniqueId == -1)
    {
        return false;
    }
    if (fDeltaTime == EPSILON)
    {
        return true;
    }

    SmoothInfo& info = NetPredict::GetSmoothInfo(pDroid->mNetUniqueId);
    info.mPlayerId = pDroid->GetPlayerId();
    info.SetMat(pDroid->mMatrix, 10.0f, 200.0f);
    return netIsLastLocalTurn;
}

// FUNCTION: SWBF2 0x005ef650
const PblMatrix& EntityHover::GetSmoothedMatrix() const
{
    if (NetGame::IsEnabled() && mNetUniqueId != -1 && NetGame::IsClient())
    {
        return NetPredict::GetSmoothInfo(mNetUniqueId).GetRenderMatrix();
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x005ef6a0
bool EntityHover::Smooth(float fDeltaTime) const
{
    if (!NetGame::IsEnabled() || (NetGame::IsHost() && NetGame::IsOneWorld()))
    {
        return true;
    }
    if (!NetGame::IsClient() || mNetUniqueId == -1)
    {
        return false;
    }
    if (fDeltaTime == EPSILON)
    {
        return true;
    }

    SmoothInfo& info = NetPredict::GetSmoothInfo(mNetUniqueId);
    info.mPlayerId = GetPlayerId();
    info.mVelocity = mVelocity;
    info.mOmega = mOmega;
    info.SetMat(mMatrix, 10.0f, 200.0f);
    return netIsLastLocalTurn;
}

// FUNCTION: SWBF2 0x005ec6e0
const PblMatrix& EntityFlyer::GetSmoothedMatrix() const
{
    if (NetGame::IsEnabled() && mNetUniqueId != -1 && NetGame::IsClient())
    {
        return NetPredict::GetSmoothInfo(mNetUniqueId).GetRenderMatrix();
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x005ec730
bool EntityFlyer::Smooth(float fDeltaTime) const
{
    if (!NetGame::IsEnabled() || (NetGame::IsHost() && NetGame::IsOneWorld()))
    {
        return true;
    }
    if (!NetGame::IsClient() || mNetUniqueId == -1)
    {
        return false;
    }
    if (fDeltaTime == EPSILON)
    {
        return true;
    }

    SmoothInfo& info = NetPredict::GetSmoothInfo(mNetUniqueId);
    info.mPlayerId = GetPlayerId();
    info.mVelocity = mVelocity;
    info.mOmega = mOmega;
    info.SetMat(mMatrix, 10.0f, 100.0f);
    return netIsLastLocalTurn;
}

void PerformSafeRemove(CollisionObject* pObject);

// FUNCTION: SWBF2 0x004608a0
bool CollisionManager::I_DeactivateItem(CollisionObject* pObject)
{
    CollisionManager* pManager = this;
    PerformSafeRemove(pObject);
    PblList<CollisionObject>::Node* pNode = (PblList<CollisionObject>::Node*)pObject->mCollisionObjectNode;
    if (!pNode->IsInList())
    {
        return false;
    }

    GameObject* pGameObject = pObject->GetGameObject();
    if (pGameObject)
    {
        TeamManager::RemoveObject(pGameObject);
    }
    if (pObject->GetCollisionType() == COLL_ITEM)
    {
        pManager->mItemBodies.Remove(pNode);
    }
    return true;
}

// FUNCTION: SWBF2 0x00460920
bool CollisionManager::DeactivateItem(CollisionObject* pObject)
{
    return sInstance->I_DeactivateItem(pObject);
}

// FUNCTION: SWBF2 0x005e94d0
void GameObject::WriteCreate(NetPktGroup&) {}

// FUNCTION: SWBF2 0x005e94e0
void GameObject::ReadCreate(NetPktGroup&) {}

// FUNCTION: SWBF2 0x005e94f0
void GameObject::WriteObject(NetPktGroup&, NetObjState*) {}

// FUNCTION: SWBF2 0x005e9500
void GameObject::ReadObject(NetPktGroup&, NetObjState*) {}

// FUNCTION: SWBF2 0x005e9510
void GameObject::ResetObject() {}

// FUNCTION: SWBF2 0x005e9520
void GameObject::SkipReadObject(NetObjState*) {}

// FUNCTION: SWBF2 0x005e9530
GameObject* GameObject::AddToClient()
{
    return netIsOneWorld ? this : 0;
}

// FUNCTION: SWBF2 0x005e9540
void GameObject::AddSends(void (*)(GameObject*, int), int) {}

// FUNCTION: SWBF2 0x005eb390
void EntitySoldier::SkipReadObject(NetObjState*) {}

// FUNCTION: SWBF2 0x005f28c0
void GameObject::WriteLateState(NetPktGroup& group)
{
    WriteMatrix(group, static_cast<const CollisionObject*>(this)->GetMatrix());
}

// FUNCTION: SWBF2 0x005f28e0
void GameObject::ReadLateState(NetPktGroup& group)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    static_cast<CollisionObject*>(this)->SetMatrix(matrix);
}

// FUNCTION: SWBF2 0x005ea120
void __fastcall PropagateTeam(Character* pCharacter, int iTeam)
{
    if (pCharacter && pCharacter->mTeamNumber != iTeam)
    {
        pCharacter->SelectTeam(iTeam);
    }
}

// FUNCTION: SWBF2 0x005c2580
void WriteEntityClass(NetPktGroup& group, const void* pValue)
{
    const EntityClass* pClass = static_cast<const EntityClass*>(pValue);
    const int value = pClass ? pClass->mNetIndex : -1;
    group.WriteBits(value, 8);
}

// FUNCTION: SWBF2 0x005c2600
void WriteWeaponClass(NetPktGroup& group, const void* pValue)
{
    const WeaponClass* pClass = static_cast<const WeaponClass*>(pValue);
    const int value = pClass ? pClass->mNetIndex : -1;
    group.WriteBits(value, 8);
}

void AddNetCollision(CollisionObject* pObject);

// FUNCTION: SWBF2 0x005f2870
void EntityProp::AddToClient()
{
    GameObject* pObject = (GameObject*)this;
    RedSceneObject* pScene = static_cast<RedSceneObject*>(pObject);
    pScene->Deactivate();
    NetGame::EnterClient();
    pScene->Activate();
    AddNetCollision(pObject ? static_cast<CollisionObject*>(pObject) : 0);
    pObject->BuildEffects();
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x005ec6a0
void EntityFlyer::Move(const PblMatrix& matrix)
{
    GameObject* pObject = (GameObject*)this;
    static_cast<CollisionObject*>(pObject)->SetMatrix(matrix);
    RedSceneObject* pScene = static_cast<RedSceneObject*>(pObject);
    pScene->Deactivate();
    pScene->Activate();
}

// FUNCTION: SWBF2 0x005ee730
void EntityDroideka::Move(const PblMatrix& matrix)
{
    GameObject* pObject = (GameObject*)this;
    static_cast<CollisionObject*>(pObject)->SetMatrix(matrix);
    RedSceneObject* pScene = static_cast<RedSceneObject*>(pObject);
    pScene->Deactivate();
    pScene->Activate();
}

// FUNCTION: SWBF2 0x005f5e80
void WriteSwitch(NetPktGroup& group, float value)
{
    WriteFixed(switchConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5eb0
void ReadSwitch(NetPktGroup& group, float& value)
{
    ReadFixed(switchConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5ee0
void WriteJetFuel(NetPktGroup& group, float value)
{
    WriteFixed(jetFuelConvert, group, value);
}

// FUNCTION: SWBF2 0x005f5ef0
void ReadJetFuel(NetPktGroup& group, float& value)
{
    ReadFixed(jetFuelConvert, group, value);
}

// FUNCTION: SWBF2 0x005e9550
void EntitySoldier::AddSends(void (*send)(GameObject*, int), int depth)
{
    if (mPlayerId != NetGame::GetCurDst() || !mCtrl)
    {
        return;
    }

    const GameObject* pReticuleTarget = mReticuleTarget[0];
    if (pReticuleTarget)
    {
        send(const_cast<GameObject*>(pReticuleTarget), depth + 1);
    }

    GameObject* pLockedTarget = mTargetLockedObj;
    if (pLockedTarget)
    {
        send(pLockedTarget, depth + 1);
    }
}

// FUNCTION: SWBF2 0x005ed7b0
void EntityDroideka::AddSends(void (*send)(GameObject*, int), int depth)
{
    if (mPlayerId != NetGame::GetCurDst() || !mCtrl)
    {
        return;
    }

    const GameObject* pReticuleTarget = mReticuleTarget[0];
    if (pReticuleTarget)
    {
        send(const_cast<GameObject*>(pReticuleTarget), depth + 1);
    }
}

void ReadDamageOwner(NetPktGroup& group, DamageOwner& owner);
void ReadPositionVector(NetPktGroup& group, PblVector3& value);

// FUNCTION: SWBF2 0x005eb920
void EntityFlyer::WriteObject(NetPktGroup& group, NetObjState* pState)
{
    const int clientId = NetGame::GetCurDst();
    const PblMatrix& matrix = GetMatrix();
    if (!NetGame::IsFullState())
    {
        SetNetLOD(ComputeNetLOD((int)&matrix.m[3][0], mPlayerId) != 0);
        SetNetPrecision(group);
    }
    WriteMatrix(group, matrix);
    group.WriteBits(mState, 3);
    group.WriteBits(mTeam, 2);
    group.WriteBits(mPerceivedTeam, 2);
    if (mPilot)
    {
        Trackable* pTrackable = static_cast<Trackable*>(mPilot);
        pTrackable->GetGameObject();
        AddSendLate();
    }
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive ? 1u : 0u, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded ? 1u : 0u, 1);
    }
    WritePilots(group, *this);
    if (mState != LANDED)
    {
        group.WriteBits(mBoost ? 1u : 0u, 1);
        WriteVelocity(group, mVelocity,
                   mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                     : mClass->mBoostFlyerSpeed, -1);
        WriteVelocity(group, mOmega,
                   mClass->mMaxCrashRollRate > 5.0f ? mClass->mMaxCrashRollRate : 5.0f, -1);
        group.WriteFixed(mBankPitchAngle, -6.2831854820251465f, 6.2831854820251465f, 16);
        group.WriteFixed(mBankRollAngle, -10.210176467895508f, 10.210176467895508f, 16);
        WriteUnitScalar(group, mFlightRatio);
        mEnergyBar.Write(group);
        group.WriteFixed(mSetSpeed, mClass->mMinFlyerSpeed > 0.0f ? 0.0f : mClass->mMinFlyerSpeed,
                         mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                           : mClass->mBoostFlyerSpeed, 16);
        group.WriteFixed(mSetStrafe, -mClass->mStrafeSpeed - 30.0f, mClass->mStrafeSpeed + 30.0f, 16);
        group.WriteFixed(mSetStrafeVertical, -mClass->mStrafeSpeed - 30.0f,
                         mClass->mStrafeSpeed + 30.0f, 16);
        group.WriteFixed(mWorldRoll, -1.0f, 1.0f, 16);
        group.WriteFixed(mAutoLevelingAllowed, 0.0f, 15.0f, 16);
        WriteThrust(group, mPitchControllerPrevUpdate);
        WriteThrust(group, mTurnControllerPrevUpdate);
        group.WriteFixed(mTrick, -1.0f, 1.0f, 9);
        group.WriteBits(*(const unsigned int*)&mTrickDoubleclick, 32);
        group.WriteBits(mInRoll ? 1u : 0u, 1);
        group.WriteBits(mInFlip ? 1u : 0u, 1);
        WriteAngle(group, mRoll);
        group.WriteFixed(mRollTarget, -13.35176944732666f, 13.35176944732666f, 16);
        group.WriteFixed(mSideRoll.x, -1.0f, 1.0f, 16);
        group.WriteFixed(mSideRoll.y, -1.0f, 1.0f, 16);
        if (mInFlip)
        {
            WriteAngle(group, mFlip);
            WriteAngle(group, mFlipTarget);
        }
        if (mPlayerId < 0)
        {
            group.WriteBits(mFollowingPath ? 1u : 0u, 1);
            group.WriteFixed(mAIEng.ai.mFreeFlyTime, 0.0f, mClass->mFreeFlyTime + 1.0f, 16);
            EntityPathFollower::WriteObject(
                group, pState, mClass->mMinFlyerSpeed > 0.0f ? 0.0f : mClass->mMinFlyerSpeed,
                mClass->mBoostFlyerSpeed > 0.0f ? mClass->mBoostFlyerSpeed : mClass->mMaxFlyerSpeed);
        }
        WriteThrust(group, mControlMove);
        WriteThrust(group, mControlStrafe);
        WriteThrust(group, mControlTurn);
        WriteThrust(group, mControlPitch);
        group.WriteFixed(mPitchBuildup, -1.0f, 1.0f, 10);
        if (mPlayerId >= 0)
        {
            WriteThrust(group, mTurnBuildup);
            group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
            if (NetGame::IsFullState() || NetGame::IsLocalTo(clientId, mPlayerId))
            {
                WriteTrigger(group, &mControlSprint);
                WriteTrigger(group, &mControlThrustFwd);
                if (mWeapon[0])
                {
                    WriteWeapon(group, mWeapon[0]);
                }
                if (mWeapon[1])
                {
                    WriteWeapon(group, mWeapon[1]);
                }
                if (mCharacter)
                {
                    mCharacter->Write(group);
                }
            }
            GameObject* target = mTargetLockedObj;
            bool hasTarget = false;
            if (target)
            {
                hasTarget = IsNeverDies(target->GetEntityType()) ||
                            GetUniqObjectId(mTargetLockedObj) != 63;
            }
            group.WriteBits(hasTarget ? 1u : 0u, 1);
            if (hasTarget)
            {
                WriteObjectPtr(group, mTargetLockedObj);
            }
        }
        for (int i = 0; i < mNumTurrets; ++i)
        {
            mTurret[i]->Write(group);
        }
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005ebfe0
void EntityFlyer::ReadObject(NetPktGroup& group, NetObjState* pState)
{
    if (!NetGame::IsFullState())
    {
        ReadNetLOD(group);
    }
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    unsigned int value = 0;
    group.ReadBits(value, 3);
    mState = (State)value;
    group.ReadBits(value, 2);
    SetTeam(value);
    group.ReadBits(value, 2);
    mPerceivedTeam = value;
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    ReadPilots(group, *this);
    if (mState != LANDED)
    {
        group.ReadBits(value, 1);
        mBoost = value != 0;
        ReadVelocity(group, mVelocity,
                   mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                     : mClass->mBoostFlyerSpeed, -1);
        ReadVelocity(group, mOmega,
                   mClass->mMaxCrashRollRate > 5.0f ? mClass->mMaxCrashRollRate : 5.0f, -1);
        group.ReadFixed(mBankPitchAngle, -6.2831854820251465f, 6.2831854820251465f, 16);
        group.ReadFixed(mBankRollAngle, -10.210176467895508f, 10.210176467895508f, 16);
        ReadUnitScalar(group, mFlightRatio);
        mEnergyBar.Read(group);
        group.ReadFixed(mSetSpeed, mClass->mMinFlyerSpeed > 0.0f ? 0.0f : mClass->mMinFlyerSpeed,
                        mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                          : mClass->mBoostFlyerSpeed, 16);
        group.ReadFixed(mSetStrafe, -mClass->mStrafeSpeed - 30.0f, mClass->mStrafeSpeed + 30.0f, 16);
        group.ReadFixed(mSetStrafeVertical, -mClass->mStrafeSpeed - 30.0f,
                        mClass->mStrafeSpeed + 30.0f, 16);
        group.ReadFixed(mWorldRoll, -1.0f, 1.0f, 16);
        group.ReadFixed(mAutoLevelingAllowed, 0.0f, 15.0f, 16);
        ReadThrust(group, mPitchControllerPrevUpdate);
        ReadThrust(group, mTurnControllerPrevUpdate);
        group.ReadFixed(mTrick, -1.0f, 1.0f, 9);
        group.ReadBits(*(unsigned int*)&mTrickDoubleclick, 32);
        group.ReadBits(value, 1);
        mInRoll = value != 0;
        group.ReadBits(value, 1);
        mInFlip = value != 0;
        ReadAngle(group, mRoll);
        group.ReadFixed(mRollTarget, -13.35176944732666f, 13.35176944732666f, 16);
        float side;
        group.ReadFixed(side, -1.0f, 1.0f, 16);
        mSideRoll.x = side;
        group.ReadFixed(side, -1.0f, 1.0f, 16);
        mSideRoll.y = side;
        mSideRoll.z = 0.0f;
        if (mInFlip)
        {
            ReadAngle(group, mFlip);
            ReadAngle(group, mFlipTarget);
        }
        else
        {
            mFlip = 0.0f;
            mFlipTarget = 0.0f;
        }
        if (mPlayerId < 0)
        {
            group.ReadBits(value, 1);
            mFollowingPath = value != 0;
            group.ReadFixed(mAIEng.ai.mFreeFlyTime, 0.0f, mClass->mFreeFlyTime + 1.0f, 16);
            EntityPathFollower::ReadObject(
                group, pState, mClass->mMinFlyerSpeed > 0.0f ? 0.0f : mClass->mMinFlyerSpeed,
                mClass->mBoostFlyerSpeed > 0.0f ? mClass->mBoostFlyerSpeed : mClass->mMaxFlyerSpeed);
        }
        ReadThrust(group, mControlMove);
        ReadThrust(group, mControlStrafe);
        ReadThrust(group, mControlTurn);
        ReadThrust(group, mControlPitch);
        group.ReadFixed(mPitchBuildup, -1.0f, 1.0f, 10);
        if (mPlayerId >= 0)
        {
            ReadThrust(group, mTurnBuildup);
            group.ReadBits(value, 1);
            mTargetInfo.mIsAiming = value != 0;
            if (NetGame::IsFullState() || NetGame::GetJoystickIndex(mPlayerId) >= 0)
            {
                ReadTrigger(group, &mControlSprint);
                ReadTrigger(group, &mControlThrustFwd);
                if (mWeapon[0])
                {
                    ReadWeapon(group, mWeapon[0]);
                }
                if (mWeapon[1])
                {
                    ReadWeapon(group, mWeapon[1]);
                }
                if (mCharacter)
                {
                    mCharacter->Read(group);
                }
            }
            group.ReadBits(value, 1);
            SetTargetLockedObj(value ? ReadObjectPtr(group) : 0);
        }
        for (int i = 0; i < mNumTurrets; ++i)
        {
            mTurret[i]->Read(group);
        }
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005f3500
void EntityFlyer::WriteLateState(NetPktGroup& group)
{
    WriteMatrix(group, GetMatrix());
    group.WriteBits(mState, 3);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive ? 1u : 0u, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded ? 1u : 0u, 1);
    }
    if (mState == LANDED)
    {
        return;
    }
    WriteVelocity(group, mVelocity,
               mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                 : mClass->mBoostFlyerSpeed, -1);
    WriteVelocity(group, mOmega,
               mClass->mMaxCrashRollRate > 5.0f ? mClass->mMaxCrashRollRate : 5.0f, -1);
    group.WriteFixed(mBankPitchAngle, -6.2831854820251465f, 6.2831854820251465f, 16);
    group.WriteFixed(mBankRollAngle, -10.210176467895508f, 10.210176467895508f, 16);
    WriteUnitScalar(group, mFlightRatio);
    mEnergyBar.Write(group);
    group.WriteFixed(mSetSpeed, mClass->mMinFlyerSpeed > 0.0f ? 0.0f : mClass->mMinFlyerSpeed,
                     mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                       : mClass->mBoostFlyerSpeed, 16);
    group.WriteFixed(mSetStrafe, -mClass->mStrafeSpeed - 30.0f, mClass->mStrafeSpeed + 30.0f, 16);
    group.WriteFixed(mSetStrafeVertical, -mClass->mStrafeSpeed - 30.0f,
                     mClass->mStrafeSpeed + 30.0f, 16);
    group.WriteFixed(mWorldRoll, -1.0f, 1.0f, 16);
    group.WriteFixed(mAutoLevelingAllowed, 0.0f, 15.0f, 16);
    WriteThrust(group, mPitchControllerPrevUpdate);
    WriteThrust(group, mTurnControllerPrevUpdate);
    group.WriteFixed(mTrick, -1.0f, 1.0f, 9);
    group.WriteBits(*(const unsigned int*)&mTrickDoubleclick, 32);
    group.WriteBits(mInRoll ? 1u : 0u, 1);
    group.WriteBits(mInFlip ? 1u : 0u, 1);
    WriteAngle(group, mRoll);
    group.WriteFixed(mRollTarget, -13.35176944732666f, 13.35176944732666f, 16);
    group.WriteFixed(mSideRoll.x, -1.0f, 1.0f, 16);
    group.WriteFixed(mSideRoll.y, -1.0f, 1.0f, 16);
    if (mInFlip)
    {
        WriteAngle(group, mFlip);
        WriteAngle(group, mFlipTarget);
    }
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    WriteThrust(group, mTurnBuildup);
    group.WriteBits(mTargetInfo.mIsAiming ? 1u : 0u, 1);
    for (int i = 0; i < mNumTurrets; ++i)
    {
        ((MountedTurret&)(*mTurret[i])).WriteLateState(group);
    }
}

// FUNCTION: SWBF2 0x005f3910
void EntityFlyer::ReadLateState(NetPktGroup& group)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    unsigned int value = 0;
    group.ReadBits(value, 3);
    mState = (State)value;
    float health;
    group.ReadFixed(health, 0.0f, mMaxHealth, 11);
    SetCurHealth(health, true, 0);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    if (mState == LANDED)
    {
        return;
    }
    ReadVelocity(group, mVelocity,
               mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                 : mClass->mBoostFlyerSpeed, -1);
    ReadVelocity(group, mOmega,
               mClass->mMaxCrashRollRate > 5.0f ? mClass->mMaxCrashRollRate : 5.0f, -1);
    group.ReadFixed(mBankPitchAngle, -6.2831854820251465f, 6.2831854820251465f, 16);
    group.ReadFixed(mBankRollAngle, -10.210176467895508f, 10.210176467895508f, 16);
    ReadUnitScalar(group, mFlightRatio);
    mEnergyBar.Read(group);
    group.ReadFixed(mSetSpeed, mClass->mMinFlyerSpeed > 0.0f ? 0.0f : mClass->mMinFlyerSpeed,
                    mClass->mMaxFlyerSpeed > mClass->mBoostFlyerSpeed ? mClass->mMaxFlyerSpeed
                                                                      : mClass->mBoostFlyerSpeed, 16);
    group.ReadFixed(mSetStrafe, -mClass->mStrafeSpeed - 30.0f, mClass->mStrafeSpeed + 30.0f, 16);
    group.ReadFixed(mSetStrafeVertical, -mClass->mStrafeSpeed - 30.0f,
                    mClass->mStrafeSpeed + 30.0f, 16);
    group.ReadFixed(mWorldRoll, -1.0f, 1.0f, 16);
    group.ReadFixed(mAutoLevelingAllowed, 0.0f, 15.0f, 16);
    ReadThrust(group, mPitchControllerPrevUpdate);
    ReadThrust(group, mTurnControllerPrevUpdate);
    group.ReadFixed(mTrick, -1.0f, 1.0f, 9);
    group.ReadBits(*(unsigned int*)&mTrickDoubleclick, 32);
    group.ReadBits(value, 1);
    mInRoll = value != 0;
    group.ReadBits(value, 1);
    mInFlip = value != 0;
    ReadAngle(group, mRoll);
    group.ReadFixed(mRollTarget, -13.35176944732666f, 13.35176944732666f, 16);
    float side;
    group.ReadFixed(side, -1.0f, 1.0f, 16);
    mSideRoll.x = side;
    group.ReadFixed(side, -1.0f, 1.0f, 16);
    mSideRoll.y = side;
    mSideRoll.z = 0.0f;
    if (mInFlip)
    {
        ReadAngle(group, mFlip);
        ReadAngle(group, mFlipTarget);
    }
    else
    {
        mFlip = 0.0f;
        mFlipTarget = 0.0f;
    }
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    ReadThrust(group, mTurnBuildup);
    group.ReadBits(value, 1);
    mTargetInfo.mIsAiming = value != 0;
    for (int i = 0; i < mNumTurrets; ++i)
    {
        ((MountedTurret&)(*mTurret[i])).ReadLateState(group);
    }
}

// FUNCTION: SWBF2 0x005ede90
void EntityDroideka::ReadObject(NetPktGroup& group, NetObjState*)
{
    if (!NetGame::IsFullState())
    {
        ReadNetLOD(group);
    }
    unsigned int value = 0;
    group.ReadBits(value, 1);
    if (!value)
    {
        if (mCtrl)
        {
            ClearControllable();
            ReleaseController();
        }
    }
    else if (!mCtrl)
    {
        ClearControllable();
        SetupController();
    }
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    ((EnergyBar*)mEnergyBar)->Read(group);
    group.ReadBits(value, 2);
    mDirection = (eDirection)value;
    group.ReadBits(value, 2);
    if (UnitControllerPtr(mCtrl))
    {
        ((AILowLevel*)UnitControllerPtr(mCtrl)->mLowLevel)->mFoldStatus = (FoldStatus)value;
    }
    group.ReadBits(value, 4);
    if ((int)value == mState)
    {
        ZephyrAnim* pAnim = mClass->mAnimations[mState == Dead ? Dying : mState];
        group.ReadFixed(mTime, 0.0f, 10.0f, 24);
        mZephyrPoseDyn.SetLoop(0);
        mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
        if (mDirection == Backward)
        {
            mZephyrPoseDyn.PlayAnimBackward();
            mZephyrPoseDyn.SetCurTime(1.0f);
            mZephyrPoseDyn.Update(mClass->mTimeTotals[mState] - mTime);
        }
        else
        {
            mZephyrPoseDyn.PlayAnimForward();
            mZephyrPoseDyn.SetCurTime(0.0f);
            mZephyrPoseDyn.Update(mTime);
        }
    }
    else
    {
        if (value == Rolling)
        {
            SetCollisionModel(&mClass->mCollisionBall);
        }
        else if (mState == Rolling)
        {
            SetCollisionModel(&mClass->mCollisionModel);
        }
        mState = value;
        if (value != Dead)
        {
            ZephyrAnim* pAnim = mClass->mAnimations[value];
            group.ReadFixed(mTime, 0.0f, 10.0f, 24);
            mZephyrPoseDyn.SetLoop(0);
            mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
            if (mDirection == Backward)
            {
                mZephyrPoseDyn.PlayAnimBackward();
                mZephyrPoseDyn.SetCurTime(1.0f);
                mZephyrPoseDyn.Update(mClass->mTimeTotals[mState] - mTime);
            }
            else
            {
                mZephyrPoseDyn.PlayAnimForward();
                mZephyrPoseDyn.SetCurTime(0.0f);
                mZephyrPoseDyn.Update(mTime);
            }
        }
        else
        {
            float fTime;
            group.ReadFixed(fTime, 0.0f, 10.0f, 24);
            ZephyrAnim* pAnim = mClass->mAnimations[Dying];
            mTime = mClass->mTimeTotals[Dying];
            mZephyrPoseDyn.SetLoop(0);
            mZephyrPoseDyn.PlayAnimForward();
            mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
            mZephyrPoseDyn.SetCurTime(0.0f);
            mZephyrPoseDyn.Update(mTime);
        }
    }
    ReadCosSin(group, (float*)&mTurnOffset);
    ReadCosSin(group, (float*)&mPitchOffset);
    BuildWorldMatrices();
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    group.ReadFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value != 0;
    }
    group.ReadBits(*(unsigned int*)&mPitchValue, 32);
    group.ReadBits(*(unsigned int*)&mBallAngularVelocity, 32);
    ReadCosSin(group, (float*)&mBallOrientation);
    ReadCosSin(group, (float*)&mBallLean);
    ReadPilots(group, *this);
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffOffenseTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffDefenseTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mBuffHealthTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
        ReadDamageOwner(group, mDebuffDamageOwner);
    }
    else
    {
        mDebuffDamageTimer = 0.0f;
    }
    if (mPlayerId >= 0)
    {
        group.ReadBits(value, 1);
        mLastCollidedWithNonTerrain = value != 0;
        ReadPositionVector(group, mLastPosition);
        ReadUnitVector(group, mGroundNormal);
        float turn;
        ReadThrust(group, turn);
        if (mCtrl)
        {
            ((PlayerController*)mCtrl)->mMouseTurnBuildup = turn;
        }
        ReadThrust(group, mTurnBuildup);
        group.ReadBits(value, 1);
        mTargetInfo.mIsAiming = value != 0;
        group.ReadFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
        group.ReadFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
        group.ReadFixed(mTurnAuto, -1.0f, 1.0f, 16);
        group.ReadFixed(mPitchAuto, -1.0f, 1.0f, 16);
        group.ReadBits(value, 1);
        mUsingTurnAdjusted = value != 0;
        group.ReadBits(value, 1);
        SetTargetLockedObj(value ? ReadObjectPtr(group) : 0);
        if (NetGame::IsFullState() || NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            ReadTrigger(group, &mControlSprint);
            ReadTrigger(group, &mControlThrustFwd);
            group.ReadBits(value, 1);
            mFallTimer = value ? 0.1f : 0.0f;
            ReadTrigger(group, &mControlJump);
            ReadTrigger(group, &mControlZoom);
            for (int i = 0; i < 2; ++i)
            {
                if (mWeaponIndex[i] < 0)
                {
                    continue;
                }
                ReadTrigger(group, &mControlFire[i]);
                ReadWeapon(group, mWeapon[mWeaponIndex[i]]);
            }
            if (mCharacter)
            {
                mCharacter->Read(group);
            }
        }
    }
    PropagateTeam(mCharacter, mTeam);
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005f47c0
void EntityDroideka::WriteLateState(NetPktGroup& group)
{
    const PblMatrix& matrix = static_cast<CollisionObject*>(this)->GetMatrix();
    group.WriteBits(mCtrl != 0, 1);
    WriteMatrix(group, matrix);
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    group.WriteBits(mDirection, 2);
    if (UnitControllerPtr(mCtrl))
    {
        group.WriteBits(((AILowLevel*)UnitControllerPtr(mCtrl)->mLowLevel)->mFoldStatus, 2);
    }
    else
    {
        group.WriteBits(0, 2);
    }
    group.WriteBits(mState, 4);
    group.WriteFixed(mTime, 0.0f, 10.0f, 24);
    WriteCosSin(group, (const float*)&mTurnOffset);
    WriteCosSin(group, (const float*)&mPitchOffset);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    group.WriteBits(*(const unsigned int*)&mPitchValue, 32);
    group.WriteBits(*(const unsigned int*)&mBallAngularVelocity, 32);
    WriteCosSin(group, (const float*)&mBallOrientation);
    WriteCosSin(group, (const float*)&mBallLean);
    const bool bOffense = mBuffOffenseTimer > 0.0f;
    group.WriteBits(bOffense, 1);
    if (bOffense)
    {
        group.WriteFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    const bool bDefense = mBuffDefenseTimer > 0.0f;
    group.WriteBits(bDefense, 1);
    if (bDefense)
    {
        group.WriteFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    const bool bHealth = mBuffHealthTimer > 0.0f;
    group.WriteBits(bHealth, 1);
    if (bHealth)
    {
        group.WriteFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    const bool bDebuff = mDebuffDamageTimer > 0.0f;
    group.WriteBits(bDebuff, 1);
    if (bDebuff)
    {
        group.WriteFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
    }
    group.WriteBits(mLastCollidedWithNonTerrain, 1);
    WritePositionVector(group, mLastPosition);
    WriteUnitVector(group, mGroundNormal);
    WriteThrust(group, mCtrl ? ((PlayerController*)mCtrl)->GetMouseBuildup() : 0.0f);
    WriteThrust(group, mTurnBuildup);
    group.WriteBits(mTargetInfo.mIsAiming, 1);
    group.WriteFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(mTurnAuto, -1.0f, 1.0f, 16);
    group.WriteFixed(mPitchAuto, -1.0f, 1.0f, 16);
    group.WriteBits(mUsingTurnAdjusted, 1);
}

// FUNCTION: SWBF2 0x005f4ba0
void EntityDroideka::ReadLateState(NetPktGroup& group)
{
    unsigned int value = 0;
    group.ReadBits(value, 1);
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    Move(matrix);
    const PblVector3& position = GetPosition();
    memcpy(&mLastPosition, &position, sizeof(position));
    ReadVelocity(group, mVelocity, 5120.0f, -1);
    group.ReadBits(value, 2);
    mDirection = (eDirection)value;
    group.ReadBits(value, 2);
    if (UnitControllerPtr(mCtrl))
    {
        ((AILowLevel*)UnitControllerPtr(mCtrl)->mLowLevel)->mFoldStatus = (FoldStatus)value;
    }
    group.ReadBits(value, 4);
    if ((int)value == mState)
    {
        ZephyrAnim* pAnim = mClass->mAnimations[mState == Dead ? Dying : mState];
        group.ReadFixed(mTime, 0.0f, 10.0f, 24);
        mZephyrPoseDyn.SetLoop(0);
        mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
        if (mDirection == Backward)
        {
            mZephyrPoseDyn.PlayAnimBackward();
            mZephyrPoseDyn.SetCurTime(1.0f);
            mZephyrPoseDyn.Update(mClass->mTimeTotals[mState] - mTime);
        }
        else
        {
            mZephyrPoseDyn.PlayAnimForward();
            mZephyrPoseDyn.SetCurTime(0.0f);
            mZephyrPoseDyn.Update(mTime);
        }
    }
    else
    {
        mState = value;
        if (value != Dead)
        {
            ZephyrAnim* pAnim = mClass->mAnimations[value];
            group.ReadFixed(mTime, 0.0f, 10.0f, 24);
            mZephyrPoseDyn.SetLoop(0);
            mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
            if (mDirection == Backward)
            {
                mZephyrPoseDyn.PlayAnimBackward();
                mZephyrPoseDyn.SetCurTime(1.0f);
                mZephyrPoseDyn.Update(mClass->mTimeTotals[mState] - mTime);
            }
            else
            {
                mZephyrPoseDyn.PlayAnimForward();
                mZephyrPoseDyn.SetCurTime(0.0f);
                mZephyrPoseDyn.Update(mTime);
            }
        }
        else
        {
            float fTime;
            group.ReadFixed(fTime, 0.0f, 10.0f, 24);
            ZephyrAnim* pAnim = mClass->mAnimations[Dying];
            mTime = mClass->mTimeTotals[Dying];
            mZephyrPoseDyn.SetLoop(0);
            mZephyrPoseDyn.PlayAnimForward();
            mZephyrPoseDyn.SetAnimation(pAnim, 30.0f);
            mZephyrPoseDyn.SetCurTime(0.0f);
            mZephyrPoseDyn.Update(mTime);
        }
    }
    ReadCosSin(group, (float*)&mTurnOffset);
    ReadCosSin(group, (float*)&mPitchOffset);
    BuildWorldMatrices();
    ReadThrust(group, mControlMove);
    ReadThrust(group, mControlStrafe);
    ReadThrust(group, mControlTurn);
    ReadThrust(group, mControlPitch);
    group.ReadBits(*(unsigned int*)&mPitchValue, 32);
    group.ReadBits(*(unsigned int*)&mBallAngularVelocity, 32);
    ReadCosSin(group, (float*)&mBallOrientation);
    ReadCosSin(group, (float*)&mBallLean);
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffOffenseTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffDefenseTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mBuffHealthTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mDebuffDamageTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    mLastCollidedWithNonTerrain = value != 0;
    ReadPositionVector(group, mLastPosition);
    ReadUnitVector(group, mGroundNormal);
    float turn;
    ReadThrust(group, turn);
    if (mCtrl)
    {
        ((PlayerController*)mCtrl)->mMouseTurnBuildup = turn;
    }
    ReadThrust(group, mTurnBuildup);
    group.ReadBits(value, 1);
    mTargetInfo.mIsAiming = value != 0;
    group.ReadFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
    group.ReadFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
    group.ReadFixed(mTurnAuto, -1.0f, 1.0f, 16);
    group.ReadFixed(mPitchAuto, -1.0f, 1.0f, 16);
    group.ReadBits(value, 1);
    mUsingTurnAdjusted = value != 0;
}

extern PblVector3 gCameraFrontPos;

namespace GameVoiceChat { void PlayerChange(); }

void WriteDamageOwner(NetPktGroup& group, const DamageOwner& owner);

// GLOBAL: SWBF2 0x01e62f64
int gSoldierLodCount;

// FUNCTION: SWBF2 0x005ed850
void EntityDroideka::WriteObject(NetPktGroup& group, NetObjState*)
{
    const int clientId = NetGame::GetCurDst();
    if (clientId == 1)
    {
        ++gDebugCount;
    }
    const PblMatrix& matrix = static_cast<CollisionObject*>(this)->GetMatrix();
    if (!NetGame::IsFullState())
    {
        SetNetLOD(ComputeNetLOD((int)&matrix.m[3][0], mPlayerId) != 0);
        SetNetPrecision(group);
    }
    group.WriteBits(mCtrl != 0, 1);
    WriteMatrix(group, matrix);
    WriteVelocity(group, mVelocity, 5120.0f, -1);
    ((EnergyBar*)mEnergyBar)->Write(group);
    group.WriteBits(mDirection, 2);
    if (UnitControllerPtr(mCtrl))
    {
        group.WriteBits(((AILowLevel*)UnitControllerPtr(mCtrl)->mLowLevel)->mFoldStatus, 2);
    }
    else
    {
        group.WriteBits(0, 2);
    }
    group.WriteBits(mState, 4);
    group.WriteFixed(mTime, 0.0f, 10.0f, 24);
    WriteCosSin(group, (const float*)&mTurnOffset);
    WriteCosSin(group, (const float*)&mPitchOffset);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded, 1);
    }
    group.WriteBits(*(const unsigned int*)&mPitchValue, 32);
    group.WriteBits(*(const unsigned int*)&mBallAngularVelocity, 32);
    WriteCosSin(group, (const float*)&mBallOrientation);
    WriteCosSin(group, (const float*)&mBallLean);
    WritePilots(group, *this);
    const bool bOffense = mBuffOffenseTimer > 0.0f;
    group.WriteBits(bOffense, 1);
    if (bOffense)
    {
        group.WriteFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    const bool bDefense = mBuffDefenseTimer > 0.0f;
    group.WriteBits(bDefense, 1);
    if (bDefense)
    {
        group.WriteFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    const bool bHealth = mBuffHealthTimer > 0.0f;
    group.WriteBits(bHealth, 1);
    if (bHealth)
    {
        group.WriteFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    const bool bDebuff = mDebuffDamageTimer > 0.0f;
    group.WriteBits(bDebuff, 1);
    if (bDebuff)
    {
        group.WriteFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
        WriteDamageOwner(group, mDebuffDamageOwner);
    }
    if (mPlayerId >= 0)
    {
        group.WriteBits(mLastCollidedWithNonTerrain, 1);
        WritePositionVector(group, mLastPosition);
        WriteUnitVector(group, mGroundNormal);
        WriteThrust(group, mCtrl ? ((PlayerController*)mCtrl)->GetMouseBuildup() : 0.0f);
        WriteThrust(group, mTurnBuildup);
        group.WriteBits(mTargetInfo.mIsAiming, 1);
        group.WriteFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
        group.WriteFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
        group.WriteFixed(mTurnAuto, -1.0f, 1.0f, 16);
        group.WriteFixed(mPitchAuto, -1.0f, 1.0f, 16);
        group.WriteBits(mUsingTurnAdjusted, 1);
        if (mTargetLockedObj && GetUniqObjectId(mTargetLockedObj) != 63)
        {
            group.WriteBits(1, 1);
            WriteObjectPtr(group, mTargetLockedObj);
        }
        else
        {
            group.WriteBits(0, 1);
        }
        if (NetGame::IsFullState() || NetGame::IsLocalTo(clientId, mPlayerId))
        {
            WriteTrigger(group, const_cast<Trigger*>(&mControlSprint));
            WriteTrigger(group, const_cast<Trigger*>(&mControlThrustFwd));
            group.WriteBits(mFallTimer >= 0.1f, 1);
            WriteTrigger(group, const_cast<Trigger*>(&mControlJump));
            WriteTrigger(group, const_cast<Trigger*>(&mControlZoom));
            for (int i = 0; i < 2; ++i)
            {
                if (mWeaponIndex[i] < 0)
                {
                    continue;
                }
                WriteTrigger(group, const_cast<Trigger*>(&mControlFire[i]));
                WriteWeapon(group, mWeapon[mWeaponIndex[i]]);
            }
            if (mCharacter)
            {
                mCharacter->Write(group);
            }
        }
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005e9660
void EntitySoldier::WriteObject(NetPktGroup& group, NetObjState*)
{
    const int clientId = NetGame::GetCurDst();
    const PblMatrix& matrix = static_cast<CollisionObject*>(this)->GetMatrix();
    if (!NetGame::IsFullState())
    {
        const PblVector3& position = *(const PblVector3*)&matrix.m[3][0];
        int precision = ComputeNetLOD((int)&position, mPlayerId);
        if (!precision && gSoldierLodCount < 3)
        {
            const float dx = gCameraFrontPos.x - position.x;
            const float dz = gCameraFrontPos.z - position.z;
            if (dz * dz + dx * dx < 625.0f)
            {
                precision = 1;
                ++gSoldierLodCount;
            }
        }
        SetNetLOD(precision != 0);
        SetNetPrecision(group);
    }
    const bool bHasController = mCtrl != 0;
    group.WriteBits(bHasController, 1);
    const bool bActive = static_cast<Controllable*>(this)->IsThreadActive();
    group.WriteBits(bActive, 1);
    if (bActive)
    {
        bool bFullMatrix = true;
        PblQuaternion q(matrix);
        if (fabs(q._fX) < 0.0001f && fabs(q._fZ) < 0.0001f)
        {
            bFullMatrix = false;
        }
        group.WriteBits(bFullMatrix, 1);
        if (bFullMatrix)
        {
            WriteMatrix(group, matrix);
        }
        else
        {
            WriteFixed(quatConvert, group, q._fY);
            WriteFixed(quatConvert, group, q._fS);
            WritePositionVector(group, *(const PblVector3*)&matrix.m[3][0]);
        }
        WriteVelocity(group, mVelocity,
                   mClass->mJumpVerticalSpeed * 2.0f + mClass->mMaxSpeed + mClass->mJetJump, -1);
        ((EnergyBar*)mEnergyBar)->Write(group);
        group.WriteBits(mActiveIndex + 1, 4);
        group.WriteBits(mActiveChannel + 1, 2);
    }
    group.WriteBits(mState, 4);
    group.WriteBits((mFlags & flags_isGrounded) != 0, 1);
    group.WriteBits((mFlags & flags_fallTimer) != 0, 1);
    group.WriteFixed(mStateTimer, 0.0f, 8.0f, 13);
    group.WriteBits(mStanceIndex, 4);
    group.WriteBits(_bActive, 1);
    int collision = 0;
    if (IsInCollisionList())
    {
        collision = static_cast<CollisionObject*>(this)->GetObjectData(0) == COLL_SOFT ? 1 : 2;
    }
    group.WriteBits(collision, 2);
    group.WriteBits(mTargetInfo.mIsAiming, 1);
    group.WriteBits(mTeam, 2);
    group.WriteBits(mPerceivedTeam, 2);
    WriteCharacter(group, mCharacter);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    WriteCharacter(group, mDamageOwner.mCharacter);
    group.WriteFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.WriteBits(mIsAlive, 1);
    if (mMaxShield > 0.0f)
    {
        group.WriteFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.WriteBits(mIsShielded, 1);
    }
    group.WriteBits(*(const unsigned int*)&mInputLockTime, 32);
    WriteSwitch(group, mControlSwitch[0]);
    WriteTrigger(group, const_cast<Trigger*>(&mControlView));
    WriteTrigger(group, const_cast<Trigger*>(&mControlCrouch));
    WriteTrigger(group, const_cast<Trigger*>(&mControlSpecialMove));
    for (int i = 0; i < 2; ++i)
    {
        if (mWeaponIndex[i] >= 0)
        {
            group.WriteBits(mWeaponIndex[i], 3);
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        if (mWeaponIndex[i] < 0)
        {
            continue;
        }
        if (mWeapon[mWeaponIndex[i]]->IsRtti(WeaponInvisibility::rttiHashWeaponInvisibility) ||
            mWeapon[mWeaponIndex[i]]->IsRtti(WeaponDisguise::rttiHashWeaponDisguise))
        {
            mWeapon[mWeaponIndex[i]]->Write(group);
        }
    }
    group.WriteBits(m_uiInvisibilityAlpha, 8);
    if (mClass->mJetFuelRechargeRate > 0.0f)
    {
        WriteJetFuel(group, mJetFuel);
    }
    const bool bOffense = mBuffOffenseTimer > 0.0f;
    group.WriteBits(bOffense, 1);
    if (bOffense)
    {
        group.WriteFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    const bool bDefense = mBuffDefenseTimer > 0.0f;
    group.WriteBits(bDefense, 1);
    if (bDefense)
    {
        group.WriteFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    const bool bHealth = mBuffHealthTimer > 0.0f;
    group.WriteBits(bHealth, 1);
    if (bHealth)
    {
        group.WriteFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    const bool bDebuff = mDebuffDamageTimer > 0.0f;
    group.WriteBits(bDebuff, 1);
    if (bDebuff)
    {
        group.WriteFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
        WriteDamageOwner(group, mDebuffDamageOwner);
    }
    const bool bSmolder = mSmolderTimer > 0.0f;
    group.WriteBits(bSmolder, 1);
    if (bSmolder)
    {
        group.WriteFixed(mSmolderTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mSmolderDamageRate, 0.0f, 1000.0f, 16);
        WriteDamageOwner(group, mSmolderDamageOwner);
    }
    if (mPlayerId >= 0)
    {
        if (static_cast<Controllable*>(this)->IsThreadActive())
        {
            group.WriteBits(mLastCollidedWithNonTerrain, 1);
            WritePositionVector(group, mLastPosition);
            WriteUnitVector(group, mGroundNormal);
        }
        WriteThrust(group, mTurnBuildup);
        group.WriteFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
        group.WriteFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
        group.WriteFixed(mTurnAuto, -1.0f, 1.0f, 16);
        group.WriteFixed(mPitchAuto, -1.0f, 1.0f, 16);
        group.WriteBits(mUsingTurnAdjusted, 1);
        GameObject* pTarget = mTargetLockedObj;
        if (pTarget && (IsNeverDies(pTarget->GetEntityType()) ||
                        GetUniqObjectId(mTargetLockedObj) != 63))
        {
            group.WriteBits(1, 1);
            WriteObjectPtr(group, mTargetLockedObj);
        }
        else
        {
            group.WriteBits(0, 1);
        }
        if (NetGame::IsLocalTo(clientId, mPlayerId) || NetGame::IsFullState())
        {
            group.WriteBits(mCanChangeClass, 1);
            WriteTrigger(group, const_cast<Trigger*>(&mControlZoom));
            WriteTrigger(group, const_cast<Trigger*>(&mControlJump));
            WriteTrigger(group, const_cast<Trigger*>(&mControlSprint));
            WriteTrigger(group, const_cast<Trigger*>(&mControlThrustFwd));
            if (mCharacter)
            {
                mCharacter->Write(group);
            }
        }
        for (int i = 0; i < 2; ++i)
        {
            if (mWeaponIndex[i] >= 0)
            {
                WriteTrigger(group, const_cast<Trigger*>(&mControlFire[i]));
            }
        }
        group.WriteBits(mMarkedForDeath, 1);
        if (bHasController)
        {
            WriteThrust(group, ((PlayerController*)mCtrl)->GetMouseBuildup());
        }
    }
    if (netLOD == 1)
    {
        WriteAngle(group, mPitchValue);
        for (int i = 0; i < 2; ++i)
        {
            if (mWeaponIndex[i] >= 0)
            {
                WriteWeapon(group, mWeapon[mWeaponIndex[i]]);
            }
        }
    }
    MedalsMgr* pMedals = mCharacter ? mCharacter->mMedalsMgr : 0;
    if (pMedals)
    {
        group.WriteBits(1, 1);
        pMedals->WriteObject(group);
    }
    else
    {
        group.WriteBits(0, 1);
    }
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005f2910
void EntitySoldier::WriteLateState(NetPktGroup& group)
{
    const PblMatrix& matrix = static_cast<CollisionObject*>(this)->GetMatrix();
    const bool bHasController = mCtrl != 0;
    group.WriteBits(bHasController, 1);
    const bool bActive = static_cast<Controllable*>(this)->IsThreadActive();
    group.WriteBits(bActive, 1);
    if (bActive)
    {
        bool bFullMatrix = true;
        PblQuaternion q(matrix);
        if (fabs(q._fX) < 0.0001f && fabs(q._fZ) < 0.0001f)
        {
            bFullMatrix = false;
        }
        group.WriteBits(bFullMatrix, 1);
        if (bFullMatrix)
        {
            WriteMatrix(group, matrix);
        }
        else
        {
            WriteFixed(quatConvert, group, q._fY);
            WriteFixed(quatConvert, group, q._fS);
            WritePositionVector(group, *(const PblVector3*)&matrix.m[3][0]);
        }
        WriteVelocity(group, mVelocity,
                   mClass->mJumpVerticalSpeed * 2.0f + mClass->mMaxSpeed + mClass->mJetJump, -1);
    }
    group.WriteBits(mState, 4);
    group.WriteBits((mFlags & flags_isGrounded) != 0, 1);
    group.WriteBits((mFlags & flags_fallTimer) != 0, 1);
    group.WriteFixed(mStateTimer, 0.0f, 8.0f, 13);
    group.WriteBits(mStanceIndex, 4);
    group.WriteBits(mTargetInfo.mIsAiming, 1);
    WriteThrust(group, mControlMove);
    WriteThrust(group, mControlStrafe);
    WriteThrust(group, mControlTurn);
    WriteThrust(group, mControlPitch);
    WriteCharacter(group, mDamageOwner.mCharacter);
    group.WriteBits(*(const unsigned int*)&mInputLockTime, 32);
    WriteSwitch(group, mControlSwitch[0]);
    WriteTrigger(group, const_cast<Trigger*>(&mControlView));
    WriteTrigger(group, const_cast<Trigger*>(&mControlCrouch));
    WriteTrigger(group, const_cast<Trigger*>(&mControlSpecialMove));
    if (mClass->mJetFuelRechargeRate > 0.0f)
    {
        WriteJetFuel(group, mJetFuel);
    }
    const bool bOffense = mBuffOffenseTimer > 0.0f;
    group.WriteBits(bOffense, 1);
    if (bOffense)
    {
        group.WriteFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    const bool bDefense = mBuffDefenseTimer > 0.0f;
    group.WriteBits(bDefense, 1);
    if (bDefense)
    {
        group.WriteFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    const bool bHealth = mBuffHealthTimer > 0.0f;
    group.WriteBits(bHealth, 1);
    if (bHealth)
    {
        group.WriteFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    const bool bDebuff = mDebuffDamageTimer > 0.0f;
    group.WriteBits(bDebuff, 1);
    if (bDebuff)
    {
        group.WriteFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
    }
    const bool bSmolder = mSmolderTimer > 0.0f;
    group.WriteBits(bSmolder, 1);
    if (bSmolder)
    {
        group.WriteFixed(mSmolderTimer, 0.0f, 350.0f, 16);
        group.WriteFixed(mSmolderDamageRate, 0.0f, 1000.0f, 16);
    }
    if (static_cast<Controllable*>(this)->IsThreadActive())
    {
        group.WriteBits(mLastCollidedWithNonTerrain, 1);
        WritePositionVector(group, mLastPosition);
        WriteUnitVector(group, mGroundNormal);
    }
    WriteThrust(group, mTurnBuildup);
    group.WriteFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
    group.WriteFixed(mTurnAuto, -1.0f, 1.0f, 16);
    group.WriteFixed(mPitchAuto, -1.0f, 1.0f, 16);
    group.WriteBits(mUsingTurnAdjusted, 1);
    WriteTrigger(group, const_cast<Trigger*>(&mControlZoom));
    WriteTrigger(group, const_cast<Trigger*>(&mControlJump));
    if (bHasController)
    {
        WriteThrust(group, ((PlayerController*)mCtrl)->GetMouseBuildup());
    }
    WriteAngle(group, mPitchValue);
}

// FUNCTION: SWBF2 0x005ea140
void EntitySoldier::ReadObject(NetPktGroup& group, NetObjState*)
{
    if (!NetGame::IsFullState())
    {
        ReadNetLOD(group);
    }
    unsigned int value = 0;
    group.ReadBits(value, 1);
    const bool bHasController = value != 0;
    group.ReadBits(value, 1);
    const bool bActive = value != 0;
    if (bActive)
    {
        PblMatrix matrix;
        group.ReadBits(value, 1);
        if (value)
        {
            ReadMatrix(group, matrix);
        }
        else
        {
            PblQuaternion q;
            float y;
            float s;
            ReadFixed(quatConvert, group, y);
            ReadFixed(quatConvert, group, s);
            PblVector3 position;
            ReadPositionVector(group, position);
            q._fX = 0.0f;
            q._fY = y;
            q._fZ = 0.0f;
            q._fS = s;
            PblQuaternion unit;
            PblMatrix rotation;
            D3DXMatrixRotationQuaternion(&rotation, D3DXQuaternionNormalize(&unit, &q));
            matrix = rotation;
            matrix.m[3][0] = position.x;
            matrix.m[3][1] = position.y;
            matrix.m[3][2] = position.z;
            matrix.m[3][3] = 1.0f;
        }
        PblVector3 velocity;
        ReadVelocity(group, velocity,
                   mClass->mJumpVerticalSpeed * 2.0f + mClass->mMaxSpeed + mClass->mJetJump, -1);
        Move(matrix, velocity);
        const PblVector3& position = static_cast<CollisionObject*>(this)->GetPosition();
        memcpy(&mLastPosition, &position, sizeof(position));
        ((EnergyBar*)mEnergyBar)->Read(group);
        const int iOldIndex = mActiveIndex;
        const int iOldChannel = mActiveChannel;
        group.ReadBits(value, 4);
        mActiveIndex = (char)(value - 1);
        group.ReadBits(value, 2);
        mActiveChannel = (char)(value - 1);
        if (mActiveChannel != iOldChannel || mActiveIndex != iOldIndex)
        {
            if (iOldIndex >= 0)
            {
                mWeapon[iOldIndex]->Deselect(iOldChannel, false);
            }
            if (mActiveIndex >= 0)
            {
                const PblMatrix& m = mMatrix;
                PblVector3 localVelocity;
                localVelocity.x = mVelocity.x * m.m[0][0] + mVelocity.y * m.m[0][1] + mVelocity.z * m.m[0][2];
                localVelocity.y = mVelocity.x * m.m[1][0] + mVelocity.y * m.m[1][1] + mVelocity.z * m.m[1][2];
                localVelocity.z = mVelocity.x * m.m[2][0] + mVelocity.y * m.m[2][1] + mVelocity.z * m.m[2][2];
                mWeapon[mActiveIndex]->Select(mActiveChannel, false);
                mWeapon[mActiveIndex]->NotifySoldierState(mState, mStateTimer, mInputLockTime > 0.0f,
                                                          mTargetInfo.mIsAiming, localVelocity);
            }
        }
    }
    group.ReadBits(value, 4);
    mNetJustChangedState = (SoldierState::SoldierState)value != mState;
    mState = (SoldierState::SoldierState)value;
    if (value == SoldierState::CHOKE)
    {
        mAnimationHash = 0x144ebf32;
    }
    unsigned int grounded = 0;
    group.ReadBits(grounded, 1);
    group.ReadBits(value, 1);
    const bool bFallTimer = value != 0;
    group.ReadFixed(mStateTimer, 0.0f, 8.0f, 13);
    group.ReadBits(value, 4);
    mStanceIndex = value;
    if (grounded)
    {
        mFlags |= flags_isGrounded;
    }
    else
    {
        mFlags &= ~flags_isGrounded;
    }
    if (bFallTimer)
    {
        mFlags |= flags_fallTimer;
    }
    else
    {
        mFlags &= ~flags_fallTimer;
    }
    const bool bThreadActive = static_cast<Controllable*>(this)->IsThreadActive();
    if (bActive)
    {
        if (!bThreadActive)
        {
            ActivatePhysics();
            TargetManager::AddTarget(this, true);
        }
    }
    else if (bThreadActive)
    {
        DeactivatePhysics();
        TargetManager::RemoveTarget(this);
    }
    if (!bHasController)
    {
        if (mCtrl)
        {
            ClearControllable();
            ReleaseController();
        }
    }
    else if (!mCtrl)
    {
        ClearControllable();
        SetupController();
        if (mCharacter && mCharacter->mRemote)
        {
            mCharacter->mRemote->SetPilot(0);
            mCharacter->mRemote->mInitialPlayerId = -1;
            mCharacter->SetRemote(0);
        }
        if (mCharacter && mCharacter->mVehicle)
        {
            mCharacter->mVehicle->SetPilot(0);
            mCharacter->mVehicle->mInitialPlayerId = -1;
            mCharacter->SetVehicle(0);
        }
    }
    RedSceneObject* pScene = static_cast<RedSceneObject*>(this);
    group.ReadBits(value, 1);
    if (value)
    {
        if (!_bActive)
        {
            pScene->Activate();
        }
    }
    else if (_bActive)
    {
        pScene->Deactivate();
    }
    group.ReadBits(value, 2);
    CollisionObject* pCollision = static_cast<CollisionObject*>(this);
    const bool bInList = pCollision->IsInCollisionList();
    if (value == 0)
    {
        if (bInList)
        {
            pCollision->RemoveFromCollisionManager();
        }
    }
    else if (value == 1)
    {
        if (!bInList || pCollision->GetObjectData(0) != COLL_SOFT)
        {
            if (bInList)
            {
                pCollision->RemoveFromCollisionManager();
            }
            pCollision->AddSoftToCollisionManager();
        }
    }
    else if (!bInList || pCollision->GetObjectData(0) != COLL_ATTACHED)
    {
        if (bInList)
        {
            pCollision->RemoveFromCollisionManager();
        }
        pCollision->AddAttachedToCollisionManager();
    }
    group.ReadBits(value, 1);
    mTargetInfo.mIsAiming = value != 0;
    group.ReadBits(value, 2);
    SetTeam(value);
    group.ReadBits(value, 2);
    mPerceivedTeam = value;
    Character* pNewCharacter = ReadCharacter(group);
    Character* pOldCharacter = mCharacter;
    Controllable* pSelf = static_cast<Controllable*>(this);
    bool bChanged = true;
    if (pNewCharacter)
    {
        if (pNewCharacter->mUnit != pSelf)
        {
            pNewCharacter->SetUnit(pSelf);
        }
    }
    else if (pOldCharacter)
    {
        const int iJoystick = NetGame::GetJoystickIndex(mInitialPlayerId);
        if (iJoystick >= 0 && Tracker::GetTracker(mInitialPlayerId)->IsFirstPersonView())
        {
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_SOLDIER, -2, 0, 0);
        }
        Controllable* pRemote = pOldCharacter->mRemote;
        if (pRemote && pRemote->GetPilot() == pSelf)
        {
            pRemote->SetPilot(0);
            pOldCharacter->SetRemote(0);
        }
        Controllable* pVehicle = pOldCharacter->mVehicle;
        if (pVehicle && pVehicle->GetPilot() == pSelf)
        {
            pVehicle->SetPilot(0);
            pOldCharacter->SetVehicle(0);
        }
        pOldCharacter->SetUnit(0);
    }
    else
    {
        bChanged = false;
    }
    if (bChanged && pNewCharacter != pOldCharacter)
    {
        GameVoiceChat::PlayerChange();
        if (pOldCharacter)
        {
            Controllable* pRemote = pOldCharacter->mRemote;
            if (pRemote)
            {
                pOldCharacter->SetRemote(0);
                if (pNewCharacter)
                {
                    pNewCharacter->SetRemote(pRemote);
                }
                pRemote->ClearControllable();
                pRemote->ReleaseController();
                pRemote->SetCharacter(pNewCharacter);
            }
            Controllable* pVehicle = pOldCharacter->mVehicle;
            if (pVehicle)
            {
                pOldCharacter->SetVehicle(0);
                if (pNewCharacter)
                {
                    pNewCharacter->SetVehicle(pVehicle);
                }
                pVehicle->ClearControllable();
                pVehicle->ReleaseController();
                pVehicle->SetCharacter(pNewCharacter);
                TurnOffJetEffect();
            }
        }
        if (bHasController)
        {
            ClearControllable();
            ReleaseController();
        }
        SetCharacter(pNewCharacter);
        if (bHasController)
        {
            SetupController();
        }
    }
    float fSwitch;
    ReadThrust(group, fSwitch);
    mControlMove = fSwitch;
    ReadThrust(group, fSwitch);
    mControlStrafe = fSwitch;
    ReadThrust(group, fSwitch);
    mControlTurn = fSwitch;
    ReadThrust(group, fSwitch);
    mControlPitch = fSwitch;
    mDamageOwner.mCharacter = ReadCharacter(group);
    group.ReadFixed(mCurHealth, 0.0f, mMaxHealth, 11);
    group.ReadBits(value, 1);
    mIsAlive = value;
    if (mMaxShield > 0.0f)
    {
        group.ReadFixed(mCurShield, 0.0f, mMaxShield, 11);
        group.ReadBits(value, 1);
        mIsShielded = value;
    }
    group.ReadBits(*(unsigned int*)&mInputLockTime, 32);
    ReadSwitch(group, fSwitch);
    SetSwitch(fSwitch, 0);
    ReadTrigger(group, &mControlView);
    ReadTrigger(group, &mControlCrouch);
    ReadTrigger(group, &mControlSpecialMove);
    for (int i = 0; i < 2; ++i)
    {
        if (mWeaponIndex[i] < 0)
        {
            continue;
        }
        group.ReadBits(value, 3);
        if (mWeaponIndex[i] != (char)value)
        {
            mWeapon[mWeaponIndex[i]]->Deselect(i, false);
            mWeaponIndex[i] = (char)value;
            mWeapon[mWeaponIndex[i]]->Select(i, false);
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        if (mWeaponIndex[i] < 0)
        {
            continue;
        }
        if (mWeapon[mWeaponIndex[i]]->IsRtti(WeaponInvisibility::rttiHashWeaponInvisibility) ||
            mWeapon[mWeaponIndex[i]]->IsRtti(WeaponDisguise::rttiHashWeaponDisguise))
        {
            mWeapon[mWeaponIndex[i]]->Read(group);
        }
    }
    const unsigned char uiOldAlpha = m_uiInvisibilityAlpha;
    group.ReadBits(value, 8);
    m_uiInvisibilityAlpha = (unsigned char)value;
    UpdateInvisibility(uiOldAlpha);
    if (mClass->mJetFuelRechargeRate > 0.0f)
    {
        ReadJetFuel(group, mJetFuel);
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffOffenseTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffDefenseTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mBuffHealthTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
        ReadDamageOwner(group, mDebuffDamageOwner);
    }
    else
    {
        mDebuffDamageTimer = 0.0f;
    }
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadFixed(mSmolderTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mSmolderDamageRate, 0.0f, 1000.0f, 16);
        ReadDamageOwner(group, mSmolderDamageOwner);
    }
    else
    {
        mSmolderTimer = 0.0f;
    }
    if (mPlayerId >= 0)
    {
        if (static_cast<Controllable*>(this)->IsThreadActive())
        {
            group.ReadBits(value, 1);
            mLastCollidedWithNonTerrain = value != 0;
            ReadPositionVector(group, mLastPosition);
            ReadUnitVector(group, mGroundNormal);
        }
        ReadThrust(group, mTurnBuildup);
        group.ReadFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
        group.ReadFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
        group.ReadFixed(mTurnAuto, -1.0f, 1.0f, 16);
        group.ReadFixed(mPitchAuto, -1.0f, 1.0f, 16);
        group.ReadBits(value, 1);
        mUsingTurnAdjusted = value != 0;
        group.ReadBits(value, 1);
        SetTargetLockedObj(value ? ReadObjectPtr(group) : 0);
        if (NetGame::GetJoystickIndex(mPlayerId) >= 0 || NetGame::IsFullState())
        {
            group.ReadBits(value, 1);
            mCanChangeClass = value != 0;
            ReadTrigger(group, &mControlZoom);
            ReadTrigger(group, &mControlJump);
            ReadTrigger(group, &mControlSprint);
            ReadTrigger(group, &mControlThrustFwd);
            if (mCharacter)
            {
                mCharacter->Read(group);
            }
        }
        for (int i = 0; i < 2; ++i)
        {
            if (mWeaponIndex[i] >= 0)
            {
                ReadTrigger(group, &mControlFire[i]);
            }
        }
        group.ReadBits(value, 1);
        mMarkedForDeath = value != 0;
        if (mMarkedForDeath)
        {
            TrackDeath(mPlayerId, 0);
        }
        if (bHasController)
        {
            Controller* pController = mCtrl;
            float fBuildup;
            ReadThrust(group, fBuildup);
            if (pController)
            {
                ((PlayerController*)pController)->SetMouseBuildup(fBuildup);
            }
        }
    }
    if (netLOD == 1)
    {
        ReadAngle(group, mPitchValue);
        for (int i = 0; i < 2; ++i)
        {
            if (mWeaponIndex[i] >= 0)
            {
                ReadWeapon(group, mWeapon[mWeaponIndex[i]]);
            }
        }
    }
    group.ReadBits(value, 1);
    if (value)
    {
        MedalsMgr* pMedals = mCharacter ? mCharacter->mMedalsMgr : 0;
        if (pMedals)
        {
            pMedals->ReadObject(group);
        }
        else
        {
            group.ReadBits(value, 16);
        }
    }
    PropagateTeam(mCharacter, mTeam);
    SetNetLOD(false);
}

// FUNCTION: SWBF2 0x005f2ec0
void EntitySoldier::ReadLateState(NetPktGroup& group)
{
    unsigned int value = 0;
    group.ReadBits(value, 1);
    const bool bHasController = value != 0;
    group.ReadBits(value, 1);
    const bool bActive = value != 0;
    if (bActive)
    {
        PblMatrix matrix;
        group.ReadBits(value, 1);
        if (value)
        {
            ReadMatrix(group, matrix);
        }
        else
        {
            PblQuaternion q;
            float y;
            float s;
            ReadFixed(quatConvert, group, y);
            ReadFixed(quatConvert, group, s);
            PblVector3 position;
            ReadPositionVector(group, position);
            q._fX = 0.0f;
            q._fY = y;
            q._fZ = 0.0f;
            q._fS = s;
            PblQuaternion unit;
            PblMatrix rotation;
            D3DXMatrixRotationQuaternion(&rotation, D3DXQuaternionNormalize(&unit, &q));
            matrix = rotation;
            matrix.m[3][0] = position.x;
            matrix.m[3][1] = position.y;
            matrix.m[3][2] = position.z;
            matrix.m[3][3] = 1.0f;
        }
        PblVector3 velocity;
        ReadVelocity(group, velocity,
                   mClass->mJumpVerticalSpeed * 2.0f + mClass->mMaxSpeed + mClass->mJetJump, -1);
        Move(matrix, velocity);
        const PblVector3& position = static_cast<CollisionObject*>(this)->GetPosition();
        memcpy(&mLastPosition, &position, sizeof(position));
    }
    group.ReadBits(value, 4);
    mState = (SoldierState::SoldierState)value;
    unsigned int grounded = 0;
    group.ReadBits(grounded, 1);
    group.ReadBits(value, 1);
    const bool bFallTimer = value != 0;
    group.ReadFixed(mStateTimer, 0.0f, 8.0f, 13);
    group.ReadBits(value, 4);
    mStanceIndex = value;
    if (grounded)
    {
        mFlags |= flags_isGrounded;
    }
    else
    {
        mFlags &= ~flags_isGrounded;
    }
    if (bFallTimer)
    {
        mFlags |= flags_fallTimer;
    }
    else
    {
        mFlags &= ~flags_fallTimer;
    }
    group.ReadBits(grounded, 1);
    mTargetInfo.mIsAiming = grounded != 0;
    float fSwitch;
    ReadThrust(group, fSwitch);
    mControlMove = fSwitch;
    ReadThrust(group, fSwitch);
    mControlStrafe = fSwitch;
    ReadThrust(group, fSwitch);
    mControlTurn = fSwitch;
    ReadThrust(group, fSwitch);
    mControlPitch = fSwitch;
    mDamageOwner.mCharacter = ReadCharacter(group);
    group.ReadBits(*(unsigned int*)&mInputLockTime, 32);
    ReadSwitch(group, fSwitch);
    SetSwitch(fSwitch, 0);
    ReadTrigger(group, &mControlView);
    ReadTrigger(group, &mControlCrouch);
    ReadTrigger(group, &mControlSpecialMove);
    if (mClass->mJetFuelRechargeRate > 0.0f)
    {
        ReadJetFuel(group, mJetFuel);
    }
    group.ReadBits(grounded, 1);
    if (grounded)
    {
        group.ReadFixed(mBuffOffenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffOffenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffOffenseTimer = 0.0f;
    }
    group.ReadBits(grounded, 1);
    if (grounded)
    {
        group.ReadFixed(mBuffDefenseTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffDefenseMult, 0.0f, 1.0f, 16);
    }
    else
    {
        mBuffDefenseTimer = 0.0f;
    }
    group.ReadBits(grounded, 1);
    if (grounded)
    {
        group.ReadFixed(mBuffHealthTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mBuffHealthRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mBuffHealthTimer = 0.0f;
    }
    group.ReadBits(grounded, 1);
    if (grounded)
    {
        group.ReadFixed(mDebuffDamageTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mDebuffDamageRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mDebuffDamageTimer = 0.0f;
    }
    group.ReadBits(grounded, 1);
    if (grounded)
    {
        group.ReadFixed(mSmolderTimer, 0.0f, 350.0f, 16);
        group.ReadFixed(mSmolderDamageRate, 0.0f, 1000.0f, 16);
    }
    else
    {
        mSmolderTimer = 0.0f;
    }
    if (bActive)
    {
        group.ReadBits(grounded, 1);
        mLastCollidedWithNonTerrain = grounded != 0;
        ReadPositionVector(group, mLastPosition);
        ReadUnitVector(group, mGroundNormal);
    }
    ReadThrust(group, mTurnBuildup);
    group.ReadFixed(mTurnAdjusted, -1.0f, 2.0f, 16);
    group.ReadFixed(mPitchAdjusted, -1.0f, 2.0f, 16);
    group.ReadFixed(mTurnAuto, -1.0f, 1.0f, 16);
    group.ReadFixed(mPitchAuto, -1.0f, 1.0f, 16);
    group.ReadBits(grounded, 1);
    mUsingTurnAdjusted = grounded != 0;
    ReadTrigger(group, &mControlZoom);
    ReadTrigger(group, &mControlJump);
    if (bHasController)
    {
        Controller* pController = mCtrl;
        float fBuildup;
        ReadThrust(group, fBuildup);
        if (pController)
        {
            ((PlayerController*)pController)->SetMouseBuildup(fBuildup);
        }
    }
    ReadAngle(group, mPitchValue);
}

// FUNCTION: SWBF2 0x005f2380
void CommandBuildingArmed::WriteObject(NetPktGroup& group, NetObjState* state)
{
    EntityBuildingArmed::WriteObject(group, state);
}

// FUNCTION: SWBF2 0x005f2390
void CommandBuildingArmed::ReadObject(NetPktGroup& group, NetObjState* state)
{
    EntityBuildingArmed::ReadObject(group, state);
}

// FUNCTION: SWBF2 0x005f2500
void CommandBuildingAnimatedArmed::WriteObject(NetPktGroup& group, NetObjState* state)
{
    EntityBuildingAnimatedArmed::WriteObject(group, state);
}

// FUNCTION: SWBF2 0x005f2510
void CommandBuildingAnimatedArmed::ReadObject(NetPktGroup& group, NetObjState* state)
{
    EntityBuildingAnimatedArmed::ReadObject(group, state);
}

// FUNCTION: SWBF2 0x005f27e0
void EntityBuildingArmedDynamic::WriteObject(NetPktGroup& group, NetObjState* state)
{
    WriteMatrix(group, static_cast<const CollisionObject*>(this)->GetMatrix());
    EntityBuildingArmed::WriteObject(group, state);
}

// FUNCTION: SWBF2 0x005f2810
void EntityBuildingArmedDynamic::ReadObject(NetPktGroup& group, NetObjState* state)
{
    PblMatrix matrix;
    ReadMatrix(group, matrix);
    static_cast<CollisionObject*>(this)->SetMatrix(matrix);
    RedSceneObject* scene = this;
    scene->Deactivate();
    scene->Activate();
    EntityBuildingArmed::ReadObject(group, state);
}

// FUNCTION: SWBF2 0x005f0370
GameObject* PowerupStation::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            Thread* thread = this;
            thread->DeactivateThread();
            Thread::EnterClient();
            thread->ActivateThread(0, PER_LOCAL_TURN);
            Thread::LeaveClient();
        }
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    EntityGeometry* client = (EntityGeometry*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, client);
    ((PowerupStation*)client)->mEffectRegion = mEffectRegion;
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f1860
GameObject* EntityBuildingAnimatedArmed::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    EntityGeometry* client = (EntityGeometry*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, client);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f11e0
void NetEntityHologram::MoveToClient()
{
    Thread* thread = &_Thread;
    if (thread->IsThreadActive())
    {
        thread->DeactivateThread();
        thread->ActivateThread(0, PER_FRAME);
    }
    if (m_pLightBeam)
    {
        FLEffectObject* effect = m_pLightBeam;
        if (effect->IsEffectActive())
        {
            effect->DeactivateEffect();
            effect->ActivateEffect();
        }
    }
}

// FUNCTION: SWBF2 0x005f1250
void NetEntityBuildingArmed::MoveToClient()
{
    if (m_pHologram)
    {
        static_cast<NetEntityHologram*>(m_pHologram)->MoveToClient();
    }
    for (int i = 0; i < mNumTurrets; ++i)
    {
        Controllable* control = mTurret[i];
        for (int j = 0; j < control->GetWeaponCount(); ++j)
        {
            Weapon* weapon = control->GetWeaponPtr(j);
            weapon->DeactivateThread();
            weapon->ActivatePhysics();
        }
    }
}

// FUNCTION: SWBF2 0x005f1f10
void NetCommandBuilding::MoveToClient()
{
    CommandPost* post = mPost;
    if (post && post->IsThreadActive())
    {
        post->DeactivateThread();
        post->ActivateThread(-20, PER_TURN);
        if (post->m_pHologram)
        {
            static_cast<NetEntityHologram*>(post->m_pHologram)->MoveToClient();
        }
    }
}

// FUNCTION: SWBF2 0x005f1db0
void CommandBuilding::ReadObject(NetPktGroup& group, NetObjState*)
{
    unsigned int value;
    if (mMaxHealth < FLT_MAX)
    {
        float health;
        group.ReadFixed(health, 0.0f, mMaxHealth, 11);
        SetCurHealth(health, true, 0);
    }
    group.ReadBits(value, 1);
    mIsAlive = value != 0;
    group.ReadBits(value, 1);
    if (value)
    {
        group.ReadBits(value, 3);
        mHealthTypeForLockOn = value;
    }
    else
    {
        mHealthTypeForLockOn = 8;
    }
    group.ReadBits(value, 1);
    mIsShielded = value != 0;
    group.ReadBits(value, 2);
    if (value != mState)
    {
        switch (value)
        {
        case EntityBuilding::STATE_ALIVE:
            static_cast<Damageable*>(this)->Respawn();
            break;
        case EntityBuilding::STATE_DEAD:
        case EntityBuilding::STATE_UNBUILT:
            EntityBuilding::Kill();
            if (mPost)
            {
                mPost->Kill();
            }
            break;
        case EntityBuilding::STATE_DYING:
            static_cast<Damageable*>(this)->Kill();
            break;
        }
    }
}

// FUNCTION: SWBF2 0x005f2680
GameObject* EntityBuilding::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            NetGame::EnterClient();
            for (int i = 0; i != 10; ++i)
            {
                const DamageEffectClass* damageClass = GetGameObjectClass()->GetDamageEffectClass(i);
                if (!damageClass)
                {
                    break;
                }
                FLEffectObject* effect = mDamageEffect[i];
                if (!effect)
                {
                    continue;
                }
                FLEffectClass* effectClass = FLEffect::Get(damageClass->mNameHash);
                if (!effectClass || effectClass->mType != 1)
                {
                    continue;
                }
                Thread* thread = effect;
                if (thread->IsThreadActive())
                {
                    thread->DeactivateThread();
                    thread->ActivateThread(0, PER_FRAME);
                }
            }
            NetGame::LeaveClient();
        }
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    static_cast<RedSceneObject*>(this)->Activate();
    TargetManager::AddTarget(this, false);
    AddNetCollision(static_cast<CollisionObject*>(this));
    BuildEffects();
    NetGame::LeaveClient();
    return this;
}

// FUNCTION: SWBF2 0x005f12e0
GameObject* EntityBuildingArmed::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            DeactivatePhysics();
            const PblList<Controllable>& list = GetControllableList();
            for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
            {
                Controllable* control = it;
                control->ReleaseController();
            }
            NetGame::EnterClient();
            ActivatePhysics();
            for (PblList<Controllable>::Itor it = list.Begin(); it != list.End(); ++it)
            {
                Controllable* control = it;
                control->SetupController();
            }
            static_cast<NetEntityBuildingArmed*>(this)->MoveToClient();
            NetGame::LeaveClient();
        }
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    EntityGeometry* client = (EntityGeometry*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, client);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f1f50
GameObject* CommandBuilding::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        if (!NetGame::IsHost())
        {
            NetGame::EnterClient();
            static_cast<NetCommandBuilding*>(this)->MoveToClient();
            NetGame::LeaveClient();
        }
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = 0;
    desc.mPlayerId = -1;
    CommandPost::ForceIndex(mPost->mPostIndex);
    CommandBuilding* client = (CommandBuilding*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, client);
    client->SetTeam(mTeam);
    client->mPerceivedTeam = mPerceivedTeam;
    client->mPost->CopyFrom(mPost);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f20e0
GameObject* CommandWalker::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = mId;
    desc.mPlayerId = static_cast<const Controllable*>(this)->mPlayerId;
    CommandWalker* client = (CommandWalker*)GetEntityClass()->Build(desc);
    client->SetTeam(mTeam);
    client->mPerceivedTeam = mPerceivedTeam;
    client->mPost->CopyFrom(mPost);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f2230
GameObject* CommandHover::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = mId;
    desc.mPlayerId = static_cast<const Controllable*>(this)->mPlayerId;
    CommandHover* client = (CommandHover*)GetEntityClass()->Build(desc);
    client->SetTeam(mTeam);
    client->mPerceivedTeam = mPerceivedTeam;
    client->mPost->CopyFrom(mPost);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f23a0
GameObject* CommandBuildingArmed::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = mId;
    desc.mPlayerId = -1;
    CommandBuildingArmed* client = (CommandBuildingArmed*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, client);
    client->SetTeam(mTeam);
    client->mPerceivedTeam = mPerceivedTeam;
    client->mPost->CopyFrom(mPost);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}

// FUNCTION: SWBF2 0x005f2520
GameObject* CommandBuildingAnimatedArmed::AddToClient()
{
    if (NetGame::IsOneWorld())
    {
        return this;
    }
    static_cast<RedSceneObject*>(this)->Deactivate();
    TargetManager::RemoveTarget(this);
    NetGame::EnterClient();
    EntityDesc desc;
    desc.mMatrix = static_cast<const CollisionObject*>(this)->GetMatrix();
    desc.mNameId = mId;
    desc.mPlayerId = -1;
    CommandBuildingAnimatedArmed* client = (CommandBuildingAnimatedArmed*)GetEntityClass()->Build(desc);
    PAAnimationGroup::ClientGeometryCreated(this, client);
    client->SetTeam(mTeam);
    client->mPerceivedTeam = mPerceivedTeam;
    client->mPost->CopyFrom(mPost);
    client->Init();
    client->BuildEffects();
    NetGame::LeaveClient();
    return client->GetGameObject();
}
