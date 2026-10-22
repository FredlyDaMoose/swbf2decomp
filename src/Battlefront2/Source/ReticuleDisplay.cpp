#include "ReticuleDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "ChaseCamera.h"
#include "Controllable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HUDElementMap.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/RedCamera.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

namespace
{
int GetMapMode(const HUD::ElementMap* pMap)
{
    return pMap->GetMode();
}
}

extern bool netFrameLock;

extern bool netIsLocalTurn;

// GLOBAL: SWBF2 0x01eaeffc
ReticuleDisplay* ReticuleDisplay::sInstance[1];

// GLOBAL: SWBF2 0x01f3a170
// SYNTHETIC: SWBF2 0x004061e0
// `dynamic initializer for 'reticuleInterpolator''
// SYNTHETIC: SWBF2 0x00769440
// `dynamic atexit destructor for 'reticuleInterpolator''
ReticuleInterpolator reticuleInterpolator[1];

// FUNCTION: SWBF2 0x00630490
ReticuleInterpolator::~ReticuleInterpolator()
{
}

// SYNTHETIC: SWBF2 0x006304a0
// ReticuleInterpolator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006309f0
bool ReticuleInterpolator::Update(float)
{
    if (mCount >= 2)
    {
        NetGame::GetTurnRatio();
    }
    return true;
}

// FUNCTION: SWBF2 0x006309b0
void ReticuleInterpolator::Create()
{
    mCount = 0;
    mGroupAimArea = 0;
    mJoystickIndex = -1;
    if (!netFrameLock)
    {
        ActivateThread(0, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x006309e0
void ReticuleInterpolator::Destroy()
{
    if (!netFrameLock)
    {
        DeactivateThread();
    }
}

// FUNCTION: SWBF2 0x00630a10
void ReticuleInterpolator::SetNext(PblVector3* pPos, RedScreenGroupElement*, int iJoystickIndex)
{
    mGroupAimArea = 0;
    mJoystickIndex = iJoystickIndex;
    if (pPos)
    {
        mPos[mCount & 1] = *pPos;
        mCount++;
    }
    else
    {
        mCount = 0;
    }
}

// FUNCTION: SWBF2 0x00630570
ReticuleDisplay::ReticuleDisplay()
{
    mCanRender = true;
    mVisible = false;
    mJoystickIndex = -1;
    mViewport = (unsigned int)-1;
}

// FUNCTION: SWBF2 0x006305d0
ReticuleDisplay::~ReticuleDisplay()
{
}

// SYNTHETIC: SWBF2 0x006305a0
// ReticuleDisplay::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006304d0
void ReticuleDisplay::Create()
{
    sInstance[0] = new ReticuleDisplay;
    if (NetGame::IsEnabled())
    {
        reticuleInterpolator[0].Create();
    }
}

// FUNCTION: SWBF2 0x00630520
void ReticuleDisplay::Destroy()
{
    if (sInstance[0])
    {
        delete sInstance[0];
        sInstance[0] = 0;
    }
    if (NetGame::IsEnabled())
    {
        reticuleInterpolator[0].Destroy();
    }
}

// FUNCTION: SWBF2 0x00630990
void ReticuleDisplay::EnableViewport(unsigned int uiViewport, bool bEnable)
{
    if (sInstance[0] && sInstance[0]->mViewport == uiViewport && !bEnable)
    {
        sInstance[0]->Hide();
    }
}

// FUNCTION: SWBF2 0x006305e0
void ReticuleDisplay::Bind(unsigned int uiJoystickIndex)
{
    mJoystickIndex = uiJoystickIndex;
    if (NetGame::IsEnabled())
    {
        ActivateThread(-101, PER_LOCAL_TURN);
    }
    else
    {
        ActivateThread(-1, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x00630650
bool ReticuleDisplay::Update(float fDeltaTime)
{
    if (!mCanRender)
    {
        return true;
    }

    Character* pCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mJoystickIndex));
    if (!pCharacter)
    {
        return true;
    }

    bool bShow = false;
    if (pCharacter->GetControllable() && pCharacter->GetControllable()->GetGameObject() &&
        pCharacter->GetControllable()->GetGameObject()->ShouldShowReticule() &&
        CameraManager::sInstance->IsChaseMode(mJoystickIndex))
    {
        Trackable* pOwner = CameraManager::sInstance->mChaseCamera[mJoystickIndex]->GetOwner();
        if (pOwner && pOwner->GetControllable() == pCharacter->GetControllable())
        {
            bShow = true;
        }
    }

    if (mVisible)
    {
        if (!bShow)
        {
            Hide(pCharacter);
        }
    }
    else if (bShow)
    {
        Show(pCharacter);
    }

    if (!mVisible)
    {
        if (!netFrameLock && NetGame::IsEnabled() && netIsLocalTurn)
        {
            reticuleInterpolator[mJoystickIndex].SetNext(0, 0, mJoystickIndex);
        }
        return true;
    }

    RedCamera* pCamera = CameraManager::sInstance->mRedCamera[mJoystickIndex];
    Controllable* pControllable = pCharacter->GetControllable();
    if (!pControllable)
    {
        return true;
    }
    if (pControllable->GetControllableClass()->mControlsUnit)
    {
        pControllable = pCharacter->mUnit;
    }

    PblVector3 vAim(pControllable->mTargetInfo.mAimStart.x + pControllable->mEyeDir.x * 1024.0f,
                    pControllable->mTargetInfo.mAimStart.y + pControllable->mEyeDir.y * 1024.0f,
                    pControllable->mTargetInfo.mAimStart.z + pControllable->mEyeDir.z * 1024.0f);
    PblVector3 vCamera;
    D3DXVec3TransformCoord(&vCamera, &vAim, &pCamera->_MatrixInverse);
    PblVector3 vPos = pCamera->TransformCameraPointToProjectionSpace(vCamera);

    HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(mJoystickIndex);
    if ((!pMap || GetMapMode(pMap) != HUD::ElementMap::mode_Large) && !GameLoop::sPauseMode)
    {
        for (int i = 0; i < 2; ++i)
        {
            vPos.x = 0.5f;
            vPos.y = vPos.y * 0.5f + 0.5f;
            vPos.z = 0.0f;
            HUD::Event event;
            event.mClass = HUD::GameEvents::gPlayerEvents[mJoystickIndex].weapons[i].reticulePosition;
            event.mData.vector3 = &vPos;
            event.Send();
        }
    }

    if (!netFrameLock && NetGame::IsEnabled() && netIsLocalTurn)
    {
        reticuleInterpolator[mJoystickIndex].SetNext(&vPos, 0, mJoystickIndex);
    }
    return true;
}

// FUNCTION: SWBF2 0x00630620
void ReticuleDisplay::Show(Character*)
{
    mVisible = true;
}

// FUNCTION: SWBF2 0x00630630
void ReticuleDisplay::Hide(Character*)
{
    Hide();
}

// FUNCTION: SWBF2 0x00630640
void ReticuleDisplay::Hide()
{
    mVisible = false;
}
