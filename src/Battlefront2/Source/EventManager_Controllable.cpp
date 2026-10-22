#include "EventManager_Timer.h"
#include "Controllable.h"

namespace EventManager
{
namespace ControllableEvent
{
extern Event<Controllable, int> mWeaponChange;
extern Event<Controllable, int> mTeamChange;
extern Event<Controllable, int> mPlayerChange;
extern Event<Controllable, TargetInfo> mTargetChange;
void Cleanup();
}
}

namespace EventManager {
namespace ControllableEvent {
extern Event<Controllable, int> mWeaponChange;
}
}
namespace EventManager {
namespace ControllableEvent {
extern Event<Controllable, int> mTeamChange;
}
}
namespace EventManager {
namespace ControllableEvent {
extern Event<Controllable, int> mPlayerChange;
}
}
namespace EventManager {
namespace ControllableEvent {
extern Event<Controllable, TargetInfo> mTargetChange;
}
}

namespace EventManager
{

template <class ITEM, class PARAM>
bool Event<ITEM, PARAM>::Callback::operator()(ITEM&, PARAM&)
{
    return true;
}

// TEMPLATE: SWBF2 0x005121f0
// EventManager::Event<Controllable,int>::Callback::operator()

// TEMPLATE: SWBF2 0x00512200
// EventManager::Event<Controllable,TargetInfo>::Callback::operator()

template bool Event<Controllable, int>::Callback::operator()(Controllable&, int&);
template bool Event<Controllable, TargetInfo>::Callback::operator()(Controllable&, TargetInfo&);

}

namespace EventManager {

// FUNCTION: SWBF2 0x005121c0
void ControllableEvent::Cleanup()
{
    mWeaponChange.Cleanup();
    mTeamChange.Cleanup();
    mPlayerChange.Cleanup();
    mTargetChange.Cleanup();
}
}

namespace EventManager
{
namespace ControllableEvent
{

// GLOBAL: SWBF2 0x007ebc80
// SYNTHETIC: SWBF2 0x004032c0
// `dynamic initializer for 'EventManager::ControllableEvent::mWeaponChange''
// SYNTHETIC: SWBF2 0x007679a0
// `dynamic atexit destructor for 'EventManager::ControllableEvent::mWeaponChange''
Event<Controllable, int> mWeaponChange("WeaponChange");

// GLOBAL: SWBF2 0x007ebbe0
// SYNTHETIC: SWBF2 0x004032f0
// `dynamic initializer for 'EventManager::ControllableEvent::mTeamChange''
// SYNTHETIC: SWBF2 0x00767a20
// `dynamic atexit destructor for 'EventManager::ControllableEvent::mTeamChange''
Event<Controllable, int> mTeamChange("TeamChange");

// GLOBAL: SWBF2 0x007ebc50
// SYNTHETIC: SWBF2 0x00403320
// `dynamic initializer for 'EventManager::ControllableEvent::mPlayerChange''
// SYNTHETIC: SWBF2 0x00767aa0
// `dynamic atexit destructor for 'EventManager::ControllableEvent::mPlayerChange''
Event<Controllable, int> mPlayerChange("PlayerChange");

// GLOBAL: SWBF2 0x007ebc20
// SYNTHETIC: SWBF2 0x00403350
// `dynamic initializer for 'EventManager::ControllableEvent::mTargetChange''
// SYNTHETIC: SWBF2 0x00767b20
// `dynamic atexit destructor for 'EventManager::ControllableEvent::mTargetChange''
Event<Controllable, TargetInfo> mTargetChange("TargetChange");

}
}

// SYNTHETIC: SWBF2 0x00512210
// EventManager::Event<Controllable,int>::Callback::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00512280
// EventManager::Event<Controllable,TargetInfo>::Callback::`scalar deleting destructor'
