#include "CommandHover.h"
#include "CollisionManager.h"
#include "NetGame.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "GameModel.h"

// GLOBAL: SWBF2 0x01eb96a8
// SYNTHETIC: SWBF2 0x004026c0
// `dynamic initializer for 'CommandHover::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e90
// `dynamic atexit destructor for 'CommandHover::sMemoryPool''
MemoryPool CommandHover::sMemoryPool("CommandHover", 0x1d20);

// GLOBAL: SWBF2 0x01eb96a4
// SYNTHETIC: SWBF2 0x004026e0
// `dynamic initializer for 'CommandHover::rttiHashCommandHover''
PblHash CommandHover::rttiHashCommandHover("CommandHover");
char CommandHover::rttiNameCommandHover[] = "CommandHover";

// FUNCTION: SWBF2 0x00478e70
bool CommandHover::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashCommandHover ||
           uRtti == EntityHover::rttiHashEntityHover ||
           uRtti == GameObject::rttiHashGameObject ||
           uRtti == EntityGeometry::rttiHashEntityGeometry ||
           uRtti == EntityEx::rttiHashEntityEx ||
           uRtti == Entity::rttiHashEntity;
}

// FUNCTION: SWBF2 0x00478ec0
unsigned int CommandHover::GetDerivedRtti() const
{
    return rttiHashCommandHover;
}

// FUNCTION: SWBF2 0x00478ed0
const char* CommandHover::GetDerivedRttiName() const
{
    return rttiNameCommandHover;
}

// FUNCTION: SWBF2 0x00478ef0
EntityType CommandHover::GetEntityType() const
{
    return ENT_POST_HOVER;
}

// SYNTHETIC: SWBF2 0x00478f00
// CommandHover::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00478fa0
CommandHover::CommandHover(CommandHoverClass* pClass, const EntityDesc& desc)
    : EntityHover(pClass, desc)
{
    mClass = pClass;
    mPost = CommandPost::BuildPost(this, pClass);
}

// FUNCTION: SWBF2 0x00479050
CommandHover::~CommandHover()
{
    if (mPost)
    {
        mPost->Kill();
    }
}

// FUNCTION: SWBF2 0x004790f0
void CommandHover::Init()
{
    EntityHover::Init();
    mPost->Respawn();
}

// FUNCTION: SWBF2 0x00479110
void CommandHover::Kill()
{
    EntityHover::Kill();
    mPost->Kill();
}

// FUNCTION: SWBF2 0x00479130
void CommandHover::Respawn()
{
    mPost->Respawn();
    GameObject::Respawn();
}

// FUNCTION: SWBF2 0x00479150
void CommandHover::SetProperty(unsigned int uProperty, const char* pszValue)
{
    GameObject::SetProperty(uProperty, pszValue);
    mPost->SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00479180
void CommandHover::SetTeam(int iTeam)
{
    int iOldTeam = mTeam;
    GameObject::SetTeam(iTeam);
    mPost->SetTeam(iOldTeam, iTeam);
}

// FUNCTION: SWBF2 0x004791b0
CommandHoverClass::CommandHoverClass(unsigned int uHash)
    : EntityHoverClass(uHash), CommandPostClass(uHash)
{
    mIsNeutralWhenEmpty = false;
}

// GLOBAL: SWBF2 0x01eb96a0
// SYNTHETIC: SWBF2 0x004026f0
// `dynamic initializer for 'CommandHoverClass::rttiHashCommandHoverClass''
PblHash CommandHoverClass::rttiHashCommandHoverClass("CommandHoverClass");
char CommandHoverClass::rttiNameCommandHoverClass[] = "CommandHoverClass";

// FUNCTION: SWBF2 0x00478f30
bool CommandHoverClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashCommandHoverClass || EntityHoverClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00478f80
unsigned int CommandHoverClass::GetDerivedRtti()
{
    return rttiHashCommandHoverClass;
}

// FUNCTION: SWBF2 0x00478f90
const char* CommandHoverClass::GetDerivedRttiName()
{
    return rttiNameCommandHoverClass;
}

// SYNTHETIC: SWBF2 0x00479220
// CommandHoverClass::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004794f6
// [thunk]:CommandHoverClass::`scalar deleting destructor'`adjustor{3696}'

// FUNCTION: SWBF2 0x00479250
CommandHoverClass::CommandHoverClass(CommandHoverClass* pBase, unsigned int uHash)
    : EntityHoverClass(pBase, uHash), CommandPostClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x004792d0
CommandHoverClass::~CommandHoverClass()
{
}

// FUNCTION: SWBF2 0x00479330
void CommandHoverClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityHoverClass::SetProperty(uProperty, pszValue);
    CommandPostClass::SetProperty(uProperty, pszValue, mModel ? mModel->_mLod[0] : 0, 0);
}

// FUNCTION: SWBF2 0x00479380
EntityClass* CommandHoverClass::Derive(unsigned int uHash)
{
    return new CommandHoverClass(this, uHash);
}

// FUNCTION: SWBF2 0x00478f70
unsigned int GetHoverClassRtti()
{
    return CommandHoverClass::rttiHashCommandHoverClass;
}

// FUNCTION: SWBF2 0x004793f0
Entity* CommandHoverClass::Build(const EntityDesc& desc)
{
    if (CommandHover::sMemoryPool.IsFull())
    {
        return 0;
    }

    CommandHover* pHover = new CommandHover(this, desc);
    if (pHover)
    {
        NetGame::Created(pHover);
        CollisionManager::AddRigidBody(pHover);
        pHover->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            pHover->BuildEffects();
        }
    }
    return pHover;
}
