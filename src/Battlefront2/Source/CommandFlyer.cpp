#include "CommandFlyer.h"
#include "CollisionManager.h"
#include "GameModel.h"
#include "NetGame.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

// GLOBAL: SWBF2 0x01eb9648
// SYNTHETIC: SWBF2 0x00402680
// `dynamic initializer for 'CommandFlyer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766e80
// `dynamic atexit destructor for 'CommandFlyer::sMemoryPool''
MemoryPool CommandFlyer::sMemoryPool("CommandFlyer", 0x1da0);

// GLOBAL: SWBF2 0x01eb9698
// SYNTHETIC: SWBF2 0x004026a0
// `dynamic initializer for 'CommandFlyer::rttiHashCommandFlyer''
PblHash CommandFlyer::rttiHashCommandFlyer("CommandFlyer");
char CommandFlyer::rttiNameCommandFlyer[] = "CommandFlyer";

// FUNCTION: SWBF2 0x00478040
CommandFlyerClass::CommandFlyerClass(unsigned int uHash)
    : EntityFlyerClass(uHash), CommandPostClass(uHash)
{
    mIsNeutralWhenEmpty = false;
}

// GLOBAL: SWBF2 0x01eb969c
// SYNTHETIC: SWBF2 0x004026b0
// `dynamic initializer for 'CommandFlyerClass::rttiHashCommandFlyerClass''
PblHash CommandFlyerClass::rttiHashCommandFlyerClass("CommandFlyerClass");
char CommandFlyerClass::rttiNameCommandFlyerClass[] = "CommandFlyerClass";

// FUNCTION: SWBF2 0x00477c30
bool CommandFlyerClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashCommandFlyerClass || EntityFlyerClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00477c80
unsigned int CommandFlyerClass::GetDerivedRtti()
{
    return rttiHashCommandFlyerClass;
}

// FUNCTION: SWBF2 0x00477c90
const char* CommandFlyerClass::GetDerivedRttiName()
{
    return rttiNameCommandFlyerClass;
}

// FUNCTION: SWBF2 0x00477c70
unsigned int GetFlyerClassRtti()
{
    return CommandFlyerClass::rttiHashCommandFlyerClass;
}

// FUNCTION: SWBF2 0x004780e0
CommandFlyerClass::CommandFlyerClass(CommandFlyerClass* pBase, unsigned int uHash)
    : EntityFlyerClass(pBase, uHash), CommandPostClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x004781c0
void CommandFlyerClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    EntityFlyerClass::SetProperty(uProperty, pszValue);
    CommandPostClass::SetProperty(uProperty, pszValue, mModel ? mModel->_mLod[0] : 0, 0);
}

// FUNCTION: SWBF2 0x00478210
EntityClass* CommandFlyerClass::Derive(unsigned int uHash)
{
    return new CommandFlyerClass(this, uHash);
}

// FUNCTION: SWBF2 0x00478280
Entity* CommandFlyerClass::Build(const EntityDesc& desc)
{
    if (!mFlyingCollisionInited)
    {
        mFlyingCollisionModel = mCollisionModel;
        CollisionMask mask = mCollisionModel.GetCollisionMask(COLL_ORDNANCE);
        CollisionMask special = mCollisionModel.GetCollisionMask(COLL_SPECIAL);
        for (int i = 0; i < 64; ++i)
        {
            if (special.IsSet(i))
            {
                mask.SetId(i);
            }
        }
        mFlyingCollisionModel.SetCollisionMask(COLL_ORDNANCE, mask);
        mFlyingCollisionInited = true;
    }

    if (CommandFlyer::sMemoryPool.IsFull())
    {
        return 0;
    }

    CommandFlyer* pFlyer = new CommandFlyer(this, desc);
    if (pFlyer)
    {
        pFlyer->AddSoftToCollisionManager();
        pFlyer->ActivatePhysics();
        pFlyer->BuildEffects();
    }
    return pFlyer;
}

// FUNCTION: SWBF2 0x00477b70
bool CommandFlyer::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashCommandFlyer || EntityFlyer::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00477bc0
unsigned int CommandFlyer::GetDerivedRtti() const
{
    return rttiHashCommandFlyer;
}

// FUNCTION: SWBF2 0x00477bd0
const char* CommandFlyer::GetDerivedRttiName() const
{
    return "CommandFlyer";
}

// FUNCTION: SWBF2 0x00477bf0
EntityType CommandFlyer::GetEntityType() const
{
    return ENT_POST_FLYER;
}

// SYNTHETIC: SWBF2 0x00477c00
// CommandFlyer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00477ca0
CommandFlyer::CommandFlyer(CommandFlyerClass* pClass, const EntityDesc& desc)
    : EntityFlyer(pClass, desc)
{
    mClass = pClass;
    mbHasTakenOffBefore = false;
    mPost = CommandPost::BuildPost(this, pClass);
    mPost->mIsAISpawnPoint = true;
}

// FUNCTION: SWBF2 0x00477d60
CommandFlyer::~CommandFlyer()
{
    if (mPost)
    {
        mPost->Kill();
    }
}

// FUNCTION: SWBF2 0x00477e00
void CommandFlyer::Init()
{
    EntityFlyer::Init();
    mPost->Respawn();
}

// FUNCTION: SWBF2 0x00477e20
void CommandFlyer::Kill()
{
    EntityFlyer::Kill();
    mPost->Kill();
}

// FUNCTION: SWBF2 0x00477e40
void CommandFlyer::Respawn()
{
    mPost->Respawn();
    GameObject::Respawn();
}

// FUNCTION: SWBF2 0x00477e60
void CommandFlyer::SetProperty(unsigned int uProperty, const char* pszValue)
{
    GameObject::SetProperty(uProperty, pszValue);
    mPost->SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00477e90
bool CommandFlyer::Update(float fDeltaTime)
{
    if (IsLanded())
    {
        if (!mPost->mbIsEnabled)
        {
            EnablePost(true);
        }
    }
    else if (mPost->mbIsEnabled)
    {
        EnablePost(false);
    }
    return EntityFlyer::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00477ee0
void CommandFlyer::SetTeam(int iTeam)
{
    int iOldTeam = mTeam;
    GameObject::SetTeam(iTeam);
    mPost->SetTeam(iOldTeam, iTeam);
}

// FUNCTION: SWBF2 0x00477fe0
void CommandFlyer::EnablePost(bool bEnable)
{
    mPost->mbIsEnabled = bEnable;
    mPost->mIsAISpawnPoint = bEnable;
    if (!bEnable)
    {
        mbHasTakenOffBefore = true;
    }
}
