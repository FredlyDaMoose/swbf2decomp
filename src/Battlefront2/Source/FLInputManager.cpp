#include "FLInputManager.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Interface/RedFont.h"

#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "lglcd/lgLcd.h"
#include "PauseMenu.h"
#include "PlayerProfile.h"

#include <string.h>
#include "FLGuiManager.h"
#include "FLJoystick.h"
#include "LuaHelper.h"

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include "Character.h"
#include "Controllable.h"
#include "Damageable.h"
#include "HUDObjectiveList.h"
#include "NetGame.h"
#include "Weapon.h"
#include "WeaponCannonInstance.h"
#include "NetState.h"
#include "EnergyBar.h"
#include "PebbleFL/Common/PblUnicode.h"

namespace PblBuildType { extern bool gDemoBuild; }

// GLOBAL: SWBF2 0x01ebdc50
// SYNTHETIC: SWBF2 0x004039d0
// `dynamic initializer for 'FLInputManager::s_instance''
// SYNTHETIC: SWBF2 0x007687a0
// `dynamic atexit destructor for 'FLInputManager::s_instance''
FLInputManager FLInputManager::s_instance;

// FUNCTION: SWBF2 0x00529ef0
FLInputManager::FLInputManager()
{
    m_HWPort_Full[0] = -1;
    m_bHudGroupsEnabled[0] = false;
    m_bHudGroupsEnabled[1] = false;
    mChatText[0] = 0;
    m_bHudGroupsEnabled[2] = false;
    mBindAll = false;
    m_HWPort[0] = 0;
    mPrimaryPort = 0;
    mLCDPageToggle[0] = false;
    mTextTimer = 0.0f;
    mPageTimer = 0.0f;
    mPageIndex = 0;
    mIgnoreMissing = false;
    mAutoAcquire = false;
    mReadAllControllers = false;
    mAllowBindMultiple = true;
    m_MissingPort = -1;
    m_bPausedByInput = false;
    mSecondaryPort = -1;
    mShadowPrimary = -1;
    mShadowSecondary = -1;
    mActiveControllers[0] = true;
}

// FUNCTION: SWBF2 0x00529b30
FLInputManager::~FLInputManager()
{
}

// FUNCTION: SWBF2 0x0052a190
RawControllerInputs* FLInputManager::GetPlayerJoystick(int iPlayer)
{
    if ((unsigned int)iPlayer >= 1)
    {
        return 0;
    }

    return &mJoysticks[iPlayer];
}

struct CustomGlyphData
{
    unsigned short unicodeValue;
    unsigned short mReserved;
    unsigned int textureHash;
};

// GLOBAL: SWBF2 0x007ec140
static const CustomGlyphData gCustomGlyphData[7] =
{
    { 0x7fff, 0, 0x7f0688e4u },
    { 0x8100, 0, 0x592f1c52u },
    { 0x8101, 0, 0x7f0688e4u },
    { 0x8200, 0, 0x00288768u },
    { 0x8201, 0, 0xee286b12u },
    { 0x0000, 0, 0x00000000u },
    { 0x0000, 0, 0x00000000u },
};

// FUNCTION: SWBF2 0x00529de0
void FLInputManager::InitCustomFontData()
{
    CustomFontData& d = RedFont::GetMagicCharFontData();

    d.nameHash_ = PblHash("SpecialFont");
    d.numGlyphs_ = 7;
    d.numTextures_ = 7;
    d.glyphHeight_ = 0x20;

    d.glyphs_ = new RedFont::Glyph[d.numGlyphs_ + 64];
    d.textureHashes_ = new unsigned int[d.numTextures_ + 64];

    d.defaultMap_.Reallocate(d.numGlyphs_ * 2);

    for (unsigned int i = 0; i < d.numGlyphs_; i++)
    {
        d.glyphs_[i].value = gCustomGlyphData[i].unicodeValue;
        d.textureHashes_[i] = gCustomGlyphData[i].textureHash;
        d.defaultMap_[d.textureHashes_[i]] = d.glyphs_[i].value;
    }

    for (unsigned int i = 0; i < d.numGlyphs_; i++)
    {
        d.glyphs_[i].texnum = (unsigned char)(0x80 + i);
        d.glyphs_[i].width = 0x32;
    }
}

