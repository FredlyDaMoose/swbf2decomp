#pragma once

#include <stddef.h>
#include "GamePath.h"

class AIPath;
class GameObject;
class PblVector3;

namespace AI {
// VTABLE: SWBF2 0x007ad640
class PatrolPath : public GamePath
{
public:

    class NodeProperties
    {
    public:

        // FUNCTION: SWBF2 0x006156a0
        NodeProperties() : mWaitMin(0.0f), mWaitMax(0.0f) {}

        static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                      RedPath::PathObject object,
                                      RedPath::PathObjectType type, void* pUserData);

        float mWaitMin;
        float mWaitMax;
    };

    PatrolPath();
    virtual ~PatrolPath();

    // SYNTHETIC: SWBF2 0x006157b0
    // AI::PatrolPath::`scalar deleting destructor'

    virtual bool Read(PblConfig& config);

    const PblVector3* GetPointPosition(int iIndex) const;
    const NodeProperties& GetNodeProperties(unsigned int uIndex) const;
    void SetReservingObject(const GameObject* pObj);
    const GameObject* GetReservingObject();
    AIPath* GetAIPath();

    static int RequestPatrolPaths(const PblVector3& pos, float fRadius, int iMax,
                                  PatrolPath** ppPaths);

protected:

    static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                  RedPath::PathObject object,
                                  RedPath::PathObjectType type, void* pUserData);

public:

    static PblList<PatrolPath> sList;
    static void DestroyAll();

    NodeProperties* mNodeProps;
    char mReservingObject[8];
    float mReservingTime;
};
}
 
 
 
 
 
 

namespace AI {
// VTABLE: SWBF2 0x0079fbd0
class PatrolPathFactory : public GamePathFactory
{
public:
    PatrolPathFactory() : GamePathFactory(0xD4966D93) {}

    // SYNTHETIC: SWBF2 0x00539210
    // AI::PatrolPathFactory::`scalar deleting destructor'

    virtual GamePath* CreatePath() const;
};
}
  
