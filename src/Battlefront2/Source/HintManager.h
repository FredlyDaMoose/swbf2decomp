#pragma once

#include <stddef.h>

#include "CollisionObject.h"
#include "Controllable.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "Reserve.h"

class GameObject;
class CommandPost;
class UnitController;

struct HintDesc
{
    PblMatrix mMatrix;
    unsigned int mNameId;
};
 

class BaseHint : public TreeGridObject
{
public:

    enum Type
    {
        HINT_UNDEFINED = 0,
        HINT_SNIPE = 1,
        HINT_PATROL = 2,
        HINT_COVER = 4,
        HINT_JETJUMP = 8,
        HINT_MINE = 16,
        HINT_LAND = 32,
        HINT_FORTIFICATION = 64,
        HINT_VEHICLECOVER = 128,
        HINT_TERMINATE = 256,
        HINT_TYPE_BITS = 8
    };

    Type GetType() const;

protected:

    BaseHint(const HintDesc& desc, Type eType);

public:

    static MemoryPool sMemoryPool;

    static void* operator new(unsigned int uSize);

    void Reserve(const GameObject* pObj) const;

    bool IsReserved(int iTeam) const;

    const PblVector3& GetZAxis() const;

    float GetReserveTime() const;

    enum Sidestep
    {
        STEP_LEFT = 0,
        STEP_RIGHT = 1
    };
    void Unreserve(const GameObject* pObj) const;
    bool HasSidestep() const;
    Sidestep GetRandomSidestep() const;

    ControllableHeight GetRandomPrimaryStance() const;

    ControllableHeight GetRandomSecondaryStance() const;

    void SetProperty(unsigned int uProperty, const char* pszValue);

    void SetCommandPost(const CommandPost* pPost);
    const CommandPost* GetCommandPost() const;

    PblMatrix BuildMatrix() const;
    void SetReserveDelay(const GameObject* pObj, float fDelay) const;

protected:

    ControllableHeight GetRandomStance(char cStanceMask) const;

public:
    ReserveObject mReserve;
    PblVector3 mForward;
    unsigned short mType : 8;
    unsigned short mPrimaryStance : 2;
    unsigned short mSecondaryStance : 2;
    unsigned short mSidestep : 2;
};
 
 
 

class JetJumpHint : public BaseHint
{
public:
    JetJumpHint(const HintDesc& desc);
};
 

class CoverHint : public BaseHint
{
public:
    CoverHint(const HintDesc& desc);
};
 

class MinefieldHint : public BaseHint
{
public:
    MinefieldHint(const HintDesc& desc);
};
 

class VehicleCoverHint : public BaseHint
{
public:
    VehicleCoverHint(const HintDesc& desc);
};
 

class PatrolHint : public BaseHint
{
public:
    PatrolHint(const HintDesc& desc);
};
 

class SnipeHint : public BaseHint
{
public:
    SnipeHint(const HintDesc& desc);
};
 

class LandHint : public BaseHint
{
public:
    LandHint(const HintDesc& desc);
};
 

class FortificationHint : public BaseHint
{
public:
    FortificationHint(const HintDesc& desc);
};
 

class TreeGrid;
struct TreeGridCallbackData;

extern TreeGrid* sHints;

namespace HintManager
{

void Initialize();

void AddHint(BaseHint* pHint);

int RequestPatrolHints(const UnitController* pController, const CommandPost* pPost,
                       int iMaxHints, const BaseHint** ppHints);

int RequestHintsCommon(int iType, const UnitController* pController,
                       const PblVector3& vPos, int iArg4, int iArg5,
                       const BaseHint** ppHints);

int RequestMinefieldHints(const UnitController* pController, const PblVector3& vPos,
                          int iArg3, int iArg4, const BaseHint** ppHints);

int RequestJetJumpHints(const UnitController* pController, const PblVector3& vPos,
                        int iArg3, int iArg4, const BaseHint** ppHints);

int RequestLandHints(const UnitController* pController, const PblVector3& vPos,
                     int iArg3, int iArg4, const BaseHint** ppHints, bool bGotoNonOwnedCP);

int RequestFortificationHints(const UnitController* pController, const PblVector3& vPos,
                              int iArg3, int iArg4, const BaseHint** ppHints);
int RequestCoverHints(const UnitController* pController, const PblVector3& vPos,
                      int iArg3, int iArg4, const BaseHint** ppHints);

int RequestVehicleCoverHints(const UnitController* pController, const PblVector3& vPos,
                             int iArg3, int iArg4, const BaseHint** ppHints);

int RequestSnipeHints(const UnitController* pController, const PblVector3& vPos,
                      int iArg3, int iArg4, const BaseHint** ppHints);

bool GenerateHintListCallback(const TreeGridObject* pObject, TreeGridCallbackData* pData, float& fUnused);

}
