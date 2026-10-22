#include <float.h>
#include <stdio.h>
#include <string.h>
#include <d3dx9math.h>

#include <math.h>

#include "HUDElementMap.h"
#include "RedEngineFL/Interface/RedHotSpot.h"
#include "BonusManager.h"
#include "CameraManager.h"
#include "Character.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "Controllable.h"
#include "EntityBuilding.h"
#include "EntitySoldier.h"
#include "EntityDroideka.h"
#include "EntityCarrier.h"
#include "EntityHover.h"
#include "EntityWalker.h"
#include "WeaponCannonInstance.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "TargetManager.h"
#include "Team.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRegion.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

#include "Steam/gfGalaxy.h"

static const unsigned int kEventToggleMapModeHash = 0x233d39ec;
static const unsigned int kEventChangeMapModeHash = 0x0d533bd4;
static const unsigned int kEventPostHideHash = 0x6d95d5e9;
static const unsigned int kEventRefreshTargetHash = 0xa9304f43;
static const unsigned int kEventRefreshPostHash = 0x837b4104;
static const unsigned int kEventRefreshMarkerHash = 0x6ffd58b8;
static const unsigned int kTargetFireColorHash = 0x8b5b4c23;
static const unsigned int kPostFlashRateHash = 0xa34f782d;
static const unsigned int kPostFlashRadiusHash = 0xc1419eb5;
static const unsigned int kPostFlashColorHash = 0x4f20a696;
static const unsigned int kPositionSmallHash = 0x706c4359;
static const unsigned int kPositionLargeHash = 0x438ca9f5;
static const unsigned int kPositionSpawnHash = 0x5cda7691;
static const unsigned int kBackgroundMaskShapeSmallHash = 0x55951ec1;
static const unsigned int kBackgroundMaskShapeLargeHash = 0xce9d2fdd;
static const unsigned int kBackgroundMaskShapeSpawnHash = 0xb5faa499;
static const unsigned int kAllowMiniMapHash = 0xb3a94169;
static const unsigned int kPostNumbersHash = 0x5f18ecef;
static const unsigned int kPositionHash = 0x934f4e0a;
static const unsigned int kBitmapFactoryHash = 0x46544626;
static const unsigned int kBitmapMaskedFactoryHash = 0xd23cb4f7;
static const unsigned int kModel3DFactoryHash = 0x1eccdefd;
static const unsigned int kTextFactoryHash = 0xbde64e3e;

struct BackdropDefaults
{
    float x;
    float y;
    float diameter;
};

// GLOBAL: SWBF2 0x00789ac4
static const BackdropDefaults gBackdropDefaultSmallOld = { 0.15f, 0.82f, 0.22f };

// GLOBAL: SWBF2 0x007e6960
static BackdropDefaults gBackdropDefaults[3] =
{
    { 0.8f, 0.82f, 0.22f },
    { 0.5f, 0.5f, 0.75f },
    { 0.6f, 0.5f, 0.65f }
};

static const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x00789ad4
char* HUD::ElementMap::sModeStrings[3] = { "Small", "Large", "Spawn" };

// GLOBAL: SWBF2 0x007e6958
char* HUD::ElementMap::sMaskShapeStrings[2] = { "Circle", "Rectangle" };

// GLOBAL: SWBF2 0x00789ae0
const float HUD::ElementMap::sSymbolScale[2] = { 6.0f, 4.0f };

// GLOBAL: SWBF2 0x01e56c3c
float HUD::ElementMap::sNorth;

// GLOBAL: SWBF2 0x01e56c3a
bool HUD::ElementMap::sFlipNorth;

