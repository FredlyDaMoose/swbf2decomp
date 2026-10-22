#pragma once

#include <stddef.h>

#include "RedEngineFL/Application/Renderable.h"
#include "Zephyr/Source/ZephyrPoseDyn.h"
#include "Zephyr/Source/ZephyrPoseStatic.h"
#include "Zephyr/Source/ZephyrSkeleton.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"

class Aimer;
class Controllable;
class RedModel;
class Weapon;
class ZephyrAnim;
class ZephyrSkeletonShared;

enum WEAPONCLASS
{
    RIFLE = 0,
    BAZOOKA = 1,
    TOOL = 2,
    GRENADE = 3,
    WEAPONCLASS_NONE = 4
};

enum SOLDIERANIMATION
{
    IDLE = 0,
    RUN = 1,
    SHOOT1 = 2,
    SHOOT2 = 3,
    CHARGE = 4,
    RELOAD = 5,
    REPAIR = 6,
    JUMP = 7,
    FLAIL = 8,
    HANDSDOWN = 9,
    JUMP_LAND = 10,
    NUM_SOLDIERANIMATIONS = 11,
    DROIDEKA_IDLE = 0,
    DROIDEKA_WALK = 1,
    DROIDEKA_SHOOT = 2,
    DROIDEKA_RELOAD = 3,
    NUM_DROIDEKAANIMATIONS = 4,
    TOTAL_NUM_ANIMATIONS = 15
};

enum eViewObjectType
{
    TYPE_SOLDIER = 0,
    TYPE_COCKPIT = 1,
    TYPE_DROIDEKA = 2,
    TYPE_HOVER = 3,
    TYPE_NONE = 4
};

class FirstPersonCameraManager
{
public:
    FirstPersonCameraManager() { mCurrCamPt = -1; }

    void ViewBob(float fScale);

    void ReturnToCenter();

    void Update(PblMatrix& rMatrix);

    float RollLeft();
    float RollRight();
    float Land();
    float ProneToStand();
    float CrouchToStand();

    float SetCameraPath(PblVector3* pPos, PblVector3* pPosSlope, PblVector3* pRot,
                        PblVector3* pRotSlope, float* pTime, unsigned int uPoints);
    void SetBobbing(bool bBobbing) { mIsBobbing = bBobbing; }

    char mHermPos[6 * 52];
    char mHermRot[6 * 52];
    float mCamTime[6];
    float mCurrCamTime;
    int mCurrCamPt;
    PblVector3 mCameraPos;
    PblVector3 mCurRotVec;
    int mNumPoints;
    bool mIsBobbing;
    bool mIsReturning;
    PblVector3 mLinVec;
    bool mIsGoingProne;
};
 
 
 

// VTABLE: SWBF2 0x0079eb80
class FirstPersonRenderable : public Renderable
{
public:

    void SetAnimAndTransition(float fTime);
    FirstPersonRenderable(unsigned int uCameraId, const ZephyrSkeletonShared* pShared);
    virtual ~FirstPersonRenderable();

    virtual void Activate(int value);
    virtual void Deactivate();

    virtual void Render(int value);

    void SetInitialPlayerId(int iPlayerId);
    void UpdateAimer(const PblMatrix& mMatrix);
    void SetAnimation(const ZephyrAnim* pAnim, bool bLoop);
    void SetSoldierState(int iState);

    void ClearFiringState();
    void UpdateSoldier(const RedModel* pModel, Controllable* pOwner, Aimer* pAimer);
    void UpdateCockpit(const RedModel* pModel, Controllable* pOwner, Aimer* pAimer);
    void UpdateDroideka(const RedModel* pModel, Controllable* pOwner, Aimer* pAimer);
    void UpdateSoldierAimer(const PblMatrix& mMatrix);
    void UpdateDroidekaAimer(const PblMatrix& mMatrix);
    void UpdateCockpitAimer(const PblMatrix& mMatrix);
    void RenderSoldier(unsigned int uFlags) const;

    void RenderCockpit(unsigned int uFlags) const;
    void RenderDroideka(unsigned int uFlags) const;

protected:
    bool IsOwnerAiming() const;

public:
    ZephyrSkeleton<32> mZephyrSkeleton;
    ZephyrPoseStatic<32> mZephyrPoseStatic;
    ZephyrPoseDyn<32> mZephyrPoseDynamic;
    float mBlendFactor;
    PblMatrix mModelMatrix;
    PblMatrix mWeaponMatrix;
    float mAnimPlaySpeed;
    unsigned int mEntitySoldierState;
    Aimer* mAimer;
    const RedModel* mObjectModel;
    Weapon* mCurrentWeapon;
    WEAPONCLASS mWeaponClass;
    SOLDIERANIMATION mSoldierState;
    eViewObjectType mObjectType;
    Controllable* mOwner;
    bool mUpdatedSoldierAimer;
    float fTransitionTimer;
    float fTransitionTimerMax;
    SOLDIERANIMATION mTransitionAnim;
    bool mBlendHandsDown;
    float mShadowVal;
    PblQuaternion mCockpitRotation;
    PblVector3 mCockpitTrans;
    unsigned int mCameraId;
    bool bReadyToRender;
    bool bStreamingModel;
};
 
 
 
 
 
 
 
 
 
 
 

namespace FirstPerson
{

    void ClearFiringState(unsigned int uPlayer);
    void RollLeft(unsigned int uPlayer);
    void RollRight(unsigned int uPlayer);
    void ProneToStand(unsigned int uPlayer);
    void CrouchToStand(unsigned int uPlayer);
    void Land(unsigned int uPlayer);

    bool IsActive(unsigned int uPlayer);

    void PreInit();

    void ClearBanks();

    void Deinit();

    bool AddBank(const char* pszName);

    const ZephyrAnim* FindAnimation(const char* pszName);

    extern int s_iAnimBankCount;

    extern char sModelName[64][256];

    void Init();

    void InitPlatform();

    void PostLoad();

    void LoadLVLFile(char const* pName);

    int AddGeometry(const char* pszName);

    void Activate(unsigned int uPlayer, bool bActive);

    void Update(unsigned int uPlayer, eViewObjectType eType, int iArg,
                Controllable* pOwner, Aimer* pAimer);

    void SetSoldierState(unsigned int uPlayer, int iState);

    void UpdateCamera(unsigned int uPlayer, PblMatrix& rMatrix);

    void UpdateAimer(unsigned int uPlayer, const PblMatrix& rMatrix);

    const RedModel* LoadDynamicModel(const char* pszName, bool bArg);
}

namespace FirstPerson { extern int sModelCount; }
