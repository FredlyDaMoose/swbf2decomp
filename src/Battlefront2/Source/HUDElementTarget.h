#pragma once

#include <stddef.h>

#include "HUDElementBitmap.h"
#include "HUDElementGroupPlayer.h"
#include "LockOnManager.h"
#include "RedEngineFL/Interface/RedGroupElement.h"

namespace HUD {
// VTABLE: SWBF2 0x007a17fc
class ElementTarget : public HUD::ElementGroupPlayer, public RedGroupElement
{
public:

    // VTABLE: SWBF2 0x007a1840

    class Factory : public HUD::ElementGroupPlayer::Factory
    {
    public:
        // FUNCTION: SWBF2 0x00559db0
        Factory();

        // SYNTHETIC: SWBF2 0x00559e70
        // HUD::ElementTarget::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);

        Property mEventResetTargetCommon;
        Property mEventResetTargetPlayer;
        Property mFourSegmentLockOn;
    };

    enum TargetCommonState
    {
        targetCommonState_OnScreen = 0,
        targetCommonState_OnScreenBehind = 1,
        targetCommonState_OffScreen = 2,
        targetCommonState_OffScreenBehind = 3,
        targetCommonState_FlagOnScreen = 4,
        targetCommonState_FlagOnScreenBehind = 5,
        targetCommonState_FlagOffScreen = 6,
        targetCommonState_FlagOffScreenBehind = 7,
        targetCommonState_MAX = 8
    };
    enum TargetPlayerState
    {
        targetPlayerState_AttackerOnScreen = 0,
        targetPlayerState_AttackerOnScreenBehind = 1,
        targetPlayerState_AttackerOffScreen = 2,
        targetPlayerState_AttackerOffScreenBehind = 3,
        targetPlayerState_LockedOnScreen = 4,
        targetPlayerState_LockedOnScreenBehind = 5,
        targetPlayerState_LockedOffScreen = 6,
        targetPlayerState_LockedOffScreenBehind = 7,
        targetPlayerState_HintOnScreen = 8,
        targetPlayerState_HintOnScreenBehind = 9,
        targetPlayerState_HintOffScreen = 10,
        targetPlayerState_HintOffScreenBehind = 11,
        targetPlayerState_HintFlagOnScreen = 12,
        targetPlayerState_HintFlagOnScreenBehind = 13,
        targetPlayerState_HintFlagOffScreen = 14,
        targetPlayerState_HintFlagOffScreenBehind = 15,
        targetPlayerState_MAX = 16
    };

    // VTABLE: SWBF2 0x007a184c
    class Target : public HUD::ElementBitmap
    {
    public:

        // VTABLE: SWBF2 0x007a18e4

        class Factory : public HUD::ElementBitmapBase::Factory
        {
        public:
            // FUNCTION: SWBF2 0x00558c90
            Factory();

            // SYNTHETIC: SWBF2 0x00558d90
            // HUD::ElementTarget::Target::Factory::`scalar deleting destructor'

            virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                          void* const pUserData);

            Property mMinScale;
            Property mMaxScale;
            Property mColorInterpolationRate;
        };

        Target(const char* const pName = 0, Factory::ItemCallback pCallback = 0,
               void* const pUserData = 0);
        virtual ~Target();

        virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
        virtual bool GetProperty(const unsigned int uHashID, void* const pValue);

        void Update(float fDeltaTime, HUD::ElementBitmap& bitmap,
                    const LockOnManager* const pManager, const LockOnManager::Target& target,
                    const LockOnManager::TargetInfo& info, float fWidth, float fHeight,
                    bool bStateChanged, HUD::ElementBitmap* const pLockOn);

    protected:
        virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
        virtual void WriteData(HUD::File& file, unsigned int uTabs);

    public:

        float mMinScale;
        float mMaxScale;
        float mColorInterpolationRate;
        RedColor mColorFriendly;
    };

    ElementTarget(const char* const pName, Factory::ItemCallback pCallback,
                  void* const pUserData);
    virtual ~ElementTarget();

    virtual void* GetParent();

    virtual void Read(PblConfig& config, Factory::ItemCallback pCallback,
                      void* const pUserData);
    virtual bool SetProperty(const unsigned int uHashID, const void* const pValue);
    virtual bool GetProperty(const unsigned int uHashID, void* const pValue);
    virtual void Update(const float fDeltaTime);

protected:
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void WriteData(HUD::File& file, unsigned int uTabs);

    static void EventRefreshTargetCommon(const HUD::Event& event, void* const pData);
    static void EventRefreshTargetPlayer(const HUD::Event& event, void* const pData);

    static TargetCommonState TranslateCommonTargetInfo(const LockOnManager::TargetInfo& info);
    static TargetPlayerState TranslatePlayerTargetInfo(const LockOnManager::TargetInfo& info);

public:

    static char* sTargetCommonStateStrings[targetCommonState_MAX];
    static char* sTargetPlayerStateStrings[targetPlayerState_MAX];
    static float sLockOnRotation[4];

    Target mTargetCommonAll[targetCommonState_MAX];
    HUD::ElementBitmap mTargetCommon[32];
    TargetCommonState mTargetCommonState[32];
    Target mTargetPlayerAll[targetPlayerState_MAX];
    HUD::ElementBitmap mTargetPlayer[64];
    TargetPlayerState mTargetPlayerState[64];
    HUD::EventHandler mEventResetTargetCommon;
    HUD::EventHandler mEventResetTargetPlayer;
    HUD::ElementBitmap mTargetLockOn[4];
    bool mFourSegmentLockOn : 1;
};
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