// FUNCTION: SWBF2 0x0054fab0
HUD::ElementMap::Factory::Factory()
    : HUD::ElementGroupPlayer::Factory(0xDFA2EFB1u)
{
    mName = "Map";

    mEventToggleMapMode.Init("EventToggleMapMode", mProperties, Property::type_Event);
    mEventChangeMapMode.Init("EventChangeMapMode", mProperties, Property::type_Event);
    mEventPostHide.Init("EventPostHide", mProperties, Property::type_Event);
    mEventRefreshTarget.Init("EventRefreshTarget", mProperties, Property::type_Event);
    mEventRefreshPost.Init("EventRefreshPost", mProperties, Property::type_Event);
    mEventRefreshMarker.Init("EventRefreshMarker", mProperties, Property::type_Event);
    mTargetFireColor.Init("TargetFireColor", mProperties, Property::type_Color);
    mPostFlashRate.Init("PostFlashRate", mProperties, 0.0f, 30.0f, 0.1f);
    mPostFlashRadius.Init("PostFlashRadius", mProperties, 0.0f, 10.0f, 0.1f);
    mPostFlashColor.Init("PostFlashColor", mProperties, Property::type_Color);
    mPosition[0].Init("PositionSmall", mProperties, Property::type_Vector3);
    mPosition[1].Init("PositionLarge", mProperties, Property::type_Vector3);
    mRelative[0].Init("PositionRelativeSmall", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mRelative[1].Init("PositionRelativeLarge", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mRelative[2].Init("PositionRelativeSpawn", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mBackgroundMaskShape[0].Init("BackgroundMaskShapeSmall", mProperties, 0, 2);
    mBackgroundMaskShape[1].Init("BackgroundMaskShapeLarge", mProperties, 0, 2);
    mNorth.Init("North", mProperties, 0.0f, 360.0f, 1.0f);
    mAllowMinimap.Init("AllowMiniMap", mProperties, Property::type_Bool);
    mPostNumbers.Init("PostNumbers", mProperties, Property::type_Bool);
}

// FUNCTION: SWBF2 0x0054f890
void HUD::ElementGroupPlayer::PlayerIndexChanged()
{
}

// SYNTHETIC: SWBF2 0x0054f8a0
// HUD::ElementGroupPlayer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0054f910
bool HUD::ElementMap::BitmapGroup::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    if (!HUD::ElementBitmapBase::SetProperty(uHashID, pValue))
    {
        return false;
    }
    if (mMap && mMap->GetMode() == mMapMode)
    {
        CopyToBitmaps(mBitmaps, mNumBitmaps, 0, false);
    }
    return true;
}

// SYNTHETIC: SWBF2 0x0054f9f0
// HUD::ElementMap::BitmapGroup::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00555c35
// [thunk]:HUD::ElementMap::BitmapGroup::`scalar deleting destructor' adjustor{288}
// FUNCTION: SWBF2 0x0054fa60
HUD::ElementMap::BitmapGroup::~BitmapGroup()
{
}

// SYNTHETIC: SWBF2 0x0054fe40
// HUD::ElementMap::Factory::~Factory

// SYNTHETIC: SWBF2 0x00550fb0
// HUD::ElementMap::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00550fe0
// HUD::ElementModel3D::`default constructor closure'

// SYNTHETIC: SWBF2 0x00550ff0
// HUD::ElementBitmapMasked::`default constructor closure'

// SYNTHETIC: SWBF2 0x00551070
// HUD::ElementBitmap::`default constructor closure'

// SYNTHETIC: SWBF2 0x005510f0
// HUD::ElementMap::BitmapGroup::`default constructor closure'

// SYNTHETIC: SWBF2 0x005511a0
// HUD::ElementText::`default constructor closure'

// SYNTHETIC: SWBF2 0x00555c2a
// [thunk]:HUD::ElementMap::`scalar deleting destructor' adjustor{384}

// FUNCTION: SWBF2 0x005511b0
HUD::ElementMap::~ElementMap()
{
    if (mMapNode.mNext)
    {
        mMapNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0054f970
HUD::Item* HUD::ElementMap::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                void* const pUserData)
{
    HUD::Item* pItem = new ElementMap(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0054ff60
HUD::ElementMap::ElementMap(const char* const pName, Factory::ItemCallback pCallback,
                            void* const pUserData)
    : HUD::ElementGroupPlayer(*static_cast<RedGroupElement*>(this), pName, pCallback, pUserData)
{
    char buffer[256];
    unsigned int i;

    mMode = mode_Large;
    mPlayerCharacter = 0;
    mInitialized = false;
    mPostHide = false;
    mAllowToggleOff = false;
    mAllowMiniMap = true;
    mPostNumbers = true;
    mNextSlowUpdate = 0.0f;
    sMapList.AddFirst(&mMapNode);

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "PostSelected%s", sModeStrings[i]);
        mPostSelectAll[i].HUD::Item::SetName(buffer);
        _snprintf(buffer, sizeof(buffer), "Marker%s", sModeStrings[i]);
        mMarkerAll[i].HUD::Item::SetName(buffer);
        mPostSelectAll[i].mWriteEnabled = false;
        mMarkerAll[i].mWriteEnabled = false;
    }

    mEventToggleMapMode.Init(EventToggleMapMode, this);
    mEventChangeMapMode.Init(EventChangeMapMode, this);
    mEventPostHide.Init(EventPostHide, this);
    mEventRefreshTarget.Init(EventRefreshTarget, this);
    mEventRefreshPost.Init(EventRefreshPost, this);
    mEventRefreshMarker.Init(EventRefreshMarker, this);
    mEventDisable.Init(EventDisable, this);

    gBackdropDefaults[0] = gBackdropDefaultSmallOld;
    for (i = 0; i < mode_MAX; ++i)
    {
        mPosition[i] = PblVector3(gBackdropDefaults[i].x, gBackdropDefaults[i].y, 0.0f);
    }

    PblMatrix matrix;
    D3DXMatrixIdentity((D3DXMATRIX*)&matrix);
    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "Backdrop%s", sModeStrings[i]);
        HUD::ElementBitmapMasked* pBackdrop = &mBackdrop[i];
        pBackdrop->HUD::Item::SetName(buffer);
        if (pBackdrop)
        {
            pBackdrop->SetFactory(Factory::FindByHashID(kBitmapMaskedFactoryHash));
        }
        pBackdrop->RedInterfaceElement::SetMatrix(matrix);
        pBackdrop->SetColor(RedColor::WHITE);
        pBackdrop->mAlpha = 0.5f;
        pBackdrop->SetZOrder(0xff);
        pBackdrop->RedInterfaceElement::SetPosition(0.0f, 0.0f, 0.0f);
        pBackdrop->HUD::ElementBitmapBase::SetRect(
            sViewportHeight / sViewportWidth * gBackdropDefaults[i].diameter,
            gBackdropDefaults[i].diameter, RedBitmapElement::BITMAP_HALIGN_CENTER,
            RedBitmapElement::BITMAP_VALIGN_CENTER);
        pBackdrop->mDynamicAlloc = false;
        ElementGroupBase::AddElement(pBackdrop);
        pBackdrop->HUD::Element::Enable(mMode == i);
        pBackdrop->mWriteEnabled = false;
    }

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "PlayerFOV%s", sModeStrings[i]);
        HUD::ElementModel3D* pFOV = &mPlayerFOV[i];
        pFOV->HUD::Item::SetName(buffer);
        pFOV->SetZOrder(123);
        if (pFOV)
        {
            pFOV->SetFactory(Factory::FindByHashID(kModel3DFactoryHash));
        }
        ElementGroupBase::AddElement(pFOV);
        pFOV->HUD::Element::Enable(mMode == i && mMode != mode_Spawn);
        pFOV->mDynamicAlloc = false;
        pFOV->mWriteEnabled = false;
    }

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "PlayerDirection%s", sModeStrings[i]);
        HUD::ElementModel3D* pDirection = &mDirection[i];
        pDirection->HUD::Item::SetName(buffer);
        pDirection->SetZOrder(124);
        if (pDirection)
        {
            pDirection->SetFactory(Factory::FindByHashID(kModel3DFactoryHash));
        }
        ElementGroupBase::AddElement(pDirection);
        pDirection->HUD::Element::Enable(mMode == i && mMode != mode_Spawn);
        pDirection->mDynamicAlloc = false;
        pDirection->mWriteEnabled = false;
    }

    const bool bSplitScreen = CameraManager::Instance()->GetNumCameras() > 1;

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "Target%s", sModeStrings[i]);
        BitmapGroup* pTarget = &mTargetAll[i];
        pTarget->HUD::Item::SetName(buffer);
        if (pTarget)
        {
            pTarget->SetFactory(Factory::FindByHashID(kBitmapFactoryHash));
        }
        pTarget->SetColor(RedColor::BLACK);
        pTarget->SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
        pTarget->HUD::ElementBitmapBase::SetRect(
            sSymbolScale[bSplitScreen] / sViewportWidth, sSymbolScale[bSplitScreen] / sViewportHeight,
            RedBitmapElement::BITMAP_HALIGN_CENTER, RedBitmapElement::BITMAP_VALIGN_CENTER);
        pTarget->SetZOrder(126);
        pTarget->mDynamicAlloc = false;
        ElementGroupBase::AddElement(pTarget);
        pTarget->Init(this, i, mTarget, 450);
        pTarget->mWriteEnabled = false;
    }
    mTargetAll[mMode].CopyToBitmaps(mTarget, 450, this, true);
    mTargetFireColor = RedColor::WHITE;

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "Post%s", sModeStrings[i]);
        BitmapGroup* pPost = &mPostAll[i];
        pPost->HUD::Item::SetName(buffer);
        if (pPost)
        {
            pPost->SetFactory(Factory::FindByHashID(kBitmapFactoryHash));
        }
        pPost->SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
        pPost->HUD::ElementBitmapBase::SetRect(
            sSymbolScale[bSplitScreen] / sViewportWidth, sSymbolScale[bSplitScreen] / sViewportHeight,
            RedBitmapElement::BITMAP_HALIGN_CENTER, RedBitmapElement::BITMAP_VALIGN_CENTER);
        pPost->SetZOrder(128);
        pPost->mDynamicAlloc = false;
        ElementGroupBase::AddElement(pPost);
        pPost->Init(this, i, mPost, 16);
        pPost->mWriteEnabled = false;
    }
    mPostAll[mMode].CopyToBitmaps(mPost, 16, this, false);

    mPostFlashRate = 10.0f;
    mPostFlashRadius = 3.0f;
    RedColor flashColor;
    flashColor.b = 0xd5;
    flashColor.g = 0x56;
    flashColor.r = 0x01;
    flashColor.a = 0xff;
    mPostFlashColor = flashColor;

    for (i = 0; i < 16; ++i)
    {
        mPost[i].CreateHotSpot((sViewportWidth / 640.0f - 1.0f) * 3.0f * sSymbolScale[bSplitScreen]);
    }

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "PostText%s", sModeStrings[i]);
        HUD::ElementText* pPostText = &mPostTextAll[i];
        pPostText->HUD::Item::SetName(buffer);
        if (pPostText)
        {
            pPostText->SetFactory(Factory::FindByHashID(kTextFactoryHash));
        }
        pPostText->SetZOrder(127);
        pPostText->mDynamicAlloc = false;
        pPostText->RedGlyphCacheElement::SetFont(0x8f4b7122u);
        pPostText->SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_BOTTOM);
        ElementGroupBase::AddElement(pPostText);
        pPostText->mWriteEnabled = false;
    }
    for (i = 0; i < 16; ++i)
    {
        mPostTextAll[mMode].CopyToTextElement(mPostText[i]);
        RedGroupElement::AddElement(&mPostText[i]);
    }

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "PostSelect%s", sModeStrings[i]);
        BitmapGroup* pPostSelect = &mPostSelectAll[i];
        pPostSelect->HUD::Item::SetName(buffer);
        if (pPostSelect)
        {
            pPostSelect->SetFactory(Factory::FindByHashID(kBitmapFactoryHash));
        }
        pPostSelect->HUD::ElementBitmap::SetTexture("hud_icon_flash");
        pPostSelect->SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
        pPostSelect->HUD::ElementBitmapBase::SetRect(
            sSymbolScale[bSplitScreen] / sViewportWidth, sSymbolScale[bSplitScreen] / sViewportHeight,
            RedBitmapElement::BITMAP_HALIGN_CENTER, RedBitmapElement::BITMAP_VALIGN_CENTER);
        pPostSelect->SetZOrder(128);
        pPostSelect->mDynamicAlloc = false;
        ElementGroupBase::AddElement(pPostSelect);
        pPostSelect->Init(this, i, mPostSelect, 16);
        pPostSelect->mWriteEnabled = false;
    }
    mPostSelectAll[mMode].CopyToBitmaps(mPostSelect, 16, this, true);

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "Marker%s", sModeStrings[i]);
        BitmapGroup* pMarker = &mMarkerAll[i];
        pMarker->HUD::Item::SetName(buffer);
        if (pMarker)
        {
            pMarker->SetFactory(Factory::FindByHashID(kBitmapFactoryHash));
        }
        pMarker->SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
        pMarker->HUD::ElementBitmapBase::SetRect(
            sSymbolScale[bSplitScreen] / sViewportWidth, sSymbolScale[bSplitScreen] / sViewportHeight,
            RedBitmapElement::BITMAP_HALIGN_CENTER, RedBitmapElement::BITMAP_VALIGN_CENTER);
        pMarker->SetZOrder(127);
        pMarker->mDynamicAlloc = false;
        ElementGroupBase::AddElement(pMarker);
        pMarker->Init(this, i, mMarker, 32);
        pMarker->mWriteEnabled = false;
    }
    mMarkerAll[mMode].CopyToBitmaps(mMarker, 32, this, false);

    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "North%s", sModeStrings[i]);
        HUD::ElementBitmap* pNorth = &mNorthIcon[i];
        pNorth->HUD::Item::SetName(buffer);
        if (pNorth)
        {
            pNorth->SetFactory(Factory::FindByHashID(kBitmapFactoryHash));
        }
        pNorth->SetStyle(RedBitmapElement::BITMAP_STYLE_SHADOW);
        pNorth->HUD::ElementBitmapBase::SetRect(
            sSymbolScale[bSplitScreen] / sViewportWidth, sSymbolScale[bSplitScreen] / sViewportHeight,
            RedBitmapElement::BITMAP_HALIGN_CENTER, RedBitmapElement::BITMAP_VALIGN_BOTTOM);
        pNorth->SetZOrder(127);
        pNorth->mDynamicAlloc = false;
        pNorth->mWriteEnabled = false;
        pNorth->HUD::ElementBitmap::SetTexture("hud_map_northpointer");
        ElementGroupBase::AddElement(pNorth);
    }

    mPlayerDirection.SetAll(0.0f);
    mPlayerDirection.z = 1.0f;
    mPlayerPosition.SetAll(0.0f);
    mPlayerMovementDirection.SetAll(0.0f);
    mPlayerMovementDirection.z = 1.0f;
    memset(mDim, 0, sizeof(mDim));
    memset(mPostScale, 0, sizeof(mPostScale));
    sNorth = 0.0f;
    sFlipNorth = false;
}

