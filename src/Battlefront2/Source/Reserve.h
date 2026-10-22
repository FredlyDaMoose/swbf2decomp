#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"

class GameObject;

class ReserveObject
{
public:

    ReserveObject();

    bool IsReserved(int iTeam, float fDelay) const;
    float GetReserveTime(int iTeam) const;
    void Reserve(const GameObject* pObj);

    void Unreserve(const GameObject* pObj);

    void SetReserveDelay(int iTeam, float fDelay);

    PblHandle<const GameObject> mReserver;
    float mReserveTime;
};
 
 
