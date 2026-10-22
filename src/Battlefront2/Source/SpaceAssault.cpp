#include "SpaceAssault.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "AIUtil.h"
#include "CameraManager.h"
#include "Character.h"
#include "Entity.h"
#include "GameObject.h"
#include "HUDElementGroup.h"
#include "HUDElementMap.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "NetGame.h"
#include "Team.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedGroupElement.h"

 

namespace
{

inline float GetCurHealth(const GameObject* p) { return p->mCurHealth; }
inline float GetMaxHealth(const GameObject* p) { return p->mMaxHealth; }
inline float GetCurShield(const GameObject* p) { return p->mCurShield; }
inline float GetMaxShield(const GameObject* p) { return p->mMaxShield; }
inline bool IsAlive(const GameObject* p) { return p->IsAlive(); }
inline int GetTeam(const GameObject* p) { return p->GetTeam(); }

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t)
{
    RedColor result;
    result.b = (unsigned char)(int)(a.b * (1.0f - t) + b.b * t + 0.5f);
    result.g = (unsigned char)(int)(a.g * (1.0f - t) + b.g * t + 0.5f);
    result.r = (unsigned char)(int)(a.r * (1.0f - t) + b.r * t + 0.5f);
    result.a = (unsigned char)(int)(a.a * (1.0f - t) + b.a * t + 0.5f);
    return result;
}

inline int GetMapMode(const HUD::ElementMap* pMap)
{
    return pMap->mMode;
}

}

// GLOBAL: SWBF2 0x01eafb2c
SpaceAssault* SpaceAssault::mInstance;

// GLOBAL: SWBF2 0x01f6de48
// SYNTHETIC: SWBF2 0x004063b0
// `dynamic initializer for 'SpaceAssault::ViewportData::sMemoryPool''
// SYNTHETIC: SWBF2 0x007694c0
// `dynamic atexit destructor for 'SpaceAssault::ViewportData::sMemoryPool''
MemoryPool SpaceAssault::ViewportData::sMemoryPool("SpaceAssault::ViewportData", 0x2410);

// FUNCTION: SWBF2 0x0064b7c0
SpaceAssault::ViewportData::ViewportData()
    : team(0), isVisible(false)
{
}

// FUNCTION: SWBF2 0x0064ba80
SpaceAssault::ViewportData::~ViewportData()
{
}

// FUNCTION: SWBF2 0x0064b860
void SpaceAssault::Create()
{
    mInstance = new SpaceAssault;
}

// FUNCTION: SWBF2 0x0064b890
void SpaceAssault::CreateClient()
{
    if (NetGame::IsOneWorld() && !NetGame::IsHost())
    {
        if (mInstance)
        {
            mInstance->DeactivateThread();
            mInstance->ActivateThread(-10, PER_TURN);
        }
    }
}

// FUNCTION: SWBF2 0x0064b8d0
void SpaceAssault::Destroy()
{
    delete mInstance;
    mInstance = 0;
}

// FUNCTION: SWBF2 0x0064b8f0
SpaceAssault::SpaceAssault()
    : mpViewportData(0),
      mNumCriticalSystems(0),
      mAreCriticalSystemsDirty(true),
      mbEnabled(false),
      mTeamATT(1),
      mTeamDEF(2),
      mShipBitmapATTHashId(0),
      mShipBitmapDEFHashId(0),
      mShieldBitmapATTHashId(0),
      mShieldBitmapDEFHashId(0),
      mCriticalSystemBitmapATTHashId(0),
      mCriticalSystemBitmapDEFHashId(0),
      mShieldFlashTimerATT(0.0f),
      mShieldFlashTimerDEF(0.0f),
      mLastShieldATT(0.0f),
      mLastShieldDEF(0.0f),
      mWidth(0.0f),
      mHeight(0.0f),
      mpGroupElement(0)
{
    memset(mCriticalSystems, 0, sizeof(mCriticalSystems));
}

// SYNTHETIC: SWBF2 0x0064b9d0
// SpaceAssault::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0064ba00
SpaceAssault::~SpaceAssault()
{
    delete[] mpViewportData;
}