#define FLIM_AT(T, off) (*reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(&FLInputManager::s_instance) + (off)))
typedef RedJoystick* tJoystickPorts10[10];
typedef unsigned char tBytes4C[0x4c];
typedef unsigned char tBytes168[0x168];

// GLOBAL: SWBF2 0x01ebe078
RawControllerInputs& gPrimaryRawInputs = FLInputManager::s_instance.mJoysticks[0];
// GLOBAL: SWBF2 0x01ec0640
void*& gGuiInputTarget = FLIM_AT(void*, 0x428 + 0x25c8);
// GLOBAL: SWBF2 0x01ec0644
bool& gProcessAfterReading = FLIM_AT(bool, 0x428 + 0x25cc);
// GLOBAL: SWBF2 0x01ec0648
void*& gProcessedInputTarget = FLIM_AT(void*, 0x428 + 0x25d0);

RedJoystick* (&gJoystickPorts)[10] = FLIM_AT(tJoystickPorts10, 0x29fc);

unsigned char (&gProcessedInputStorage)[0x4c] = FLIM_AT(tBytes4C, 0x2cc8);
unsigned char (&gGuiInputStorage)[0x168] = FLIM_AT(tBytes168, 0x2d14);
// GLOBAL: SWBF2 0x01ec0acc
bool& gControllerMissingFlag = FLIM_AT(bool, 0x2e7c);
#undef FLIM_AT

struct ProcessedInputMap
{
    unsigned int hashID;
    unsigned int input;
};

// GLOBAL: SWBF2 0x007e67d0
static const ProcessedInputMap gProcessedInputMap[0x22] =
{
    { 0x9e5f87d9u,  0 }, { 0x880ddb41u,  1 }, { 0x0fd525a1u,  2 },
    { 0x2c4caa39u,  3 }, { 0x48d1dc33u,  4 }, { 0x20b0229au,  5 },
    { 0x9ddbe9ccu,  6 }, { 0x9b656248u,  7 }, { 0xbfe61030u,  8 },
    { 0xf85071beu,  9 }, { 0xac98902du, 10 }, { 0xcd1022bdu, 11 },
    { 0x1ed91b8bu, 12 }, { 0xc00bacc0u, 13 }, { 0x287ca200u, 14 },
    { 0x55cacad8u, 15 }, { 0x356988b8u, 16 }, { 0x0b3b6548u, 17 },
    { 0xa24bf9abu, 18 }, { 0x20a43ce3u, 19 }, { 0x43a268abu, 20 },
    { 0x42a26718u, 21 }, { 0x45a26bd1u, 22 }, { 0x44a26a3eu, 23 },
    { 0x47a26ef7u, 24 }, { 0x46a26d64u, 25 }, { 0x49a2721du, 26 },
    { 0x48a2708au, 27 }, { 0xdfa2efb1u, 28 }, { 0xc3b9429eu, 29 },
    { 0x82feda2eu, 30 }, { 0xbd8888f0u, 31 }, { 0x9537fa8au, 32 },
    { 0xf8b15dcfu, 33 },
};

// FUNCTION: SWBF2 0x00529c10
unsigned short GetRawFromProcessed_(unsigned int uiAction)
{
    unsigned char* pBindings =
        (unsigned char*)FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport());

    const unsigned int uiIndex = *(unsigned int*)(pBindings + 8) * 0x2b + uiAction;
    const unsigned short uKey =
        *(unsigned short*)(pBindings + uiIndex * 6 + 0x20bc);

    const unsigned short uUnused =
        *(unsigned short*)(pBindings + uiIndex * 6 + 0x20c0);
    (void)uUnused;

    if (uKey > 0x100)
    {
        return (unsigned short)(uKey + 0x8000);
    }

    RedKeyboard* pKeyboard = RedInputManager::GetKeyboard(0);
    if (pKeyboard)
    {
        return pKeyboard->pcGetAsciiValue(uKey);
    }

    return uKey;
}