// FUNCTION: SWBF2 0x00551920
void HUD::ElementMap::Read(PblConfig& config, Factory::ItemCallback pCallback, void* const pUserData)
{
    unsigned int i;
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            for (i = 0; i < mode_MAX; ++i)
            {
                if (data.GetId() == mBackdrop[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mBackdrop[i].Read(scope, 0, 0);
                    break;
                }
                if (data.GetId() == mPlayerFOV[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mPlayerFOV[i].Read(scope, 0, 0);
                    break;
                }
                if (data.GetId() == mDirection[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mDirection[i].Read(scope, 0, 0);
                    break;
                }
                if (data.GetId() == mTargetAll[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mTargetAll[i].Read(scope, 0, 0);
                    mTargetAll[mMode].CopyToBitmaps(mTarget, 450, 0, true);
                    break;
                }
                if (data.GetId() == mPostAll[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mPostAll[i].Read(scope, 0, 0);
                    mPostAll[mMode].CopyToBitmaps(mPost, 16, 0, true);
                    break;
                }
                if (data.GetId() == mPostTextAll[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mPostTextAll[i].Read(scope, 0, 0);
                    for (unsigned int j = 0; j < 16; ++j)
                    {
                        mPostTextAll[mMode].CopyToTextElement(mPostText[j]);
                    }
                    break;
                }
                if (data.GetId() == mPostSelectAll[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mPostSelectAll[i].Read(scope, 0, 0);
                    mPostSelectAll[mMode].CopyToBitmaps(mPostSelect, 16, 0, true);
                    break;
                }
                if (data.GetId() == mMarkerAll[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mMarkerAll[i].Read(scope, 0, 0);
                    mMarkerAll[mMode].CopyToBitmaps(mMarker, 32, 0, true);
                    break;
                }
                if (data.GetId() == mNorthIcon[i].GetNameHash())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mNorthIcon[i].Read(scope, 0, 0);
                    break;
                }
            }
            if (i >= mode_MAX)
            {
                ReadChild(config, data, pCallback, pUserData);
            }
        }
        else
        {
            ReadData(config, data);
        }
    }

    char buffer[256];
    for (i = 0; i < mode_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "%s.PostSelected%s", GetName(), sModeStrings[i]);
        mPostSelectAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.Marker%s", GetName(), sModeStrings[i]);
        mMarkerAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.Backdrop%s", GetName(), sModeStrings[i]);
        mBackdrop[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.Player%s", GetName(), sModeStrings[i]);
        mPlayerFOV[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.PlayerDirection%s", GetName(), sModeStrings[i]);
        mDirection[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.Target%s", GetName(), sModeStrings[i]);
        mTargetAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.Post%s", GetName(), sModeStrings[i]);
        mPostAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.PostText%s", GetName(), sModeStrings[i]);
        mPostTextAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.PostSelect%s", GetName(), sModeStrings[i]);
        mPostSelectAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.Marker%s", GetName(), sModeStrings[i]);
        mMarkerAll[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.North%s", GetName(), sModeStrings[i]);
        mNorthIcon[i].SetDisplayName(buffer);
    }

    PostReadSetup();
}

// FUNCTION: SWBF2 0x005520b0
void HUD::ElementMap::PostReadSetup()
{
    HUD::ElementGroupBase::PostReadSetup();

    for (unsigned int i = 0; i < mode_MAX; ++i)
    {
        PblMatrix matrix = _IdentityMatrix;
        matrix.m[3][0] = mPosition[i].x;
        matrix.m[3][1] = mPosition[i].y;
        matrix.m[3][2] = mPosition[i].z;
        matrix.m[3][3] = 1.0f;
        mMatrix[i].SetMatrix(&matrix);
        mMatrix[i].ApplyRelativeMode(GetContainerFrameWidth(), GetContainerFrameHeight(),
                                     GetContainerViewWidth(), GetContainerViewHeight(),
                                     InScreenGroup());
        mMatrix[i].SetMatrix(0);
        mPosition[i] = PblVector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
        mBackdrop[i].RedInterfaceElement::Scale(1.0f, 1.0f / (0.75f / RedRenderer::GetScreenAspectRatio()));
    }

    if (0.75f / RedRenderer::GetScreenAspectRatio() > 1.0f)
    {
        RedGroupElement::Scale(1.0f, (0.75f / RedRenderer::GetScreenAspectRatio() - 1.0f) * 0.25f + 1.0f);
    }
    else
    {
        RedGroupElement::Scale(1.0f, 0.75f / RedRenderer::GetScreenAspectRatio());
    }
}

// FUNCTION: SWBF2 0x00554e60
bool HUD::ElementMap::ReadData(PblConfig& config, PblConfig::Data& data)
{
    PblMatrix matrix;
    unsigned int uShape;
    RedColor color;

    switch (data.GetId())
    {
    case kEventToggleMapModeHash:
        Item::ReadEvent(data, mEventToggleMapMode);
        return true;
    case kEventChangeMapModeHash:
        Item::ReadEvent(data, mEventChangeMapMode);
        return true;
    case kEventPostHideHash:
        Item::ReadEvent(data, mEventPostHide);
        return true;
    case kEventRefreshTargetHash:
        Item::ReadEvent(data, mEventRefreshTarget);
        return true;
    case kEventRefreshPostHash:
        Item::ReadEvent(data, mEventRefreshPost);
        return true;
    case kEventRefreshMarkerHash:
        Item::ReadEvent(data, mEventRefreshMarker);
        return true;
    case kTargetFireColorHash:
        color.b = data.GetUint8Arg(2);
        color.g = data.GetUint8Arg(1);
        color.r = data.GetUint8Arg(0);
        color.a = 0xff;
        mTargetFireColor = color;
        return true;
    case kPostFlashRateHash:
        mPostFlashRate = data.GetFloatArg(0);
        return true;
    case kPostFlashRadiusHash:
        mPostFlashRadius = data.GetFloatArg(0);
        return true;
    case kPostFlashColorHash:
        color.b = data.GetUint8Arg(2);
        color.g = data.GetUint8Arg(1);
        color.r = data.GetUint8Arg(0);
        color.a = 0xff;
        mPostFlashColor = color;
        return true;
    case kPositionSmallHash:
        mMatrix[mode_Small].SetMatrix(&matrix);
        mMatrix[mode_Small].ReadPosition(data, kPositionHash);
        mMatrix[mode_Small].SetMatrix(0);
        mPosition[mode_Small] = PblVector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
        return true;
    case kPositionLargeHash:
        mMatrix[mode_Large].SetMatrix(&matrix);
        mMatrix[mode_Large].ReadPosition(data, kPositionHash);
        mMatrix[mode_Large].SetMatrix(0);
        mPosition[mode_Large] = PblVector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
        return true;
    case kPositionSpawnHash:
        mMatrix[mode_Spawn].SetMatrix(&matrix);
        mMatrix[mode_Spawn].ReadPosition(data, kPositionHash);
        mMatrix[mode_Spawn].SetMatrix(0);
        mPosition[mode_Spawn] = PblVector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
        return true;
    case kBackgroundMaskShapeSmallHash:
        uShape = TranslateString((const char* const*)sMaskShapeStrings, maskShape_MAX, data.GetStringArg(0));
        if (uShape < maskShape_MAX)
        {
            mDim[mode_Small].shape = (MaskShape)uShape;
        }
        return true;
    case kBackgroundMaskShapeLargeHash:
        uShape = TranslateString((const char* const*)sMaskShapeStrings, maskShape_MAX, data.GetStringArg(0));
        if (uShape < maskShape_MAX)
        {
            mDim[mode_Large].shape = (MaskShape)uShape;
        }
        return true;
    case kBackgroundMaskShapeSpawnHash:
        uShape = TranslateString((const char* const*)sMaskShapeStrings, maskShape_MAX, data.GetStringArg(0));
        if (uShape < maskShape_MAX)
        {
            mDim[mode_Spawn].shape = (MaskShape)uShape;
        }
        return true;
    case kAllowMiniMapHash:
        mAllowMiniMap = data.GetBoolArg(0);
        mAllowToggleOff = !mAllowMiniMap;
        return true;
    case kPostNumbersHash:
        mPostNumbers = data.GetBoolArg(0);
        return true;
    }
    return ElementGroupPlayer::ReadData(config, data);
}

// FUNCTION: SWBF2 0x00555840
HUD::ElementMap* HUD::ElementMap::FindByPlayerIndex(int iPlayerIndex)
{
    for (PblListSingle::Node* pNode = sMapList.mHead.mNext; pNode != &sMapList.mHead; pNode = pNode->mNext)
    {
        ElementMap* pMap = (ElementMap*)((char*)pNode - offsetof(ElementMap, mMapNode));
        if (pMap->mPlayerIndex == iPlayerIndex)
        {
            return pMap;
        }
    }
    return 0;
}
 
 

 
 
 
 
 
 
 
 
 
 
 

// GLOBAL: SWBF2 0x01e56c38
static bool updating;

namespace FriendBuddyList { bool __fastcall IsFriendBuddy(int iLocalIndex, int iPlayerId); }

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);

extern RedColor gHudHighlightColor;
namespace GameLoop { extern bool mbHistoricalRules; }
extern bool netFrameLock;
extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x00551420
void HUD::ElementMap::Update(const float fElapsed)
{
    if (!mInitialized)
    {
        Init();
    }

    if (fElapsed != 0.0f && RedGroupElement::IsEnabled())
    {
        if (!updating)
        {
            updating = true;
            if (netIsDedicated)
            {
                SetMode(mode_Large);
            }
            else if (!mPlayerCharacter || (mMode != mode_Spawn && !mPlayerCharacter->mUnit))
            {
                HUD::Element::Enable(false);
            }
        }

        if (mPlayerCharacter && SpawnDisplay::Instance(mPlayerIndex)->mSpectateMode == true &&
            RedGroupElement::IsEnabled())
        {
            UpdateEnableState(enableState_Disabling);
        }

        Controllable* pControllable = mPlayerCharacter ? mPlayerCharacter->GetControllable() : 0;
        if (pControllable)
        {
            GameObject* pObject = pControllable->GetGameObject();
            const bool bInterpolate = NetGame::IsEnabled() && !netFrameLock && pObject;
            const Controllable* pVehicle = mPlayerCharacter->mVehicle;

            PblMatrix matrix = CameraManager::Instance()->GetCamera(mPlayerIndex)->GetMatrix();
            mPlayerDirection = matrix.GetZAxis();
            mPlayerPosition = matrix.GetTranslation();

            const GameObject* pVehicleObject;
            if (pVehicle && (pVehicleObject = pVehicle->GetGameObject()) != 0)
            {
                if (bInterpolate)
                {
                    gameObjectInterpolator.GetMatrix(pVehicleObject->GetNetUniqueId(), matrix);
                }
                else
                {
                    matrix = pVehicleObject->GetMatrix();
                }
                mPlayerMovementDirection.x = -matrix.m[2][0];
                mPlayerMovementDirection.y = -matrix.m[2][1];
                mPlayerMovementDirection.z = -matrix.m[2][2];
            }
            else
            {
                mPlayerMovementDirection = mPlayerDirection;
            }

            float fRadius;
            if (NetGame::IsEnabled() && (pControllable->GetGameObject()->GetGameObjectType() & GO_WALKER))
            {
                fRadius = 125.0f;
            }
            else
            {
                GameObject* pControlled = pControllable->GetGameObject();
                const PblVector3& vVelocity = pControlled->GetVelocity();
                const float fSpeed = sqrtf(vVelocity.x * vVelocity.x + vVelocity.z * vVelocity.z);

                Controllable* pPilot = 0;
                const PblList<Controllable>& list = pControlled->GetControllableList();
                if (list.GetCount())
                {
                    for (PblList<Controllable>::Itor itor(list); (Controllable*)itor; ++itor)
                    {
                        if (itor->GetPlayerId() >= 0)
                        {
                            pPilot = itor;
                            break;
                        }
                    }
                }
                const EntityClass* pClass = pPilot ? pPilot->GetEntity()->GetEntityClass()
                                                   : pControlled->GetEntityClass();
                const EntityClass::MapParams& params = pClass->mMapParams;

                float fFactor = (fSpeed - params.mSpeedMin) / (params.mSpeedMax - params.mSpeedMin);
                if (fFactor < 0.0f)
                {
                    fFactor = 0.0f;
                }
                else if (fFactor > 1.0f)
                {
                    fFactor = 1.0f;
                }
                fRadius = (params.mViewRangeMax - params.mViewRangeMin) * fFactor + params.mViewRangeMin;
            }

            mDim[mode_Small].centerX = mPlayerPosition.x;
            mDim[mode_Small].centerZ = mPlayerPosition.z;
            mDim[mode_Small].radius = fRadius;
            mDim[mode_Small].scale = 0.5f / fRadius;
        }

        float fRotation = 0.0f;
        if (mMode == mode_Small || sNorth != 0.0f)
        {
            if (mMode == mode_Small)
            {
                fRotation = -PblMath::Atan2(-mPlayerDirection.x, mPlayerDirection.z);
            }
            else
            {
                fRotation = sNorth;
            }

            PblMatrix rotation;
            D3DXMatrixRotationZ((D3DXMATRIX*)&rotation, fRotation);
            rotation.m[3][0] = gVectorZero.x;
            rotation.m[3][1] = gVectorZero.y;
            rotation.m[3][2] = gVectorZero.z;
            rotation.m[3][3] = 1.0f;
            PblMatrix matrix;
            D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&rotation,
                               (const D3DXMATRIX*)&mGroupMatrix);
            RedGroupElement::SetMatrix(matrix);
            UpdateBackdropTexCoords();
        }

        UpdateNorthIcon(fElapsed);
        if (!netIsDedicated)
        {
            UpdatePlayerIcon();
        }
        UpdateTargetIcons(pControllable);
        UpdatePostIcons(fElapsed, fRotation);
        UpdateMarkerIcons();

        if (mNextSlowUpdate <= 0.0f)
        {
            mNextSlowUpdate = 0.0f;
        }
        mNextSlowUpdate -= fElapsed;

        HUD::ElementGroupPlayer::Update(fElapsed);
    }
}

// FUNCTION: SWBF2 0x005529a0
bool HUD::ElementMap::CalculateScreenCoordinates(const PblVector3& vPosition, float& fX, float& fY,
                                                 bool bClamp, float* const pfDistanceSq) const
{
    const Dim& dim = mDim[mMode];
    fY = (vPosition.z - dim.centerZ) * dim.scale;
    fX = (vPosition.x - dim.centerX) * dim.scale;
    const float fDistanceSq = fY * fY + fX * fX;
    if (pfDistanceSq)
    {
        *pfDistanceSq = fDistanceSq;
    }

    if (dim.shape == maskShape_Circle)
    {
        if (fDistanceSq > 0.25f)
        {
            if (!bClamp)
            {
                return false;
            }
            const float fScale = 0.5f / (float)sqrt((double)fDistanceSq);
            fX *= fScale;
            fY *= fScale;
        }
    }
    else
    {
        float fAbsX = fabsf(fX);
        if (fAbsX > 0.5f || fabsf(fY) > 0.5f)
        {
            if (!bClamp)
            {
                return false;
            }
            const float fAbsY = fabsf(fY);
            if (!(fAbsX > fAbsY))
            {
                fAbsX = fAbsY;
            }
            const float fScale = 0.5f / fAbsX;
            fX *= fScale;
            fY *= fScale;
        }
    }

    float fLeft, fTop, fRight, fBottom;
    mBackdrop[mMode].GetRect(fLeft, fTop, fRight, fBottom);
    fX = (fRight - fLeft) * fX;
    fY = (fBottom - fTop) * fY;
    return true;
}

// FUNCTION: SWBF2 0x00552c10
void HUD::ElementMap::UpdateBackdropTexCoords()
{
    const Dim& dim = mDim[mMode];
    const Dim& large = mDim[mode_Large];
    static_cast<RedBitmapElement&>(mBackdrop[mMode]).SetTexCoords(
        (dim.centerX - dim.radius - large.centerX) * large.scale + 0.5f,
        (dim.centerZ - dim.radius - large.centerZ) * large.scale + 0.5f,
        (dim.centerX + dim.radius - large.centerX) * large.scale + 0.5f,
        (dim.centerZ + dim.radius - large.centerZ) * large.scale + 0.5f,
        false);
}

// FUNCTION: SWBF2 0x00552cd0
void HUD::ElementMap::UpdateNorthIcon(float)
{
    const PblVector3& vScale = mNorthIcon[mMode].HUD::Element::mMatrix.GetScale();
    float fLeft, fTop, fRight, fBottom;
    mBackdrop[mMode].GetRect(fLeft, fTop, fRight, fBottom);
    mNorthIcon[mMode].SetMatrixPosRotScale(0.0f, (fBottom - fTop) * -0.5f, 0.0f, vScale.x, vScale.y);

    if (sNorth != 0.0f)
    {
        PblMatrix matrix = mNorthIcon[mMode].m_mLocal;
        float fAngle = sNorth;
        if (mMode == mode_Small && !sFlipNorth)
        {
            fAngle = 0.0f;
        }
        PblMatrix rotation;
        D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&matrix,
                           D3DXMatrixRotationZ((D3DXMATRIX*)&rotation, fAngle));
        mNorthIcon[mMode].SetMatrix(matrix);
    }
}

// FUNCTION: SWBF2 0x00552e30
void HUD::ElementMap::UpdatePlayerIcon()
{
    float fScreenX, fScreenY;
    CalculateScreenCoordinates(mPlayerPosition, fScreenX, fScreenY, true, 0);

    float fX = 0.0f;
    float fNorth;
    if (mMode == mode_Small)
    {
        fNorth = 0.0f;
    }
    else if (sFlipNorth)
    {
        fNorth = 0.0f;
    }
    else
    {
        fNorth = sNorth;
    }

    const float fViewAngle = PblMath::Atan2(-mPlayerDirection.x, mPlayerDirection.z) + fNorth;
    float fMoveAngle;
    if (mPlayerCharacter && mPlayerCharacter->mVehicle)
    {
        fMoveAngle = PblMath::Atan2(-mPlayerMovementDirection.x, mPlayerMovementDirection.z) + fNorth;
    }
    else
    {
        fMoveAngle = fViewAngle;
    }

    float fY;
    if (mMode == mode_Small)
    {
        fY = 0.0f;
    }
    else
    {
        fY = fScreenY;
        fX = fScreenX;
    }

    const PblMatrix mFlat(
        PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
        PblVector4{ 0.0f, 0.0f, -1.0f, 0.0f },
        PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
        PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });
    PblMatrix temp;

    const PblVector3& vViewScale = mPlayerFOV[mMode].HUD::Element::mMatrix.GetScale();
    PblMatrix matrix = mFlat;
    D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&matrix,
                       D3DXMatrixRotationZ((D3DXMATRIX*)&temp, fViewAngle - PblMath::fPi));
    D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&matrix,
                       D3DXMatrixScaling((D3DXMATRIX*)&temp, vViewScale.x, vViewScale.y, vViewScale.z));
    matrix.m[3][0] = fX;
    matrix.m[3][1] = fY;
    matrix.m[3][2] = 0.0f;
    matrix.m[3][3] = 1.0f;
    mPlayerFOV[mMode].RedInterfaceElement::SetMatrix(matrix);

    const bool bShowDirection = (float)fabs((double)(fMoveAngle - fViewAngle)) >= 0.2617994f;
    if (bShowDirection)
    {
        const PblVector3& vMoveScale = mDirection[mMode].HUD::Element::mMatrix.GetScale();
        matrix = mFlat;
        D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&matrix,
                           D3DXMatrixRotationZ((D3DXMATRIX*)&temp, fMoveAngle - PblMath::fPi));
        D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&matrix,
                           D3DXMatrixScaling((D3DXMATRIX*)&temp, vMoveScale.x, vMoveScale.y, vMoveScale.z));
        matrix.m[3][0] = fX;
        matrix.m[3][1] = fY;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
    }
    mDirection[mMode].RedInterfaceElement::SetMatrix(matrix);
    mDirection[mMode].HUD::Element::Enable(bShowDirection);
}

