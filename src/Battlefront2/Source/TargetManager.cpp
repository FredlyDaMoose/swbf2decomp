#include <string.h>

#include "TargetManager.h"
#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"
#include "CommandPost.h"
#include "HUDGameEvents.h"
#include "LockOnManager.h"
#include "NetGame.h"
#include "Trackable.h"

// FUNCTION: SWBF2 0x0057acb0
void LockOnManager::Target::InitCommon(GameObject* const pObject, unsigned int uTeamMask)
{
    obj.mObject = pObject;
    obj.mSavedHandleId = pObject ? pObject->mHandleId : 0;
    teamMask = uTeamMask;
    maxDistTrack = 10000.0f;
    maxDist = 10000.0f;
    minDist = 100.0f;
    timer = 0.0f;
}

// FUNCTION: SWBF2 0x0057da40
void LockOnManager::AddTargetCommon(GameObject* pActor, unsigned int uTeamMask)
{
    unsigned int i = 0;
    for (; i < 32; ++i)
    {
        LockOnManager::Target& target = LockOnManager::sTargetCommon[i];
        if (!target.obj.mObject)
        {
            break;
        }
        if (target.obj.mObject->mHandleId != target.obj.mSavedHandleId)
        {
            target.obj.mObject = 0;
            target.obj.mSavedHandleId = 0;
            break;
        }
        if (target.obj.mObject == pActor || !target.teamMask)
        {
            break;
        }
    }
    if (i == 32)
    {
        return;
    }
    LockOnManager::Target& target = LockOnManager::sTargetCommon[i];
    target.InitCommon(pActor, uTeamMask | target.teamMask);
    for (unsigned int j = 0; j < 4; ++j)
    {
        LockOnManager* pManager = LockOnManager::sLockOnManagers[j];
        if (pManager)
        {
            LockOnManager::ClearPositionValid(pManager->mTargetCommonInfo, 32);
            LockOnManager::ClearVisibilityChecked(pManager->mTargetCommonInfo, 32);
        }
    }
    HUD::Event event;
    event.mClass = HUD::GameEvents::gTargetResetCommon;
    event.mData.uintValue = i;
    event.Send();
}

namespace
{

inline GameObject* HandleObject(PblHandle<GameObject>& handle)
{
    if (handle.mObject &&
        handle.mObject->mHandleId != handle.mSavedHandleId)
    {
        handle.mObject = 0;
        handle.mSavedHandleId = 0;
    }
    return handle.mObject;
}

inline unsigned int HandleId(const GameObject* pObject)
{
    return pObject->mHandleId;
}

inline EntityClass* CharacterClass(const Character* pCharacter)
{
    return pCharacter->mClassPtr;
}

inline GameObject* UnitObject(Controllable* pUnit)
{
    return static_cast<Trackable*>(pUnit)->GetGameObject();
}

inline void SendRefresh(HUD::EventClass* pClass, unsigned int uIndex)
{
    HUD::Event event;
    event.mClass = pClass;
    event.mData.uintValue = uIndex;
    event.Send();
}

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t)
{
    RedColor result;
    result.b = (unsigned char)(int)(a.b * (1.0f - t) + b.b * t + 0.5f);
    result.g = (unsigned char)(int)(a.g * (1.0f - t) + b.g * t + 0.5f);
    result.r = (unsigned char)(int)(a.r * (1.0f - t) + b.r * t + 0.5f);
    result.a = (unsigned char)(int)(a.a * (1.0f - t) + b.a * t + 0.5f);
    return result;
}

}

