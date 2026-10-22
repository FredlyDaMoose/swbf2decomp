#include "CaptureDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "ToolTips.h"

#include "SoundFL/source/win/sndengine.h"
#include "PebbleFL/Common/PblTime.h"
#include "FLInputManager.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatUtil.h"
#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "BootDisplay.h"
#include "BoundaryManager.h"
#include "Cloth.h"
#include "CollisionMesh.h"
#include "EntityHologram.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "LockOnManager.h"
#include "LuaScript.h"
#include "MessageDisplay.h"
#include "MuzzleFlash.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceTowCable.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterObject.h"
#include "PatrolPath.h"
#include "PauseMenu.h"
#include "ScreenTransition.h"
#include "SkyDome.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "Tracker.h"
#include "VehicleSpawn.h"
#include "Battlefront2/Platform/Pc/pcGrassPatch.h"
#include "Battlefront2/Platform/Pc/pcLeafPatch.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "RedEngineFL/Util/ext/hashtable.h"
#include "PebbleFL/Common/PblRegion.h"
#include "CollisionManager.h"
#include "CommandLine.h"
#include "GameMovie.h"
#include "ProceduralAnimation.h"
#include "ShellLoop.h"
#include "RedEngineFL/Interface/RedShadowBlob.h"

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// FUNCTION: SWBF2 0x0044f950
void CaptureDisplay::Create()
{
    sInstance[0] = new CaptureDisplay;
}

// FUNCTION: SWBF2 0x0044f9c0
CaptureDisplay::CaptureDisplay()
{
    mCanRender = true;
    mOwner = 0;
    mSplitRatio = 0.0f;
    mFlagGroup.Enable(false);
    mFlagGroup.SetPosition(0.0f, -30.0f);
    mFlagTop.Enable(true);
    mFlagTop.SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
    mFlagBottom.Enable(true);
    mFlagBottom.SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
    mFlagBottom.SetPosition(0.0f, 64.0f);
    mFlagLocked.Enable(true);
    mFlagLocked.SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
    mFlagGroup.AddElement(&mFlagTop);
    mFlagGroup.AddElement(&mFlagBottom);
    mFlagGroup.AddElement(&mFlagLocked);
}

// SYNTHETIC: SWBF2 0x0044fb00
// CaptureDisplay::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0044fb30
CaptureDisplay::~CaptureDisplay()
{
}

// FUNCTION: SWBF2 0x0044fbb0
void CaptureDisplay::Init()
{
    mFlagTop.SetTexture(0x621cb6df);
    mFlagBottom.SetTexture(0x621cb6df);
    mFlagLocked.SetTexture(0x7f0688e4);
    mVisible = false;
}

