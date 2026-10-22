#include "CommandBuildingArmed.h"
#include "GameModel.h"
#include "NetGame.h"

// GLOBAL: SWBF2 0x01eb928c
// SYNTHETIC: SWBF2 0x00402660
// `dynamic initializer for 'CommandBuildingArmed::rttiHashCommandBuildingArmed''
PblHash CommandBuildingArmed::rttiHashCommandBuildingArmed("CommandBuildingArmed");
char CommandBuildingArmed::rttiNameCommandBuildingArmed[] = "CommandBuildingArmed";

// FUNCTION: SWBF2 0x00476ee0
bool CommandBuildingArmed::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashCommandBuildingArmed ||
           uRtti == EntityBuildingArmed::rttiHashEntityBuildingArmed ||
           uRtti == EntityBuilding::rttiHashEntityBuilding ||
           uRtti == GameObject::rttiHashGameObject ||
           uRtti == EntityGeometry::rttiHashEntityGeometry ||
           uRtti == EntityEx::rttiHashEntityEx ||
           uRtti == Entity::rttiHashEntity;
}

// FUNCTION: SWBF2 0x00476f30
unsigned int CommandBuildingArmed::GetDerivedRtti() const
{
    return rttiHashCommandBuildingArmed;
}

// FUNCTION: SWBF2 0x00476f40
const char* CommandBuildingArmed::GetDerivedRttiName() const
{
    return rttiNameCommandBuildingArmed;
}

// FUNCTION: SWBF2 0x00476f50
EntityType CommandBuildingArmed::GetEntityType() const
{
    return ENT_POST;
}

// FUNCTION: SWBF2 0x00477000
CommandBuildingArmed::CommandBuildingArmed(CommandBuildingArmedClass* pClass, const EntityDesc& desc)
    : EntityBuildingArmed(pClass, desc)
{
    mClass = pClass;
    mPost = CommandPost::BuildPost(this, pClass);
}

// FUNCTION: SWBF2 0x004770a0
CommandBuildingArmed::~CommandBuildingArmed()
{
}

// SYNTHETIC: SWBF2 0x00476f60
// CommandBuildingArmed::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00477496
// [thunk]:CommandBuildingArmed::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x0047746a
// [thunk]:CommandBuildingArmed::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x00477475
// [thunk]:CommandBuildingArmed::`scalar deleting destructor' adjustor{608}
// SYNTHETIC: SWBF2 0x00477480
// [thunk]:CommandBuildingArmed::`scalar deleting destructor' adjustor{148}
// SYNTHETIC: SWBF2 0x0047748b
// [thunk]:CommandBuildingArmed::`scalar deleting destructor' adjustor{320}

// FUNCTION: SWBF2 0x004770e0
void CommandBuildingArmed::Init()
{
    EntityBuildingArmed::Init();
    if (mState == STATE_ALIVE)
    {
        mPost->Respawn();
    }
}

// FUNCTION: SWBF2 0x00477100
void CommandBuildingArmed::Kill()
{
    EntityBuildingArmed::Kill();
    mPost->Kill();
}

// FUNCTION: SWBF2 0x00477120
void CommandBuildingArmed::Respawn()
{
    EntityBuildingArmed::Respawn();
    mPost->Respawn();
}

// FUNCTION: SWBF2 0x00477140
void CommandBuildingArmed::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityBuildingArmed::SetProperty(uProperty, pszValue);
    mPost->SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00477170
void CommandBuildingArmed::SetTeam(int iTeam)
{
    int iOldTeam = mTeam;
    GameObject::SetTeam(iTeam);
    mPost->SetTeam(iOldTeam, iTeam);
}

// FUNCTION: SWBF2 0x004771a0
CommandBuildingArmedClass::CommandBuildingArmedClass(unsigned int uHash)
    : EntityBuildingArmedClass(uHash), CommandPostClass(uHash)
{
}

// FUNCTION: SWBF2 0x00477230
CommandBuildingArmedClass::CommandBuildingArmedClass(CommandBuildingArmedClass* pBase, unsigned int uHash)
    : EntityBuildingArmedClass(pBase, uHash), CommandPostClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x004772b0
CommandBuildingArmedClass::~CommandBuildingArmedClass()
{
}

// SYNTHETIC: SWBF2 0x00477200
// CommandBuildingArmedClass::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0047749e
// [thunk]:CommandBuildingArmedClass::`scalar deleting destructor' adjustor{1656}

// GLOBAL: SWBF2 0x01eb9554
// SYNTHETIC: SWBF2 0x00402670
// `dynamic initializer for 'CommandBuildingArmedClass::rttiHashCommandBuildingArmedClass''
PblHash CommandBuildingArmedClass::rttiHashCommandBuildingArmedClass("CommandBuildingArmedClass");
char CommandBuildingArmedClass::rttiNameCommandBuildingArmedClass[] = "CommandBuildingArmedClass";

// FUNCTION: SWBF2 0x00476f90
bool CommandBuildingArmedClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashCommandBuildingArmedClass || EntityBuildingArmedClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00476fe0
unsigned int CommandBuildingArmedClass::GetDerivedRtti()
{
    return rttiHashCommandBuildingArmedClass;
}

// FUNCTION: SWBF2 0x00476ff0
const char* CommandBuildingArmedClass::GetDerivedRttiName()
{
    return rttiNameCommandBuildingArmedClass;
}

// FUNCTION: SWBF2 0x00477310
void CommandBuildingArmedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityBuildingArmedClass::SetProperty(uProperty, pszValue);
    CommandPostClass::SetProperty(uProperty, pszValue, mModel ? mModel->_mLod[0] : 0, 0);
}

// FUNCTION: SWBF2 0x00477350
EntityClass* CommandBuildingArmedClass::Derive(unsigned int uHash)
{
    return new CommandBuildingArmedClass(this, uHash);
}

// FUNCTION: SWBF2 0x004773c0
Entity* CommandBuildingArmedClass::Build(const EntityDesc& desc)
{
    CommandBuildingArmed* pBuilding = new CommandBuildingArmed(this, desc);
    if (pBuilding)
    {
        pBuilding->AddStaticToCollisionManager();
        pBuilding->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            pBuilding->BuildEffects();
        }
    }
    return pBuilding;
}
