#pragma once

#include <stddef.h>

#include "HUDElementGroup.h"
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class Character;
class FLInputManager;

namespace HUD {

class Objective
{
public:

    unsigned int GetTextHashID() const;

    enum ObjectiveState
    {
        INACTIVE = 0,
        NORMAL = 1,
        ACTIVE = 2,
        COMPLETED = 3,
    };

    // FUNCTION: SWBF2 0x00529c00
    ObjectiveState GetState() { return _state; }

    Objective();
    ~Objective();
    void SetState(ObjectiveState state);
    void SetText(unsigned int uiTextHashID);

    int _team;
    ObjectiveState _state;
    RedTextElement _textElement;
    unsigned int _textHashID;
    unsigned int _popupTextID;
};

class ObjectiveList : public HUD::ElementGroup
{
public:

    ObjectiveList(const char* const pName, Factory::ItemCallback pCallback, void* const pUserData);

    virtual ~ObjectiveList();

    void RemoveAllObjectives();

    virtual void Update(float fDeltaTime);

    // VTABLE: SWBF2 0x007a33b8

    class Factory : public HUD::ElementGroupBase::Factory
    {
    public:
        // FUNCTION: SWBF2 0x00565e90
        Factory();

        // SYNTHETIC: SWBF2 0x00565eb0
        // HUD::ObjectiveList::Factory::`scalar deleting destructor'

        virtual HUD::Item* CreateItem(const char* const pName, ItemCallback pCallback,
                                      void* const pUserData);
    };

    void BeginIter();
    Objective* GetCurrentIter();
    void IncrementIter();

    void AddObjective(unsigned int uiTextHashID, unsigned int uiPopupTextID, int iTeam);
    void SetState(unsigned int uiTextHashID, Objective::ObjectiveState state);
    Objective::ObjectiveState GetState(unsigned int uiTextHashID);
    Objective* FindObjective(unsigned int uiTextHashID);

protected:

    Character* GetOwner();
    friend class ::FLInputManager;

    void SetupText();

public:

    PblList<Objective> _objectives;
    PblList<Objective>::Itor _itor;
    int _viewportId;
    int _team;
    bool _bLastButtonState;
    float _lastWidth;
    float _lastHeight;
};
}

 
 
 
 
 
 
 
 
 
 
