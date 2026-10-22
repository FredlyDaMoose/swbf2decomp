#include "CommandBuildingAnimatedArmed.h"
#include "GameModel.h"
#include "NetGame.h"

// FUNCTION: SWBF2 0x00476aa0
CommandBuildingAnimatedArmedClass::CommandBuildingAnimatedArmedClass(unsigned int uHash)
    : EntityBuildingAnimatedArmedClass(uHash), CommandPostClass(uHash)
{
}

// GLOBAL: SWBF2 0x01eb83c8
// SYNTHETIC: SWBF2 0x00402640
// `dynamic initializer for 'CommandBuildingAnimatedArmed::rttiHashCommandBuildingAnimatedArmed''
PblHash CommandBuildingAnimatedArmed::rttiHashCommandBuildingAnimatedArmed("CommandBuildingAnimatedArmed");
char CommandBuildingAnimatedArmed::rttiNameCommandBuildingAnimatedArmed[] = "CommandBuildingAnimatedArmed";

// FUNCTION: SWBF2 0x004767d0
bool CommandBuildingAnimatedArmed::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashCommandBuildingAnimatedArmed ||
           uRtti == EntityBuildingAnimatedArmed::rttiHashEntityBuildingAnimatedArmed ||
           uRtti == EntityBuildingAnimated::rttiHashEntityBuildingAnimated ||
           uRtti == EntityBuilding::rttiHashEntityBuilding ||
           uRtti == GameObject::rttiHashGameObject ||
           uRtti == EntityGeometry::rttiHashEntityGeometry ||
           uRtti == EntityEx::rttiHashEntityEx ||
           uRtti == Entity::rttiHashEntity;
}

// FUNCTION: SWBF2 0x00476830
unsigned int CommandBuildingAnimatedArmed::GetDerivedRtti() const
{
    return rttiHashCommandBuildingAnimatedArmed;
}

// FUNCTION: SWBF2 0x00476840
const char* CommandBuildingAnimatedArmed::GetDerivedRttiName() const
{
    return rttiNameCommandBuildingAnimatedArmed;
}

// GLOBAL: SWBF2 0x01eb83cc
// SYNTHETIC: SWBF2 0x00402650
// `dynamic initializer for 'CommandBuildingAnimatedArmedClass::rttiHashCommandBuildingAnimatedArmedClass''
PblHash CommandBuildingAnimatedArmedClass::rttiHashCommandBuildingAnimatedArmedClass("CommandBuildingAnimatedArmedClass");
char CommandBuildingAnimatedArmedClass::rttiNameCommandBuildingAnimatedArmedClass[] = "CommandBuildingAnimatedArmedClass";

// FUNCTION: SWBF2 0x00476890
bool CommandBuildingAnimatedArmedClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashCommandBuildingAnimatedArmedClass ||
           EntityBuildingAnimatedArmedClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004768e0
unsigned int CommandBuildingAnimatedArmedClass::GetDerivedRtti()
{
    return rttiHashCommandBuildingAnimatedArmedClass;
}

// FUNCTION: SWBF2 0x004768f0
const char* CommandBuildingAnimatedArmedClass::GetDerivedRttiName()
{
    return rttiNameCommandBuildingAnimatedArmedClass;
}

// SYNTHETIC: SWBF2 0x00476b00
// CommandBuildingAnimatedArmedClass::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00476d93
// [thunk]:CommandBuildingAnimatedArmedClass::`scalar deleting destructor' adjustor{1712}

// FUNCTION: SWBF2 0x00476b30
CommandBuildingAnimatedArmedClass::CommandBuildingAnimatedArmedClass(CommandBuildingAnimatedArmedClass* pBase,
                                                                     unsigned int uHash)
    : EntityBuildingAnimatedArmedClass(pBase, uHash), CommandPostClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x00476bb0
CommandBuildingAnimatedArmedClass::~CommandBuildingAnimatedArmedClass()
{
}

// FUNCTION: SWBF2 0x00476c10
void CommandBuildingAnimatedArmedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityBuildingAnimatedArmedClass::SetProperty(uProperty, pszValue);
    CommandPostClass::SetProperty(uProperty, pszValue, mModel ? mModel->GetRedModel(0) : 0, 0);
}

// FUNCTION: SWBF2 0x00476c50
EntityClass* CommandBuildingAnimatedArmedClass::Derive(unsigned int uHash)
{
    return new CommandBuildingAnimatedArmedClass(this, uHash);
}

// FUNCTION: SWBF2 0x00476850
EntityType CommandBuildingAnimatedArmed::GetEntityType() const
{
    return ENT_POST;
}

// FUNCTION: SWBF2 0x00476900
CommandBuildingAnimatedArmed::CommandBuildingAnimatedArmed(CommandBuildingAnimatedArmedClass* pClass,
                                                           const EntityDesc& desc)
    : EntityBuildingAnimatedArmed(pClass, desc)
{
    mClass = pClass;
    mPost = CommandPost::BuildPost(this, pClass);
}

// FUNCTION: SWBF2 0x004769a0
CommandBuildingAnimatedArmed::~CommandBuildingAnimatedArmed()
{
}

// SYNTHETIC: SWBF2 0x00476860
// CommandBuildingAnimatedArmed::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00476d6a
// [thunk]:CommandBuildingAnimatedArmed::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x00476d75
// [thunk]:CommandBuildingAnimatedArmed::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x00476d7d
// [thunk]:CommandBuildingAnimatedArmed::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x00476d88
// [thunk]:CommandBuildingAnimatedArmed::`scalar deleting destructor' adjustor{608}
// SYNTHETIC: SWBF2 0x00476d9e
// [thunk]:CommandBuildingAnimatedArmed::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x004769e0
void CommandBuildingAnimatedArmed::Init()
{
    EntityBuildingAnimatedArmed::Init();
    if (mState == STATE_ALIVE)
    {
        mPost->Respawn();
    }
}

// FUNCTION: SWBF2 0x00476a00
void CommandBuildingAnimatedArmed::Kill()
{
    EntityBuildingAnimatedArmed::Kill();
    mPost->Kill();
}

// FUNCTION: SWBF2 0x00476a20
void CommandBuildingAnimatedArmed::Respawn()
{
    EntityBuildingAnimatedArmed::Respawn();
    mPost->Respawn();
}

// FUNCTION: SWBF2 0x00476a40
void CommandBuildingAnimatedArmed::SetProperty(unsigned int uProperty, const char* pszValue)
{
    GameObject::SetProperty(uProperty, pszValue);
    mPost->SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00476a70
void CommandBuildingAnimatedArmed::SetTeam(int iTeam)
{
    int iOldTeam = mTeam;
    GameObject::SetTeam(iTeam);
    mPost->SetTeam(iOldTeam, iTeam);
}

// FUNCTION: SWBF2 0x00476cc0
Entity* CommandBuildingAnimatedArmedClass::Build(const EntityDesc& desc)
{
    CommandBuildingAnimatedArmed* pBuilding = new CommandBuildingAnimatedArmed(this, desc);
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
