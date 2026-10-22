#include "ItemTimerDisplay.h"

#include <float.h>
#include <math.h>

#include "CameraManager.h"
#include "Character.h"
#include "CollisionObject.h"
#include "EntityItem.h"
#include "GameLoop.h"
#include "NetGame.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Interface/RedFont.h"

namespace FLRenderer
{
void AddInterfaceElement(unsigned int hView, RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

static bool EntityItem_NoDisplayLifeTimer(const EntityItem* pItem)
{
    return pItem->mClass->mNoLifeSpanDisplay;
}

static float EntityItem_GetLifeTimer(const EntityItem* pItem)
{
    return pItem->mLifeTimer;
}

static const TreeGridObject* EntityItem_GetTreeGridObject(const EntityItem* pItem)
{
    return static_cast<const TreeGridObject*>(pItem);
}

static Character* EntityItem_GetOwner(const EntityItem* pItem)
{
    return pItem->mOwner;
}

// GLOBAL: SWBF2 0x01f9c2e0
ItemTimerDisplay::DisplayThread* sDisplayThread[1];

static bool ShowTimers()
{
    return !NetGame::IsEnabled() || NetGame::IsClient() ||
           (NetGame::IsOneWorld() && NetGame::IsHost() && !NetGame::IsDedicated());
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                       const PblMatrix* pM);

// FUNCTION: SWBF2 0x0056a6b0
const PblVector3 RedCamera::TransformWorldPointToProjectionSpace(const PblVector3& rPoint) const
{
    PblVector3 vCameraPoint;
    D3DXVec3TransformCoord(&vCameraPoint, &rPoint, &_MatrixInverse);
    return TransformCameraPointToProjectionSpace(vCameraPoint);
}

ItemTimerDisplay::DisplayThread::~DisplayThread()
{
}

// FUNCTION: SWBF2 0x0056a6f0
void ItemTimerDisplay::Create()
{
    for (int i = 0; i < 1; ++i)
    {
        sDisplayThread[i] = new DisplayThread;
        sDisplayThread[i]->mJoystickIndex = i;
    }
}

// FUNCTION: SWBF2 0x0056a840
void ItemTimerDisplay::Init()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        const unsigned int hView = CameraManager::sInstance->m_hView[i];
        DisplayThread* pThread = sDisplayThread[i];
        pThread->ActivateThread(0, PER_FRAME);
        pThread->mItemTimerCount = 0;

        for (int j = 0; j < 8; ++j)
        {
            pThread->mItemArray[j] = 0;
            RedScreenGroupElement* pGroup = &pThread->mItemGroup[j];
            pGroup->Enable(false);
            pGroup->AddElement(&pThread->mItemText[j]);

            RedTextElement* pText = &pThread->mItemText[j];
            pText->SetFont(RedFont::GAMEFONT_TINY);
            pText->SetIntercharacterSpacing(0);
            pText->m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
            pText->SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_BOTTOM);

            FLRenderer::AddInterfaceElement(hView, pGroup, 0.5f, 0.5f, false);
        }
    }
}

// FUNCTION: SWBF2 0x0056a970
void ItemTimerDisplay::Reset()
{
    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            sDisplayThread[i]->mItemText[j].Cleanup();
        }
    }
}

// FUNCTION: SWBF2 0x0056a9c0
void ItemTimerDisplay::AddItemTimer(EntityItem* pItem)
{
    if (!ShowTimers())
    {
        return;
    }
    if (EntityItem_NoDisplayLifeTimer(pItem))
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        DisplayThread* pThread = sDisplayThread[i];
        if (pThread->mItemTimerCount >= 8)
        {
            continue;
        }

        Character* pPlayer = Character::FindPlayer(NetGame::GetLocalIndex(i));
        Character* pOwner = EntityItem_GetOwner(pItem);
        if ((!pOwner || pOwner == pPlayer) && EntityItem_GetLifeTimer(pItem) < FLT_MAX)
        {
            pThread->mItemArray[pThread->mItemTimerCount++] = pItem;
        }
    }
}