// FUNCTION: SWBF2 0x00529ca0
void InitMagicCharMap(void)
{
    util::HashTable<unsigned short, unsigned int, util::NewAllocator>& rMap =
        *RedTextElement::GetMagicCharMap();

    unsigned int uiKey = 0xE70B21B8u;
    rMap[uiKey] = 0x8100;

    uiKey = 0x1E8375FAu;
    rMap[uiKey] = 0x8101;

    for (int i = 0; i < 0x22; i++)
    {
        rMap[gProcessedInputMap[i].hashID] = GetRawFromProcessed_(gProcessedInputMap[i].input);
    }
}

// FUNCTION: SWBF2 0x0052a080
void FLInputManager::Init(void)
{
    const unsigned char ucPorts = (unsigned char)RedInputManager::GetNumJoystickPorts();

    for (unsigned char i = 0; i < ucPorts; i++)
    {
        if (RedInputManager::GetJoystick(i)->m_connected)
        {
            gJoystickPorts[i] = RedInputManager::GetJoystick(i);
        }
        else
        {
            gJoystickPorts[i] = 0;
        }
    }

    gProcessAfterReading = true;
    gProcessedInputTarget = gProcessedInputStorage;
    gGuiInputTarget = gGuiInputStorage;
    gControllerMissingFlag = false;

    gPrimaryRawInputs.CloseChatMode();
    InitMagicCharMap();

    RedLogitechLCD* pPanel = RedInputManager::GetLCDPanel(0);
    if (pPanel && pPanel->mOpened)
    {
        ClearLCD(pPanel);
        DrawLCDText(pPanel, "Star Wars", 0x37, 4);
        DrawLCDText(pPanel, "Battlefront II", 0x32, 0x14);
        PresentLCD(pPanel);
    }
}

// FUNCTION: SWBF2 0x00529ae0
GUIInputs::GUIInputs()
{
    memset(mGUIInputs, 0, sizeof(mGUIInputs));
    memset(mLastGUIInputs, 0, sizeof(mLastGUIInputs));
    memset(mProcessedGUIInputs, 0, sizeof(mProcessedGUIInputs));
}

// FUNCTION: SWBF2 0x00529b20
GUIInputs::~GUIInputs()
{
}

// FUNCTION: SWBF2 0x0052a150
void FLInputManager::Reset()
{
    s_instance.mReadAllControllers = false;
    s_instance.m_bPausedByInput = false;
}

// FUNCTION: SWBF2 0x0052a160
int FLInputManager::GetBindCount() const
{
    int iCount = 0;
    for (int i = 0; i < 1; i++)
    {
        if (mActiveControllers[i])
        {
            iCount++;
        }
    }
    return iCount;
}

// FUNCTION: SWBF2 0x0052a180
void FLInputManager::RescanForMissing()
{
    m_MissingPort = -1;
}

// FUNCTION: SWBF2 0x0052a1c0
ProcessedInputs* FLInputManager::GetPlayerInputs(int iPlayer)
{
    if ((unsigned int)iPlayer >= 1)
    {
        return 0;
    }

    return &m_inputs[iPlayer];
}

// FUNCTION: SWBF2 0x0052a1e0
GUIInputs* FLInputManager::GetPlayerGuiInputs(int iPlayer)
{
    if ((unsigned int)iPlayer >= 1)
    {
        return 0;
    }

    return &m_guiInputs[iPlayer];
}

// FUNCTION: SWBF2 0x0052a210
bool FLInputManager::IsControllerBound(int iPlayer) const
{
    if ((unsigned int)iPlayer >= 1)
    {
        return false;
    }

    return mActiveControllers[iPlayer];
}