// FUNCTION: SWBF2 0x0064bb10
bool SpaceAssault::IsCriticalSystem(GameObject* pObject) const
{
    if (!mbEnabled)
    {
        return false;
    }
    if (!NetGame::IsEnabled() || !NetGame::IsHost())
    {
        return false;
    }

    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        GameObject* pSystem = mCriticalSystems[i].pGameObject;
        if (pSystem && pSystem == pObject)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0064bb90
unsigned int SpaceAssault::GetIsAliveTurnNumber(GameObject* pObject) const
{
    if (!mbEnabled)
    {
        return 0;
    }
    if (!NetGame::IsEnabled() || !NetGame::IsHost())
    {
        return 0;
    }

    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        GameObject* pSystem = mCriticalSystems[i].pGameObject;
        if (pSystem && pSystem == pObject)
        {
            return mCriticalSystems[i].isAliveTurnNumber;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0064bc10
bool SpaceAssault::Update(float fDeltaTime)
{
    if (!mbEnabled)
    {
        return true;
    }

    if (NetGame::IsEnabled() && NetGame::IsHost())
    {
        for (int i = 0; i < mNumCriticalSystems; ++i)
        {
            CriticalSystemData& system = mCriticalSystems[i];
            if (!system.pGameObject)
            {
                continue;
            }
            if (system.isAlive)
            {
                if (!IsAlive(system.pGameObject))
                {
                    system.isAliveTurnNumber = NetGame::GetTurnNumber();
                    system.isAlive = false;
                }
            }
            else if (IsAlive(system.pGameObject))
            {
                system.isAliveTurnNumber = NetGame::GetTurnNumber();
                system.isAlive = true;
            }
        }
    }

    if (!mbEnabled)
    {
        return true;
    }
    if (NetGame::IsEnabled() && NetGame::IsHost() && NetGame::IsDedicated())
    {
        return true;
    }

    for (int iView = 0; iView < (int)CameraManager::Instance()->GetNumCameras(); ++iView)
    {
        ViewportData& vp = mpViewportData[iView];
        Character* pPlayer = NetGame::GetLocalPlayer(iView);
        if (!pPlayer)
        {
            continue;
        }

        if (mAreCriticalSystemsDirty || vp.team != pPlayer->mTeamNumber || !mpGroupElement ||
            mpGroupElement->GetHeightPixelsFloat() != mHeight ||
            mpGroupElement->GetWidthPixelsFloat() != mWidth)
        {
            vp.team = pPlayer->mTeamNumber;
            InitBitmaps(iView);
        }
        if (!mpGroupElement)
        {
            continue;
        }

        vp.elemShipFriendly.SetColor(Team::GetColorByAffiliation(1));
        vp.elemShipEnemy.SetColor(Team::GetColorByAffiliation(-1));

        for (int i = 0; i < mNumCriticalSystems; ++i)
        {
            CriticalSystemData& system = mCriticalSystems[i];
            RedBitmapElement& marker = vp.elemCriticalSystem[i];
            if (!system.bShowMarker || !IsAlive(system.pGameObject))
            {
                marker.SetColor(RedColor::BLACK);
                continue;
            }

            float fHealth = CalcHealthPercentage(system);
            RedColor color = CalcDamageColor(fHealth);
            if (system.lastHealth > fHealth && system.flashTimer <= 0.0f)
            {
                system.flashTimer = 1.0f;
            }
            if (system.flashTimer > 0.0f)
            {
                color = Interpolate(color, RedColor::WHITE, system.flashTimer);
                system.flashTimer -= fDeltaTime;
            }

            marker.m_color.b = color.b;
            marker.m_color.g = color.g;
            marker.m_color.r = color.r;
            system.lastHealth = fHealth;
        }

        int iFriendly = vp.team;
        bool bFriendlyDone = false;
        bool bEnemyDone = false;
        int iEnemy = (iFriendly == 1) ? 2 : 1;
        for (int i = 0; i < mNumCriticalSystems; ++i)
        {
            CriticalSystemData& system = mCriticalSystems[i];
            int iTeam = system.team;
            if (!iTeam || iTeam > 2)
            {
                continue;
            }
            if (iTeam == iFriendly && bFriendlyDone)
            {
                continue;
            }
            if (iTeam == iEnemy && bEnemyDone)
            {
                continue;
            }

            float fMaxShield = GetMaxShield(system.pGameObject);
            if (fMaxShield <= 0.0001f)
            {
                continue;
            }
            float fCurShield = GetCurShield(system.pGameObject);

            const RedColor empty = { 255, 0, 0, 0 };
            const RedColor full = { 0, 200, 150, 255 };
            RedColor color = Interpolate(empty, full, fCurShield / fMaxShield);

            float* pLastShield = (iTeam == mTeamATT) ? &mLastShieldATT : &mLastShieldDEF;
            float* pFlashTimer = (iTeam == mTeamATT) ? &mShieldFlashTimerATT : &mShieldFlashTimerDEF;
            if (*pLastShield > fCurShield && *pFlashTimer <= 0.0f)
            {
                *pFlashTimer = 1.0f;
            }
            if (*pFlashTimer > 0.0f)
            {
                color = Interpolate(color, RedColor::WHITE, *pFlashTimer);
                *pFlashTimer -= fDeltaTime;
            }
            *pLastShield = GetCurShield(system.pGameObject);

            if (system.team == vp.team)
            {
                vp.elemShieldFriendly.SetColor(color);
                bFriendlyDone = true;
            }
            else if (system.team == iEnemy)
            {
                vp.elemShieldEnemy.SetColor(color);
                bEnemyDone = true;
            }
            else if (bFriendlyDone && bEnemyDone)
            {
                break;
            }
        }

        HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(iView);
        if (pMap)
        {
            HUD::Event event;
            if (GetMapMode(pMap) == HUD::ElementMap::mode_Small)
            {
                if (!vp.isVisible)
                {
                    vp.isVisible = true;
                    event.mClass = HUD::GameEvents::gPlayerEvents[iView].spaceAssaultStatus;
                    event.mData.boolValue = true;
                    event.Send();
                }
            }
            else if (vp.isVisible)
            {
                vp.isVisible = false;
                event.mClass = HUD::GameEvents::gPlayerEvents[iView].spaceAssaultStatusDisable;
                event.mData.boolValue = true;
                event.Send();
            }
        }
    }

    mAreCriticalSystemsDirty = false;
    return true;
}

// FUNCTION: SWBF2 0x0064c390
void SpaceAssault::AddCriticalSystem(const char* pName, short pointValue, float hudPosX,
                                     float hudPosY, bool bShowMarker)
{
    if (mNumCriticalSystems >= MAX_CRITICAL_SYSTEMS)
    {
        return;
    }

    unsigned int uNameHashId = PblHash(pName);
    EntityEx* pEntity = EntityEx::mIdMap.Find(uNameHashId);
    if (!pEntity)
    {
        return;
    }

    CriticalSystemData& system = mCriticalSystems[mNumCriticalSystems];
    system.pointValue = pointValue;
    system.hudPosX = hudPosX;
    system.nameHashId = uNameHashId;
    system.hudPosY = hudPosY;
    system.bShowMarker = bShowMarker;
    system.pGameObject = pEntity->GetGameObject();
    if (system.pGameObject)
    {
        system.team = GetTeam(system.pGameObject);
        ++mNumCriticalSystems;
        mAreCriticalSystemsDirty = true;
    }
}

// FUNCTION: SWBF2 0x0064c440
void SpaceAssault::LinkCriticalSystems(unsigned int* const pNameHashIds, int iCount)
{
    CriticalSystemData* pPrev = FindCriticalSystemData(pNameHashIds[0]);
    if (!pPrev)
    {
        return;
    }

    for (int i = 1; i < iCount; ++i)
    {
        CriticalSystemData* pNext = FindCriticalSystemData(pNameHashIds[i]);
        pPrev->pNextLinkedSystem = pNext;
        if (!pNext)
        {
            return;
        }
        pNext->bShowMarker = false;
        pPrev = pNext;
    }
    pPrev->pNextLinkedSystem = 0;
}

// FUNCTION: SWBF2 0x0064c4a0
short SpaceAssault::GetPointValue(unsigned int uNameHashId, Character* pCharacter)
{
    if (!pCharacter || !uNameHashId)
    {
        return 0;
    }

    short value = 0;
    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        if (mCriticalSystems[i].nameHashId == uNameHashId)
        {
            value = mCriticalSystems[i].pointValue;
            break;
        }
    }

    EntityEx* pEntity = EntityEx::mIdMap.Find(uNameHashId);
    if (pEntity && GetTeam(pEntity->GetGameObject()) == pCharacter->mTeamNumber)
    {
        return -value;
    }
    return value;
}

// FUNCTION: SWBF2 0x0064c530
void SpaceAssault::Enable(bool bEnable)
{
    mbEnabled = bEnable;
    if (bEnable)
    {
        ActivateThread(0, PER_LOCAL_TURN);
    }
    else
    {
        DeactivateThread();
    }

    if (!mpViewportData && bEnable)
    {
        ViewportData::SetupMemoryPool(1, ViewportData::sMemoryPool.GetSize(),
                                      ViewportData::sMemoryPool.GetGrow());
        mpViewportData = new ViewportData[1];
    }
}

// FUNCTION: SWBF2 0x0064c5f0
void SpaceAssault::SetTeamNumbers(int iTeamATT, int iTeamDEF)
{
    mTeamATT = iTeamATT;
    mTeamDEF = iTeamDEF;
    mAreCriticalSystemsDirty = true;
}

// FUNCTION: SWBF2 0x0064c610
void SpaceAssault::SetHUDBitmaps(unsigned int uShipATT, unsigned int uShipDEF,
                                 unsigned int uShieldATT, unsigned int uShieldDEF,
                                 unsigned int uCriticalSystemATT, unsigned int uCriticalSystemDEF)
{
    mShipBitmapATTHashId = uShipATT;
    mShipBitmapDEFHashId = uShipDEF;
    mShieldBitmapATTHashId = uShieldATT;
    mShieldBitmapDEFHashId = uShieldDEF;
    mCriticalSystemBitmapATTHashId = uCriticalSystemATT;
    mCriticalSystemBitmapDEFHashId = uCriticalSystemDEF;
    mAreCriticalSystemsDirty = true;
}

// FUNCTION: SWBF2 0x0064c660
void SpaceAssault::InitBitmaps(int iViewport)
{
    ViewportData& vp = mpViewportData[iViewport];

    char acName[64];
    _snprintf(acName, sizeof(acName), "player%dspaceassaultgroup", iViewport + 1);
    HUD::Element* pElement = HUD::Element::FindByHashID(PblHash(acName));
    if (!pElement || !pElement->GetRedInterfaceElement().IsGroup())
    {
        return;
    }

    mpGroupElement = static_cast<HUD::ElementGroup*>(pElement);
    RedGroupElement& group = static_cast<RedGroupElement&>(pElement->GetRedInterfaceElement());
    mWidth = mpGroupElement->GetWidthPixelsFloat();
    mHeight = mpGroupElement->GetHeightPixelsFloat();

    RedBitmapElement& shipFriendly = vp.elemShipFriendly;
    RedBitmapElement& shipEnemy = vp.elemShipEnemy;
    RedBitmapElement& shieldFriendly = vp.elemShieldFriendly;
    RedBitmapElement& shieldEnemy = vp.elemShieldEnemy;

    shipFriendly.SetPosition(mWidth * 0.5f, 0.0f);
    shipFriendly.SetRect(mWidth * 0.5f, mHeight, RedBitmapElement::BITMAP_HALIGN_LEFT,
                         RedBitmapElement::BITMAP_VALIGN_TOP);
    shieldFriendly.SetPosition(mWidth * 0.5f, 0.0f);
    shieldFriendly.SetRect(mWidth * 0.5f, mHeight, RedBitmapElement::BITMAP_HALIGN_LEFT,
                           RedBitmapElement::BITMAP_VALIGN_TOP);
    shipEnemy.SetPosition(0.0f, 0.0f);
    shipEnemy.SetRect(mWidth * 0.5f, mHeight, RedBitmapElement::BITMAP_HALIGN_LEFT,
                      RedBitmapElement::BITMAP_VALIGN_TOP);
    shieldEnemy.SetPosition(0.0f, 0.0f);
    shieldEnemy.SetRect(mWidth * 0.5f, mHeight, RedBitmapElement::BITMAP_HALIGN_LEFT,
                        RedBitmapElement::BITMAP_VALIGN_TOP);

    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        CriticalSystemData& system = mCriticalSystems[i];
        RedBitmapElement& marker = vp.elemCriticalSystem[i];
        float fX;
        if (system.team == vp.team)
        {
            fX = system.hudPosX * (mWidth * 0.5f) + mWidth * 0.5f;
        }
        else
        {
            fX = mWidth * 0.5f * system.hudPosX;
        }
        marker.SetPosition(fX, mHeight * system.hudPosY);
        marker.SetRect(mWidth * 0.15f, mHeight * 0.15f, RedBitmapElement::BITMAP_HALIGN_CENTER,
                       RedBitmapElement::BITMAP_VALIGN_CENTER);
    }

    group.RemoveElement(&shipFriendly);
    group.RemoveElement(&shipEnemy);
    group.RemoveElement(&shieldFriendly);
    group.RemoveElement(&shieldEnemy);
    group.AddElement(&shipFriendly);
    group.AddElement(&shipEnemy);
    group.AddElement(&shieldFriendly);
    group.AddElement(&shieldEnemy);
    for (int i = 0; i < MAX_CRITICAL_SYSTEMS; ++i)
    {
        group.RemoveElement(&vp.elemCriticalSystem[i]);
        group.AddElement(&vp.elemCriticalSystem[i]);
    }

    if (vp.team == mTeamATT)
    {
        shipFriendly.SetTexture(mShipBitmapATTHashId);
        shieldFriendly.SetTexture(mShieldBitmapATTHashId);
        shipEnemy.SetTexture(mShipBitmapDEFHashId);
        shieldEnemy.SetTexture(mShieldBitmapDEFHashId);
    }
    else if (vp.team == mTeamDEF)
    {
        shipFriendly.SetTexture(mShipBitmapDEFHashId);
        shieldFriendly.SetTexture(mShieldBitmapDEFHashId);
        shipEnemy.SetTexture(mShipBitmapATTHashId);
        shieldEnemy.SetTexture(mShieldBitmapATTHashId);
    }

    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        RedBitmapElement& marker = vp.elemCriticalSystem[i];
        if (mCriticalSystems[i].team == mTeamATT)
        {
            marker.SetTexture(mCriticalSystemBitmapATTHashId);
        }
        else if (mCriticalSystems[i].team == mTeamDEF)
        {
            marker.SetTexture(mCriticalSystemBitmapDEFHashId);
        }
    }

    shipFriendly.Enable(true);
    shipFriendly.SetZOrder(50);
    shipEnemy.Enable(true);
    shipEnemy.SetZOrder(50);
    shieldFriendly.Enable(true);
    shieldFriendly.SetZOrder(49);
    shieldEnemy.Enable(true);
    shieldEnemy.SetZOrder(49);

    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        RedBitmapElement& marker = vp.elemCriticalSystem[i];
        marker.Enable(mCriticalSystems[i].bShowMarker);
        marker.SetZOrder(48);
    }
}

