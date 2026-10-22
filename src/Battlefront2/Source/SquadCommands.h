#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"

class Character;
class GameObject;

namespace SquadCommands
{

enum SquadCommandType
{
    SC_DROPCOMMAND = 0,
    SC_DISPERSE = 1,
    SC_HOLD = 2,
    SC_FOLLOW = 3,
    SC_BOARDVEHICLE = 4,
    SC_EXITVEHICLE = 5,
    SC_COUNT = 6
};

class SquadCommandOrder
{
public:

    SquadCommandOrder(SquadCommandType eOrder, Character* pWhoGaveIt, PblHandle<GameObject> obj);

    SquadCommandOrder() : mWhoGaveIt(0) {}

    Character* mWhoGaveIt;
    SquadCommandType mWhatOrder;
    float mTime;
    PblHandle<GameObject> mGameObj;
};

bool IsSquadCommand(int iCmd);

void AIShoutOrder(SquadCommandOrder& order, float fDelay);
}

 
 
