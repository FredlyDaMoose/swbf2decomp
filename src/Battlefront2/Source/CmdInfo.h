#pragma once

#include "CommandPost.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"

class Character;
class GameObject;

class CmdInfo
{
public:
    CmdInfo();
    CmdInfo(const CmdInfo& other);
    ~CmdInfo();
    CmdInfo& operator=(const CmdInfo& other);

protected:

    CmdInfo(int iWhat, PblHandle<GameObject> who, const PblVector3& pos, PblHandle<GameObject> owner);
    CmdInfo(int iWhat, ConstCommandPostHandle post);

public:

    int GetCmdType() const { return mWhat; }
    int GetParam() const { return mParam; }

    int mWhat;
    PblHandle<GameObject> mWho;
    PblHandle<GameObject> mOwner;
    CommandPostHandle mWhoCommandPost;
    Character* mChr;
    PblHandle<const GameObject> mFlag;
    PblVector3 mPos;
    int mParam;
};
