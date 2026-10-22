#include "CmdInfo.h"
#include "CommandPost.h"
#include "GameObject.h"

void CommandPostHandle::operator=(CommandPost* pPost)
{
    mPost = pPost;
}

// FUNCTION: SWBF2 0x00450410
ConstCommandPostHandle::operator const CommandPost*() const
{
    if (mPost)
    {
        PblHandle<GameObject>& object = mPost->mObject;
        if (object.mObject &&
            object.mObject->mHandleId != object.mSavedHandleId)
        {
            object.mObject = 0;
            object.mSavedHandleId = 0;
        }
        if (!object.mObject)
        {
            return 0;
        }
    }
    return mPost;
}

// FUNCTION: SWBF2 0x0045cb40
CmdInfo::CmdInfo()
{
    mWho.mObject = 0;
    mWho.mSavedHandleId = 0;
    mOwner.mObject = 0;
    mOwner.mSavedHandleId = 0;
    mFlag.mObject = 0;
    mFlag.mSavedHandleId = 0;
    mWhat = 0;
    mPos = PblVector3(0, 0, 0);
    mWhoCommandPost = 0;
    mParam = 0;
}

// FUNCTION: SWBF2 0x0045cbe0
CmdInfo::CmdInfo(int iWhat, PblHandle<GameObject> who, const PblVector3& pos, PblHandle<GameObject> owner)
{
    mFlag.mObject = 0;
    mFlag.mSavedHandleId = 0;
    mWhat = iWhat;
    mWho = who;
    mOwner = owner;
    mWhoCommandPost = 0;
    mPos = pos;
    mParam = 0;
    mChr = 0;
}

// FUNCTION: SWBF2 0x0045cc70
CmdInfo::CmdInfo(int iWhat, ConstCommandPostHandle post)
{
    mWho.mObject = 0;
    mWho.mSavedHandleId = 0;
    mOwner.mObject = 0;
    mOwner.mSavedHandleId = 0;
    mFlag.mObject = 0;
    mFlag.mSavedHandleId = 0;
    mWhat = iWhat;
    mPos = PblVector3(0, 0, 0);
    mParam = 0;
    mChr = 0;
    mWhoCommandPost = const_cast<CommandPost*>(static_cast<const CommandPost*>(post));
}

// FUNCTION: SWBF2 0x0045cd40
CmdInfo::CmdInfo(const CmdInfo& other)
{
    *this = other;
}

// FUNCTION: SWBF2 0x0045cdd0
CmdInfo::~CmdInfo()
{
}

// FUNCTION: SWBF2 0x0045cde0
CmdInfo& CmdInfo::operator=(const CmdInfo& other)
{
    mWhat = other.mWhat;
    mWho = other.mWho;
    mOwner = other.mOwner;
    mWhoCommandPost = other.mWhoCommandPost;
    mPos = other.mPos;
    mParam = other.mParam;
    mChr = other.mChr;
    mFlag = other.mFlag;
    return *this;
}