// FUNCTION: SWBF2 0x0052a230
void FLInputManager::ProcessChatMessageInput(const unsigned short* pwText)
{
    mTextTimer = 6.0f;
    PblUnicode::ustrlcpy(mChatText, pwText, 0x200);
}

// FUNCTION: SWBF2 0x0052a260
bool CanProcessChatMessage(void)
{
    const unsigned char* pObj =
        *(const unsigned char* const*)((const unsigned char*)&FLInputManager::s_instance + 0x2a24);
    return pObj && pObj[5];
}

// GLOBAL: SWBF2 0x009cef76
bool gDemoBindControllers;

namespace ObjectiveManager
{
HUD::ObjectiveList* GetObjectiveList();
}

void DrawLCDLine(RedLogitechLCD* pThis, int x0, int y0, int x1, int y1);
void DrawLCDWideText(RedLogitechLCD* pThis, const unsigned short* pwString, int x, int y);
void DrawLCDWrappedText(RedLogitechLCD* pThis, const unsigned short* pwString);

// FUNCTION: SWBF2 0x0052a280
bool FLInputManager::Update(float fDeltaTime)
{
    static unsigned int s_uiFrame;

    BindPlayerInput();
    RedInputManager::Update(fDeltaTime);

    if (mAutoAcquire)
    {
        for (unsigned char i = 0; i < RedInputManager::GetNumJoystickPorts(); i++)
        {
            if (RedInputManager::GetJoystick(i)->m_connected && i < 1)
            {
                mActiveControllers[i] = 1;
                m_HWPort[i] = i;
                if (mPrimaryPort < 0)
                {
                    mPrimaryPort = i;
                }
                if (mSecondaryPort < 0 && mPrimaryPort != i)
                {
                    mSecondaryPort = i;
                }
            }
        }
    }

    if (PblBuildType::gDemoBuild && gDemoBindControllers)
    {
        for (unsigned char i = 0; i < RedInputManager::GetNumJoystickPorts(); i++)
        {
            if (RedInputManager::GetJoystick(i)->m_connected && i < 1)
            {
                mActiveControllers[i] = 1;
            }
        }
    }

    for (int iPlayer = 0; iPlayer < 1; iPlayer++)
    {
        mJoysticks[iPlayer].ReadLatest(fDeltaTime);

        float& fTextTimer = mTextTimer;
        fTextTimer -= fDeltaTime;
        if (fTextTimer < 0.0f)
        {
            fTextTimer = 0.0f;
        }
        float& fPageTimer = mPageTimer;
        fPageTimer -= fDeltaTime;
        if (fPageTimer < 0.0f)
        {
            fPageTimer = 0.0f;
        }
        int& iPageIndex = mPageIndex;

        RedLogitechLCD* pLcd = mJoysticks[0].mLCDPanel;
        if (!pLcd || !pLcd->mOpened)
        {
            continue;
        }

        const int iLocal = NetGame::GetLocalIndex(0);
        Character* pCharacter = 0;
        if ((unsigned int)iLocal < 64)
        {
            pCharacter = Character::GetPtr(iLocal);
        }

        if (pCharacter)
        {
            Controllable* pControllable = pCharacter->GetControllable();
            if (pControllable)
            {
                if ((pLcd->mSoftButtons & 1) && !(pLcd->mPrevSoftButtons & 1))
                {
                    mLCDPageToggle[0] = !mLCDPageToggle[0];
                }

                bool bObjectives;
                if ((pLcd->mSoftButtons & 2) && !(pLcd->mPrevSoftButtons & 2))
                {
                    if (fPageTimer > 0.0f)
                    {
                        iPageIndex++;
                    }
                    else
                    {
                        iPageIndex = 0;
                    }
                    fPageTimer = 4.0f;
                    bObjectives = true;
                }
                else
                {
                    bObjectives = fPageTimer > 0.0f;
                }

                if (bObjectives)
                {
                    HUD::ObjectiveList* pList = ObjectiveManager::GetObjectiveList();
                    if (pList && pList->GetOwner())
                    {
                        pList->BeginIter();
                        int iCount = 0;
                        const int iTeam = pList->GetOwner()->mTeamNumber;
                        HUD::Objective* pObjective;
                        for (pObjective = pList->GetCurrentIter(); pObjective;
                             pList->IncrementIter(), pObjective = pList->GetCurrentIter())
                        {
                            if (pObjective->_state == HUD::Objective::ACTIVE &&
                                pObjective->_team == iTeam)
                            {
                                if (iCount == iPageIndex)
                                {
                                    break;
                                }
                                iCount++;
                            }
                        }

                        if (pObjective)
                        {
                            DrawLCDWrappedText(pLcd, pObjective->_textElement.m_text.GetData());
                        }
                        else
                        {
                            fPageTimer = 0.0f;
                        }
                    }
                    continue;
                }

                for (int iChannel = 0, y = 2; y < 30; iChannel++, y += 14)
                {
                    const int iWeapon = pControllable->GetWeaponIndex(iChannel);
                    if (iWeapon < 0)
                    {
                        continue;
                    }
                    const Weapon* pWeapon = pControllable->GetWeaponPtr(iWeapon);
                    if (!pWeapon)
                    {
                        continue;
                    }

                    const WeaponClass* pClass = pWeapon->mClass;
                    if (!pClass->mLabel)
                    {
                        continue;
                    }

                    const float fRounds = (float)pClass->mAmmoCounterClass.mRoundsPerClip;

                    const AmmoCounter* pAmmo = pWeapon->m_pAmmoCounter;
                    const float fNumClips = pAmmo->mNumClips;

                    char acBuffer[32];
                    if (fRounds >= (float)INT_MAX || pClass->mAmmoCounterClass.mAmmoPerRound == 0.0f)
                    {
                        sprintf(acBuffer, "* : ");
                    }
                    else
                    {
                        const float fCurClip = pAmmo->mCurClip;
                        const int iClip = (int)(fCurClip * fRounds);
                        const int iTotal =
                            (fNumClips == FLT_MAX) ? -1 : (int)((fCurClip + fNumClips) * fRounds);

                        if (iClip == -1)
                        {
                            if (iTotal == -1)
                            {
                                sprintf(acBuffer, "* : ");
                            }
                            else
                            {
                                sprintf(acBuffer, "%d : ", iTotal);
                            }
                        }
                        else if (iTotal == -1)
                        {
                            sprintf(acBuffer, "%d / * : ", iClip);
                        }
                        else
                        {
                            sprintf(acBuffer, "%d / %d : ", iClip, iTotal);
                        }
                    }

                    unsigned short awLine[256];
                    PblUnicode::ustrncnv(awLine, acBuffer, 256);
                    PblUnicode::ustrlcat(awLine, pClass->mLabel, 256);
                    DrawLCDWideText(pLcd, awLine, 5, y);
                }

                GameObject* pObject = pControllable->GetGameObject();
                if (pObject)
                {
                    const Damageable* pDamageable = static_cast<const Damageable*>(pObject);
                    const int iWidth = pLcd->mWidth;

                    if (pDamageable->mMaxShield > 0.0f)
                    {
                        const int iLen = (int)((float)iWidth * pDamageable->mCurShield /
                                               pDamageable->mMaxShield);
                        if (iLen > 0)
                        {
                            DrawLCDLine(pLcd, 0, 32, iLen - 1, 32);
                        }
                    }

                    if (pDamageable->mMaxHealth > 0.0f)
                    {
                        const int iLen = (int)((float)iWidth * pDamageable->mCurHealth /
                                               pDamageable->mMaxHealth);
                        if (iLen > 0)
                        {
                            DrawLCDLine(pLcd, 0, 35, iLen - 1, 35);
                            DrawLCDLine(pLcd, 0, 36, iLen - 1, 36);
                            DrawLCDLine(pLcd, 0, 37, iLen - 1, 37);
                        }
                    }

                    const EnergyBar* pBar = pControllable->GetEnergyBar();
                    if (pBar)
                    {
                        const float fMax = pBar->m_pClass->m_fEnergyMax;
                        if (fMax > 0.0f)
                        {
                            const int iLen =
                                (int)((float)iWidth * pBar->m_fEnergy / fMax);
                            if (iLen > 0)
                            {
                                DrawLCDLine(pLcd, 0, 40, iLen - 1, 40);
                                DrawLCDLine(pLcd, 0, 41, iLen - 1, 41);
                            }
                        }
                    }
                    continue;
                }
            }

            if (fTextTimer > 0.0f)
            {
                DrawLCDWrappedText(pLcd, mChatText);
                continue;
            }
        }

        DrawLCDText(pLcd, "Star Wars", 55, 4);
        DrawLCDText(pLcd, "Battlefront II", 50, 20);
    }

    m_rumble[0].Update(fDeltaTime);

    if ((++s_uiFrame & 7) == 0)
    {
        InitMagicCharMap();
    }

    return true;
}

