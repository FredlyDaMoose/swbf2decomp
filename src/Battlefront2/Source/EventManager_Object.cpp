#include "EventManager_Object.h"
#include "Character.h"
#include "LuaHelper.h"
#include "LuaFL/include/lauxlib.h"
#include "EventManager_LuaUser.h"
#include "NetGame.h"

#include "SkyDome.h"
#include "SkyStars.h"
#include "WeatherEffect.h"
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/PC/pcRedIndexBuffer.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcInterfaceShader.h"
#include "RedEngineFL/Interface/RedBorderElement.h"
#include "RedEngineFL/Interface/RedHotSpot.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "LuaCallbacks_PC.h"
#include "LuaCallbacks_Net.h"
#include "FLInputManager.h"
#include "PlayerProfile.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Interface/Red3DModelAnimator.h"
#include "RedEngineFL/Interface/Red3DModelElement.h"
#include "RedEngineFL/movie/RedFileAsync.h"
#include "SoundFL/source/sndstreamimaadpcmdecoder.h"
#include "BoundaryManager.h"
#include "CommandPost.h"
#include "EndDisplay.h"
#include "EventManager_CommandPost.h"
#include "EventManager_Team.h"
#include "FLEffect.h"
#include "GameObject.h"
#include "WeaponTowCable.h"
#include "HUDElementBarBitmap.h"
#include "HUDElementMap.h"
#include "HUDElementText.h"
#include "HUDEvent.h"
#include "HUDProceduralBarBitmap.h"
#include "LeafEffect.h"
#include "LoadUtil.h"
#include "LuaCallbacks_Sound.h"
#include "OrdnanceBeam.h"
#include "StatusDisplay.h"
#include "TargetManager.h"
#include "Team.h"
#include "VOHelper.h"
#include "VoiceOverManager.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndstreambase.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "SoundFL/source/win/sndengine.h"
#include "SoundFL/source/win/sndstream.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "FLJoystick.h"

namespace EventManager
{
namespace ObjectEvent
{
extern Event<GameObject, char> mCreate;
extern Event<GameObject, char> mInit;
extern Event<GameObject, DamageOwner> mDamage;
extern Event<GameObject, DamageOwner> mRepair;
extern Event<GameObject, DamageOwner> mHack;
extern Event<GameObject, DamageOwner> mKill;
extern Event<GameObject, DamageOwner> mHeadshot;
extern Event<GameObject, char> mRespawn;
extern Event<GameObject, char> mDelete;
extern Event<GameObject, int> mTeamChange;
extern Event<GameObject, float> mHealthChange;
extern Event<GameObject, float> mShieldChange;
void Cleanup();
}
}

namespace EventManager {
namespace CommandPostEvent {
extern Event<CommandPost, DamageOwner> mKill;
}
}
namespace EventManager {
namespace CommandPostEvent {
extern Event<CommandPost, char> mRespawn;
}
}
namespace EventManager {
namespace ControllableEvent {
extern Event<Controllable, int> mTeamChange;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, char> mCreate;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, char> mInit;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, DamageOwner> mDamage;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, DamageOwner> mRepair;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, DamageOwner> mHack;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, DamageOwner> mHeadshot;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, char> mDelete;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, float> mHealthChange;
}
}
namespace EventManager {
namespace ObjectEvent {
extern Event<GameObject, float> mShieldChange;
}
}