// FUNCTION: SWBF2 0x005531a0
const Team* HUD::ElementMap::GetPlayerTeam() const
{
    if (mPlayerCharacter)
    {
        return mPlayerCharacter->mTeamPtr;
    }
    return Team::GetPtr(1);
}

// FUNCTION: SWBF2 0x005531c0
void HUD::ElementMap::UpdateTargetIcons(const Controllable* pPlayer)
{
    if (!netIsDedicated)
    {
        if (!GetPlayerTeam())
        {
            return;
        }
        if (!pPlayer)
        {
            return;
        }
    }

    const bool bSensorArray =
        pPlayer && BonusManager::Instance()->IsTeamBonusActive(pPlayer->GetGameObject()->GetTeam(), 0x56e2ce71);
    const bool bEasy = mPlayerIndex >= 0 && GameLoop::mbHistoricalRules &&
                       PlayerProfile::GetProfile(mPlayerIndex)->mGameOptions.mCurDiffLevel <= eDIFFICULTY_MEDIUM;

    for (unsigned int i = 0; i < TargetManager::MAX_TARGETS; ++i)
    {
        TargetManager::Target& target = TargetManager::_gTarget[i];
        RedBitmapElement& bitmap = mTarget[i];

        GameObject* pObject = target.actor;
        if (!pObject)
        {
            continue;
        }
        const int iTeam = pObject->mPerceivedTeam;
        const Team* pTeam = Team::GetPtr(iTeam);
        if (!pTeam)
        {
            continue;
        }

        const Team* pPlayerTeam = GetPlayerTeam();
        RedColor color = pPlayerTeam->mColor[iTeam];
        const int iAffiliation = pTeam->GetAffiliation(pPlayerTeam->mTeamNumber);
        bitmap.m_color.a = (unsigned char)(int)(mTargetAll[mMode].mAlpha * 255.0f);

        PblVector3 vDelta;
        float fDistance;
        if (pPlayer)
        {
            GameObject* pTarget = target.actor;
            const PblMatrix& mPlayer = pPlayer->GetMatrix();
            const PblMatrix& mTargetMatrix = pTarget->GetMatrix();
            vDelta.x = mTargetMatrix.m[3][0] - mPlayer.m[3][0];
            vDelta.y = mTargetMatrix.m[3][1] - mPlayer.m[3][1];
            vDelta.z = mTargetMatrix.m[3][2] - mPlayer.m[3][2];

            const float fLength2EstAlpha = 0.94805944f;
            const float fLength2EstBeta = 0.39269909f;
            float fA = (float)fabs((double)vDelta.x);
            float fB = (float)fabs((double)vDelta.y);
            float fEst = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                                 : fA * fLength2EstBeta + fB * fLength2EstAlpha;
            fA = (float)fabs((double)fEst);
            fB = (float)fabs((double)vDelta.z);
            fDistance = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                                : fA * fLength2EstBeta + fB * fLength2EstAlpha;
        }
        else
        {
            fDistance = 0.0f;
            vDelta.x = 0.0f;
            vDelta.y = 0.0f;
            vDelta.z = 0.0f;
        }

        const float fScale = CalcFireIndication(target, fDistance);

        if (iAffiliation >= 0 && target.alwaysVisibleToFriends)
        {
            bitmap.Enable(true);
            if (NetGame::IsEnabled())
            {
                GameObject* pTarget = target.actor;
                if (pTarget->GetControllable() &&
                    FriendBuddyList::IsFriendBuddy(NetGame::GetLocalIndex(mPlayerIndex),
                               ((GameObject*)target.actor)->GetControllable()->GetPlayerId()))
                {
                    color = gHudHighlightColor;
                }
            }
        }
        else
        {
            if ((!pPlayer || (!bSensorArray && !bEasy && !IsInRadarRange(pPlayer, target, vDelta, fDistance))) &&
                !netIsDedicated && fScale <= 0.0f)
            {
                bitmap.Enable(false);
                continue;
            }
            GameObject* pTarget = target.actor;
            if (pTarget->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
                reinterpret_cast<const EntitySoldier*>((GameObject*)target.actor)->GetInvisibilityFactor() > 0.0f)
            {
                bitmap.Enable(false);
                continue;
            }
            bitmap.Enable(true);
        }

        const PblMatrix& mTargetMatrix = ((GameObject*)target.actor)->GetMatrix();
        float fX, fY;
        if (!CalculateScreenCoordinates(mTargetMatrix.GetTranslation(), fX, fY, false, 0))
        {
            bitmap.Enable(false);
            continue;
        }

        const bool bBuilding = ((GameObject*)target.actor)->IsRtti(EntityBuilding::rttiHashEntityBuilding);
        if (((GameObject*)target.actor)->IsAlive())
        {
            bitmap.SetColorOnly(Interpolate(color, mTargetFireColor, fScale));
        }
        else
        {
            if (bBuilding && ((GameObject*)target.actor)->mEntityClass->mRemoveFromMapWhenDead)
            {
                bitmap.Enable(false);
                continue;
            }
            bitmap.SetColorOnly(Interpolate(color, RedColor::DKGREY, 0.5f));
        }

        if (bBuilding && mNextSlowUpdate > 0.0f)
        {
            continue;
        }

        const float fSize = target.mapParams.mScale;
        if (fSize == 0.0f)
        {
            bitmap.Enable(false);
            continue;
        }

        const PblMatrix& mHeading = ((GameObject*)target.actor)->GetMatrix();
        const PblVector3 vDirection =
            NormalizeEst(PblVector3(-mHeading.m[2][0], 0.0f, -mHeading.m[2][2]));
        PblMatrix matrix;
        matrix.m[0][0] = vDirection.z * fSize;
        matrix.m[0][1] = -(vDirection.x * fSize);
        matrix.m[0][2] = 0.0f;
        matrix.m[0][3] = 0.0f;
        matrix.m[1][0] = vDirection.x * fSize;
        matrix.m[1][1] = vDirection.z * fSize;
        matrix.m[1][2] = 0.0f;
        matrix.m[1][3] = 0.0f;
        matrix.m[2][0] = 0.0f;
        matrix.m[2][1] = 0.0f;
        matrix.m[2][2] = fSize;
        matrix.m[2][3] = 0.0f;
        matrix.m[3][0] = fX;
        matrix.m[3][1] = fY;
        matrix.m[3][2] = 0.0f;
        matrix.m[3][3] = 1.0f;
        bitmap.SetMatrix(matrix);
    }
}