// FUNCTION: SWBF2 0x0052a890
void FLInputManager::UnbindController(int iPlayer)
{
    const int i = iPlayer - 1;

    if ((unsigned int)i < 1)
    {
        mActiveControllers[i] = 0;
        m_HWPort[i] = -1;
        m_HWPort_Full[i] = -1;

        mJoysticks[i].ClearInputsToOff();
        m_inputs[i].Clear();
        m_guiInputs[i].Clear();

        if (i == mPrimaryPort)
        {
            mPrimaryPort = -1;
            mShadowPrimary = -1;
        }
        if (i == mSecondaryPort)
        {
            mSecondaryPort = -1;
            mShadowSecondary = -1;
        }
        if (m_MissingPort == i)
        {
            m_MissingPort = -1;
            GuiManager* pGui = GuiManager::Instance();
            pGui->CloseError(false, false);
            LuaHelper::CallTableProcWThis(pGui->mCurrentScope, "fnPostMissingController");
        }
    }

    if (!mActiveControllers[0])
    {
        mBindAll = 0;
        mIgnoreMissing = 0;
    }
    mActiveControllers[0] = 1;
    m_HWPort[0] = 0;
    mPrimaryPort = 0;
}

// FUNCTION: SWBF2 0x0052aa20
void FLInputManager::BindPlayerInput()
{
    const unsigned char ucPorts = (unsigned char)RedInputManager::GetNumJoystickPorts();

    for (unsigned char i = 0; i < ucPorts; i++)
    {
        if (RedInputManager::GetJoystick(i)->m_connected)
        {
            mJoysticks[0]._redJoystick[i] = RedInputManager::GetJoystick(i);
        }
        else
        {
            mJoysticks[0]._redJoystick[i] = 0;
        }
    }

    PlayerPrefs::mAllUserPrefs[0].mJoystick = 0;
    m_HWPort[0] = 0;

    if (RedInputManager::GetNumMousePorts())
    {
        s_instance.mJoysticks[0]._redMouse = RedInputManager::GetMouse(0);
    }
    if (RedInputManager::GetNumKeybaordPorts())
    {
        s_instance.mJoysticks[0]._redKeyboard = RedInputManager::GetKeyboard(0);
    }
    if (RedInputManager::GetNumLCDPorts())
    {
        s_instance.mJoysticks[0].mLCDPanel = RedInputManager::GetLCDPanel(0);
    }
}

