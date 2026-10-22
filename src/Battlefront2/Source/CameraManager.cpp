#include <string.h>

#include "CameraManager.h"
#include "ChaseCamera.h"
#include "DeathCamera.h"
#include "FreeCamera.h"
#include "MapCamera.h"
#include "NetGame.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "GameObject.h"
#include "RumbleRegion.h"
#include "ScopeDisplay.h"
#include "Team.h"
#include "Trackable.h"
#include "Tracker.h"
#include "TriggerSoundRegion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

#include <math.h>
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndspace.h"
#include "SoundFL/source/win/sndlistener.h"
#include "SoundFL/source/win/sndengine.h"

namespace FLRenderer
{
unsigned int AddView(const RedViewport& rViewport, RedCamera* pCamera);
void RemoveView(unsigned int uiView);
void SetSceneEnable(unsigned int uiView, bool bEnable);
}

class PblMatrix;
extern PblMatrix gMatrixIdentity;

extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netFrameLock;

extern bool netIsLocalTurn;
extern bool netIsLateTurn;
extern bool gCentreSpeakerEnhancement;

typedef const PblVector3& (__fastcall *ObjectVelocityFn)(GameObject* pThis, void*);
typedef Controllable* (__fastcall *ObjectControllableFn)(GameObject* pThis, void*);
static CollisionObject* GetObjectCollision(GameObject* pObject)
{
    return static_cast<CollisionObject*>(pObject);
}

// GLOBAL: SWBF2 0x01eb0fd0
// SYNTHETIC: SWBF2 0x00402370
// `dynamic initializer for 'cameraInterpolator''
// SYNTHETIC: SWBF2 0x00766dc0
// `dynamic atexit destructor for 'cameraInterpolator''
CameraInterpolator cameraInterpolator[1];

// GLOBAL: SWBF2 0x01e30324
CameraManager* CameraManager::sInstance;

// FUNCTION: SWBF2 0x0044e650
void CameraManager::Create(int iNumCameras)
{
    sInstance = new CameraManager;
    sInstance->SetNumCameras(iNumCameras);
}

// FUNCTION: SWBF2 0x0044e6f0
CameraManager::CameraManager()
{
    for (int i = 0; i < 16; i++)
    {
        mShotRotation[i]._fX = 0.0f;
        mShotRotation[i]._fY = 0.0f;
        mShotRotation[i]._fZ = 0.0f;
        mShotRotation[i]._fS = 1.0f;
    }

    m_iNumCam = 0;
    float fAspectRatio = RedRenderer::GetScreenAspectRatio();
    mRedCamera[0] = new RedCamera;
    mRedCamera[0]->SetMatrix(gMatrixIdentity);
    mRedCamera[0]->SetPerspective(0.7f, 120.0f, 0.83775806f, fAspectRatio);

    mChaseCamera[0] = new ChaseCamera(this, 0);
    mFreeCamera[0] = new FreeCamera(this, 0);
    mMapCamera[0] = new MapCamera(this, 0);
    mDeathCamera[0] = new DeathCamera(this, 0);
    mCurGameCamera[0] = mChaseCamera[0];

    mListener[0] = Snd::EngineBase::GetFreeListener();
    mListener[0]->Enable(true);
    mListener[0]->SetPositionOrientation(&Snd::Globals::smNullVector, &Snd::Globals::smFrontVector,
                                         &Snd::Globals::smUpVector, 0);
    mListener[0]->SetSpace(&Snd::Space::smDefaultSpace);
    m_hView[0] = (unsigned int)-1;
    mTrackDistance[0] = 1.0f;
    mListener[0]->Enable(true);

    mNumShots = 0;
    memset(mShotRotation, 0, sizeof(mShotRotation));
    memset(mShotPosition, 0, sizeof(mShotPosition));

    RedRenderer::SetCamera(mRedCamera[0]);

    if (NetGame::IsEnabled() && NetGame::IsOneWorld() && NetGame::IsHost())
    {
        ActivateThread(-99, PER_TURN);
        if (!netFrameLock)
        {
            cameraInterpolator[0].CreateClient();
        }
    }
    else
    {
        ActivateThread(1, PER_FRAME);
    }

    mUpdatedCameraId = 0;
}