static inline float Length2EstXZ(float fX, float fZ)
{
    const float fLength2EstAlpha = 0.94805944f;
    const float fLength2EstBeta = 0.39269909f;
    const float fA = (float)fabs((double)fX);
    const float fB = (float)fabs((double)fZ);
    return fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                   : fA * fLength2EstBeta + fB * fLength2EstAlpha;
}

// FUNCTION: SWBF2 0x005538a0
bool HUD::ElementMap::IsInRadarRange(const Controllable* pPlayer, TargetManager::Target& target,
                                     const PblVector3& vDelta, float fDistance) const
{
    const EntityClass* pClass =
        static_cast<const Trackable*>(pPlayer)->GetGameObject()->GetEntityClass();

    if (!((GameObject*)target.actor)->GetEntity()->IsRtti(EntitySoldier::rttiHashEntitySoldier) &&
        !((GameObject*)target.actor)->GetEntity()->IsRtti(EntityDroideka::rttiHashEntityDroideka))
    {
        if (!((GameObject*)target.actor)->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer) &&
            !((GameObject*)target.actor)->GetEntity()->IsRtti(EntityHover::rttiHashEntityHover) &&
            !target.actor.operator->()->GetEntity()->IsRtti(EntityWalker::rttiHashEntityWalker))
        {
            return true;
        }
        if (((GameObject*)target.actor)->GetTeam() == pPlayer->mCharacter->mTeamNumber)
        {
            return true;
        }
        if (((GameObject*)target.actor)->GetTeam() == 0)
        {
            return true;
        }
    }

    if (pClass->mMapParams.mRangeOverall > fDistance)
    {
        return true;
    }
    if (!(pClass->mMapParams.mRangeViewCone > fDistance))
    {
        return false;
    }

    const float fDot = vDelta.x * pPlayer->mEyeDir.x + vDelta.z * pPlayer->mEyeDir.z;
    const float fEye = Length2EstXZ(pPlayer->mEyeDir.x, pPlayer->mEyeDir.z);
    const float fDelta = Length2EstXZ(vDelta.x, vDelta.z);
    return fDot / (fDelta * fEye) > pClass->mMapParams.mViewConeCosine;
}

// FUNCTION: SWBF2 0x00553b30
float HUD::ElementMap::CalcFireIndication(TargetManager::Target& target, float fDistance) const
{
    if (fDistance > ((GameObject*)target.actor)->GetEntityClass()->mMapParams.mRangeShooting)
    {
        return 0.0f;
    }

    float fScale = 0.0f;
    const PblList<Controllable>& list = ((GameObject*)target.actor)->GetControllableList();
    for (PblList<Controllable>::Itor it = list.Begin(); (Controllable*)it; ++it)
    {
        Controllable* pControllable = it;
        const int iCount = pControllable->GetWeaponCount();
        for (int i = 0; i < iCount && fScale == 0.0f; ++i)
        {
            Weapon* pWeapon = pControllable->GetWeaponPtr(i);
            if (pWeapon && (pWeapon->mState == Weapon::FIRE || pWeapon->mState == Weapon::FIRE2))
            {
                const float fFire = 1.0f - pWeapon->mStateTimer / pWeapon->mStateLimit;
                if (!(fScale > fFire))
                {
                    fScale = fFire;
                }
            }
        }
    }
    return fScale;
}

// FUNCTION: SWBF2 0x00553c40
void HUD::ElementMap::UpdatePostIcons(float fElapsed, float fRotation)
{
    const Team* pTeam = GetPlayerTeam();
    CommandPost* pPlayerPost = mPlayerCharacter ? (CommandPost*)mPlayerCharacter->mPost : 0;

    for (unsigned int i = 0; i < TargetManager::MAX_POSTS; ++i)
    {
        TargetManager::Post& post = TargetManager::_gPost[i];
        RedBitmapElement& bitmap = mPost[i];
        RedBitmapElement& select = mPostSelect[i];
        RedTextElement& text = mPostText[i];

        if (!mPostHide || mMode == mode_Spawn)
        {
            if (!post.actor)
            {
                continue;
            }
            if (post.actor->GetGameObject() && post.actor->GetGameObject()->IsAlive())
            {
                const int iTeam = post.actor->GetGameObject()->GetTeam();
                bitmap.SetColorOnly(pTeam->mColor[iTeam]);

                const int iHoldTeam = post.actor->mHoldTeam;
                if (iHoldTeam != 0 && iHoldTeam != iTeam)
                {
                    const float fTime = (float)fmod((double)GameLoop::GetMissionTime(), 1.0);
                    if (fTime < 0.25f)
                    {
                        const RedColor holdColor = pTeam->mColor[iHoldTeam < 0 ? 0 : iHoldTeam];
                        if (fTime < 0.125f)
                        {
                            bitmap.m_color = Interpolate(bitmap.m_color, holdColor, fTime * 8.0f);
                        }
                        else
                        {
                            bitmap.m_color = Interpolate(bitmap.m_color, holdColor, (0.25f - fTime) * 8.0f);
                        }
                    }
                }

                float fX, fY, fDistanceSq;
                if (CalculateScreenCoordinates(post.actor->GetMatrix().GetTranslation(), fX, fY, true,
                                               &fDistanceSq))
                {
                    if (post.actor->GetGameObject()->GetGameObjectType() == GO_BUILDING)
                    {
                        bitmap.SetMatrixPosScale(fX, fY, post.mapParams.mScale, post.mapParams.mScale);
                    }
                    else
                    {
                        const PblVector3& vAxis = post.actor->GetMatrix().GetZAxis();
                        const PblVector3 vForward(-vAxis.x, -vAxis.y, -vAxis.z);
                        bitmap.SetMatrixPosRotScale(fX, fY, PblMath::Atan2(-vForward.x, vForward.z),
                                                    post.mapParams.mScale, post.mapParams.mScale);
                    }
                    bitmap.Enable(true);

                    if (mPostNumbers)
                    {
                        float fSin, fCos;
                        PblMath::SinCos(&fSin, &fCos, -fRotation);
                        text.SetMatrixPosRotScale(fSin * 5.0f + fX, fY - fCos * 5.0f, -fRotation, 1.0f,
                                                  1.0f);
                        RedColor color = mPostTextAll[mMode].m_color;
                        if (mMode == mode_Small)
                        {
                            float fAlpha = 1.0f - fDistanceSq * 4.0f;
                            if (fAlpha < 0.0f)
                            {
                                fAlpha = 0.0f;
                            }
                            else if (fAlpha > 1.0f)
                            {
                                fAlpha = 1.0f;
                            }
                            color.a = (unsigned char)(color.a * (int)(fAlpha * 255.0f) / 255);
                        }
                        text.m_color = color;
                        if (!text.IsEnabled() && post.actor->mDisplayHUDIndex)
                        {
                            unsigned short buffer[128];
                            PblUnicode::usnprintf(buffer, sizeof(buffer) - 1, "%d", post.actor->mHUDPostIndex);
                            text.SetText(buffer, true);
                            text.Enable(true);
                        }
                    }

                    if (mMode == mode_Spawn && mPlayerCharacter && NetGame::ConvertCP(post.actor) == pPlayerPost)
                    {
                        mPostScale[i] = 2.0f;
                        select.m_color = mPostFlashColor;
                        const float fScale = mMode == mode_Spawn
                                                 ? mPostScale[i] * post.mapParams.mScale
                                                 : mPostFlashRadius * mPostScale[i] + post.mapParams.mScale;
                        select.SetMatrixPosScale(fX, fY, fScale, fScale);
                        mPostScale[i] -= mPostFlashRate * fElapsed;
                        if (mPostScale[i] < 0.0f)
                        {
                            mPostScale[i] = 0.0f;
                        }
                        select.Enable(true);
                    }
                    else
                    {
                        mPostScale[i] = 0.0f;
                        select.Enable(false);
                    }
                    continue;
                }
            }
        }

        bitmap.Enable(false);
        select.Enable(false);
        text.Enable(false);
    }
}

