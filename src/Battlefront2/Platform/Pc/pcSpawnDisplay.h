#pragma once

#include <stddef.h>
#include "Battlefront2/Source/DisplayDroideka.h"
#include "Battlefront2/Source/DisplaySoldier.h"
#include "Battlefront2/Source/GameSound.h"
#include "Battlefront2/Source/Thread.h"
#include "PebbleFL/Common/PblHandle.h"
#include "RedEngineFL/Interface/Red3DModelElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class Character;
class CommandPost;
class EntityClass;
class GameObject;
class GameObjectClass;
class GuiManager;
class NetPktGroup;
class Red3DModelElement;
class RedBitmapElement;
class RedGroupElement;
class RedInterfaceElement;

// VTABLE: SWBF2 0x0079663c
class SpawnDisplay : public Thread
{
public:

    enum Mode
    {
        eSIDESELECT = 0,
        eBRIEFING = 1,
        eSPAWNDISPLAY = 2,
        eCHANGECLASS = 3,
        eFINISHED = 4
    };

    SpawnDisplay(int iIndex);
    virtual ~SpawnDisplay();
    virtual bool Update(float fDeltaTime);

    static void Create();
    static void CreateClient();
    static void Destroy();
    static void Init();

    static void __fastcall EnableViewport(int iViewport);

    static SpawnDisplay* Instance(int iIndex);

    // FUNCTION: SWBF2 0x0053c020
    bool IsSelectingTeam() { return mMode == eSIDESELECT; }

    void Show(Character* pCharacter);
    void Hide(Character* pCharacter);
    void ChangeClass(Character* pCharacter);
    void Write(NetPktGroup& group, int iPlayer);

    void SelectTeam(int iTeam);
    void SelectClass(int iClass);
    void SelectPost(CommandPost* pPost);
    CommandPost* GetCommandPost();
    void SetCommandPost(CommandPost* pPost);
    void Initialize();

    bool IsSpectateMode() const { return mSpectateMode; }
    bool GetOptionsAccepted() { return !mVisible || mMode == eFINISHED; }

protected:
    void UpdateUnitCountText();
    void UpdateObjectText(GameObjectClass* pClass, int iSlot);
    void UpdateTimer();
    bool CanShowHeroes();
    void ChangeMode(Mode eMode);
    void SetupSlots();
    void SetSlotInfo(EntityClass* pClass, int iSlot);
    void SetEntityElement(EntityClass* pClass);
    void UpdateSpectate(Character* pCharacter);
    int GetTeamSelection();
    void UpdateSideSelectText(int iScreen);
    void AcceptSideSelect();

    void UpdateInput();

    CommandPost* FindPost(float fX, float fY);
public:

    void SendSpawnCancelMessage();
    void SendSpawnReadyMessage();
    void Read(NetPktGroup& group);
protected:

public:

    static float sInGameBusGain;
    static float sInGameBusFadeTime;
    static unsigned int sNumLiveHumansTeam1;
    static unsigned int sNumLiveHumansTeam2;
    static GuiManager* m_pGuiManager;

    Mode mMode;
    RedTextElement mTextReady;
    RedTextElement mTextSpawnTimer;
    RedScreenGroupElement mGroupMainTL;
    RedScreenGroupElement mGroupMainTC;
    RedScreenGroupElement mGroupMainCC;
    RedScreenGroupElement mGroupMainCB;
    RedScreenGroupElement mGroupTimer;
    RedScreenGroupElement mGroupMainNoSafeCC;
    RedTextElement* m_pInfoBoxText[10];
    RedTextElement* m_pUnitNameText[10];
    RedBitmapElement* m_pClassIcon[10];
    RedGroupElement* m_pSlotBorder[10];
    RedTextElement* m_pUnitCountText;
    RedBitmapElement* m_pSideIcon[2];
    RedGroupElement* m_pSideBG[2];
    Red3DModelElement* m_pSideModel[2];
    RedBitmapElement* m_AcceptBox;
    RedTextElement* m_pPopUpText;
    RedTextElement* m_pPopUpTitle;
    RedGroupElement* m_pPopUpBorder;
    EntityElement* m_pEntityElement;
    DroidekaElement m_DroidekaElement;
    SoldierElement m_SoldierElement;
    RedBitmapElement* m_SelectedIcon;
    RedInterfaceElement* m_pBackButton;
    Red3DModelElement mBackdropModel;
    RedInterfaceElement* m_pBackButton1;
    RedInterfaceElement* m_pButton_ChangeView;
    RedInterfaceElement* m_pButton_JoinGame;
    Character* mOwner;
    CommandPost* mSpawn;
    int mClassCount;
    bool mCanRender;
    bool mStickMoved;
    bool mVisible;
    bool mFirstShow;
    float mVisibleTimer;
    bool mNetWaitingForTeam;
    float mTeamDisplayRot;
    float mMouseOverTime;
    int mPlayerIdx;
    int mJoystickIndex;
    bool mLastAcceptPressed;
    bool mLastBackPressed;
    bool mDPadPressed;
    float mLastSpawnTime;
    bool mWasOwnerAliveLastUpdate;
    GameObjectClass* mLastObjectClass;
    EntityClass* mLastInfoClass;
    EntityClass* mLastUnitCountClass;
    unsigned int mBaseTurn;
    float mBaseCycleTime;
    bool mSpectateMode;
    GameSound mUnitChange;
    GameSound mUnitAccept;
    GameSound mSpawnPointChange;
    GameSound mSpawnPointAccept;
    GameSound mBack;
    bool mSpawnReady;
    int mTeamNumber;
    CommandPost* mCommandPost;
    PblHandle<GameObject> mCommandObject;
    int mClassIndex;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