// FUNCTION: SWBF2 0x0056aac0
void ItemTimerDisplay::RemoveItemTimer(EntityItem* pItem)
{
    if (!ShowTimers())
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        DisplayThread* pThread = sDisplayThread[i];
        int j = 0;
        for (; j < pThread->mItemTimerCount; ++j)
        {
            if (pThread->mItemArray[j] == pItem)
            {
                break;
            }
        }
        if (j < pThread->mItemTimerCount)
        {
            --pThread->mItemTimerCount;
            for (; j < pThread->mItemTimerCount; ++j)
            {
                pThread->mItemArray[j] = pThread->mItemArray[j + 1];
            }
        }
    }
}

// FUNCTION: SWBF2 0x0056ab90
void ItemTimerDisplay::HideAll()
{
    if (!ShowTimers())
    {
        return;
    }

    for (unsigned int i = 0; i < CameraManager::sInstance->m_iNumCam; ++i)
    {
        for (int j = 0; j < sDisplayThread[i]->mItemTimerCount; ++j)
        {
            sDisplayThread[i]->mItemGroup[j].Enable(false);
        }
    }
}

// FUNCTION: SWBF2 0x0056ac50
bool ItemTimerDisplay::DisplayThread::Update(float fDeltaTime)
{
    RedCamera* pCamera = CameraManager::sInstance->GetCamera(mJoystickIndex);

    for (int i = 0; i < 8; ++i)
    {
        RedScreenGroupElement* pGroup = &mItemGroup[i];
        pGroup->Enable(false);
    }

    if (GameLoop::IsGameOver())
    {
        return true;
    }

    for (int i = 0; i < mItemTimerCount; ++i)
    {
        const TreeGridObject* pObject = EntityItem_GetTreeGridObject(mItemArray[i]);
        float fHeight = pObject->GetCollisionRadius() * 1.2f;
        const PblVector3& vCenter = pObject->GetCollisionCenter();
        PblVector3 vPos(vCenter.x + gVectorUnitY.x * fHeight,
                        vCenter.y + gVectorUnitY.y * fHeight,
                        vCenter.z + gVectorUnitY.z * fHeight);

        PblVector3 vScreen = pCamera->TransformWorldPointToProjectionSpace(vPos);
        if (vScreen.z < -1.0f || vScreen.z > 1.0f)
        {
            continue;
        }

        const PblMatrix& mCamera = pCamera->GetMatrix();
        PblVector3 vCamPos = *(const PblVector3*)&mCamera.m[3][0];
        PblVector3 vDelta(vPos.x - vCamPos.x, vPos.y - vCamPos.y, vPos.z - vCamPos.z);
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &vDelta);
        if (vDir.y * -mCamera.m[2][1] + vDir.x * -mCamera.m[2][0] + vDir.z * -mCamera.m[2][2] <= 0.0f)
        {
            continue;
        }

        float fDist = sqrtf((vPos.x - vCamPos.x) * (vPos.x - vCamPos.x) +
                            (vPos.y - vCamPos.y) * (vPos.y - vCamPos.y) +
                            (vPos.z - vCamPos.z) * (vPos.z - vCamPos.z));
        unsigned char uAlpha = 0xff;
        if (fDist > 20.0f)
        {
            int iAlpha = 255 - ((int)fDist - 20) * 4;
            uAlpha = (unsigned char)(iAlpha < 0 ? 0 : iAlpha);
            if (!uAlpha)
            {
                continue;
            }
        }

        RedTextElement* pText = &mItemText[i];
        RedColor color;
        color.b = 0xf2;
        color.g = 0xa7;
        color.r = 0x43;
        color.a = uAlpha;
        pText->SetColor(color);
        pText->SetTextFormatted("%d", (int)ceilf(EntityItem_GetLifeTimer(mItemArray[i])));

        RedScreenGroupElement* pGroup = &mItemGroup[i];
        pGroup->SetScreenRelativePos(vScreen.x * 0.5f + 0.5f, vScreen.y * 0.5f + 0.5f);
        pGroup->Enable(true);
    }
    return true;
}