// FUNCTION: SWBF2 0x00554330
void HUD::ElementMap::UpdateMarkerIcons()
{
    const unsigned int uTeamMask = 1 << (mPlayerCharacter ? mPlayerCharacter->mTeamNumber : 0);
    const Team* pTeam = GetPlayerTeam();

    for (unsigned int i = 0; i < TargetManager::MAX_MARKERS; ++i)
    {
        RedBitmapElement& bitmap = mMarker[i];
        TargetManager::Marker& marker = TargetManager::gMarkers[i];
        bitmap.Enable(false);
        if (!(uTeamMask & marker.teamMask))
        {
            continue;
        }
        if (!marker.actor && !marker.region)
        {
            continue;
        }

        RedColor color = marker.color;
        float fTime;
        if (marker.flashAlpha || marker.pulseSize)
        {
            fTime = (float)fmod((double)GameLoop::GetMissionTime(), 1.0);
        }
        else
        {
            fTime = 0.0f;
        }
        if (marker.flashAlpha)
        {
            color.a = (unsigned char)(int)((1.0f - fTime) * 255.0f);
        }

        PblVector3 vPosition;
        if (marker.region)
        {
            vPosition = marker.region->mMatrix.GetTranslation();
        }
        else
        {
            const PblMatrix& mMatrix = ((GameObject*)marker.actor)->GetMatrix();
            const PblMatrix& mSmoothed = ((GameObject*)marker.actor)->GetSmoothedMatrix();
            const float fOffsetX = mSmoothed.m[3][0] - mMatrix.m[3][0];
            const float fOffsetY = mSmoothed.m[3][1] - mMatrix.m[3][1];
            const float fOffsetZ = mSmoothed.m[3][2] - mMatrix.m[3][2];
            const PblVector3& vCenter = ((GameObject*)marker.actor)->GetCollisionCenter();
            vPosition.x = vCenter.x + fOffsetX;
            vPosition.y = vCenter.y + fOffsetY;
            vPosition.z = vCenter.z + fOffsetZ;
            if (marker.color.a == 0)
            {
                color = pTeam->mColor[((GameObject*)marker.actor)->GetTeam()];
            }
        }
        bitmap.m_color = color;

        float fX, fY;
        if (!CalculateScreenCoordinates(vPosition, fX, fY, true, 0))
        {
            continue;
        }

        float fScale = marker.scale;
        if (marker.pulseSize)
        {
            fScale = fScale * fTime + marker.pulseStartScale;
        }
        if (mMode == mode_Small)
        {
            bitmap.SetMatrixPosRotScale(fX, fY, PblMath::Atan2(-mPlayerDirection.x, mPlayerDirection.z), fScale,
                                        fScale);
        }
        else if (sNorth != 0.0f)
        {
            bitmap.SetMatrixPosRotScale(fX, fY, -sNorth, fScale, fScale);
        }
        else
        {
            bitmap.SetMatrixPosScale(fX, fY, fScale, fScale);
        }
        bitmap.Enable(true);
    }
}

// FUNCTION: SWBF2 0x00554780
void HUD::ElementMap::Init()
{
    float fMinX = FLT_MAX;
    float fMinZ = FLT_MAX;
    float fMaxX = -FLT_MAX;
    float fMaxZ = -FLT_MAX;
    float fRadius;

    const PblRegion* pBounds = PblRegion::Find(0x866e09f4);
    if (pBounds && (pBounds->mType == PblRegion::type_box || pBounds->mType == PblRegion::type_box_aligned))
    {
        const float fHalfX = pBounds->mMatrix.m[0][0] * pBounds->mSize.x;
        const float fHalfZ = pBounds->mMatrix.m[2][2] * pBounds->mSize.z;
        const PblVector3 vCenter = pBounds->mMatrix.GetTranslation();
        fMinX = vCenter.x - fHalfX;
        fMaxX = vCenter.x + fHalfX;
        fMaxZ = vCenter.z + fHalfZ;
        fMinZ = vCenter.z - fHalfZ;
        fRadius = (fMaxX - fMinX) * 0.5f;
        if (!(fRadius > (fMaxZ - fMinZ) * 0.5f))
        {
            fRadius = (fMaxZ - fMinZ) * 0.5f;
        }
    }
    else
    {
        for (CommandPostItor itor = CommandPostManager::Instance()->GetItor(); itor; ++itor)
        {
            const PblVector3& vPost = (*itor)->GetMatrix().GetTranslation();
            if (fMinX > vPost.x)
            {
                fMinX = vPost.x;
            }
            if (fMinZ > vPost.z)
            {
                fMinZ = vPost.z;
            }
            if (vPost.x > fMaxX)
            {
                fMaxX = vPost.x;
            }
            if (vPost.z > fMaxZ)
            {
                fMaxZ = vPost.z;
            }
        }
        fRadius = (fMaxX - fMinX) * 0.5f + 100.0f;
        if (!(fRadius > (fMaxZ - fMinZ) * 0.5f + 100.0f))
        {
            fRadius = (fMaxZ - fMinZ) * 0.5f + 100.0f;
        }
    }

    mDim[mode_Large].radius = fRadius;
    mDim[mode_Large].centerX = (fMaxX + fMinX) * 0.5f;
    mDim[mode_Large].centerZ = (fMaxZ + fMinZ) * 0.5f;
    mDim[mode_Large].scale = 0.5f / fRadius;
    mDim[mode_Spawn] = mDim[mode_Large];

    if (mPlayerIndex >= 0)
    {
        mPlayerCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mPlayerIndex));
    }

    mGroupMatrix = RedGroupElement::m_mLocal;

    for (unsigned int i = 0; i < mode_MAX; ++i)
    {
        char buffer[256];
        _snprintf(buffer, sizeof(buffer), "%s_map", GameLoop::mTerrainFileName);
        mBackdrop[i].SetTexture((unsigned int)PblHash(buffer));
        mBackdrop[i].SetMaskTexture("map_mask");
    }

    mInitialized = true;
    updating = false;
}

// FUNCTION: SWBF2 0x00554ab0
void HUD::ElementMap::RefreshMapDependancy()
{
    if (mPlayerIndex < 0)
    {
        return;
    }

    const GameEvents::PlayerEvents& events = GameEvents::gPlayerEvents[mPlayerIndex];
    const bool bEnabled = mEnableState == enableState_Enabling || mEnableState == enableState_Enabled;
    if (bEnabled && mMode != mode_Small)
    {
        HUD::Event(events.objectiveListEnable, true).Send();
    }
    else
    {
        HUD::Event(events.objectiveListDisable, true).Send();
    }

    if (bEnabled)
    {
        switch (mMode)
        {
        case mode_Small:
            HUD::Event(events.mapSpawnLargeDisable, true).Send();
            break;
        case mode_Spawn:
            HUD::Event(events.mapSpawn, true).Send();

        case mode_Large:
            HUD::Event(events.mapSpawnLarge, true).Send();
            break;
        }
    }
    else
    {
        HUD::Event(events.mapSpawnLargeDisable, true).Send();
    }
}

// FUNCTION: SWBF2 0x00554b70
void HUD::ElementMap::SetMode(Mode eMode)
{
    if ((mEnableState == enableState_Enabled || mEnableState == enableState_Enabling) && eMode == mMode)
    {
        return;
    }

    if (mEnableState != enableState_Enabled)
    {
        UpdateEnableState(enableState_Enabling);
        if (mPlayerIndex >= 0)
        {
            HUD::Event(GameEvents::gPlayerEvents[mPlayerIndex].mapEnable, true).Send();
        }
    }

    EnableModeElements(false);
    mMode = eMode;
    mNextSlowUpdate = 0.0f;
    if (eMode == mode_Small && !mAllowMiniMap && mAllowToggleOff)
    {
        UpdateEnableState(enableState_Disabling);
        RefreshMapDependancy();
        return;
    }

    RedGroupElement::SetPosition(mPosition[eMode]);
    mGroupMatrix.m[3][0] = mPosition[mMode].x;
    mGroupMatrix.m[3][1] = mPosition[mMode].y;
    mGroupMatrix.m[3][2] = mPosition[mMode].z;
    mGroupMatrix.m[3][3] = 1.0f;
    EnableModeElements(true);

    if (mMode != mode_Small && mInitialized)
    {
        PblMatrix rotation;
        D3DXMatrixRotationZ((D3DXMATRIX*)&rotation, 0.0f);
        rotation.m[3][0] = gVectorZero.x;
        rotation.m[3][1] = gVectorZero.y;
        rotation.m[3][2] = gVectorZero.z;
        rotation.m[3][3] = 1.0f;
        PblMatrix matrix;
        D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&rotation,
                           (const D3DXMATRIX*)&mGroupMatrix);
        RedGroupElement::SetMatrix(matrix);
    }

    Update(0.001f);
    RefreshMapDependancy();
}

