#pragma once

#include <stddef.h>

#include "HUDElementBitmap.h"
#include "HUDElementGroupPlayer.h"
#include "HUDElementModel3D.h"
#include "HUDElementText.h"
#include "TargetManager.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Interface/RedGroupElement.h"

class Character;
class CommandPost;
class Controllable;
class Team;

namespace HUD {
class ElementMap : public HUD::ElementGroupPlayer, public RedGroupElement
{
public:

    static PblListSingle sMapList;

    enum Mode
    {
        mode_Small = 0,
        mode_Large = 1,
        mode_Spawn = 2,
        mode_MAX   = 3
    };

    static ElementMap* FindByPlayerIndex(int iPlayerIndex);

    static void SetNorth(float fNorth, bool bFlipNorth);
    static ElementMap* FindByIndex(int iIndex);
    CommandPost* GetPost(float fX, float fY, CommandPost* pExcludedPost, int iTeam);

    enum MaskShape
    {
        maskShape_Circle    = 0,
        maskShape_Rectangle = 1,
        maskShape_MAX       = 2
    };

    struct Dim
    {
        float centerX;
        float centerZ;
        float radius;
        float scale;
        MaskShape shape;
    };

    Mode GetMode() const { return mMode; }

    // VTABLE: SWBF2 0x007a121c
    class BitmapGroup : public HUD::ElementBitmap
    {
    public:

        BitmapGroup(const char* const pName = 0)
            : HUD::ElementBitmap(pName), mMap(0), mMapMode(0), mBitmaps(0), mNumBitmaps(0)
        {
        }
        virtual ~BitmapGroup();

        void Init(HUD::ElementMap* pMap, unsigned int uMapMode, RedBitmapElement* pBitmaps,
                  unsigned int uNumBitmaps)
        {
            mMap = pMap;
            mMapMode = uMapMode;
            mBitmaps = pBitmaps;
            mNumBitmaps = uNumBitmaps;
        }

        virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);

        HUD::ElementMap* mMap;
        unsigned int mMapMode;
        RedBitmapElement* mBitmaps;
        unsigned int mNumBitmaps;
    };

    // VTABLE: SWBF2 0x007a1170

    class Factory : public HUD::ElementGroupPlayer::Factory
    {
    public:
        // FUNCTION: SWBF2 0x0054fab0
        Factory();

        // SYNTHETIC: SWBF2 0x0054fe10
        // HUD::ElementMap::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mEventToggleMapMode;
        Property mEventChangeMapMode;
        Property mEventPostHide;
        Property mEventRefreshTarget;
        Property mEventRefreshPost;
        Property mEventRefreshMarker;
        Property mBackgroundMaskShape[3];
        Property mTargetFireColor;
        Property mPostFlashRate;
        Property mPostFlashRadius;
        Property mPostFlashColor;
        Property mPosition[3];
        Property mRelative[3];
        Property mNorth;
        Property mAllowMinimap;
        Property mPostNumbers;
    };

    static char* sModeStrings[3];
    static char* sMaskShapeStrings[2];
    static const float sSymbolScale[2];
    static float sNorth;
    static bool sFlipNorth;

    static void EventToggleMapMode(const HUD::Event& event, void* const pData);
    static void EventChangeMapMode(const HUD::Event& event, void* const pData);
    static void EventPostHide(const HUD::Event& event, void* const pData);
    static void EventRefreshTarget(const HUD::Event& event, void* const pData);
    static void EventRefreshPost(const HUD::Event& event, void* const pData);
    static void EventRefreshMarker(const HUD::Event& event, void* const pData);
    static void EventDisable(const HUD::Event& event, void* const pData);

    void HideCommandPosts(bool bHide);

    ElementMap(const char* const pName, Factory::ItemCallback pCallback,
               void* const pUserData);

    virtual ~ElementMap();
    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback,
                      void* const pUserData);
    virtual bool SetProperty(const unsigned int, const void*);
    virtual bool GetProperty(const unsigned int, void*);
    virtual const char* PropertyTranslateEnum(const unsigned int, const unsigned int);
    virtual bool ReadData(PblConfig&, PblConfig::Data&);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File&, unsigned int);
    virtual void Update(const float);

    bool CalculateScreenCoordinates(const PblVector3& vPosition, float& fX, float& fY, bool bClamp,
                                    float* const pfDistanceSq) const;

protected:
    virtual void PlayerIndexChanged();

    static void __fastcall WriteBitmapElement(HUD::ElementBitmapBase* pBitmap, HUD::File* pFile,
                                              unsigned int uTabs);

    void EnableModeElements(bool bEnable);
    void UpdateNorthIcon(float fElapsed);
    const Team* GetPlayerTeam() const;
    void UpdatePlayerIcon();
    void UpdateTargetIcons(const Controllable* pPlayer);
    void UpdatePostIcons(float fElapsed, float fRotation);
    void UpdateMarkerIcons();
    void Init();
    void RefreshMapDependancy();
    void SetMode(Mode eMode);

    void UpdateBackdropTexCoords();

    float CalcFireIndication(TargetManager::Target& target, float fDistance) const;
    bool IsInRadarRange(const Controllable* pPlayer, TargetManager::Target& target,
                        const PblVector3& vDelta, float fDistance) const;

public:

    HUD::ElementModel3D mPlayerFOV[3];
    HUD::ElementModel3D mDirection[3];
    HUD::ElementBitmapMasked mBackdrop[3];
    HUD::ElementBitmap mNorthIcon[3];
    BitmapGroup mTargetAll[3];
    RedBitmapElement mTarget[450];
    BitmapGroup mPostAll[3];
    RedBitmapElement mPost[16];
    float mPostScale[16];
    HUD::ElementText mPostTextAll[3];
    RedTextElement mPostText[16];
    BitmapGroup mPostSelectAll[3];
    RedBitmapElement mPostSelect[16];
    BitmapGroup mMarkerAll[3];
    RedBitmapElement mMarker[32];
    EventHandler mEventToggleMapMode;
    EventHandler mEventChangeMapMode;
    EventHandler mEventPostHide;
    EventHandler mEventRefreshTarget;
    EventHandler mEventRefreshPost;
    EventHandler mEventRefreshMarker;
    PblListSingle::Node mMapNode;
    PblVector3 mPosition[3];
    HUD::Matrix mMatrix[3];
    Dim mDim[3];
    Mode mMode;
    PblMatrix mGroupMatrix;
    Character* mPlayerCharacter;
    PblVector3 mPlayerPosition;
    PblVector3 mPlayerDirection;
    PblVector3 mPlayerMovementDirection;
    RedColor mTargetFireColor;
    float mPostFlashRate;
    float mPostFlashRadius;
    RedColor mPostFlashColor;
    float mNextSlowUpdate;
    bool mInitialized : 1;
    bool mPostHide : 1;
    bool mAllowToggleOff : 1;
    bool mAllowMiniMap : 1;
    bool mPostNumbers : 1;
};
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
