#include "Reserve.h"

#include "GameLoop.h"

// FUNCTION: SWBF2 0x0062f440
ReserveObject::ReserveObject() : mReserveTime(-10.0f)
{
}

// FUNCTION: SWBF2 0x0062f530
void ReserveObject::Unreserve(const GameObject*  )
{
    mReserver = PblHandle<const GameObject>();
    mReserveTime = -10.0f;
}

// FUNCTION: SWBF2 0x0062f4c0
void ReserveObject::SetReserveDelay(int  , float fDelay)
{
    mReserveTime = GameLoop::GetMissionTime() + fDelay;
}
