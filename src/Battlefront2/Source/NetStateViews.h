#pragma once

#include "EntityHologram.h"
#include "EntityBuildingArmed.h"
#include "CommandBuilding.h"

class NetEntityHologram : public EntityHologram
{
public:
    void MoveToClient();
};

class NetEntityBuildingArmed : public EntityBuildingArmed
{
public:
    void MoveToClient();
};

class NetCommandBuilding : public CommandBuilding
{
public:
    void MoveToClient();
};