namespace TargetManager
{

// GLOBAL: SWBF2 0x01eafba0
ClassMarker _gClassMarker[MAX_CLASS_MARKERS];

// GLOBAL: SWBF2 0x01eafb9c
Marker* _gMarkers;

// GLOBAL: SWBF2 0x01f6de9c
// SYNTHETIC: SWBF2 0x00406600
// `dynamic initializer for 'TargetManager::gMarkers''
Marker* gMarkers = _gMarkers;

// GLOBAL: SWBF2 0x01f76410
// SYNTHETIC: SWBF2 0x00406520
// `dynamic initializer for 'TargetManager::_gPost''
Post _gPost[MAX_POSTS];

// SYNTHETIC: SWBF2 0x00652170
// TargetManager::Target::Target
// SYNTHETIC: SWBF2 0x006521e0
// TargetManager::Target::~Target

// GLOBAL: SWBF2 0x01f76710
// SYNTHETIC: SWBF2 0x004064f0
// `dynamic initializer for 'TargetManager::_gTarget''
// SYNTHETIC: SWBF2 0x00769500
// `dynamic atexit destructor for 'TargetManager::_gTarget''
Target _gTarget[MAX_TARGETS];

// GLOBAL: SWBF2 0x007ec320
// SYNTHETIC: SWBF2 0x00406610
// `dynamic initializer for 'gColorTable''
Color gColorTable[MAX_COLORS] =
{
    { 0x00000000, { 0, 0, 0, 0 } },
    { 0xde020766, RedColor::WHITE },
    { 0x7ea5f49e, RedColor::LTGREY },
    { 0xe41b46af, RedColor::MDGREY },
    { 0x3657edc3, RedColor::DKGREY },
    { 0x568f4ba4, RedColor::BLACK },
    { 0x40f480dc, RedColor::RED },
    { 0x011decbc, RedColor::GREEN },
    { 0x82fbf5cd, RedColor::BLUE },
    { 0x4961533a, RedColor::CYAN },
    { 0x63e629e8, RedColor::MAGENTA },
    { 0x05bf6449, RedColor::YELLOW },
    { 0x45b473eb, Interpolate(RedColor::YELLOW, RedColor::RED, 0.5f) },
};

// FUNCTION: SWBF2 0x006521f0
void Marker::Clear()
{
    memset(this, 0, sizeof(*this));
}

// FUNCTION: SWBF2 0x00652210
void ClassMarker::Clear()
{
    if (classID)
    {
        for (PblHashTable<EntityEx, 1024>::Itor it = EntityEx::mIdMap.Begin();
             (EntityEx*)it != (EntityEx*)EntityEx::mIdMap.End(); ++it)
        {
            if (((EntityEx*)it)->GetEntityClass()->mId == classID)
            {
                RemoveMarker(((EntityEx*)it)->GetGameObject(), 0, teamMask);
            }
        }
        for (int i = 0; i < Character::sCharacterLimit; ++i)
        {
            Character* pCharacter = Character::GetPtr(i);
            Controllable* pUnit = pCharacter->mUnit;
            if (pUnit && CharacterClass(pCharacter)->mId == classID)
            {
                RemoveMarker(UnitObject(pUnit), 0, teamMask);
            }
        }
    }
    memset(this, 0, sizeof(*this));
}

// FUNCTION: SWBF2 0x00652340
const RedColor& ColorByHash(unsigned int hashID)
{
    for (unsigned int i = 0; i < MAX_COLORS; ++i)
    {
        if (gColorTable[i].hashID == hashID)
        {
            return gColorTable[i].color;
        }
    }
    return gColorTable[0].color;
}

// FUNCTION: SWBF2 0x00652360
unsigned int HashByColor(RedColor& color)
{
    for (int i = 0; i < MAX_COLORS; ++i)
    {
        if (memcmp(&color, &gColorTable[i].color, sizeof(color)) == 0)
        {
            return gColorTable[i].hashID;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006523d0
bool UpdateInterface()
{
    if (!NetGame::IsEnabled() || NetGame::IsOneWorld())
    {
        return true;
    }
    return (NetGame::IsHost() && NetGame::IsDedicated()) != NetGame::IsClient();
}

extern Marker _gHostMarkers[MAX_MARKERS];
extern Marker _gClientMarkers[MAX_MARKERS];

// FUNCTION: SWBF2 0x00652d60
void CreateClient()
{
    memset(_gClientMarkers, 0, sizeof(_gClientMarkers));
}

// FUNCTION: SWBF2 0x00652d80
void DestroyClient() {}

// FUNCTION: SWBF2 0x00652d90
void EnterClient()
{
    _gMarkers = _gClientMarkers;
    gMarkers = _gClientMarkers;
}

// FUNCTION: SWBF2 0x00652db0
void LeaveClient()
{
    _gMarkers = _gHostMarkers;
    gMarkers = _gHostMarkers;
}

// FUNCTION: SWBF2 0x00652440
void Init()
{
    memset(_gTarget, 0, sizeof(_gTarget));
    memset(_gPost, 0, sizeof(_gPost));
    memset(_gClassMarker, 0, sizeof(ClassMarker));
    memset(_gHostMarkers, 0, sizeof(_gHostMarkers));
    memset(_gClientMarkers, 0, sizeof(_gClientMarkers));
    _gMarkers = _gHostMarkers;
    gMarkers = _gHostMarkers;
}

// FUNCTION: SWBF2 0x006524b0
void AddTarget(GameObject* const actor, bool alwaysVisibleToFriends)
{
    if (!actor)
    {
        return;
    }
    if (!UpdateInterface())
    {
        return;
    }
    EntityClass* pClass = ((Entity*)actor)->GetEntityClass();
    if (!pClass)
    {
        return;
    }

    CheckObjectClassMarker(actor);

    unsigned int uIndex = MAX_TARGETS;
    for (unsigned int i = 0; i < MAX_TARGETS; ++i)
    {
        if (HandleObject(_gTarget[i].actor) == actor)
        {
            uIndex = i;
            break;
        }
        if (uIndex == MAX_TARGETS && !HandleObject(_gTarget[i].actor))
        {
            uIndex = i;
        }
    }

    if (uIndex != MAX_TARGETS)
    {
        Target& target = _gTarget[uIndex];
        target.actor.mObject = actor;
        target.actor.mSavedHandleId = HandleId(actor);
        target.alwaysVisibleToFriends = alwaysVisibleToFriends;
        target.mapParams = pClass->mMapParams;
        SendRefresh(HUD::GameEvents::gPlayerEvents[0].mapRefreshTarget, uIndex);
    }
}

// FUNCTION: SWBF2 0x00652600
void RemoveTarget(GameObject* const actor)
{
    if (!actor || !UpdateInterface() || !((Entity*)actor)->GetEntityClass())
    {
        return;
    }

    unsigned int i;
    for (i = 0; i < MAX_TARGETS; ++i)
    {
        if (HandleObject(_gTarget[i].actor) == actor)
        {
            _gTarget[i].actor.mObject = 0;
            _gTarget[i].actor.mSavedHandleId = 0;
            _gTarget[i].alwaysVisibleToFriends = false;
            break;
        }
    }

    if (i != MAX_TARGETS)
    {
        SendRefresh(HUD::GameEvents::gPlayerEvents[0].mapRefreshTarget, i);
    }
}

// FUNCTION: SWBF2 0x006526a0
void AddClassMarker(unsigned int classID, unsigned int textureHash, float scale,
                    unsigned int teamMask, unsigned int colorHash, bool showHudTarget,
                    bool flashAlpha, bool pulseSize, float pulseStartScale)
{
    ClassMarker* pMarker = 0;
    for (int i = 0; i < MAX_CLASS_MARKERS; ++i)
    {
        if (_gClassMarker[i].classID == classID && _gClassMarker[i].teamMask == teamMask)
        {
            return;
        }
        if (!_gClassMarker[i].classID)
        {
            pMarker = &_gClassMarker[i];
            break;
        }
    }
    if (!pMarker)
    {
        return;
    }

    pMarker->colorHash = colorHash;
    pMarker->showHudTarget = showHudTarget;
    pMarker->scale = scale;
    pMarker->classID = classID;
    pMarker->textureHash = textureHash;
    pMarker->teamMask = teamMask;
    pMarker->flashAlpha = flashAlpha;
    pMarker->pulseSize = pulseSize;
    pMarker->pulseStartScale = pulseStartScale;

    for (PblHashTable<EntityEx, 1024>::Itor it = EntityEx::mIdMap.Begin();
         (EntityEx*)it != (EntityEx*)EntityEx::mIdMap.End(); ++it)
    {
        if (((EntityEx*)it)->GetEntityClass()->mId == classID)
        {
            AddMarker(((EntityEx*)it)->GetGameObject(), 0, textureHash, scale, teamMask,
                      colorHash, showHudTarget, flashAlpha, pulseSize, pulseStartScale);
        }
    }
    for (int i = 0; i < Character::sCharacterLimit; ++i)
    {
        Character* pCharacter = Character::GetPtr(i);
        Controllable* pUnit = pCharacter->mUnit;
        if (pUnit && CharacterClass(pCharacter)->mId == classID)
        {
            AddMarker(UnitObject(pUnit), 0, textureHash, scale, teamMask, colorHash,
                      showHudTarget, flashAlpha, pulseSize, pulseStartScale);
        }
    }
}

// FUNCTION: SWBF2 0x00652890
void RemoveClassMarker(unsigned int classID, unsigned int teamMask)
{
    for (int i = 0; i < MAX_CLASS_MARKERS; ++i)
    {
        if (_gClassMarker[i].classID == classID && (_gClassMarker[i].teamMask & teamMask))
        {
            _gClassMarker[i].Clear();
            return;
        }
    }
}

// FUNCTION: SWBF2 0x006528d0
void AddPost(CommandPost* const post)
{
    if (!post || !UpdateInterface())
    {
        return;
    }
    GameObject* pObject = HandleObject(post->mObject);
    if (!pObject)
    {
        return;
    }
    const EntityClass* pClass = ((const Entity*)pObject)->GetEntityClass();
    if (!pClass)
    {
        return;
    }

    unsigned int uIndex = MAX_POSTS;
    for (unsigned int i = 0; i < MAX_POSTS; ++i)
    {
        if (_gPost[i].actor == post)
        {
            uIndex = i;
            break;
        }
        if (uIndex == MAX_POSTS && !_gPost[i].actor)
        {
            uIndex = i;
        }
    }

    if (uIndex != MAX_POSTS)
    {
        _gPost[uIndex].actor = post;
        _gPost[uIndex].mapParams = pClass->mMapParams;
        SendRefresh(HUD::GameEvents::gPlayerEvents[0].mapRefreshPost, uIndex);
    }
}

// FUNCTION: SWBF2 0x006529b0
void RemovePost(CommandPost* const post)
{
    if (!post || !UpdateInterface())
    {
        return;
    }
    GameObject* pObject = HandleObject(post->mObject);
    if (!pObject)
    {
        return;
    }
    if (!((const Entity*)pObject)->GetEntityClass())
    {
        return;
    }

    unsigned int i;
    for (i = 0; i < MAX_POSTS; ++i)
    {
        if (_gPost[i].actor == post)
        {
            _gPost[i].actor = 0;
            break;
        }
    }
    SendRefresh(HUD::GameEvents::gPlayerEvents[0].mapRefreshPost, i);
}

// FUNCTION: SWBF2 0x00652a40
void AddMarker(GameObject* const actor, const RedRegion* const region, unsigned int textureHash,
               float scale, unsigned int teamMask, unsigned int colorHash,
               bool showHudTarget, bool flashAlpha, bool pulseSize, float pulseStartScale)
{
    if (actor ? region != 0 : region == 0)
    {
        return;
    }

    unsigned int uIndex = MAX_MARKERS;
    unsigned int uTeams = teamMask;
    bool bFound = false;
    for (unsigned int i = 0; i < MAX_MARKERS; ++i)
    {
        Marker& marker = _gMarkers[i];
        if (!marker.teamMask)
        {
            if (uIndex == MAX_MARKERS)
            {
                uIndex = i;
            }
            if (!uTeams)
            {
                break;
            }
            continue;
        }
        if (!((actor && HandleObject(marker.actor) == actor) ||
              (region && marker.region == region)))
        {
            continue;
        }

        if (marker.scale == scale && marker.textureHash == textureHash)
        {
            uIndex = i;
            marker.teamMask |= teamMask;
            marker.pulseStartScale = pulseStartScale;
            marker.flashAlpha = flashAlpha;
            marker.pulseSize = pulseSize;
            bFound = true;
            break;
        }
        if (uTeams)
        {
            unsigned int uTaken = marker.teamMask & uTeams;
            if (uTaken)
            {
                marker.teamMask &= ~uTaken;
                uTeams &= ~uTaken;
                if (!marker.teamMask)
                {
                    marker.Clear();
                    if (uIndex == MAX_MARKERS)
                    {
                        uIndex = i;
                    }
                }
                if (!uTeams && uIndex < MAX_MARKERS)
                {
                    break;
                }
            }
        }
    }

    if (!bFound)
    {
        if (uIndex < MAX_MARKERS)
        {
            Marker& marker = _gMarkers[uIndex];
            marker.actor.mObject = actor;
            marker.actor.mSavedHandleId = actor ? HandleId(actor) : 0;
            marker.region = const_cast<RedRegion*>(region);
            marker.teamMask = teamMask;
            marker.scale = scale;
            marker.textureHash = textureHash;
            marker.color = ColorByHash(colorHash);
            marker.pulseStartScale = pulseStartScale;
            marker.flashAlpha = flashAlpha;
            marker.pulseSize = pulseSize;
            marker.showHudTarget = showHudTarget;
        }
        else
        {
            uIndex = 0;
        }
    }

    if (actor && showHudTarget)
    {
        LockOnManager::AddTargetCommon(actor, teamMask);
    }
    if (UpdateInterface())
    {
        SendRefresh(HUD::GameEvents::gPlayerEvents[0].mapRefreshMarker, uIndex);
    }
}

// FUNCTION: SWBF2 0x00652c30
void RemoveMarker(GameObject* const actor, const RedRegion* const region, unsigned int teamMask)
{
    if (actor ? region != 0 : region == 0)
    {
        return;
    }

    unsigned int uTeams = teamMask;
    for (unsigned int i = 0; i < MAX_MARKERS; ++i)
    {
        Marker& marker = _gMarkers[i];
        if (!((actor && HandleObject(marker.actor) == actor) ||
              (region && marker.region == region)))
        {
            continue;
        }

        unsigned int uTaken = marker.teamMask & uTeams;
        if (!uTaken)
        {
            continue;
        }
        uTeams &= ~uTaken;
        marker.teamMask &= ~uTaken;
        if (!marker.teamMask)
        {
            marker.Clear();
        }
        if (!uTeams)
        {
            break;
        }
    }

    if (UpdateInterface() && actor)
    {
        LockOnManager::RemoveTargetCommon(actor, teamMask);
    }
}

// FUNCTION: SWBF2 0x00652ce0
void CheckObjectClassMarker(GameObject* const actor)
{
    for (int i = 0; i < MAX_CLASS_MARKERS; ++i)
    {
        const ClassMarker& marker = _gClassMarker[i];
        if (marker.classID == ((Entity*)actor)->GetEntityClass()->mId)
        {
            AddMarker(actor, 0, marker.textureHash, marker.scale, marker.teamMask,
                      marker.colorHash, marker.showHudTarget, marker.flashAlpha,
                      marker.pulseSize, marker.pulseStartScale);
            return;
        }
    }
}

}

namespace TargetManager
{

// GLOBAL: SWBF2 0x01f75f10
// SYNTHETIC: SWBF2 0x004065a0
// `dynamic initializer for 'TargetManager::_gHostMarkers''
// SYNTHETIC: SWBF2 0x00769520
// `dynamic atexit destructor for 'TargetManager::_gHostMarkers''
Marker _gHostMarkers[MAX_MARKERS];

// GLOBAL: SWBF2 0x01f7c980
// SYNTHETIC: SWBF2 0x004065d0
// `dynamic initializer for 'TargetManager::_gClientMarkers''
// SYNTHETIC: SWBF2 0x00769540
// `dynamic atexit destructor for 'TargetManager::_gClientMarkers''
Marker _gClientMarkers[MAX_MARKERS];

}

// SYNTHETIC: SWBF2 0x005c6370
// TargetManager::Marker::Marker