// FUNCTION: SWBF2 0x0052ab80
void FLInputManager::SetHotController(int iJoystick)
{
    if (mShadowPrimary >= 0 || iJoystick < 0)
    {
        RestorePrimarySecondary();
        if (iJoystick < 0)
        {
            return;
        }
    }
    if (iJoystick < 1 && m_HWPort[iJoystick] >= 0)
    {
        mShadowPrimary = mPrimaryPort;
        mPrimaryPort = iJoystick;
    }
}

// FUNCTION: SWBF2 0x0052abd0
void FLInputManager::RestorePrimarySecondary()
{
    int iPrimary = mShadowPrimary;
    if (iPrimary < 0)
    {
        return;
    }
    mPrimaryPort = iPrimary;
    int iSecondary = mShadowSecondary;
    mShadowPrimary = -1;
    if (iSecondary < 0)
    {
        return;
    }
    mSecondaryPort = iSecondary;
    mShadowSecondary = -1;
}

// FUNCTION: SWBF2 0x0052ad10
void FLInputManager::StopIntermittentRumble(int iPlayer)
{
    bool eligible = true;
    if (NetGame::IsEnabled())
    {
        if (netEnabledNext && netClientActive)
        {
            eligible = !NetGame::IsSpectator(iPlayer);
        }
        else
        {
            eligible = NetGame::IsHost() && !netIsDedicated && netIsOneWorld;
        }
    }
    if (!eligible)
    {
        return;
    }

    const int iJoystick = NetGame::GetJoystickIndex(iPlayer);
    if (iJoystick < 0)
    {
        return;
    }
    Rumble* pRumble = &m_rumble[iJoystick];
    if (pRumble->mRumbleMode == Rumble::eIntermittent)
    {
        pRumble->mRumbleMode = Rumble::eNone;
    }
}