// FUNCTION: SWBF2 0x0044fc00
void CaptureDisplay::Bind(unsigned int uiViewport)
{
    mViewport = uiViewport;
    if (CameraManager::sInstance->m_iNumCam > 1)
    {
        mFlagGroup.Scale(0.75f);
    }
    mJoystickIndex = uiViewport;
    FLRenderer::AddInterfaceElement(CameraManager::sInstance->m_hView[uiViewport], &mFlagGroup, 1.0f, 0.5f,
                                    true);
    ActivateThread(-1, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x0044fc80
void CaptureDisplay::Show(Character* pOwner)
{
    if (!mOwner)
    {
        mVisible = true;
        mOwner = pOwner;
    }
}

// FUNCTION: SWBF2 0x0044fca0
void CaptureDisplay::Hide(Character* pOwner)
{
    if (pOwner == mOwner)
    {
        mVisible = false;
        mOwner = 0;
        mFlagGroup.Enable(false);
    }
}

// FUNCTION: SWBF2 0x0044fed0
void CaptureDisplay::EnableViewport(int iViewport, bool bEnable)
{
    CaptureDisplay* pDisplay = sInstance[0];
    if (pDisplay && pDisplay->mViewport == iViewport)
    {
        pDisplay->mCanRender = bEnable;
        if (!bEnable)
        {
            pDisplay->Hide(pDisplay->mOwner);
        }
    }
}

// FUNCTION: SWBF2 0x0044fce0
bool CaptureDisplay::Update(float fDeltaTime)
{
    Character* pChr = Character::FindPlayer(NetGame::GetLocalIndex(mViewport));
    if (!pChr)
    {
        return true;
    }

    if (mCanRender && pChr->mUnit && !GameLoop::IsGameOver())
    {
        if (!mVisible)
        {
            Show(pChr);
        }
    }
    else
    {
        if (!mVisible)
        {
            return true;
        }
        Hide(pChr);
    }
    if (!mVisible)
    {
        return true;
    }

    if (NetGame::IsEnabled() && !(NetGame::IsOneWorld() && NetGame::IsHost()))
    {
        return true;
    }

    UpdateClear();
    for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
    {
        CommandPost* pPost = *itor;
        if (!(pPost->mHoldPlayerMask & (1 << pChr->mPlayerId)))
        {
            continue;
        }

        int iPostTeam = pPost->GetGameObject()->GetTeam();
        ToolTipType eTip = (iPostTeam == pChr->mTeamNumber) ? TTT_ApproachSelfCP : TTT_ApproachEnemyCP;
        if (iPostTeam == 0)
        {
            int iOtherTeam = 0;
            if (pPost->mHoldTeam < 0)
            {
                for (iOtherTeam = 1; iOtherTeam < 8; ++iOtherTeam)
                {
                    if (iOtherTeam != pPost->mBiasTeam && pPost->mHoldCount[iOtherTeam] > 0)
                    {
                        break;
                    }
                }
            }
            UpdateCapture(pPost->mBiasTeam, iOtherTeam, pPost->GetCaptureRatio());
        }
        else if (iPostTeam != pChr->mTeamNumber)
        {
            UpdateNeutralize(iPostTeam, pPost->GetNeutralizeRatio());
        }
        if (eTip != TTT_NONE)
        {
            ToolTipsDisplay::sInstance->ShowTip(eTip, 0);
        }
        return true;
    }
    return true;
}

// FUNCTION: SWBF2 0x0044ff00
void CaptureDisplay::UpdateClear()
{
    mFlagGroup.Enable(false);
}

// FUNCTION: SWBF2 0x0044ff10
void CaptureDisplay::UpdateIcon()
{
    mFlagTop.m_color = mColorTop;
    mFlagTop.SetTexCoords(0.0f, 0.0f, 1.0f, mSplitRatio, false);
    mFlagTop.SetRect(64.0f, mSplitRatio * 64.0f,
                     RedBitmapElement::BITMAP_HALIGN_RIGHT, RedBitmapElement::BITMAP_VALIGN_TOP);

    mFlagBottom.m_color = mColorBottom;
    mFlagBottom.SetTexCoords(0.0f, mSplitRatio, 1.0f, 1.0f, false);
    mFlagBottom.SetRect(64.0f, (1.0f - mSplitRatio) * 64.0f,
                        RedBitmapElement::BITMAP_HALIGN_RIGHT, RedBitmapElement::BITMAP_VALIGN_BOTTOM);

    mFlagLocked.m_color = RedColor::WHITE;
    mFlagLocked.SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);
    mFlagLocked.SetRect(64.0f, 64.0f,
                        RedBitmapElement::BITMAP_HALIGN_RIGHT, RedBitmapElement::BITMAP_VALIGN_TOP);
    mFlagLocked.Enable(false);
    if (mOwner)
    {
        mFlagLocked.Enable(false);
    }
    mFlagGroup.Enable(true);
}

// FUNCTION: SWBF2 0x00450100
void CaptureDisplay::UpdateCapture(int iTeam, int iOtherTeam, float fRatio)
{
    if (!mOwner)
    {
        return;
    }
    mSplitRatio = 1.0f - fRatio;
    int iTop = iOtherTeam;
    if (!iOtherTeam || ((int)(GameLoop::GetMissionTime() * 2.0f) & 1))
    {
        iTop = 0;
    }
    mColorTop = mOwner->mTeamPtr->mColor[iTop];
    mColorBottom = mOwner->mTeamPtr->mColor[iTeam];
    UpdateIcon();
}

// FUNCTION: SWBF2 0x004501e0
void CaptureDisplay::UpdateNeutralize(int iTeam, float fRatio)
{
    if (!mOwner)
    {
        return;
    }
    mSplitRatio = fRatio;
    mColorTop = mOwner->mTeamPtr->mColor[0];
    mColorBottom = mOwner->mTeamPtr->mColor[iTeam];
    UpdateIcon();
}

// GLOBAL: SWBF2 0x01e30328
CaptureDisplay* CaptureDisplay::sInstance[1];