// FUNCTION: SWBF2 0x0064cb60
float SpaceAssault::CalcHealthPercentage(CriticalSystemData& data)
{
    float fCur = 0.0f;
    float fMax = 0.0f;
    for (CriticalSystemData* p = &data; p; p = p->pNextLinkedSystem)
    {
        fCur += GetCurHealth(p->pGameObject);
        fMax += GetMaxHealth(p->pGameObject);
    }
    if (fMax > 0.0f)
    {
        return fCur / fMax;
    }
    return 0.0f;
}

// FUNCTION: SWBF2 0x0064cbc0
SpaceAssault::CriticalSystemData* SpaceAssault::FindCriticalSystemData(unsigned int uNameHashId)
{
    for (int i = 0; i < MAX_CRITICAL_SYSTEMS; ++i)
    {
        CriticalSystemData* pData = &mCriticalSystems[i];
        if (!pData->nameHashId)
        {
            break;
        }
        if (pData->nameHashId == uNameHashId)
        {
            return pData;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0064cbf0
bool SpaceAssault::IsOkForRandom(const CriticalSystemData& data, int iTeam)
{
    return data.nameHashId && data.team == iTeam && data.bShowMarker && data.pGameObject &&
           GetCurHealth(data.pGameObject) > 0.0f;
}

// FUNCTION: SWBF2 0x0064cc20
GameObject* SpaceAssault::GetRandomCritSys(const Team* pTeam, float fBias) const
{
    int iTeam = pTeam->mTeamNumber;
    int iCount = 0;
    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        if (IsOkForRandom(mCriticalSystems[i], iTeam))
        {
            ++iCount;
        }
    }
    if (!iCount)
    {
        return 0;
    }

    float fRandom = AIUtil::GetRandomFloat(0.0f, 1.0f);
    fRandom = (float)fmod((double)(fRandom * fRandom + fBias), 1.0);
    int iPick = (int)(fRandom * (float)iCount);
    for (int i = 0; i < mNumCriticalSystems; ++i)
    {
        if (IsOkForRandom(mCriticalSystems[i], iTeam) && --iPick < 0)
        {
            return mCriticalSystems[i].pGameObject;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0064cd20
RedColor SpaceAssault::CalcDamageColor(float fHealth)
{
    if (fHealth <= 0.5f)
    {
        return Interpolate(RedColor::RED, RedColor::YELLOW, fHealth * 2.0f);
    }
    return Interpolate(RedColor::YELLOW, RedColor::GREEN, (fHealth - 0.5f) * 2.0f);
}