// FUNCTION: SWBF2 0x00554cf0
void HUD::ElementMap::EnableModeElements(bool bEnable)
{
    mBackdrop[mMode].HUD::Element::Enable(bEnable);
    mPlayerFOV[mMode].HUD::Element::Enable(bEnable && mMode != mode_Spawn);
    mDirection[mMode].HUD::Element::Enable(bEnable && mMode != mode_Spawn);
    mNorthIcon[mMode].HUD::Element::Enable(bEnable);
    if (bEnable)
    {
        mTargetAll[mMode].CopyToBitmaps(mTarget, 450, 0, false);
        mPostAll[mMode].CopyToBitmaps(mPost, 16, 0, false);
        mPostSelectAll[mMode].CopyToBitmaps(mPostSelect, 16, 0, false);
        mMarkerAll[mMode].CopyToBitmaps(mMarker, 32, 0, false);
        for (unsigned int i = 0; i < 16; ++i)
        {
            RedTextElement* pText = &mPostText[i];
            pText->Enable(false);
            mPostTextAll[mMode].CopyToTextElement(*pText);
        }
    }
}

// FUNCTION: SWBF2 0x005557d0
void HUD::ElementMap::PlayerIndexChanged()
{
    if (mPlayerIndex >= 0)
    {
        mPlayerCharacter = Character::FindPlayer(NetGame::GetLocalIndex(mPlayerIndex));
    }
    else
    {
        mPlayerCharacter = 0;
    }
}

// FUNCTION: SWBF2 0x00555980
void HUD::ElementMap::EventChangeMapMode(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Uint ||
        event.GetClass()->GetType() == EventClass::type_Int)
    {
        ((ElementMap*)pData)->SetMode((Mode)event.GetData().intValue);
    }
}

// FUNCTION: SWBF2 0x00552bf0
void HUD::ElementMap::HideCommandPosts(bool bHide)
{
    mPostHide = bHide;
}

// FUNCTION: SWBF2 0x005559d0
void HUD::ElementMap::EventPostHide(const HUD::Event& event, void* const pData)
{
    ((ElementMap*)pData)->HideCommandPosts(event.GetDataBool());
}

// FUNCTION: SWBF2 0x00555c10
void HUD::ElementMap::EventDisable(const HUD::Event& event, void* const pData)
{
    HUD::Element::EventDisable(event, pData);
    ((ElementMap*)pData)->RefreshMapDependancy();
}

