#include "ScopeDisplay.h"
#include "ScopeBlurEffect.h"
#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Tracker.h"
#include "Weapon.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"

class Weapon : public Thread
{
public:
    bool ZoomFirstPerson() const;

    char mWeaponState[0x64 - sizeof(Thread)];
    WeaponClass* mClass;
};
 

namespace FLRenderer
{

void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement,
                         float fX, float fY, bool bUseSafeZone);
}

// GLOBAL: SWBF2 0x01eaf020
ScopeDisplay* ScopeDisplay::sInstance[1];

// GLOBAL: SWBF2 0x007e904a
bool ScopeDisplay::sCanRender = true;

// FUNCTION: SWBF2 0x006334e0
ScopeDisplay::BinocularSound::BinocularSound()
{
}

// FUNCTION: SWBF2 0x00633520
void ScopeDisplay::BinocularSound::Enter()
{
    mAmbient.Play(&mAmbientProps, 0, 0, true);
    ZoomIn();
    ZoomStop();
}

// FUNCTION: SWBF2 0x00633550
void ScopeDisplay::BinocularSound::Exit()
{
    ZoomOut();
    ZoomStop();
    mAmbient.Stop(true);
}

// FUNCTION: SWBF2 0x00633570
void ScopeDisplay::BinocularSound::ZoomIn()
{
    if (!mZoomIn.GetPlaying())
    {
        mZoomIn.Play(&mZoomInProps, 0, 0, true);
    }
}

// FUNCTION: SWBF2 0x00633590
void ScopeDisplay::BinocularSound::ZoomOut()
{
    if (!mZoomOut.GetPlaying())
    {
        ZoomStop();
        mZoomOut.Play(&mZoomOutProps, 0, 0, true);
    }
}

// FUNCTION: SWBF2 0x006335c0
void ScopeDisplay::BinocularSound::ZoomStop()
{
    mZoomOut.Stop(false);
}

// FUNCTION: SWBF2 0x006335d0
void ScopeDisplay::BinocularSound::Update()
{
    mAmbient.Update(&mAmbientProps, 0, 0);
    mZoomIn.Update(&mZoomInProps, 0, 0);
    mZoomOut.Update(&mZoomOutProps, 0, 0);
}

// FUNCTION: SWBF2 0x00633610
void ScopeDisplay::BinocularSound::SetAmbientID(const char* pName)
{
    mAmbientProps.SetID(pName, false, false);
}

// FUNCTION: SWBF2 0x00633970
ScopeDisplay::BinocularSound::~BinocularSound()
{
}

// FUNCTION: SWBF2 0x00633630
void ScopeDisplay::BinocularSound::SetZoomInID(const char* pName)
{
    mZoomInProps.SetID(pName, false, false);
}

// FUNCTION: SWBF2 0x00633650
void ScopeDisplay::BinocularSound::SetZoomOutID(const char* pName)
{
    mZoomOutProps.SetID(pName, false, false);
}

// FUNCTION: SWBF2 0x00633670
void ScopeDisplay::Create()
{
    sInstance[0] = new ScopeDisplay;
}

// FUNCTION: SWBF2 0x006336e0
ScopeDisplay::ScopeDisplay()
{
    mJoystickIndex = -1;
    const unsigned int uHalfWidth = RedInterfaceScreen::GetScreenWidth() / 2;
    const unsigned int uHalfHeight = RedInterfaceScreen::s_screenFull.m_uiHeightInPixels / 2;
    sCanRender = true;
    mGroupWeaponScope.Enable(false);
    mGroupWeaponScope.SendToBack();

    const float fX = (float)uHalfWidth;
    const float fY = (float)uHalfHeight;
    mBitmapWeaponScope[0].SetRect(fX, fY, RedBitmapElement::BITMAP_HALIGN_RIGHT, RedBitmapElement::BITMAP_VALIGN_BOTTOM);
    mBitmapWeaponScope[0].SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);
    mGroupWeaponScope.AddElement(&mBitmapWeaponScope[0]);
    mBitmapWeaponScope[1].SetRect(fX, fY, RedBitmapElement::BITMAP_HALIGN_LEFT, RedBitmapElement::BITMAP_VALIGN_BOTTOM);
    mBitmapWeaponScope[1].SetTexCoords(1.0f, 0.0f, 0.0f, 1.0f, false);
    mGroupWeaponScope.AddElement(&mBitmapWeaponScope[1]);
    mBitmapWeaponScope[2].SetRect(fX, fY, RedBitmapElement::BITMAP_HALIGN_RIGHT, RedBitmapElement::BITMAP_VALIGN_TOP);
    mBitmapWeaponScope[2].SetTexCoords(0.0f, 1.0f, 1.0f, 0.0f, false);
    mGroupWeaponScope.AddElement(&mBitmapWeaponScope[2]);
    mBitmapWeaponScope[3].SetRect(fX, fY, RedBitmapElement::BITMAP_HALIGN_LEFT, RedBitmapElement::BITMAP_VALIGN_TOP);
    mBitmapWeaponScope[3].SetTexCoords(1.0f, 1.0f, 0.0f, 0.0f, false);
    mGroupWeaponScope.AddElement(&mBitmapWeaponScope[3]);

    mScopeTexture = 0;
    mIsZooming = false;
}

