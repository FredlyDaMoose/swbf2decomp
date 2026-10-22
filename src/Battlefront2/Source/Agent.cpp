#include "Agent.h"
#include "PebbleFL/Common/PblHashTable.h"

// GLOBAL: SWBF2 0x01eafdfc
// SYNTHETIC: SWBF2 0x00402190
// `dynamic initializer for 'Agent::rttiHashAgent''
PblHash Agent::rttiHashAgent("Agent");
char Agent::rttiNameAgent[] = "Agent";

// FUNCTION: SWBF2 0x004384c0
bool Agent::Update(float dt, bool b)
{
    FSMUpdate(dt, b);
    return mCurState == 100;
}
