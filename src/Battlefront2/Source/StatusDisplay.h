#pragma once

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class Character;
class PblVector3;
class StatusDisplay;
class Weapon;

class DamageArrowGroup
{
public:
    struct Arrow
    {

        Arrow();

        RedScreenGroupElement mGroup;
        RedBitmapElement mBitmap;
        float mAlpha;
    };

    DamageArrowGroup();
    ~DamageArrowGroup();
    void Init();
    void Bind(unsigned int hView);
    void Enable(bool bEnable);
    void Update(float fDeltaTime);
    void SetDirection(const PblVector3& vDir);

    struct Data
    {
        float mRelativeX;
        float mRelativeY;
        float mPosX;
        float mPosY;
        float mRot;
    };

    static Data sData[4];

    static const float sWidth;
    static const float sHeight;

    Arrow mArrows[4];
};

class OutOfBoundsGroup : public RedScreenGroupElement
{
public:
    // FUNCTION: SWBF2 0x00650fd0
    OutOfBoundsGroup(StatusDisplay* pParent);

    // SYNTHETIC: SWBF2 0x006510a0
    // OutOfBoundsGroup::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006510d0
    virtual ~OutOfBoundsGroup();

    void Init();
    void Bind(unsigned int hView);
    void Unbind();
    void Update(float fDeltaTime);

    StatusDisplay* mParent;
    RedTextElement mText;
    float mAlpha;
    float mDAlpha;

    static const float sOmegaSquared;
};

class SquadCommandGroup : public RedScreenGroupElement
{
public:
    struct Command
    {

        Command();

        RedTextElement mText;
    };

    struct CommandText
    {
        float mPosX;
        float mPosY;
        RedTextElement::TextHAlignT mHAlign;
        RedTextElement::TextVAlignT mVAlign;
    };

    SquadCommandGroup();
    virtual ~SquadCommandGroup();
    static void Create();
    static void Destroy();
    void Init();
    void Bind(unsigned int hView);
    void Unbind();
    void Update(float fDeltaTime);
    void SetCommand(int iContext, int iOrder);

    float mAlpha;
    Command mCommands[4];

    static float sRelativeX;
    static float sRelativeY;
    static float sFadeSpeed;

    static const float sPosX;
    static const float sPosY;

    static CommandText sTexts[4];

    static char* sContexts[4];
    static char* sOrders[4];
    static unsigned short* sCommands[4][4];
};

class VoiceChatGroup : public RedScreenGroupElement
{
public:
    struct Line
    {
        // SYNTHETIC: SWBF2 0x00651460
        // VoiceChatGroup::Line::Line
        // SYNTHETIC: SWBF2 0x00651470
        // VoiceChatGroup::Line::~Line

        int mId;
        RedTextElement mText;
    };

    // FUNCTION: SWBF2 0x00651340
    VoiceChatGroup(StatusDisplay* pParent);

    // SYNTHETIC: SWBF2 0x00651430
    // VoiceChatGroup::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x00651480
    virtual ~VoiceChatGroup();

    void Init();
    void Bind(unsigned int hView);
    void Unbind();
    void Update(float fDeltaTime);

private:
    void AddPlayers();
    void UpdateLines();

public:
    StatusDisplay* mParent;
    Line mLines[4];
};

class StatusDisplay : public Thread
{
public:
    // FUNCTION: SWBF2 0x00651a00
    StatusDisplay();

    // SYNTHETIC: SWBF2 0x00651ab0
    // StatusDisplay::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x00651ae0
    virtual ~StatusDisplay();

    static void Create();
    static void Destroy();
    static void EnableViewport(unsigned int uiViewport, bool bEnable);

    static void InitAll();

    void Init();
    void Bind(unsigned int uiViewport);
    void Unbind();
    void Show(Character* pCharacter);
    void Hide(Character* pCharacter);

    virtual bool Update(float fDeltaTime);

    int mJoystickIndex;
    unsigned int mViewport;
    bool mCanRender;
    RedBitmapElement mBitmapFrame;
    OutOfBoundsGroup mOutOfBoundsGroup;
    DamageArrowGroup mDamageArrowGroup;
    SquadCommandGroup mSquadCommandGroup;
    VoiceChatGroup mVoiceChatGroup;
    float mChargeBlinkTimer;
    Character* mOwner;
    bool mVisible;

    static StatusDisplay* Instance(unsigned int uIndex) { return uIndex < 1 ? sInstance[uIndex] : 0; }
    void SetDamageDirection(const PblVector3& vDir) { mDamageArrowGroup.SetDirection(vDir); }

    void SetRepairWeapon(const Weapon*) {}

    static StatusDisplay* sInstance[1];
    static bool mDisable;
};

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