// FUNCTION: SWBF2 0x00552b20
CommandPost* HUD::ElementMap::GetPost(float fX, float fY, CommandPost* pExcludedPost, int iTeam)
{
    for (unsigned int i = 0; i < TargetManager::MAX_POSTS; ++i)
    {
        TargetManager::Post& post = TargetManager::_gPost[i];
        if (!post.actor || !mPost[i].IsEnabled())
        {
            continue;
        }
        GameObject* pObject = post.actor->mObject;
        if (pObject->mTeam != iTeam || !mPost[i].m_pHotSpot)
        {
            continue;
        }
        if (mPost[i].m_pHotSpot->IsPointInside(fX, fY) && post.actor != pExcludedPost)
        {
            return post.actor;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005526b0
bool HUD::ElementMap::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kEventToggleMapModeHash: *(const char**)pValue = mEventToggleMapMode.GetRegisteredClassName(false); return true;
    case kEventChangeMapModeHash: *(const char**)pValue = mEventChangeMapMode.GetRegisteredClassName(false); return true;
    case kEventPostHideHash: *(const char**)pValue = mEventPostHide.GetRegisteredClassName(false); return true;
    case kEventRefreshTargetHash: *(const char**)pValue = mEventRefreshTarget.GetRegisteredClassName(false); return true;

    case kEventRefreshPostHash: *(const char**)pValue = mEventRefreshTarget.GetRegisteredClassName(false); return true;
    case kEventRefreshMarkerHash: *(const char**)pValue = mEventRefreshMarker.GetRegisteredClassName(false); return true;
    case kPositionSmallHash: *(PblVector3*)pValue = mPosition[mode_Small]; return true;
    case kPositionLargeHash: *(PblVector3*)pValue = mPosition[mode_Large]; return true;
    case kPositionSpawnHash: *(PblVector3*)pValue = mPosition[mode_Spawn]; return true;
    case 0x2ebfaf89u: *(unsigned int*)pValue = mMatrix[mode_Small].GetRelativeMode(); return true;
    case 0x5dec0ec1u: *(unsigned int*)pValue = mMatrix[mode_Spawn].GetRelativeMode(); return true;
    case 0x69e17625u: *(unsigned int*)pValue = mMatrix[mode_Large].GetRelativeMode(); return true;
    case kBackgroundMaskShapeSmallHash: *(unsigned int*)pValue = mDim[mode_Small].shape; return true;
    case kBackgroundMaskShapeLargeHash: *(unsigned int*)pValue = mDim[mode_Large].shape; return true;
    case kTargetFireColorHash: *(RedColor*)pValue = mTargetFireColor; return true;
    case kPostFlashRateHash: *(float*)pValue = mPostFlashRate; return true;
    case kPostFlashRadiusHash: *(float*)pValue = mPostFlashRadius; return true;
    case kPostFlashColorHash: *(RedColor*)pValue = mPostFlashColor; return true;
    case kAllowMiniMapHash: *(bool*)pValue = mAllowMiniMap; return true;
    case kPostNumbersHash: *(bool*)pValue = mPostNumbers; return true;
    case 0xdaef9304u: *(float*)pValue = sNorth; return true;
    default: return ElementGroupPlayer::GetProperty(uHashID, pValue);
    }
}

// FUNCTION: SWBF2 0x005522a0
bool HUD::ElementMap::SetProperty(const unsigned int uHashID, const void* pValue)
{
    PblMatrix matrix = _IdentityMatrix;
    matrix.m[3][0] = mPosition[mMode].x;
    matrix.m[3][1] = mPosition[mMode].y;
    matrix.m[3][2] = mPosition[mMode].z;
    matrix.m[3][3] = 1.0f;
    HUD::Element::mMatrix.SetMatrix(&matrix);
    HUD::Element::mMatrix.ApplyScale();
    HUD::Element::mMatrix.SetMatrix(0);
    RedGroupElement::SetMatrix(matrix);

    bool bResult = true;
    switch (uHashID)
    {
    case kEventToggleMapModeHash:
        Item::SetEvent(*(const char* const*)pValue, mEventToggleMapMode);
        break;
    case kEventChangeMapModeHash:
        Item::SetEvent(*(const char* const*)pValue, mEventChangeMapMode);
        break;
    case kEventPostHideHash:
        Item::SetEvent(*(const char* const*)pValue, mEventPostHide);
        break;
    case kEventRefreshTargetHash:
    case kEventRefreshPostHash:
        Item::SetEvent(*(const char* const*)pValue, mEventRefreshTarget);
        break;
    case kEventRefreshMarkerHash:
        Item::SetEvent(*(const char* const*)pValue, mEventRefreshMarker);
        break;
    case 0x2ebfaf89u:
        mMatrix[mode_Small].SetRelativeMode(*(const RelativeMode*)pValue);
        break;
    case 0x69e17625u:
        mMatrix[mode_Large].SetRelativeMode(*(const RelativeMode*)pValue);
        break;
    case 0x5dec0ec1u:
        mMatrix[mode_Spawn].SetRelativeMode(*(const RelativeMode*)pValue);
        break;
    case kPositionSmallHash:
        mPosition[mode_Small] = *(const PblVector3*)pValue;
        break;
    case kPositionLargeHash:
        mPosition[mode_Large] = *(const PblVector3*)pValue;
        break;
    case kPositionSpawnHash:
        mPosition[mode_Spawn] = *(const PblVector3*)pValue;
        break;
    case kBackgroundMaskShapeSmallHash:
        mDim[mode_Small].shape = *(const MaskShape*)pValue;
        break;
    case kBackgroundMaskShapeLargeHash:
        mDim[mode_Large].shape = *(const MaskShape*)pValue;
        break;
    case kTargetFireColorHash:
    {
        const RedColor* pColor = (const RedColor*)pValue;
        RedColor color;
        color.a = 0xff;
        color.b = pColor->b;
        color.g = pColor->g;
        color.r = pColor->r;
        mTargetFireColor = color;
        break;
    }
    case kPostFlashRateHash:
        mPostFlashRate = *(const float*)pValue;
        break;
    case kPostFlashRadiusHash:
        mPostFlashRadius = *(const float*)pValue;
        break;
    case kPostFlashColorHash:
    {
        const RedColor* pColor = (const RedColor*)pValue;
        RedColor color;
        color.a = 0xff;
        color.b = pColor->b;
        color.g = pColor->g;
        color.r = pColor->r;
        mPostFlashColor = color;
        break;
    }
    case kAllowMiniMapHash:
        mAllowMiniMap = *(const bool*)pValue;
        mAllowToggleOff = !mAllowMiniMap;
        break;
    case kPostNumbersHash:
        mPostNumbers = *(const bool*)pValue;
        break;
    case 0xdaef9304u:
        sNorth = *(const float*)pValue;
        break;
    case kPositionHash:
        bResult = ElementGroupPlayer::SetProperty(kPositionHash, pValue);
        if (bResult)
        {
            mPosition[mMode] = *(const PblVector3*)pValue;
        }
        break;
    default:
        bResult = ElementGroupPlayer::SetProperty(uHashID, pValue);
        break;
    }

    if (bResult)
    {
        mGroupMatrix = RedGroupElement::m_mLocal;
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00555290
void HUD::ElementMap::WriteData(HUD::File& file, unsigned int uTabs)
{
    for (unsigned int i = 0; i < mode_MAX; ++i)
    {
        PblMatrix matrix(PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f }, PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
                         PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
                         PblVector4{ mPosition[i].x, mPosition[i].y, mPosition[i].z, 1.0f });
        mMatrix[i].SetMatrix(&matrix);
        char buffer[256];
        _snprintf(buffer, sizeof(buffer), "Position%s", sModeStrings[i]);
        mMatrix[i].WritePosition(file, uTabs, GetContainerFrameWidth(), GetContainerFrameHeight(),
                                 GetContainerViewWidth(), GetContainerViewHeight(), buffer, InScreenGroup());
        mMatrix[i].SetMatrix(0);
    }

    file.WriteProperty("EventToggleMapMode", uTabs, mEventToggleMapMode.GetRegisteredClassName(true), "");
    file.WriteProperty("EventChangeMapMode", uTabs, mEventChangeMapMode.GetRegisteredClassName(true), "");
    file.WriteProperty("EventPostHide", uTabs, mEventPostHide.GetRegisteredClassName(true), "");
    file.WriteProperty("EventRefreshTarget", uTabs, mEventRefreshTarget.GetRegisteredClassName(true), "");
    file.WriteProperty("EventRefreshPost", uTabs, mEventRefreshPost.GetRegisteredClassName(true), "");
    file.WriteProperty("EventRefreshMarker", uTabs, mEventRefreshMarker.GetRegisteredClassName(true), "");
    file.WriteProperty("BackgroundMaskShapeSmall", uTabs, sMaskShapeStrings[mDim[mode_Small].shape],
                       sMaskShapeStrings[0]);
    file.WriteProperty("BackgroundMaskShapeLarge", uTabs, sMaskShapeStrings[mDim[mode_Large].shape],
                       sMaskShapeStrings[0]);
    file.WriteProperty("TargetFireColor", uTabs, mTargetFireColor.r, mTargetFireColor.g, mTargetFireColor.b,
                       RedColor::WHITE.r, RedColor::WHITE.g, RedColor::WHITE.b);
    file.WriteProperty("PostFlashRate", uTabs, mPostFlashRate, 10.0f);
    file.WriteProperty("PostFlashRadius", uTabs, mPostFlashRadius, 3.0f);
    file.WriteProperty("PostFlashColor", uTabs, mPostFlashColor.r, mPostFlashColor.g, mPostFlashColor.b, 1, 86,
                       213);
    file.WriteProperty("AllowMiniMap", uTabs, (int)mAllowMiniMap, 1);
    file.WriteProperty("PostNumbers", uTabs, (int)mPostNumbers, 1);
    ElementGroupPlayer::WriteData(file, uTabs);
    file.WriteString("\n");

    for (unsigned int i = 0; i < mode_MAX; ++i)
    {
        HUD::ElementMap::WriteBitmapElement(&mBackdrop[i], &file, uTabs);
        mPlayerFOV[i].Write(file, uTabs, mPlayerFOV[i].GetName());
        mDirection[i].Write(file, uTabs, mDirection[i].GetName());
        HUD::ElementMap::WriteBitmapElement(&mTargetAll[i], &file, uTabs);
        HUD::ElementMap::WriteBitmapElement(&mPostAll[i], &file, uTabs);
        mPostTextAll[i].Write(file, uTabs, mPostTextAll[i].GetName());
        HUD::ElementMap::WriteBitmapElement(&mPostSelectAll[i], &file, uTabs);
        HUD::ElementMap::WriteBitmapElement(&mMarkerAll[i], &file, uTabs);
        HUD::ElementMap::WriteBitmapElement(&mNorthIcon[i], &file, uTabs);
    }
}

// FUNCTION: SWBF2 0x00552950
const char* HUD::ElementMap::PropertyTranslateEnum(const unsigned int uHashID,
                                                    const unsigned int uValue)
{
    switch (uHashID)
    {
    case 0x2ebfaf89u:
    case 0x5dec0ec1u:
    case 0x69e17625u: return gRelativeModeStrings[uValue];
    case kBackgroundMaskShapeSmallHash:
    case kBackgroundMaskShapeLargeHash:
        return sMaskShapeStrings[uValue];
    default: return ElementGroupBase::PropertyTranslateEnum(uHashID, uValue);
    }
}

namespace FriendBuddyList {

// FUNCTION: SWBF2 0x0052e140
unsigned int GetFriendBuddyCount()
{
    return gf::friends::friendCount();
}
}

namespace FriendBuddyList {

// FUNCTION: SWBF2 0x0052e150
int __fastcall GetFriendBuddy(int, unsigned int index)
{
    galaxy::api::GalaxyID id = gf::friends::friendByIndex(index);
    if (id.value)
    {
        return NetGame::FindPlayerByGSID((int)id.value, (int)(id.value >> 32));
    }
    return 0;
}
}

namespace FriendBuddyList {

// FUNCTION: SWBF2 0x0052e190
bool __fastcall IsFriendBuddy(int, int playerId)
{
    const unsigned int count = FriendBuddyList::GetFriendBuddyCount();
    for (unsigned int i = 0; i < count; ++i)
    {
        if (FriendBuddyList::GetFriendBuddy(0, i) == playerId)
        {
            return true;
        }
    }
    return false;
}
}

// FUNCTION: SWBF2 0x005558a0
void HUD::ElementMap::EventToggleMapMode(const HUD::Event& event, void* const pData)
{
    if (!event.GetDataBool())
    {
        return;
    }
    ElementMap* pMap = (ElementMap*)pData;
    if (pMap->mMode == mode_Spawn)
    {
        return;
    }

    const bool wasEnabled = pMap->IsEnabled();
    if (wasEnabled)
    {
        Mode nextMode = (Mode)(pMap->mMode + 1);
        if (nextMode == mode_Spawn)
        {
            if (pMap->mAllowToggleOff)
            {
                pMap->SetMode(mode_Small);
                pMap->UpdateEnableState(enableState_Disabling);
                pMap->RefreshMapDependancy();
            }
            else
            {
                pMap->SetMode(mode_Small);
            }
        }
        else
        {
            pMap->SetMode(nextMode);
        }
    }
    else
    {
        pMap->SetMode(pMap->mAllowMiniMap ? mode_Small : mode_Large);
    }

    pMap->Update(0.001f);
    if (pMap->IsEnabled() != wasEnabled)
    {
        HUD::Event(pMap->IsEnabled()
            ? GameEvents::gPlayerEvents[pMap->mPlayerIndex].mapEnable
            : GameEvents::gPlayerEvents[pMap->mPlayerIndex].mapDisable, true).Send();
    }
}

// FUNCTION: SWBF2 0x00555740
void __fastcall HUD::ElementMap::WriteBitmapElement(HUD::ElementBitmapBase* pBitmap, HUD::File* pFile,
                           unsigned int uTabs)
{
    RedBitmapElement* pElement = (RedBitmapElement*)pBitmap->mElement;
    const RedBitmapElement::BitmapHAlignT hAlign =
        (RedBitmapElement::BitmapHAlignT)pBitmap->mHAlign;
    const RedBitmapElement::BitmapVAlignT vAlign =
        (RedBitmapElement::BitmapVAlignT)pBitmap->mVAlign;
    float left, top, right, bottom;
    pElement->GetRect(left, top, right, bottom);
    pBitmap->SetRect(0.0f, 0.0f, (RedBitmapElement::BitmapHAlignT)0,
                     (RedBitmapElement::BitmapVAlignT)0);
    pBitmap->Write(*pFile, uTabs, pBitmap->GetName());
    pBitmap->SetRect(right - left, bottom - top, hAlign, vAlign);
}

// FUNCTION: SWBF2 0x00555b70
void HUD::ElementMap::EventRefreshMarker(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != HUD::EventClass::type_Uint)
    {
        return;
    }
    const unsigned int index = event.mData.uintValue;
    if (index >= TargetManager::MAX_MARKERS)
    {
        return;
    }

    ElementMap* pMap = (ElementMap*)pData;
    const TargetManager::Marker& marker = TargetManager::gMarkers[index];
    RedBitmapElement& bitmap = pMap->mMarker[index];
    bitmap.SetTexture(marker.textureHash);
    RedColor empty(0, 0, 0, 0);
    bitmap.SetColor(empty);
    bitmap.SetMatrixPosScale(0.0f, 0.0f, marker.scale, marker.scale);
    bitmap.Enable(false);
}

// FUNCTION: SWBF2 0x005559f0
void HUD::ElementMap::EventRefreshTarget(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != EventClass::type_Uint)
    {
        return;
    }
    const unsigned int uIndex = event.GetData().uintValue;
    if (uIndex >= TargetManager::MAX_TARGETS)
    {
        return;
    }

    ElementMap* pMap = (ElementMap*)pData;
    RedBitmapElement& target = pMap->mTarget[uIndex];
    GameObject* pObject = TargetManager::_gTarget[uIndex].actor;
    if (pObject)
    {
        target.SetTexture(pObject->GetEntityClass()->mMapParams.mTexture);
    }
    RedColor color = { 0, 0, 0, 0 };
    target.SetColor(color);
    target.Enable(false);
}

// FUNCTION: SWBF2 0x00555aa0
void HUD::ElementMap::EventRefreshPost(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != EventClass::type_Uint)
    {
        return;
    }
    const unsigned int uIndex = event.GetData().uintValue;
    if (uIndex >= TargetManager::MAX_POSTS)
    {
        return;
    }

    ElementMap* pMap = (ElementMap*)pData;
    RedBitmapElement& post = pMap->mPost[uIndex];
    CommandPost* pPost = TargetManager::_gPost[uIndex].actor;
    if (pPost)
    {
        post.SetTexture(pPost->mObject->GetEntityClass()->mMapParams.mTexture);
    }
    RedColor color = { 0, 0, 0, 0 };
    post.SetColor(color);
    post.Enable(false);
    pMap->mPostText[uIndex].Enable(false);
}

// GLOBAL: SWBF2 0x007eb9fc
// SYNTHETIC: SWBF2 0x00403fc0
// `dynamic initializer for 'HUD::ElementMap::sMapList''
// SYNTHETIC: SWBF2 0x00768b40
// `dynamic atexit destructor for 'HUD::ElementMap::sMapList''
PblListSingle HUD::ElementMap::sMapList;

// GLOBAL: SWBF2 0x01e56c39
bool gMapDisplayAll = false;