namespace EventManager
{

// FUNCTION: SWBF2 0x00514710
template <>
bool LuaPushItem<GameObject>(GameObject& object)
{
    lua_pushlightuserdata(LuaHelper::mState, &object);
    return true;
}

// FUNCTION: SWBF2 0x00514730
template <>
bool LuaPushItem<DamageOwner>(DamageOwner& owner)
{
    if (owner.mCharacter)
    {
        lua_pushnumber(LuaHelper::mState, (lua_Number)(owner.mCharacter - Character::sCharacters));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return true;
}

template <class PARAM>
const char* ObjectMatches<PARAM>::GetName()
{
    return "Match";
}

template <class PARAM>
const char* ObjectMatchesName<PARAM>::GetName()
{
    return "Name";
}

template <class PARAM>
const char* ObjectMatchesTeam<PARAM>::GetName()
{
    return "Team";
}

template <class PARAM>
const char* ObjectMatchesClass<PARAM>::GetName()
{
    return "Class";
}

template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::LuaCallback(int iRef)
{
    mFunction = -1;
    if (LuaHelper::PushRefProc(iRef))
    {
        mFunction = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
    }
}

template <class ITEM, class PARAM>
LuaCallback<ITEM, PARAM>::~LuaCallback()
{
    if (mFunction != -1)
    {
        lua_unref(LuaHelper::mState, mFunction);
    }
}

template <class ITEM, class PARAM>
bool LuaCallback<ITEM, PARAM>::operator()(ITEM& item, PARAM& param)
{
    if (mFunction == -1 || !LuaHelper::PushRefProc(mFunction))
    {
        return false;
    }

    if (!LuaPushItem(item))
    {
        lua_pop(LuaHelper::mState, 1);
        return false;
    }

    if (!LuaPushItem(param))
    {
        lua_pop(LuaHelper::mState, 2);
        return false;
    }

    LuaHelper::CallProc(0);
    return true;
}

template <class ITEM, class PARAM>
template <class PREDICATE>
const char* Event<ITEM, PARAM>::Filter<PREDICATE>::GetName()
{
    return PREDICATE::GetName();
}

template <class ITEM, class PARAM>
template <class PREDICATE>
bool Event<ITEM, PARAM>::Filter<PREDICATE>::operator()(ITEM& item, PARAM& param)
{
    return mPredicate(item, param);
}

template <class ITEM, class PARAM>
bool Event<ITEM, PARAM>::Callback::operator()(ITEM&, PARAM&)
{
    return true;
}

// TEMPLATE: SWBF2 0x00515010
// EventManager::Event<GameObject,DamageOwner>::Callback::operator()

// TEMPLATE: SWBF2 0x00517fb0
// EventManager::LuaCallback<GameObject,char>::operator()

// TEMPLATE: SWBF2 0x00518010
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatches<char> >::operator()

// TEMPLATE: SWBF2 0x00518030
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatches<char> >::GetName

// TEMPLATE: SWBF2 0x00518040
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesName<char> >::operator()

// TEMPLATE: SWBF2 0x00518060
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesName<char> >::GetName

// TEMPLATE: SWBF2 0x00518070
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesTeam<char> >::operator()

// TEMPLATE: SWBF2 0x00518090
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesTeam<char> >::GetName

// TEMPLATE: SWBF2 0x005180a0
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesClass<char> >::operator()

// TEMPLATE: SWBF2 0x005180c0
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesClass<char> >::GetName

// TEMPLATE: SWBF2 0x005180d0
// EventManager::LuaCallback<GameObject,DamageOwner>::LuaCallback<GameObject,DamageOwner>

// TEMPLATE: SWBF2 0x00518160
// EventManager::LuaCallback<GameObject,DamageOwner>::operator()

// TEMPLATE: SWBF2 0x005181d0
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatches<DamageOwner> >::operator()

// TEMPLATE: SWBF2 0x005181f0
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatches<DamageOwner> >::GetName

// TEMPLATE: SWBF2 0x00518200
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesName<DamageOwner> >::operator()

// TEMPLATE: SWBF2 0x00518220
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesName<DamageOwner> >::GetName

// TEMPLATE: SWBF2 0x00518230
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesTeam<DamageOwner> >::operator()

// TEMPLATE: SWBF2 0x00518250
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesTeam<DamageOwner> >::GetName

// TEMPLATE: SWBF2 0x00518260
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesClass<DamageOwner> >::operator()

// TEMPLATE: SWBF2 0x00518280
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesClass<DamageOwner> >::GetName

// TEMPLATE: SWBF2 0x00518290
// EventManager::LuaCallback<GameObject,int>::LuaCallback<GameObject,int>

// TEMPLATE: SWBF2 0x00518320
// EventManager::LuaCallback<GameObject,int>::operator()

// TEMPLATE: SWBF2 0x00518390
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatches<int> >::operator()

// TEMPLATE: SWBF2 0x005183b0
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatches<int> >::GetName

// TEMPLATE: SWBF2 0x005183c0
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesName<int> >::operator()

// TEMPLATE: SWBF2 0x005183e0
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesName<int> >::GetName

// TEMPLATE: SWBF2 0x005183f0
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesTeam<int> >::operator()

// TEMPLATE: SWBF2 0x00518410
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesTeam<int> >::GetName

// TEMPLATE: SWBF2 0x00518420
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesClass<int> >::operator()

// TEMPLATE: SWBF2 0x00518440
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesClass<int> >::GetName

// TEMPLATE: SWBF2 0x00518450
// EventManager::LuaCallback<GameObject,float>::LuaCallback<GameObject,float>

// TEMPLATE: SWBF2 0x005184e0
// EventManager::LuaCallback<GameObject,float>::operator()

// TEMPLATE: SWBF2 0x00518550
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatches<float> >::operator()

// TEMPLATE: SWBF2 0x00518570
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatches<float> >::GetName

// TEMPLATE: SWBF2 0x00518580
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesName<float> >::operator()

// TEMPLATE: SWBF2 0x005185a0
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesName<float> >::GetName

// TEMPLATE: SWBF2 0x005185b0
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesTeam<float> >::operator()

// TEMPLATE: SWBF2 0x005185d0
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesTeam<float> >::GetName

// TEMPLATE: SWBF2 0x005185e0
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesClass<float> >::operator()

// TEMPLATE: SWBF2 0x00518600
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesClass<float> >::GetName

// SYNTHETIC: SWBF2 0x00518610
// EventManager::LuaCallback<GameObject,char>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005186b0
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatches<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518720
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesName<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518790
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesTeam<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518800
// EventManager::Event<GameObject,char>::Filter<EventManager::ObjectMatchesClass<char> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518870
// EventManager::LuaCallback<GameObject,DamageOwner>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518910
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatches<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518980
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesName<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005189f0
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesTeam<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518a60
// EventManager::Event<GameObject,DamageOwner>::Filter<EventManager::ObjectMatchesClass<DamageOwner> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518ad0
// EventManager::LuaCallback<GameObject,int>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518b70
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatches<int> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518be0
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesName<int> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518c50
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesTeam<int> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518cc0
// EventManager::Event<GameObject,int>::Filter<EventManager::ObjectMatchesClass<int> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518d30
// EventManager::LuaCallback<GameObject,float>::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518dd0
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatches<float> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518e40
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesName<float> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518eb0
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesTeam<float> >::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00518f20
// EventManager::Event<GameObject,float>::Filter<EventManager::ObjectMatchesClass<float> >::`scalar deleting destructor'

template class LuaCallback<GameObject, char>;
template class LuaCallback<GameObject, DamageOwner>;
template class LuaCallback<GameObject, int>;
template class LuaCallback<GameObject, float>;
template bool Event<GameObject, DamageOwner>::Callback::operator()(GameObject&, DamageOwner&);
template class Event<GameObject, char>::Filter<ObjectMatches<char> >;
template class Event<GameObject, char>::Filter<ObjectMatchesName<char> >;
template class Event<GameObject, char>::Filter<ObjectMatchesTeam<char> >;
template class Event<GameObject, char>::Filter<ObjectMatchesClass<char> >;
template class Event<GameObject, DamageOwner>::Filter<ObjectMatches<DamageOwner> >;
template class Event<GameObject, DamageOwner>::Filter<ObjectMatchesName<DamageOwner> >;
template class Event<GameObject, DamageOwner>::Filter<ObjectMatchesTeam<DamageOwner> >;
template class Event<GameObject, DamageOwner>::Filter<ObjectMatchesClass<DamageOwner> >;
template class Event<GameObject, int>::Filter<ObjectMatches<int> >;
template class Event<GameObject, int>::Filter<ObjectMatchesName<int> >;
template class Event<GameObject, int>::Filter<ObjectMatchesTeam<int> >;
template class Event<GameObject, int>::Filter<ObjectMatchesClass<int> >;
template class Event<GameObject, float>::Filter<ObjectMatches<float> >;
template class Event<GameObject, float>::Filter<ObjectMatchesName<float> >;
template class Event<GameObject, float>::Filter<ObjectMatchesTeam<float> >;
template class Event<GameObject, float>::Filter<ObjectMatchesClass<float> >;

}

namespace EventManager {

// FUNCTION: SWBF2 0x00514ec0
void ObjectEvent::Cleanup()
{
    mCreate.Cleanup();
    mInit.Cleanup();
    mDamage.Cleanup();
    mRepair.Cleanup();
    mHack.Cleanup();
    mKill.Cleanup();
    mHeadshot.Cleanup();
    mRespawn.Cleanup();
    mDelete.Cleanup();
    mTeamChange.Cleanup();
    mHealthChange.Cleanup();
    mShieldChange.Cleanup();
}
}

namespace EventManager {

// FUNCTION: SWBF2 0x00514fb0
template Event<GameObject, DamageOwner>::Callback::~Callback();
}

namespace EventManager {

// FUNCTION: SWBF2 0x00515020
template Event<GameObject, int>::Callback::~Callback();
}

namespace EventManager {

// FUNCTION: SWBF2 0x00515090
template Event<GameObject, float>::Callback::~Callback();
}

namespace EventManager
{

int DummyLuaCallback(lua_State* L);
namespace ObjectEvent { void Init(); }
namespace ObjectEvent
{
extern Event<GameObject, DamageOwner> mRepair;
extern Event<GameObject, DamageOwner> mHack;
extern Event<GameObject, DamageOwner> mHeadshot;
extern Event<GameObject, char> mDelete;
extern Event<GameObject, int> mTeamChange;
}

template <class PARAM>
static ObjectMatches<PARAM>* ReadObject(ObjectMatches<PARAM>* pMatch)
{
    if (lua_type(LuaHelper::mState, 2) == LUA_TLIGHTUSERDATA)
    {
        pMatch->mObject = (EntityEx*)lua_topointer(LuaHelper::mState, 2);
    }
    else
    {
        pMatch->mObject = EntityEx::mIdMap.Find(PblHash(luaL_checklstring(LuaHelper::mState, 2, 0)));
    }
    return pMatch;
}

template <class PARAM>
static ObjectMatchesName<PARAM>* ReadObjectName(ObjectMatchesName<PARAM>* pMatch)
{
    pMatch->mName = PblHash(luaL_checklstring(LuaHelper::mState, 2, 0));
    return pMatch;
}

template <class PARAM>
static ObjectMatchesClass<PARAM>* ReadObjectClass(ObjectMatchesClass<PARAM>* pMatch)
{
    unsigned int uiHash = PblHash(luaL_checklstring(LuaHelper::mState, 2, 0));
    pMatch->mClass = Factory<Entity, EntityClass, EntityDesc>::Find(uiHash);
    return pMatch;
}

template <class MATCH>
static MATCH* ReadNumber(MATCH* pMatch)
{
    pMatch->mTeam = (unsigned int)luaL_checknumber(LuaHelper::mState, 2);
    return pMatch;
}

// FUNCTION: SWBF2 0x00514780
template <>
ObjectMatches<char> LuaGetItem<ObjectMatches<char> >(int)
{
    ObjectMatches<char> match;
    ReadObject(&match);
    return match;
}

// FUNCTION: SWBF2 0x005147f0
template <>
ObjectMatches<DamageOwner> LuaGetItem<ObjectMatches<DamageOwner> >(int)
{
    ObjectMatches<DamageOwner> match;
    ReadObject(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514860
template <>
ObjectMatches<int> LuaGetItem<ObjectMatches<int> >(int)
{
    ObjectMatches<int> match;
    ReadObject(&match);
    return match;
}

// FUNCTION: SWBF2 0x005148d0
template <>
ObjectMatches<float> LuaGetItem<ObjectMatches<float> >(int)
{
    ObjectMatches<float> match;
    ReadObject(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514940
template <>
ObjectMatchesName<char> LuaGetItem<ObjectMatchesName<char> >(int)
{
    ObjectMatchesName<char> match;
    ReadObjectName(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514970
template <>
ObjectMatchesName<DamageOwner> LuaGetItem<ObjectMatchesName<DamageOwner> >(int)
{
    ObjectMatchesName<DamageOwner> match;
    ReadObjectName(&match);
    return match;
}

// FUNCTION: SWBF2 0x005149a0
template <>
ObjectMatchesName<int> LuaGetItem<ObjectMatchesName<int> >(int)
{
    ObjectMatchesName<int> match;
    ReadObjectName(&match);
    return match;
}

// FUNCTION: SWBF2 0x005149d0
template <>
ObjectMatchesName<float> LuaGetItem<ObjectMatchesName<float> >(int)
{
    ObjectMatchesName<float> match;
    ReadObjectName(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514a00
template <>
ObjectMatchesTeam<char> LuaGetItem<ObjectMatchesTeam<char> >(int)
{
    ObjectMatchesTeam<char> match(0);
    ReadNumber(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514a20
template <>
ObjectMatchesTeam<DamageOwner> LuaGetItem<ObjectMatchesTeam<DamageOwner> >(int)
{
    ObjectMatchesTeam<DamageOwner> match(0);
    ReadNumber(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514a40
template <>
ObjectMatchesTeam<int> LuaGetItem<ObjectMatchesTeam<int> >(int)
{
    ObjectMatchesTeam<int> match(0);
    ReadNumber(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514a60
template <>
ObjectMatchesTeam<float> LuaGetItem<ObjectMatchesTeam<float> >(int)
{
    ObjectMatchesTeam<float> match(0);
    ReadNumber(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514a80
template <>
ObjectMatchesClass<char> LuaGetItem<ObjectMatchesClass<char> >(int)
{
    ObjectMatchesClass<char> match;
    ReadObjectClass(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514ae0
template <>
ObjectMatchesClass<DamageOwner> LuaGetItem<ObjectMatchesClass<DamageOwner> >(int)
{
    ObjectMatchesClass<DamageOwner> match;
    ReadObjectClass(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514b40
template <>
ObjectMatchesClass<int> LuaGetItem<ObjectMatchesClass<int> >(int)
{
    ObjectMatchesClass<int> match;
    ReadObjectClass(&match);
    return match;
}

// FUNCTION: SWBF2 0x00514ba0
template <>
ObjectMatchesClass<float> LuaGetItem<ObjectMatchesClass<float> >(int)
{
    ObjectMatchesClass<float> match;
    ReadObjectClass(&match);
    return match;
}

template <class ITEM, class PARAM>
int UnregisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_islightuserdata(LuaHelper::mState, 1))
    {
        typename Event<ITEM, PARAM>::Callback* pCallback =
            (typename Event<ITEM, PARAM>::Callback*)lua_topointer(LuaHelper::mState, 1);
        if (pCallback)
        {
            pEvent->Unregister(*pCallback);
        }
    }
    return 0;
}

template <class ITEM, class PARAM>
int RegisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_isfunction(LuaHelper::mState, 1))
    {
        lua_pushvalue(LuaHelper::mState, 1);
        int iRef = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
        LuaCallback<ITEM, PARAM>* pCallback = new LuaCallback<ITEM, PARAM>(iRef);
        pCallback->AttachToParent(&pEvent->mRoot);
        luaL_unref(LuaHelper::mState, LUA_REGISTRYINDEX, iRef);
        lua_pushlightuserdata(LuaHelper::mState, pCallback);
        pCallback->Release();
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

template <class ITEM, class PARAM, class PREDICATE>
int RegisterLuaCallback(lua_State*)
{
    Event<ITEM, PARAM>* pEvent =
        (Event<ITEM, PARAM>*)lua_topointer(LuaHelper::mState, lua_upvalueindex(1));
    if (lua_isfunction(LuaHelper::mState, 1))
    {
        lua_pushvalue(LuaHelper::mState, 1);
        int iRef = luaL_ref(LuaHelper::mState, LUA_REGISTRYINDEX);
        PREDICATE predicate = LuaGetItem<PREDICATE>(2);
        typename Event<ITEM, PARAM>::template Filter<PREDICATE>* pFilter =
            pEvent->GetFilter(predicate);
        LuaCallback<ITEM, PARAM>* pCallback = new LuaCallback<ITEM, PARAM>(iRef);
        pCallback->AttachToParent(pFilter ? (Node*)pFilter : (Node*)&pEvent->mRoot);
        luaL_unref(LuaHelper::mState, LUA_REGISTRYINDEX, iRef);
        pFilter->Release();
        lua_pushlightuserdata(LuaHelper::mState, pCallback);
        pCallback->Release();
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

template <class ITEM, class PARAM>
void Event<ITEM, PARAM>::Unregister(Callback& callback)
{
    Node* pParent = callback.mParent;
    callback.DetachFromParent();
    while (pParent && pParent->mParent && !pParent->mChild)
    {
        Node* pNext = pParent->mParent;
        pParent->DetachFromParent();
        pParent = pNext;
    }
}

template <class ITEM, class PARAM>
template <class PREDICATE>
Event<ITEM, PARAM>::Filter<PREDICATE>::Filter(PREDICATE& predicate)
    : mPredicate(predicate)
{
}

template <class ITEM, class PARAM>
template <class PREDICATE>
auto Event<ITEM, PARAM>::GetFilter(PREDICATE& predicate, Callback* pParent)
    -> Filter<PREDICATE>*
{
    if (!pParent)
    {
        pParent = &mRoot;
    }
    for (Node* pNode = pParent->mChild; pNode; pNode = pNode->mSibling)
    {
        if (pNode->GetName() == PREDICATE::GetName() &&
            ((Filter<PREDICATE>*)pNode)->mPredicate == predicate)
        {
            pNode->AddRef();
            return (Filter<PREDICATE>*)pNode;
        }
    }
    Filter<PREDICATE>* pFilter = new Filter<PREDICATE>(predicate);
    pFilter->AttachToParent(pParent);
    return pFilter;
}

template <class ITEM, class PARAM>
void RegisterUnregisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "Release%s", event.mName);
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, UnregisterLuaCallback<ITEM, PARAM>, 1);
    }
}

template <class ITEM, class PARAM>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "On%s", event.mName);
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, RegisterLuaCallback<ITEM, PARAM>, 1);
    }
}

template <class ITEM, class PARAM, class PREDICATE>
void RegisterRegisterLuaCallback(Event<ITEM, PARAM>& event, lua_CFunction pfnClient)
{
    char acName[64];
    _snprintf(acName, sizeof(acName) - 1, "On%s%s", event.mName, PREDICATE::GetName());
    acName[sizeof(acName) - 1] = 0;
    lua_pushlightuserdata(LuaHelper::mState, &event);
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        LuaHelper::Register(acName, pfnClient, 0);
    }
    else
    {
        LuaHelper::Register(acName, RegisterLuaCallback<ITEM, PARAM, PREDICATE>, 1);
    }
}

// FUNCTION: SWBF2 0x00514c00
void ObjectEvent::Init()
{
    mCreate.Init();
    RegisterUnregisterLuaCallback(mCreate, DummyLuaCallback);
    RegisterRegisterLuaCallback(mCreate, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatches<char> >(mCreate, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesName<char> >(mCreate, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesTeam<char> >(mCreate, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesClass<char> >(mCreate, DummyLuaCallback);

    mInit.Init();
    RegisterUnregisterLuaCallback(mInit, DummyLuaCallback);
    RegisterRegisterLuaCallback(mInit, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatches<char> >(mInit, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesName<char> >(mInit, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesTeam<char> >(mInit, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesClass<char> >(mInit, DummyLuaCallback);

    mDamage.Init();
    RegisterUnregisterLuaCallback(mDamage, DummyLuaCallback);
    RegisterRegisterLuaCallback(mDamage, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatches<DamageOwner> >(mDamage, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesName<DamageOwner> >(mDamage, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesTeam<DamageOwner> >(mDamage, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesClass<DamageOwner> >(mDamage, DummyLuaCallback);

    mRepair.Init();
    RegisterUnregisterLuaCallback(mRepair, DummyLuaCallback);
    RegisterRegisterLuaCallback(mRepair, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatches<DamageOwner> >(mRepair, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesName<DamageOwner> >(mRepair, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesTeam<DamageOwner> >(mRepair, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesClass<DamageOwner> >(mRepair, DummyLuaCallback);

    mHack.Init();
    RegisterUnregisterLuaCallback(mHack, DummyLuaCallback);
    RegisterRegisterLuaCallback(mHack, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatches<DamageOwner> >(mHack, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesName<DamageOwner> >(mHack, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesTeam<DamageOwner> >(mHack, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesClass<DamageOwner> >(mHack, DummyLuaCallback);

    mKill.Init();
    RegisterUnregisterLuaCallback(mKill, DummyLuaCallback);
    RegisterRegisterLuaCallback(mKill, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatches<DamageOwner> >(mKill, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesName<DamageOwner> >(mKill, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesTeam<DamageOwner> >(mKill, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesClass<DamageOwner> >(mKill, DummyLuaCallback);

    mHeadshot.Init();
    RegisterUnregisterLuaCallback(mHeadshot, DummyLuaCallback);
    RegisterRegisterLuaCallback(mHeadshot, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatches<DamageOwner> >(mHeadshot, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesName<DamageOwner> >(mHeadshot, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesTeam<DamageOwner> >(mHeadshot, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, DamageOwner, ObjectMatchesClass<DamageOwner> >(mHeadshot, DummyLuaCallback);

    mRespawn.Init();
    RegisterUnregisterLuaCallback(mRespawn, DummyLuaCallback);
    RegisterRegisterLuaCallback(mRespawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatches<char> >(mRespawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesName<char> >(mRespawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesTeam<char> >(mRespawn, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesClass<char> >(mRespawn, DummyLuaCallback);

    mDelete.Init();
    RegisterUnregisterLuaCallback(mDelete, DummyLuaCallback);
    RegisterRegisterLuaCallback(mDelete, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatches<char> >(mDelete, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesName<char> >(mDelete, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesTeam<char> >(mDelete, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, char, ObjectMatchesClass<char> >(mDelete, DummyLuaCallback);

    mTeamChange.Init();
    RegisterUnregisterLuaCallback(mTeamChange, DummyLuaCallback);
    RegisterRegisterLuaCallback(mTeamChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, int, ObjectMatches<int> >(mTeamChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, int, ObjectMatchesName<int> >(mTeamChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, int, ObjectMatchesTeam<int> >(mTeamChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, int, ObjectMatchesClass<int> >(mTeamChange, DummyLuaCallback);

    mHealthChange.Init();
    RegisterUnregisterLuaCallback(mHealthChange, DummyLuaCallback);
    RegisterRegisterLuaCallback(mHealthChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatches<float> >(mHealthChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatchesName<float> >(mHealthChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatchesTeam<float> >(mHealthChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatchesClass<float> >(mHealthChange, DummyLuaCallback);

    mShieldChange.Init();
    RegisterUnregisterLuaCallback(mShieldChange, DummyLuaCallback);
    RegisterRegisterLuaCallback(mShieldChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatches<float> >(mShieldChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatchesName<float> >(mShieldChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatchesTeam<float> >(mShieldChange, DummyLuaCallback);
    RegisterRegisterLuaCallback<GameObject, float, ObjectMatchesClass<float> >(mShieldChange, DummyLuaCallback);
}

template int UnregisterLuaCallback<GameObject, char>(lua_State*);
template int UnregisterLuaCallback<GameObject, int>(lua_State*);
template int UnregisterLuaCallback<GameObject, float>(lua_State*);
template int RegisterLuaCallback<GameObject, int>(lua_State*);
template int RegisterLuaCallback<GameObject, float>(lua_State*);
template int RegisterLuaCallback<GameObject, int, ObjectMatches<int> >(lua_State*);
template int RegisterLuaCallback<GameObject, int, ObjectMatchesName<int> >(lua_State*);
template int RegisterLuaCallback<GameObject, int, ObjectMatchesTeam<int> >(lua_State*);
template int RegisterLuaCallback<GameObject, int, ObjectMatchesClass<int> >(lua_State*);
template int RegisterLuaCallback<GameObject, float, ObjectMatches<float> >(lua_State*);
template int RegisterLuaCallback<GameObject, float, ObjectMatchesName<float> >(lua_State*);
template int RegisterLuaCallback<GameObject, float, ObjectMatchesTeam<float> >(lua_State*);
template int RegisterLuaCallback<GameObject, float, ObjectMatchesClass<float> >(lua_State*);
template int RegisterLuaCallback<GameObject, char>(lua_State*);
template int RegisterLuaCallback<GameObject, char, ObjectMatches<char> >(lua_State*);
template int RegisterLuaCallback<GameObject, char, ObjectMatchesName<char> >(lua_State*);
template int RegisterLuaCallback<GameObject, char, ObjectMatchesTeam<char> >(lua_State*);
template int RegisterLuaCallback<GameObject, char, ObjectMatchesClass<char> >(lua_State*);
template void Event<GameObject, char>::Unregister(Callback&);
template void Event<GameObject, int>::Unregister(Callback&);
template void Event<GameObject, float>::Unregister(Callback&);
template Event<GameObject, char>::Filter<ObjectMatches<char> >*
Event<GameObject, char>::GetFilter(ObjectMatches<char>&, Callback*);
template Event<GameObject, char>::Filter<ObjectMatchesName<char> >*
Event<GameObject, char>::GetFilter(ObjectMatchesName<char>&, Callback*);
template Event<GameObject, char>::Filter<ObjectMatchesTeam<char> >*
Event<GameObject, char>::GetFilter(ObjectMatchesTeam<char>&, Callback*);
template Event<GameObject, char>::Filter<ObjectMatchesClass<char> >*
Event<GameObject, char>::GetFilter(ObjectMatchesClass<char>&, Callback*);
template Event<GameObject, int>::Filter<ObjectMatches<int> >*
Event<GameObject, int>::GetFilter(ObjectMatches<int>&, Callback*);
template Event<GameObject, int>::Filter<ObjectMatchesName<int> >*
Event<GameObject, int>::GetFilter(ObjectMatchesName<int>&, Callback*);
template Event<GameObject, int>::Filter<ObjectMatchesTeam<int> >*
Event<GameObject, int>::GetFilter(ObjectMatchesTeam<int>&, Callback*);
template Event<GameObject, int>::Filter<ObjectMatchesClass<int> >*
Event<GameObject, int>::GetFilter(ObjectMatchesClass<int>&, Callback*);
template Event<GameObject, float>::Filter<ObjectMatches<float> >*
Event<GameObject, float>::GetFilter(ObjectMatches<float>&, Callback*);
template Event<GameObject, float>::Filter<ObjectMatchesName<float> >*
Event<GameObject, float>::GetFilter(ObjectMatchesName<float>&, Callback*);
template Event<GameObject, float>::Filter<ObjectMatchesTeam<float> >*
Event<GameObject, float>::GetFilter(ObjectMatchesTeam<float>&, Callback*);
template Event<GameObject, float>::Filter<ObjectMatchesClass<float> >*
Event<GameObject, float>::GetFilter(ObjectMatchesClass<float>&, Callback*);

// TEMPLATE: SWBF2 0x00514f40
// EventManager::Node::~Node, emitted as an unwind copy

// TEMPLATE: SWBF2 0x00515310
// EventManager::RegisterUnregisterLuaCallback<GameObject,char>

// TEMPLATE: SWBF2 0x005153a0
// EventManager::RegisterRegisterLuaCallback<GameObject,char>

// TEMPLATE: SWBF2 0x00515550
// EventManager::RegisterRegisterLuaCallback<GameObject,char,EventManager::ObjectMatches<char> >

// TEMPLATE: SWBF2 0x00515760
// EventManager::RegisterRegisterLuaCallback<GameObject,char,EventManager::ObjectMatchesName<char> >

// TEMPLATE: SWBF2 0x00515970
// EventManager::RegisterRegisterLuaCallback<GameObject,char,EventManager::ObjectMatchesTeam<char> >

// TEMPLATE: SWBF2 0x00515b80
// EventManager::RegisterRegisterLuaCallback<GameObject,char,EventManager::ObjectMatchesClass<char> >

// TEMPLATE: SWBF2 0x00515de0
// EventManager::RegisterUnregisterLuaCallback<GameObject,DamageOwner>

// TEMPLATE: SWBF2 0x00515e70
// EventManager::RegisterRegisterLuaCallback<GameObject,DamageOwner>

// TEMPLATE: SWBF2 0x00516020
// EventManager::RegisterRegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatches<DamageOwner> >

// TEMPLATE: SWBF2 0x00516230
// EventManager::RegisterRegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatchesName<DamageOwner> >

// TEMPLATE: SWBF2 0x00516440
// EventManager::RegisterRegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatchesTeam<DamageOwner> >

// TEMPLATE: SWBF2 0x00516650
// EventManager::RegisterRegisterLuaCallback<GameObject,DamageOwner,EventManager::ObjectMatchesClass<DamageOwner> >

// TEMPLATE: SWBF2 0x005168b0
// EventManager::RegisterUnregisterLuaCallback<GameObject,int>

// TEMPLATE: SWBF2 0x00516940
// EventManager::RegisterRegisterLuaCallback<GameObject,int>

// TEMPLATE: SWBF2 0x00516af0
// EventManager::RegisterRegisterLuaCallback<GameObject,int,EventManager::ObjectMatches<int> >

// TEMPLATE: SWBF2 0x00516d00
// EventManager::RegisterRegisterLuaCallback<GameObject,int,EventManager::ObjectMatchesName<int> >

// TEMPLATE: SWBF2 0x00516f10
// EventManager::RegisterRegisterLuaCallback<GameObject,int,EventManager::ObjectMatchesTeam<int> >

// TEMPLATE: SWBF2 0x00517140
// EventManager::RegisterRegisterLuaCallback<GameObject,int,EventManager::ObjectMatchesClass<int> >

// TEMPLATE: SWBF2 0x005173a0
// EventManager::RegisterUnregisterLuaCallback<GameObject,float>

// TEMPLATE: SWBF2 0x00517430
// EventManager::RegisterRegisterLuaCallback<GameObject,float>

// TEMPLATE: SWBF2 0x005175e0
// EventManager::RegisterRegisterLuaCallback<GameObject,float,EventManager::ObjectMatches<float> >

// TEMPLATE: SWBF2 0x005177f0
// EventManager::RegisterRegisterLuaCallback<GameObject,float,EventManager::ObjectMatchesName<float> >

// TEMPLATE: SWBF2 0x00517a00
// EventManager::RegisterRegisterLuaCallback<GameObject,float,EventManager::ObjectMatchesTeam<float> >

// TEMPLATE: SWBF2 0x00517c10
// EventManager::RegisterRegisterLuaCallback<GameObject,float,EventManager::ObjectMatchesClass<float> >

// TEMPLATE: SWBF2 0x005152c0
// EventManager::UnregisterLuaCallback<classGameObject,char>

// TEMPLATE: SWBF2 0x00516860
// EventManager::UnregisterLuaCallback<classGameObject,int>

// TEMPLATE: SWBF2 0x005169d0
// EventManager::RegisterLuaCallback<classGameObject,int>

// TEMPLATE: SWBF2 0x00516b80
// EventManager::RegisterLuaCallback<classGameObject,int,classEventManager::ObjectMatches<int>>

// TEMPLATE: SWBF2 0x00516d90
// EventManager::RegisterLuaCallback<classGameObject,int,classEventManager::ObjectMatchesName<int>>

// TEMPLATE: SWBF2 0x00516fc0
// EventManager::RegisterLuaCallback<classGameObject,int,classEventManager::ObjectMatchesTeam<int>>

// TEMPLATE: SWBF2 0x005171d0
// EventManager::RegisterLuaCallback<classGameObject,int,classEventManager::ObjectMatchesClass<int>>

// TEMPLATE: SWBF2 0x00517350
// EventManager::UnregisterLuaCallback<classGameObject,float>

// TEMPLATE: SWBF2 0x005174c0
// EventManager::RegisterLuaCallback<classGameObject,float>

// TEMPLATE: SWBF2 0x00517670
// EventManager::RegisterLuaCallback<classGameObject,float,classEventManager::ObjectMatches<float>>

// TEMPLATE: SWBF2 0x00517880
// EventManager::RegisterLuaCallback<classGameObject,float,classEventManager::ObjectMatchesName<float>>

// TEMPLATE: SWBF2 0x00517a90
// EventManager::RegisterLuaCallback<classGameObject,float,classEventManager::ObjectMatchesTeam<float>>

// TEMPLATE: SWBF2 0x00517ca0
// EventManager::RegisterLuaCallback<classGameObject,float,classEventManager::ObjectMatchesClass<float>>

// TEMPLATE: SWBF2 0x00517e20
// EventManager::Event<GameObject,char>::Unregister

// TEMPLATE: SWBF2 0x00517ea0
// EventManager::Event<GameObject,int>::Unregister

// TEMPLATE: SWBF2 0x00517ee0
// EventManager::Event<GameObject,float>::Unregister

// TEMPLATE: SWBF2 0x00517f20
// EventManager::LuaCallback<GameObject,char>::LuaCallback<GameObject,char>

// TEMPLATE: SWBF2 0x00518f90
// EventManager::Event<GameObject,char>::GetFilter<EventManager::ObjectMatches<char> >

// TEMPLATE: SWBF2 0x00519040
// EventManager::Event<GameObject,char>::GetFilter<EventManager::ObjectMatchesName<char> >

// TEMPLATE: SWBF2 0x005190f0
// EventManager::Event<GameObject,char>::GetFilter<EventManager::ObjectMatchesTeam<char> >

// TEMPLATE: SWBF2 0x005191a0
// EventManager::Event<GameObject,char>::GetFilter<EventManager::ObjectMatchesClass<char> >

// TEMPLATE: SWBF2 0x00519510
// EventManager::Event<GameObject,int>::GetFilter<EventManager::ObjectMatches<int> >

// TEMPLATE: SWBF2 0x005195c0
// EventManager::Event<GameObject,int>::GetFilter<EventManager::ObjectMatchesName<int> >

// TEMPLATE: SWBF2 0x00519670
// EventManager::Event<GameObject,int>::GetFilter<EventManager::ObjectMatchesTeam<int> >

// TEMPLATE: SWBF2 0x00519720
// EventManager::Event<GameObject,int>::GetFilter<EventManager::ObjectMatchesClass<int> >

// TEMPLATE: SWBF2 0x005197d0
// EventManager::Event<GameObject,float>::GetFilter<EventManager::ObjectMatches<float> >

// TEMPLATE: SWBF2 0x00519880
// EventManager::Event<GameObject,float>::GetFilter<EventManager::ObjectMatchesName<float> >

// TEMPLATE: SWBF2 0x00519930
// EventManager::Event<GameObject,float>::GetFilter<EventManager::ObjectMatchesTeam<float> >

// TEMPLATE: SWBF2 0x005199e0
// EventManager::Event<GameObject,float>::GetFilter<EventManager::ObjectMatchesClass<float> >

// TEMPLATE: SWBF2 0x00515430
// EventManager::RegisterLuaCallback<GameObject,char>

// TEMPLATE: SWBF2 0x005155e0
// EventManager::RegisterLuaCallback<GameObject,char,ObjectMatches<char> >

// TEMPLATE: SWBF2 0x005157f0
// EventManager::RegisterLuaCallback<GameObject,char,ObjectMatchesName<char> >

// TEMPLATE: SWBF2 0x00515a00
// EventManager::RegisterLuaCallback<GameObject,char,ObjectMatchesTeam<char> >

// TEMPLATE: SWBF2 0x00515c10
// EventManager::RegisterLuaCallback<GameObject,char,ObjectMatchesClass<char> >

}

namespace EventManager
{
// SYNTHETIC: SWBF2 0x00515100
// EventManager::Event<GameObject,char>::Callback::`scalar deleting destructor'
}

namespace EventManager
{
// SYNTHETIC: SWBF2 0x00515170
// EventManager::Event<GameObject,DamageOwner>::Callback::`scalar deleting destructor'
}

namespace EventManager
{
// SYNTHETIC: SWBF2 0x005151e0
// EventManager::Event<GameObject,int>::Callback::`scalar deleting destructor'
}

namespace EventManager
{
// SYNTHETIC: SWBF2 0x00515250
// EventManager::Event<GameObject,float>::Callback::`scalar deleting destructor'
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00519250
// EventManager::Event<GameObject,DamageOwner>::GetFilter<EventManager::ObjectMatches<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00519300
// EventManager::Event<GameObject,DamageOwner>::GetFilter<EventManager::ObjectMatchesName<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x005193b0
// EventManager::Event<GameObject,DamageOwner>::GetFilter<EventManager::ObjectMatchesTeam<DamageOwner> >
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00519460
// EventManager::Event<GameObject,DamageOwner>::GetFilter<EventManager::ObjectMatchesClass<DamageOwner> >
}

namespace EventManager
{

// TEMPLATE: SWBF2 0x00514fa0
// EventManager::Event<GameObject,char>::Callback::operator()
template bool Event<GameObject, char>::Callback::operator()(GameObject&, char&);
}

namespace EventManager
{

// TEMPLATE: SWBF2 0x005150f0
// EventManager::Event<GameObject,float>::Callback::operator()
template bool Event<GameObject, float>::Callback::operator()(GameObject&, float&);
}

namespace EventManager
{
namespace ObjectEvent
{

// GLOBAL: SWBF2 0x007ebfa0
// SYNTHETIC: SWBF2 0x00403470
// `dynamic initializer for 'EventManager::ObjectEvent::mCreate''
// SYNTHETIC: SWBF2 0x00767e20
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mCreate''
Event<GameObject, char> mCreate("ObjectCreate");

// GLOBAL: SWBF2 0x007ebf00
// SYNTHETIC: SWBF2 0x004034a0
// `dynamic initializer for 'EventManager::ObjectEvent::mInit''
// SYNTHETIC: SWBF2 0x00767ea0
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mInit''
Event<GameObject, char> mInit("ObjectInit");

// GLOBAL: SWBF2 0x007ec060
// SYNTHETIC: SWBF2 0x004034d0
// `dynamic initializer for 'EventManager::ObjectEvent::mDamage''
// SYNTHETIC: SWBF2 0x00767f20
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mDamage''
Event<GameObject, DamageOwner> mDamage("ObjectDamage");

// GLOBAL: SWBF2 0x007ebf60
// SYNTHETIC: SWBF2 0x00403500
// `dynamic initializer for 'EventManager::ObjectEvent::mRepair''
// SYNTHETIC: SWBF2 0x00767fa0
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mRepair''
Event<GameObject, DamageOwner> mRepair("ObjectRepair");

// GLOBAL: SWBF2 0x007ebfe0
// SYNTHETIC: SWBF2 0x00403530
// `dynamic initializer for 'EventManager::ObjectEvent::mHack''
// SYNTHETIC: SWBF2 0x00768020
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mHack''
Event<GameObject, DamageOwner> mHack("ObjectHack");

// GLOBAL: SWBF2 0x007ebf80
// SYNTHETIC: SWBF2 0x00403560
// `dynamic initializer for 'EventManager::ObjectEvent::mKill''
// SYNTHETIC: SWBF2 0x007680a0
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mKill''
Event<GameObject, DamageOwner> mKill("ObjectKill");

// GLOBAL: SWBF2 0x007ec000
// SYNTHETIC: SWBF2 0x00403590
// `dynamic initializer for 'EventManager::ObjectEvent::mHeadshot''
// SYNTHETIC: SWBF2 0x00768120
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mHeadshot''
Event<GameObject, DamageOwner> mHeadshot("ObjectHeadshot");

// GLOBAL: SWBF2 0x007ebf20
// SYNTHETIC: SWBF2 0x004035c0
// `dynamic initializer for 'EventManager::ObjectEvent::mRespawn''
// SYNTHETIC: SWBF2 0x007681a0
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mRespawn''
Event<GameObject, char> mRespawn("ObjectRespawn");

// GLOBAL: SWBF2 0x007ebfc0
// SYNTHETIC: SWBF2 0x004035f0
// `dynamic initializer for 'EventManager::ObjectEvent::mDelete''
// SYNTHETIC: SWBF2 0x00768220
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mDelete''
Event<GameObject, char> mDelete("ObjectDelete");

// GLOBAL: SWBF2 0x007ebf40
// SYNTHETIC: SWBF2 0x00403620
// `dynamic initializer for 'EventManager::ObjectEvent::mTeamChange''
// SYNTHETIC: SWBF2 0x007682a0
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mTeamChange''
Event<GameObject, int> mTeamChange("TeamChange");

// GLOBAL: SWBF2 0x007ec020
// SYNTHETIC: SWBF2 0x00403650
// `dynamic initializer for 'EventManager::ObjectEvent::mHealthChange''
// SYNTHETIC: SWBF2 0x00768320
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mHealthChange''
Event<GameObject, float> mHealthChange("HealthChange");

// GLOBAL: SWBF2 0x007ec040
// SYNTHETIC: SWBF2 0x00403680
// `dynamic initializer for 'EventManager::ObjectEvent::mShieldChange''
// SYNTHETIC: SWBF2 0x007683a0
// `dynamic atexit destructor for 'EventManager::ObjectEvent::mShieldChange''
Event<GameObject, float> mShieldChange("ShieldChange");

}
}