// FUNCTION: SWBF2 0x0044eac0
void CameraManager::SetNumCameras(unsigned int uiNumCameras)
{
    if (uiNumCameras == m_iNumCam)
    {
        return;
    }

    mListener[0]->Enable(false);
    if (m_hView[0] != (unsigned int)-1)
    {
        FLRenderer::RemoveView(m_hView[0]);
    }
    m_hView[0] = (unsigned int)-1;

    m_iNumCam = uiNumCameras;
    for (unsigned int i = 0; i < m_iNumCam; i++)
    {
        mListener[i]->Enable(true);
    }

    float fAspectRatio = RedRenderer::GetScreenAspectRatio();
    switch (m_iNumCam)
    {
    case 1:
    {
        RedViewport full = { 0.0f, 0.0f, 1.0f, 1.0f };
        mRedCamera[0]->SetPerspective(0.7f, 120.0f, 0.83775806f, fAspectRatio);
        m_hView[0] = FLRenderer::AddView(full, mRedCamera[0]);
        FLRenderer::SetSceneEnable(m_hView[0], false);
        break;
    }
    case 2:
    {
        RedViewport top = { 0.0f, 0.0f, 1.0f, 0.5f };
        RedViewport bottom = { 0.0f, 0.5f, 1.0f, 0.5f };
        fAspectRatio *= 0.5f;
        mRedCamera[0]->SetPerspective(0.7f, 120.0f, 0.83775806f, fAspectRatio);
        m_hView[0] = FLRenderer::AddView(top, mRedCamera[0]);
        FLRenderer::SetSceneEnable(m_hView[0], false);
        mRedCamera[1]->SetPerspective(0.7f, 120.0f, 0.83775806f, fAspectRatio);
        m_hView[1] = FLRenderer::AddView(bottom, mRedCamera[1]);
        FLRenderer::SetSceneEnable(m_hView[1], false);
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0044e3c0
bool Tracker::IsFirstPersonView()
{
    Trackable* pTrackable = mTrackable;
    if (pTrackable && pTrackable != (Trackable*)-1 &&
        (unsigned int)pTrackable->mTracker != 0xDDDDDDDDu &&
        pTrackable->GetControllable())
    {
        Controllable* pCtrl = pTrackable->GetControllable();
        switch (pCtrl->GetControllableClass()->mForceMode)
        {
        case 1:
            return false;
        case 2:
            return true;
        }
    }
    return mIsFirstPersonView;
}

// SYNTHETIC: SWBF2 0x0044e500
// CameraInterpolator::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0044e6c0
void CameraManager::CreateClient()
{
    sInstance->DeactivateThread();
    sInstance->ActivateThread(-99, PER_LOCAL_TURN);
    cameraInterpolator[0].CreateClient();
}

// SYNTHETIC: SWBF2 0x0044e9d0
// CameraManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0044ea00
CameraManager::~CameraManager()
{
    DeactivateThread();
    delete mRedCamera[0];
    delete mChaseCamera[0];
    delete mDeathCamera[0];
    delete mFreeCamera[0];
    delete mMapCamera[0];
    mListener[0]->Enable(false);
}

// FUNCTION: SWBF2 0x0044ec90
void CameraManager::EnableScene(bool bEnable)
{
    for (unsigned int i = 0; i < m_iNumCam; i++)
    {
        FLRenderer::SetSceneEnable(m_hView[i], bEnable);
    }
}

// FUNCTION: SWBF2 0x0044ecc0
void CameraManager::SetChaseCameraRandomShot(int iCam)
{

    // GLOBAL: SWBF2 0x01faa454

    static unsigned int sLastChaseShot;

    if (mNumShots == 0)
    {
        return;
    }

    unsigned int uiShot;
    if (mNumShots > 2)
    {
        uiShot = PblRandom::GetGlobalInt() % mNumShots;
        for (int i = 0; uiShot == sLastChaseShot && i < 16; i++)
        {
            uiShot = PblRandom::GetGlobalInt() % mNumShots;
        }
        if (uiShot == sLastChaseShot)
        {
            uiShot = (sLastChaseShot + 1) % mNumShots;
        }
    }
    else
    {
        uiShot = (sLastChaseShot + 1) % mNumShots;
    }

    mChaseCamera[iCam]->SetRotation(mShotRotation[uiShot]);
    sLastChaseShot = uiShot;
    mChaseCamera[iCam]->SetPosition(mShotPosition[uiShot]);
}

// FUNCTION: SWBF2 0x0044e330
void CameraManager::SetChaseCameraRotation(const PblQuaternion& qRot, int iCam)
{
    mChaseCamera[iCam]->SetRotation(qRot);
}

// FUNCTION: SWBF2 0x0044ef30
void CameraManager::SetChaseCameraShot(int iCam, int iShot)
{
    if (mNumShots == 0)
    {
        return;
    }

    if (iShot > (int)mNumShots - 1)
    {
        iShot = mNumShots - 1;
    }

    mChaseCamera[iCam]->SetRotation(mShotRotation[iShot]);
    mChaseCamera[iCam]->SetPosition(mShotPosition[iShot]);
}

// FUNCTION: SWBF2 0x0044ef90
void CameraManager::QuickSetupCameras()
{
    for (mUpdatedCameraId = 0; mUpdatedCameraId < m_iNumCam; mUpdatedCameraId++)
    {
        mCurGameCamera[mUpdatedCameraId]->SetupCamera(mRedCamera[mUpdatedCameraId], 0.0f);
    }
    mUpdatedCameraId = 0;
}

// FUNCTION: SWBF2 0x0044efd0
bool CameraManager::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    RumbleRegionManager::Update(fDeltaTime);

    for (mUpdatedCameraId = 0; mUpdatedCameraId < m_iNumCam; mUpdatedCameraId++)
    {
        const bool bScope = ScopeDisplay::Instance(mUpdatedCameraId) &&
                            ScopeDisplay::Instance(mUpdatedCameraId)->IsActive();

        mCurGameCamera[mUpdatedCameraId]->Update(fDeltaTime);
        mCurGameCamera[mUpdatedCameraId]->SetupCamera(mRedCamera[mUpdatedCameraId], fDeltaTime);
        mCurGameCamera[mUpdatedCameraId]->SetupIconVisibility(mUpdatedCameraId);

        if (fDeltaTime == 0.0f)
        {
            continue;
        }

        const PblVector3* pTargetPos = 0;
        bool bTracked = false;
        float fForward = 2.0f;

        if (IsChaseMode(0))
        {
            Trackable* pOwner = mChaseCamera[mUpdatedCameraId]->GetOwner();
            if (pOwner)
            {
                GameObject* pObject = pOwner->GetGameObject();
                if (pObject)
                {
                    mListener[mUpdatedCameraId]->SetVelocity(&pObject->GetVelocity());
                    pTargetPos = &GetObjectCollision(pObject)->GetMatrix().GetTranslation();

                    Controllable* pCtrl = pObject->GetControllable();
                    if (pCtrl)
                    {
                        Character* pCharacter = pCtrl->mCharacter;
                        pCtrl->GetControllableClass();
                        if (pCharacter)
                        {
                            const Team* pTeam = pCharacter->mTeamPtr;
                            TriggerSoundRegion::Update(
                                pTeam->GetSide(),
                                (float)pTeam->GetCurReinforcements() /
                                    (float)pTeam->GetMaxReinforcements(),
                                pTargetPos);
                        }
                    }

                    const int iLocal = NetGame::GetLocalIndex(mUpdatedCameraId);
                    if (iLocal >= 0)
                    {
                        const PblVector3 vPos = *pTargetPos;
                        RumbleRegionManager::CheckForRumble(iLocal, vPos,
                                                            mChaseCamera[mUpdatedCameraId], 0.0f);
                    }

                    if (bScope)
                    {
                        mListenerPos[mUpdatedCameraId] = *pTargetPos;
                    }
                    bTracked = true;
                }

                Tracker* pTracker = pOwner->GetTracker();
                if (pTracker && pTracker->IsFirstPersonView())
                {
                    fForward = 7.0f;
                    Snd::Engine::SetCentrePeakMode(false);
                }
                else
                {
                    Snd::Engine::SetCentrePeakMode(gCentreSpeakerEnhancement);
                }
            }
        }

        if (!bTracked)
        {
            mListener[mUpdatedCameraId]->SetVelocity(0);
        }

        const PblMatrix& rMatrix = mRedCamera[mUpdatedCameraId]->GetMatrix();
        if (!bScope)
        {
            mListenerPos[mUpdatedCameraId] = rMatrix.GetTranslation();
        }

        const PblVector3& rFront = rMatrix.GetZAxis();
        PblVector3& rPos = mListenerPos[mUpdatedCameraId];
        rPos.x = rFront.x * fForward + rPos.x;
        rPos.y = rPos.y + rFront.y * fForward;
        rPos.z = rPos.z + rFront.z * fForward;
        mListener[mUpdatedCameraId]->SetPositionOrientation(&rPos, &rFront, &rMatrix.GetYAxis(),
                                                           &rMatrix.GetXAxis());
        if (pTargetPos)
        {
            mListener[mUpdatedCameraId]->SetPositonSpace(pTargetPos);
        }
    }

    if (!netFrameLock && NetGame::IsEnabled() && netIsLocalTurn)
    {
        for (mUpdatedCameraId = 0; mUpdatedCameraId < m_iNumCam; mUpdatedCameraId++)
        {
            cameraInterpolator[mUpdatedCameraId].SetNext(mCurGameCamera[mUpdatedCameraId],
                                                        mRedCamera[mUpdatedCameraId]);
        }
    }

    mUpdatedCameraId = 0;
    return true;
}

// FUNCTION: SWBF2 0x0044f3a0
void CameraManager::Chase(Trackable* pTrackable, int iCam)
{
    if (mChaseCamera[iCam]->GetOwner() != pTrackable)
    {
        mTrackDistance[iCam] = 1.0f;
    }
    mChaseCamera[iCam]->SetOwner(pTrackable);
    if (mCurGameCamera[iCam] == mDeathCamera[iCam])
    {
        mDeathCamera[iCam]->mOwner = pTrackable;
        mCurGameCamera[iCam] = mChaseCamera[iCam];
    }
}

// FUNCTION: SWBF2 0x0044f3f0
void CameraManager::TrackDeath(Trackable* pTrackable, int iCam, Character* pKiller)
{
    if (mCurGameCamera[iCam] == mChaseCamera[iCam])
    {
        mDeathCamera[iCam]->SetInfo(pTrackable, pKiller, mChaseCamera[iCam]->GetMatrix());
        mCurGameCamera[iCam] = mDeathCamera[iCam];
    }
}

// FUNCTION: SWBF2 0x0044f430
void CameraManager::ChaseMode(int iCam)
{
    mCurGameCamera[iCam] = mChaseCamera[iCam];
}

// FUNCTION: SWBF2 0x0044f440
void CameraManager::FreeLook(int iCam)
{
    mFreeCamera[iCam]->SetMatrix(mRedCamera[iCam]->_Matrix);
    mCurGameCamera[iCam] = mFreeCamera[iCam];
}

// FUNCTION: SWBF2 0x0044f460
void CameraManager::MapMode(int iCam)
{
    mMapCamera[iCam]->SetMatrix(gMatrixIdentity);
    mCurGameCamera[iCam] = mMapCamera[iCam];
}

// FUNCTION: SWBF2 0x0044f480
bool CameraManager::IsChaseMode(int iCam) const
{
    return mCurGameCamera[iCam] == mChaseCamera[iCam];
}

// FUNCTION: SWBF2 0x0044f4a0
bool CameraManager::IsFreeLook(int iCam) const
{
    return mCurGameCamera[iCam] == mFreeCamera[iCam];
}

// FUNCTION: SWBF2 0x0044f4c0
void CameraManager::ApplyShake(float fAmount, float fLength)
{
    mChaseCamera[0]->ApplyShake(fAmount, fLength);
}

// FUNCTION: SWBF2 0x0044f500
void CameraManager::ClearShake()
{
    mChaseCamera[0]->ClearShake();
}

// FUNCTION: SWBF2 0x0044f520
bool CameraManager::IsInsideAnyCamera(const PblVector3& vPos, float fRadius, float fDistance)
{
    for (unsigned int i = 0; i < m_iNumCam; i++)
    {
        RedCamera* pCamera = mRedCamera[i];
        if (pCamera)
        {
            float fMaxDistance = pCamera->GetZoom() * 8.0f + fDistance;
            float fNear, fFar, fWidth, fHeight;
            pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
            if (fFar <= fMaxDistance)
            {
                fMaxDistance = fFar;
            }

            RedCamera::VisibilityT eVisibility;
            if (fRadius > 0.0f)
            {
                eVisibility = pCamera->ClassifySphere(vPos, fRadius, fMaxDistance);
            }
            else
            {
                eVisibility = pCamera->ClassifyPoint(vPos, fMaxDistance);
            }
            if (eVisibility != RedCamera::VISIBILITY_NONE)
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0044f610
int CameraManager::GetCameraIdForView(unsigned int uiView) const
{
    for (int i = 0; i < 1; i++)
    {
        if (m_hView[i] == uiView)
        {
            return i;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0044f640
float CameraManager::UpdateTrackDistance(int iCam, float fDistance, float fDeltaTime)
{
    float fCurrent = mTrackDistance[iCam];
    if (fCurrent > fDistance)
    {
        mTrackDistance[iCam] = fDistance;
        return fDistance;
    }
    if (fDistance == 1.0f || fDistance > fCurrent + 0.15f)
    {
        float fNew = fDeltaTime * 0.75f + fCurrent;
        if (fDistance <= fNew)
        {
            fNew = fDistance;
        }
        mTrackDistance[iCam] = fNew;
        return fNew;
    }
    return fCurrent;
}

// FUNCTION: SWBF2 0x0044f6b0
void CameraInterpolator::CreateClient()
{
    count = 0;
    mCamera = 0;
    mGameCamera = 0;
    if (!netFrameLock)
    {
        ActivateThread(0, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x0044f6e0
bool CameraInterpolator::Update(float)
{
    if (count >= 2)
    {
        float fT = NetGame::GetTurnRatio();
        PblMatrix mNew = mat[count & 1];
        PblMatrix mOld = mat[(count - 1) & 1];
        PblQuaternion qNew = PblQuaternion(mNew);
        PblQuaternion qOld = PblQuaternion(mOld);
        PblVector3 vPos((mOld.m[3][0] - mNew.m[3][0]) * fT + mNew.m[3][0],
                        (mOld.m[3][1] - mNew.m[3][1]) * fT + mNew.m[3][1],
                        (mOld.m[3][2] - mNew.m[3][2]) * fT + mNew.m[3][2]);

        PblQuaternion qRot = Interpolate(qNew, qOld, fT);
        PblQuaternion qUnit;
        PblMatrix mBlend;
        D3DXMatrixRotationQuaternion(&mBlend, D3DXQuaternionNormalize(&qUnit, &qRot));
        mBlend.m[3][0] = vPos.x;
        mBlend.m[3][1] = vPos.y;
        mBlend.m[3][2] = vPos.z;
        mBlend.m[3][3] = 1.0f;
        mCamera->SetMatrix(mBlend);
        mGameCamera->SetupInterpolatedCamera(mCamera);
    }
    return true;
}

// FUNCTION: SWBF2 0x0044f8b0
void CameraInterpolator::SetNext(GameCamera* pGameCamera, RedCamera* pCamera)
{
    mGameCamera = pGameCamera;
    mCamera = pCamera;
    if (pCamera)
    {
        mat[count & 1] = pCamera->_Matrix;
        count++;
    }
    else
    {
        count = 0;
    }
}

// FUNCTION: SWBF2 0x0044e530
float GetDefaultFOV()
{
    return 0.83775806f;
}

// FUNCTION: SWBF2 0x0044e540
float GetWidescreenFOVAdjustment(float fFOV)
{
    const unsigned int uScreenWidth = RedInterfaceScreen::GetScreenWidth();
    const unsigned int uScreenHeight = RedInterfaceScreen::GetScreenHeight();
    const float fAspect =
        RedRenderer::GetScreenAspectRatio() *
        ((float)FLRenderer::GetViewHeight(0) * (float)uScreenWidth) /
        ((float)FLRenderer::GetViewWidth(0) * (float)uScreenHeight);
    const float fScale = 0.75f / fAspect;
    return (float)atan((double)(fScale * (float)tan((double)(fFOV * 0.5f)))) * 2.0f;
}