// FUNCTION: SWBF2 0x0052b190
template bool util::Vector<unsigned int, util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::NewAllocator> > >::SetSize_(int, bool);
// FUNCTION: SWBF2 0x0052b250
template bool util::BaseArray<unsigned short,
    util::TypedArrayAllocator<unsigned short, util::NewAllocator> >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x0052b320
template bool util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::NewAllocator> >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x0052b440
template unsigned int* util::TypedArrayAllocator<unsigned int, util::NewAllocator>::Allocate_(int&);

// FUNCTION: SWBF2 0x0052ab40
void FLInputManager::SwapPrimarySecondary()
{
    if (mPrimaryPort < 0 || mSecondaryPort < 0)
    {
        return;
    }
    if (mShadowPrimary < 0)
    {
        mShadowPrimary = mPrimaryPort;
        mShadowSecondary = mSecondaryPort;
    }
    int iPrimary = mPrimaryPort;
    mPrimaryPort = mSecondaryPort;
    mSecondaryPort = iPrimary;
}

// FUNCTION: SWBF2 0x006f0120
void DrawLCDLine(RedLogitechLCD* pThis, int x0, int y0, int x1, int y1)
{
    if (!pThis->mOpened || !pThis->mConnected)
    {
        return;
    }
    PrepareLCDDrawing(pThis);
    HGDIOBJ hOldPen = SelectObject(pThis->mhDC, GetStockObject(WHITE_PEN));
    MoveToEx(pThis->mhDC, x0, y0, 0);
    LineTo(pThis->mhDC, x1, y1);
    SelectObject(pThis->mhDC, hOldPen);
}

// FUNCTION: SWBF2 0x006f0230
void DrawLCDWideText(RedLogitechLCD* pThis, const unsigned short* pwString, int x, int y)
{
    if (!pThis->mOpened || !pThis->mConnected)
    {
        return;
    }
    PrepareLCDDrawing(pThis);
    if (!pThis->mhFont)
    {
        return;
    }
    int iMapMode = SetMapMode(pThis->mhDC, MM_TEXT);
    int iBkMode = SetBkMode(pThis->mhDC, TRANSPARENT);
    HGDIOBJ hOldFont = SelectObject(pThis->mhDC, pThis->mhFont);
    const WCHAR* psz = (const WCHAR*)pwString;
    TextOutW(pThis->mhDC, x, y, psz, lstrlenW(psz));
    SetMapMode(pThis->mhDC, iMapMode);
    SetBkMode(pThis->mhDC, iBkMode);
    SelectObject(pThis->mhDC, hOldFont);
}

