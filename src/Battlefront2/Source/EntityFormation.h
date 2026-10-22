#pragma once

#include <stddef.h>
#include "GamePath.h"

class PblMatrix;

// VTABLE: SWBF2 0x0079b5cc
class EntityFormation : public GamePath
{
public:

    struct NodeCallbackData
    {
        EntityFormation* formation;
        unsigned int rootNodeIndex;
    };

    EntityFormation();
    virtual ~EntityFormation();
    virtual bool Read(PblConfig& config);

    void CalculatePoint(int i, const PblMatrix& matrix, PblVector3& point) const;
    int GetNumSlots() const;
    int GetRootIndex() const;
    bool RootIsSlot() const;
    bool MemberIsSupported(unsigned int uMember, int iSlot) const;

    static bool ReadPropertyScope(PblConfig& config, PblConfig::Data& data,
                                  PathObject object, PathObjectType type, void* pUserData);
    static bool ReadNodePropertyScope(PblConfig& config, PblConfig::Data& data,
                                      PathObject object, PathObjectType type, void* pUserData);

    static const EntityFormation* Find(unsigned int uName);
    static void DestroyAll();

    static PblList<EntityFormation> sList;

    int mRootIndex;
    bool mRootIsSlot : 1;
    unsigned int* mNodeMembers;
};
 
 
 

// VTABLE: SWBF2 0x0079fb94
class EntityFormationFactory : public GamePathFactory
{
public:
    EntityFormationFactory() : GamePathFactory(0xB7B616EC) {}

    // SYNTHETIC: SWBF2 0x005394e0
    // EntityFormationFactory::`scalar deleting destructor'

    virtual GamePath* CreatePath() const;
};
  