// FUNCTION: SWBF2 0x006339d0
ScopeDisplay::~ScopeDisplay()
{
}

// FUNCTION: SWBF2 0x00633a80
void ScopeDisplay::Init()
{
    mInBinocularMode = false;
    mIsZooming = false;
}

// FUNCTION: SWBF2 0x00633a90
void ScopeDisplay::Bind(unsigned int uJoystick)
{
    mJoystickIndex = uJoystick;
    FLRenderer::AddInterfaceElement(CameraManager::sInstance->m_hView[uJoystick],
                                    &mGroupWeaponScope, 0.5f, 0.5f, false);
    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x00633ae0
void ScopeDisplay::Show()
{
    if (!mInBinocularMode)
    {
        mBinocularSound.Enter();
        mPrevZoom = 0.0f;
        mInBinocularMode = true;
        mIsZooming = false;
        ScopeBlurEffect::Start(mJoystickIndex, 255);
        return;
    }
    mIsZooming = false;
}

// FUNCTION: SWBF2 0x00633b30
void ScopeDisplay::Hide()
{
    if (mInBinocularMode)
    {
        mBinocularSound.Exit();
        mGroupWeaponScope.Enable(false);
        mInBinocularMode = false;
        mIsZooming = false;
        ScopeBlurEffect::Stop(mJoystickIndex);
        return;
    }
    mIsZooming = false;
}

// FUNCTION: SWBF2 0x00633b80
void ScopeDisplay::HideAll()
{
    sInstance[0]->Hide();
}

// FUNCTION: SWBF2 0x00633b90
bool ScopeDisplay::Update(float fDeltaTime)
{
    Character* pCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mJoystickIndex));
    if (!pCharacter)
    {
        return true;
    }

    Controllable* pControllable = pCharacter->GetControllable();
    if (!pControllable || !pControllable->GetGameObject() || GameLoop::IsGameOver())
    {
        Hide();
        return true;
    }

    bool bChaseMode = CameraManager::sInstance->IsChaseMode(mJoystickIndex);
    Damageable* pDamageable = static_cast<Damageable*>(pControllable->GetGameObject());
    bool bAiming = pControllable->mTargetInfo.mIsAiming & pDamageable->mIsAlive & bChaseMode;

    Weapon* pWeapon = pControllable->GetWeaponPtr(pControllable->GetWeaponIndex(0));
    bool bFirstPerson = (pWeapon && pWeapon->ZoomFirstPerson()) ||
                        (pControllable->GetTrackable() &&
                         pControllable->GetTrackable()->mTracker &&
                         pControllable->GetTrackable()->mTracker->IsFirstPersonView());

    float fZoom = CameraManager::sInstance->mRedCamera[mJoystickIndex]->GetZoom();
    bool bShow = bFirstPerson & (fZoom > 1.0f) & bAiming;
    if (mInBinocularMode)
    {
        if (!bShow)
        {
            Hide();
        }
    }
    else if (bShow)
    {
        Show();
    }

    if (!mInBinocularMode)
    {
        return true;
    }

    mGroupWeaponScope.Enable(false);
    mBinocularSound.Update();
    if (mIsZooming)
    {
        if (mPrevZoom == fZoom)
        {
            mIsZooming = false;
        }
    }
    else if (mPrevZoom != fZoom)
    {
        mIsZooming = true;
        if (mPrevZoom > fZoom)
        {
            mBinocularSound.ZoomOut();
        }
        else if (fZoom > mPrevZoom)
        {
            mBinocularSound.ZoomIn();
        }
    }
    mPrevZoom = fZoom;

    if (sCanRender && pWeapon)
    {

        unsigned int uTexture = RedTexture::Find(pWeapon->mClass->mScopeTexture)
                                    ? pWeapon->mClass->mScopeTexture
                                    : 0xF6BCFA8E;
        if (mScopeTexture != uTexture)
        {
            mScopeTexture = uTexture;
            for (int i = 0; i < 4; ++i)
            {
                mBitmapWeaponScope[i].SetTexture(mScopeTexture);
            }
        }
        if (mScopeTexture)
        {
            mGroupWeaponScope.Enable(true);
        }
    }
    return true;
}
