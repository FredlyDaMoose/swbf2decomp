#include "NameDisplay.h"
#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "Controllable.h"
#include "DeathCamera.h"
#include "EntityCarrier.h"
#include "GameObject.h"
#include "NetGame.h"
#include "Team.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Graphics/RedCamera.h"

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3*, const PblVector3*, const PblMatrix*);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3*, const PblVector3*);

// FUNCTION: SWBF2 0x0043edd0
bool __fastcall HasNameDisplayPilotType(const void* pCharacter)
{
    const int iState = static_cast<const Controllable*>(pCharacter)->mPilotType;
    return iState == 2 || iState == 4;
}

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

extern bool netShowNames;

extern bool netFrameLock;
extern int netCurMaxPlayers;
extern RedColor gHudHighlightColor;
namespace FriendBuddyList { bool __fastcall IsFriendBuddy(int iLocalIndex, int iPlayerId); }
namespace AIDebug { bool GetTestSpawnMode(); }
Controllable* GetControllablePilot(Controllable* pControllable);

// GLOBAL: SWBF2 0x01faa69c

bool gShowLocalPlayerName;

// GLOBAL: SWBF2 0x007e7fe4
float xOffset = 0.51f;
// GLOBAL: SWBF2 0x007e7f9c
float yOffset = 0.54f;

Trackable* CameraManager::GetDeathCameraTarget(int iCam)
{
    if ((GameCamera*)mDeathCamera[iCam] == mCurGameCamera[iCam])
    {
        return mDeathCamera[iCam]->mOwner;
    }
    return 0;
}

Character* CameraManager::GetDeathCameraKiller(int iCam)
{
    if ((GameCamera*)mDeathCamera[iCam] == mCurGameCamera[iCam])
    {
        return mDeathCamera[iCam]->mKiller;
    }
    return 0;
}

// GLOBAL: SWBF2 0x01fa2494
NameDisplayThread* sNameDisplayThread[1];

NameDisplayThread::NameDisplayThread()
{
}

NameDisplayThread::~NameDisplayThread()
{
}

// SYNTHETIC: SWBF2 0x005aa830
// NameDisplayThread::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005aa770
void NameDisplay::Create()
{
    for (int i = 0; i < 1; ++i)
    {
        sNameDisplayThread[i] = new NameDisplayThread;
        sNameDisplayThread[i]->mJoystickIndex = i;
    }
}

// FUNCTION: SWBF2 0x005aa8c0
void NameDisplay::Init()
{
    CameraManager* pCameraManager = CameraManager::sInstance;
    if (pCameraManager->m_iNumCam == 1 && !(NetGame::IsEnabled() && netShowNames))
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        unsigned int hView = CameraManager::sInstance->m_hView[i];
        NameDisplayThread* pThread = sNameDisplayThread[i];
        pThread->ActivateThread(0, PER_FRAME);
        for (int j = 0; j < 64; ++j)
        {
            RedScreenGroupElement* pGroup = &sNameDisplayThread[i]->mPlayerNames[j];
            pGroup->Enable(false);
            pGroup->AddElement(&sNameDisplayThread[i]->mPlayerNamesText[j]);
            RedTextElement* pText = &sNameDisplayThread[i]->mPlayerNamesText[j];
            pText->SetFont(RedFont::GAMEFONT_TINY);
            pText->SetIntercharacterSpacing(0);

            pText->m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
            pText->SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_BOTTOM);
            FLRenderer::AddInterfaceElement(hView, &sNameDisplayThread[i]->mPlayerNames[j],
                                            RedInterfaceScreen::CENTER, RedInterfaceScreen::CENTER,
                                            false);
        }
    }
}

// FUNCTION: SWBF2 0x005aaa10
void NameDisplay::Reset()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        for (int j = 0; j < 64; ++j)
        {
            sNameDisplayThread[i]->mPlayerNamesText[j].Cleanup();
        }
    }
}