// FUNCTION: SWBF2 0x006f02d0
void DrawLCDWrappedText(RedLogitechLCD* pThis, const unsigned short* pwString)
{
    if (!pThis->mOpened || !pThis->mConnected)
    {
        return;
    }
    PrepareLCDDrawing(pThis);
    if (!pThis->mhFont)
    {
        return;
    }
    int iMapMode = SetMapMode(pThis->mhDC, MM_TEXT);
    int iBkMode = SetBkMode(pThis->mhDC, TRANSPARENT);
    HGDIOBJ hOldFont = SelectObject(pThis->mhDC, pThis->mhFont);
    RECT rc = { 0, 0, pThis->mWidth - 1, pThis->mHeight - 1 };
    const WCHAR* psz = (const WCHAR*)pwString;
    DrawTextW(pThis->mhDC, psz, lstrlenW(psz), &rc, DT_WORDBREAK);
    SetMapMode(pThis->mhDC, iMapMode);
    SetBkMode(pThis->mhDC, iBkMode);
    SelectObject(pThis->mhDC, hOldFont);
}

// FUNCTION: SWBF2 0x0052aae0
void FLInputManager::SetReadAll(bool bReadAll)
{
    if (mReadAllControllers == (unsigned char)bReadAll)
    {
        return;
    }

    mReadAllControllers = bReadAll;
    if (!bReadAll && GetBindCount() <= 1 && GetPrimaryPort() >= 0)
    {
        mActiveControllers[GetPrimaryPort()] = true;
    }
    if (mReadAllControllers && GetPrimaryPort() >= 0)
    {
        mActiveControllers[GetPrimaryPort()] = true;
    }

    BindPlayerInput();
    GuiManager::DebounceJoystick(0);
}

// FUNCTION: SWBF2 0x0052ac50
int FLInputManager::GetPlayerFromHWPort(int iHWPort) const
{
    const int* piHWPort = m_HWPort;
    int iPlayer = 0;
    for (unsigned int i = 0; i < 1; ++i)
    {
        if (piHWPort[i] == iHWPort)
        {
            return iPlayer;
        }
        if (piHWPort[i] >= 0)
        {
            ++iPlayer;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0052ad50
void FLInputManager::StopAllRumble()
{
    for (int i = 0; i < 1; i++)
    {
        m_rumble[i].StopAllRumble();
    }
}

// FUNCTION: SWBF2 0x0052ad60
void FLInputManager::EnableRumble(int iPort, bool bEnable)
{
    m_rumble[iPort].mEnabled = bEnable;
}

// FUNCTION: SWBF2 0x0052aa00
bool FLInputManager::AnyMissing() const
{
    const bool* pbChat = &gControllerMissingFlag;
    for (unsigned char i = 0; i < 1; i++)
    {
        if (pbChat[i])
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x0052ac10
int FLInputManager::GetHWPort(int iViewport)
{
    const int* piPort = m_HWPort;
    int iBound = 0;
    for (unsigned int i = 0; i < 1; i++)
    {
        if (piPort[i] >= 0)
        {
            if (iViewport == iBound)
            {
                return piPort[i];
            }
            iBound++;
        }
    }
    return -1;
}

// FUNCTION: SWBF2 0x0052ac90
bool __fastcall is_rumble_enable(int iPlayer)
{
    if (!NetGame::IsEnabled())
    {
        return true;
    }
    if (netEnabledNext && netClientActive)
    {
        return !NetGame::IsSpectator(iPlayer);
    }
    return NetGame::IsHost() && !netIsDedicated && netIsOneWorld;
}

unsigned short __cdecl RawInputToActionKey(unsigned short usRawInput);

struct AlternateActionKeyEntry
{
    unsigned short mKey;
    unsigned int mValue;
};

// GLOBAL: SWBF2 0x007ec168
// SYNTHETIC: SWBF2 0x004039f0
// `dynamic initializer for 'gAlternateActionKeys''
AlternateActionKeyEntry gAlternateActionKeys[2] =
{
    { (unsigned short)(RawInputToActionKey('B') + 0x8000), 0xDD7D0382 },
    { (unsigned short)(RawInputToActionKey('C') + 0x8000), 0xDD7D0382 },
};
