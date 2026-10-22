#pragma once

#include "Thread.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class GameObject;
namespace Snd { class VoiceVirtualManager; }

class SoundDebugDisplay : public Thread
{
public:

    enum Mode
    {
        mode_Inactive = 0,
        mode_VirtualVoices = 1,
        mode_Sounds = 2,
        mode_ChaseObject = 3,
        mode_Music = 4,
        mode_Stream = 5,
        mode_VoiceChat = 6,
        mode_VoiceChatPackets = 7,
        mode_Space = 8,
        mode_MAX = 9
    };

    enum { NUM_TEXT_ELEMENTS = 28 };

    SoundDebugDisplay();
    virtual ~SoundDebugDisplay();
    void Init();
    virtual bool Update(float fDeltaTime);

    void SetMode(int iUnused);

private:
    void Clear();
    void Hide();
    void Printf(const char* pcFormat, ...);
    void DisplaySpaceInfo();
    void DisplayVirtualVoices(Snd::VoiceVirtualManager* pManager);
    void DisplayChaseInfo(float fDeltaTime);
    void DisplayMusicInfo();
    const char* GetMusicPlayStateString(unsigned int uiPlayState);
    const char* GetPlayStateStringShort(unsigned int uiPlayState);
    void DisplayStreamInfo();
    void DisplayVoiceChatInfo();

public:
    static SoundDebugDisplay* sInstance;

    RedScreenGroupElement mGroupElement;
    RedBitmapElement mBitmapElement;
    RedTextElement mTextElement[NUM_TEXT_ELEMENTS];
    Mode mMode;
    unsigned char mCursorPosY;
    GameObject* mGameObjectPrev;
    float mSpeedPrev;
    unsigned int mStreamID;
    unsigned int mSampleID;
};
