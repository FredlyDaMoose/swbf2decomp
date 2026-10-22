#pragma once

#include "Entity.h"
#include "PebbleFL/Common/PblHandle.h"
#include "RedEngineFL/Graphics/RedColor.h"

class CommandPost;
class GameObject;
class RedRegion;

namespace TargetManager
{

struct Color
{
    unsigned int hashID;
    RedColor color;
};

struct ClassMarker
{
    void Clear();

    unsigned int classID;
    unsigned int teamMask;
    unsigned int textureHash;
    float scale;
    unsigned int colorHash;
    bool showHudTarget;
    float pulseStartScale;
    bool flashAlpha : 1;
    bool pulseSize : 1;
};

struct Marker
{
    void Clear();

    PblHandle<GameObject> actor;
    RedRegion* region;
    unsigned int teamMask;
    unsigned int textureHash;
    float scale;
    RedColor color;
    bool showHudTarget;
    float pulseStartScale;
    bool flashAlpha : 1;
    bool pulseSize : 1;
};

struct Post
{
    CommandPost* actor;
    EntityClass::MapParams mapParams;
};

struct Target
{
    PblHandle<GameObject> actor;
    bool alwaysVisibleToFriends;
    EntityClass::MapParams mapParams;
};

enum
{
    MAX_CLASS_MARKERS = 8,
    MAX_MARKERS = 32,
    MAX_POSTS = 16,
    MAX_TARGETS = 450,
    MAX_COLORS = 13
};

bool UpdateInterface();
const RedColor& ColorByHash(unsigned int hashID);
void CheckObjectClassMarker(GameObject* const actor);

void AddTarget(GameObject* const actor, bool alwaysVisibleToFriends);
void RemoveTarget(GameObject* const actor);
void AddPost(CommandPost* const post);
void RemovePost(CommandPost* const post);
void AddClassMarker(unsigned int classID, unsigned int textureHash, float scale,
                    unsigned int teamMask, unsigned int colorHash, bool showHudTarget,
                    bool flashAlpha, bool pulseSize, float pulseStartScale);
void RemoveClassMarker(unsigned int classID, unsigned int teamMask);
void AddMarker(GameObject* const actor, const RedRegion* const region, unsigned int textureHash,
               float scale, unsigned int teamMask, unsigned int colorHash,
               bool showHudTarget, bool flashAlpha, bool pulseSize, float pulseStartScale);
void RemoveMarker(GameObject* const actor, const RedRegion* const region, unsigned int teamMask);

void Init();
void CreateClient();
void DestroyClient();
void EnterClient();
void LeaveClient();

extern ClassMarker _gClassMarker[MAX_CLASS_MARKERS];
extern Marker* _gMarkers;
extern Marker* gMarkers;
extern Post _gPost[MAX_POSTS];
extern Target _gTarget[MAX_TARGETS];
extern Color gColorTable[MAX_COLORS];
unsigned int HashByColor(RedColor& color);

}

 
 
 
 
 
 
 
