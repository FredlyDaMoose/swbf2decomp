#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblListDouble.h"
#include "PebbleFL/Common/PblVector.h"
#include "ODFPropertySet.h"

class EntityFormation;
class PblMatrix;

class EntitySquadronClass
{
public:
    EntitySquadronClass();

    bool SetProperty(unsigned int uProperty, const char* pszValue);

    const EntityFormation* SelectFormation(unsigned int uNumSlots) const;

    bool FormationSupported(unsigned int uFormation) const;

    // GLOBAL: SWBF2 0x00789668
    static const ODFPropertySet::ODFProperty sPropertySet[1];

    unsigned int mFormations[10];
    unsigned int mNumFormations;
    float mSlideTime;
    unsigned int mMember;
};
 
 
 
 
 

// VTABLE: SWBF2 0x0079d4cc
class EntitySquadron
{
public:
    // SYNTHETIC: SWBF2 0x004fbe70
    // EntitySquadron::`scalar deleting destructor'

    EntitySquadron();
    virtual ~EntitySquadron();
    virtual const EntitySquadronClass* GetEntitySquadronClass() const = 0;
    virtual const PblMatrix& GetMatrix() const = 0;

    bool CreateSquadron(unsigned int uFormation);
    bool AddToSquadron(EntitySquadron* const pMember, int iSlot);
    void RemoveFromSquadron();
    void SetSquadronLeader(EntitySquadron* pLeader);
    bool IsSquadronLeader() const;
    bool InSquadron() const;
    EntitySquadron* GetSquadronLeader() const;
    int GetFormationSlot() const;
    const EntityFormation* GetFormation() const;

    void UpdatePosition(const EntitySquadronClass* const pClass, float fElapsed, const PblMatrix& matrix,
                        PblVector3& vPosition);

    EntityFormation* mFormation;
    int mFormationSlot;
    PblListDouble::Node mNode;
    EntitySquadron* mLeader;
    PblListDouble mList;
    float mTime;
    PblVector3 mOffset;

};
  
 
 
 
 
