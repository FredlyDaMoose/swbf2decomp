#include "EventManager.h"
#include "Battlefront2/Source/EventManager.h"

#include "EventManager_CommandPost.h"
#include "EventManager_Object.h"
#include "EventManager_LuaUser.h"

namespace EventManager
{
namespace CommandPostEvent { void Cleanup(); }
namespace ControllableEvent { void Cleanup(); }
namespace FlagEvent { void Cleanup(); }
namespace ObjectEvent { void Cleanup(); }
namespace CharacterEvent { void Cleanup(); }
namespace RegionEvent { void Cleanup(); }
namespace TeamEvent { void Cleanup(); }
namespace TimerEvent { void Cleanup(); }
}

namespace EventManager
{
namespace CommandPostEvent { void Init(); }
namespace FlagEvent { void Init(); }
namespace CharacterEvent { void Init(); }
namespace RegionEvent { void Init(); }
namespace LuaUserEvent { void Init(); }
namespace ObjectEvent { void Init(); }
namespace TeamEvent { void Init(); }

namespace TimerEvent { void Init(); }
}

namespace EventManager
{

// FUNCTION: SWBF2 0x0050b3a0
void Init()
{
    CommandPostEvent::Init();
    FlagEvent::Init();
    LuaUserEvent::Init();
    ObjectEvent::Init();
    CharacterEvent::Init();
    RegionEvent::Init();
    TeamEvent::Init();
    TimerEvent::Init();
}

// FUNCTION: SWBF2 0x0050b2e0
void Node::AttachToParent(Node* pParent)
{
    mParent = pParent;
    mSibling = pParent->mChild;
    pParent->mChild = this;
    AddRef();
}

// FUNCTION: SWBF2 0x0050b310
void Node::DetachFromParent()
{
    if (!mParent)
    {
        return;
    }

    if (mParent->mChild == this)
    {
        mParent->mChild = mSibling;
    }
    else
    {
        for (Node* pNode = mParent->mChild; pNode; pNode = pNode->mSibling)
        {
            if (pNode->mSibling == this)
            {
                pNode->mSibling = mSibling;
                break;
            }
        }
    }

    mParent = 0;
    Release();
}

}

namespace EventManager {

// FUNCTION: SWBF2 0x0050b3d0
void Cleanup()
{
    CommandPostEvent::Cleanup();
    ControllableEvent::Cleanup();
    FlagEvent::Cleanup();
    ObjectEvent::Cleanup();
    CharacterEvent::Cleanup();
    RegionEvent::Cleanup();
    TeamEvent::Cleanup();
    TimerEvent::Cleanup();
}
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x005105e0
// EventManager::UnregisterLuaCallback<CommandPost,DamageOwner>
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00515d90
// EventManager::UnregisterLuaCallback<GameObject,DamageOwner>
}

namespace EventManager
{
// TEMPLATE: SWBF2 0x00515f00
// EventManager::RegisterLuaCallback<GameObject,DamageOwner>
}

// FUNCTION: SWBF2 0x0050b380
void EventManager::Node::Clear()
{
    while (mChild)
    {
        mChild->DetachFromParent();
    }
}