// FUNCTION: SWBF2 0x005aaa60
bool NameDisplayThread::Update(float)
{
    int iLocalIndex = NetGame::GetLocalIndex(mJoystickIndex);
    int iSpectator = NetGame::GetSpectator(iLocalIndex);
    if (iSpectator != -1)
    {
        iLocalIndex = iSpectator;
    }

    Character* pLocalCharacter = 0;
    Character* pKiller = 0;
    GameObject* pLocalObject = 0;
    if (Character::sCharacters)
    {
        pLocalCharacter = Character::FindPlayer(iLocalIndex);
        Controllable* pLocalUnit = 0;
        if (pLocalCharacter)
        {
            pLocalUnit = pLocalCharacter->GetVehicle();
            if (!pLocalUnit)
            {
                pLocalUnit = pLocalCharacter->GetUnit();
            }
        }
        if (pLocalUnit)
        {
            pLocalObject = pLocalUnit->GetGameObject();
        }
    }
    if (!pLocalObject)
    {
        Trackable* pTarget = CameraManager::sInstance->GetDeathCameraTarget(mJoystickIndex);
        if (pTarget)
        {
            pLocalObject = pTarget->GetGameObject();
            pKiller = CameraManager::sInstance->GetDeathCameraKiller(mJoystickIndex);
        }
    }

    RedCamera* pCamera = CameraManager::sInstance->mRedCamera[mJoystickIndex];
    int iNumSlots = (int)CameraManager::sInstance->m_iNumCam;
    if (iNumSlots < netCurMaxPlayers)
    {
        iNumSlots = netCurMaxPlayers;
    }
    for (int i = 0; i < iNumSlots; ++i)
    {
        RedScreenGroupElement& group = mPlayerNames[i];
        RedTextElement& text = mPlayerNamesText[i];
        group.Enable(false);
        if (!pLocalObject)
        {
            continue;
        }
        if (!gShowLocalPlayerName && i == NetGame::GetLocalIndex(mJoystickIndex))
        {
            continue;
        }
        Character* pCharacter = Character::FindPlayer(i);
        if (!pCharacter || !pCharacter->IsPlayer())
        {
            continue;
        }
        Controllable* pUnit = pCharacter->GetVehicle();
        if (!pUnit)
        {
            pUnit = pCharacter->GetUnit();
        }
        if (!pUnit || pUnit->GetPlayerId() < 0)
        {
            continue;
        }
        GameObject* pObject = pUnit->GetGameObject();
        if (!pObject)
        {
            continue;
        }
        CollisionObject* pCollision = pObject->GetCollisionObject();
        if (!pCollision || AIDebug::GetTestSpawnMode())
        {
            continue;
        }

        int iLocalTeam = pLocalObject->GetTeam();
        if (iLocalTeam == pLocalObject->GetPerceivedTeam() && iLocalTeam != pObject->GetTeam())
        {
            if (pObject->GetPerceivedTeam() != pObject->GetTeam())
            {
                continue;
            }
            if (pLocalCharacter && !pKiller)
            {
                if (iSpectator == -1)
                {
                    Controllable* pLocalUnit = pLocalCharacter->GetControllable();
                    if (!pLocalUnit)
                    {
                        continue;
                    }
                    const PblHandle<const GameObject>& target =
                        pLocalUnit->mReticuleTarget[pLocalUnit->GetWpnChannel()];
                    if (!(const GameObject*)target)
                    {
                        continue;
                    }
                    if (target != pObject)
                    {
                        Controllable* pOnFoot = pCharacter->GetUnit();
                        if (!pOnFoot || target != pOnFoot->GetGameObject())
                        {
                            continue;
                        }
                    }
                }
            }
            else if (pKiller != pCharacter)
            {
                continue;
            }
        }
        if (pLocalObject == pObject)
        {
            continue;
        }

        float fHeight = pCollision->GetCollisionRadius() * 1.2f;
        PblVector3 vPos = pObject->GetMatrix().GetTranslation();
        if (!netFrameLock && pCollision->GetGameObject())
        {
            GameObject* pHost = pCollision->GetGameObject();
            PblMatrix mHost(pHost->GetMatrix());
            float fOffsetX = vPos.x - mHost.m[3][0];
            float fOffsetY = vPos.y - mHost.m[3][1];
            float fOffsetZ = vPos.z - mHost.m[3][2];
            gameObjectInterpolator.GetMatrix(pHost->GetNetUniqueId(), mHost);
            vPos.x = mHost.m[3][0] + fOffsetX;
            vPos.y = mHost.m[3][1] + fOffsetY;
            vPos.z = mHost.m[3][2] + fOffsetZ;
        }
        vPos.x += 0.0f;
        vPos.y += fHeight;
        vPos.z += 0.0f;

        PblVector3 vScreen;
        vScreen = pCamera->TransformWorldPointToProjectionSpace(vPos);
        if (vScreen.z < -1.0f || vScreen.z > 1.0f)
        {
            continue;
        }
        const PblMatrix& mCamera = pCamera->GetMatrix();
        PblVector3 vEye = mCamera.GetTranslation();
        PblVector3 vToUnit;
        vToUnit.x = vPos.x - vEye.x;
        vToUnit.y = vPos.y - vEye.y;
        vToUnit.z = vPos.z - vEye.z;
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vToUnit);
        const PblVector3& vAt = mCamera.GetZAxis();
        if (!(vDir.y * -vAt.y + vDir.x * -vAt.x + vDir.z * -vAt.z > 0.0f))
        {
            continue;
        }

        Entity* pEntity = pUnit->GetEntity();
        float fFadeRange = pEntity && pEntity->IsRtti(EntityFlyer::rttiHashEntityFlyer) ? 350.0f : 20.0f;
        float fDistance = sqrtf(vToUnit.y * vToUnit.y + vToUnit.x * vToUnit.x + vToUnit.z * vToUnit.z);
        unsigned char ucAlpha = 0xff;
        if (fDistance > fFadeRange)
        {
            int iAlpha = 335 - ((int)fDistance << 2);
            ucAlpha = iAlpha < 0 ? 0 : (unsigned char)iAlpha;
            if (!ucAlpha)
            {
                continue;
            }
        }

        if (pLocalCharacter)
        {
            text.m_color = pLocalCharacter->GetTeamPtr()->GetColor(pCharacter->GetTeamNumber());
            if (NetGame::IsEnabled() && pCharacter->GetTeamNumber() == pLocalCharacter->GetTeamNumber() &&
                FriendBuddyList::IsFriendBuddy(NetGame::GetLocalIndex(mJoystickIndex), i))
            {
                RedColor white(gHudHighlightColor);
                text.m_color = white;
            }
        }
        else
        {
            text.m_color = pCharacter->GetTeamPtr()->GetColor(pLocalObject->GetTeam());
        }
        text.m_color.a = ucAlpha;
        text.SetText(pCharacter->GetName(), true);

        int iOtherRiders = 0;
        Controllable* pVehicle = pCharacter->GetVehicle();
        if (pVehicle)
        {
            const PblList<Controllable>& riders = pVehicle->GetGameObject()->GetControllableList();
            for (PblList<Controllable>::Itor it = riders.Begin(); (Controllable*)it; ++it)
            {
                Controllable* pRider = it;
                if ((HasNameDisplayPilotType(pRider) || pRider->mPilotType == PILOT_REMOTE) && GetControllablePilot(pRider))
                {
                    if (pRider == pVehicle)
                    {
                        break;
                    }
                    ++iOtherRiders;
                }
            }
        }
        if (iOtherRiders > 0)
        {
            group.SetScreenRelativePos(xOffset + vScreen.x * 0.5f, yOffset + vScreen.y * 0.5f);
        }
        else
        {
            group.SetScreenRelativePos(vScreen.x * 0.5f + 0.5f, vScreen.y * 0.5f + 0.5f);
        }
        group.Enable(true);
    }
    return true;
}
