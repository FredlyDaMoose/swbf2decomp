#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/Red3DModelElement.h"
#include "RedEngineFL/Interface/RedBorderElement.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class GameSound;
class PblFileChunk;
class RedBitmapElement;
class RedCamera;
class RedModel;
class RedSkeleton;
class RedTexture;

class LoadDisplay
{
public:
    enum LoadStateT
    {
        LOAD_STATE_LOADING = 0,
        LOAD_STATE_FINISHED = 1
    };

    class IDDatabase
    {
    public:
        struct IDType
        {
            unsigned int parent;
            unsigned int id;
        };

        IDDatabase();

        ~IDDatabase();

        void Initialize();

        void AddID(unsigned int uiParent, unsigned int uiID);

        void RemoveAll();

        unsigned int GetRandomID();

        IDType m_loadedIDs[200];
        unsigned int m_numLoadedIDs;
        IDType m_usedIDs[200];
        unsigned int m_usedLoadedIDsStartIndex;
        unsigned int m_usedLoadedIDsEndIndex;
        PblRandom rand;
    };

    class ProgressIndicator : public RedGroupElement
    {
    public:

        enum LEDModeT
        {
            LED_MODE_TIMER = 0,
            LED_MODE_WRAP = 1,
            LED_MODE_OSCILLATE = 2,
            LED_MODE_RANDOM = 3
        };

        ProgressIndicator();

        virtual ~ProgressIndicator();

        void SetLEDSize(float fWidth, float fHeight)
        {
            m_LEDWidth = fWidth;
            m_LEDHeight = fHeight;
        }

        void Init(int iNumLEDs, float fSpacing, const unsigned int* puiTextureHashes,
                  int iNumTextures, const RedColor* pColors, int iNumColors, float fSpeed,
                  LEDModeT eMode);

        void SetAllOn();

        void Update(float fDelta, GameSound* const pSound, unsigned int uiSoundHash);

        RedBitmapElement* m_pLED;
        float* m_pIntensity;
        RedColor* m_pColor;
        int m_curOnLED;
        int m_progressBarDirection;
        unsigned int m_LEDTexHash;
        float m_width;
        int m_numLEDs;
        float m_OnLEDFraction;
        float m_LEDSpeed;
        float m_LEDWidth;
        float m_LEDHeight;
        LEDModeT m_mode;
    };

    static void Create();

    LoadDisplay();

    static void Destroy();

    ~LoadDisplay();

    static LoadDisplay* Instance() { return sInstance; }

    void Begin();

    void End();

    void Update();

    LoadStateT GetState() const;

    static LoadDisplay* sInstance;

    static IDDatabase sTipsDatabase;
    static IDDatabase sLVLDatabase;

private:

    void SetTextElementDefaults(RedTextElement& element);
    void BuildCampaignLayout();
    void BuildLoadingTips();
    void BuildSimpleLoadingBar();
    void PostLoad();
    void SetLoadState();
    void Render();
    void RenderScreen() const;
    void DeleteData();
    void LoadData();

    unsigned int LoadRandomLVLChunk(const char* pcFileName, IDDatabase& database);

    void LoadDataFile(const char* pcFileName);
    void LoadDataChunk(PblFileChunk& chunk);
    void LoadConfig(PblFileChunk& chunk);

    void PlatformRender();

    void PlatformRenderTexture(unsigned int uiTextureHash, float x0, float y0, float x1, float y1,
                               const RedColor& color, bool bFlag, float u0, float v0, float u1,
                               float v1, float uScale, float vScale, float uOffset,
                               float vOffset) const;

public:
    bool m_bDisplay;
    bool m_campaignLayout;
    unsigned int m_missionHash;
    unsigned int m_entireMissionHash;
    unsigned int m_campaignNameHash;
    unsigned int m_missionNameHash;
    unsigned int m_modeNameHash;
    int m_team1Num;
    int m_team2Num;
    char m_entireMissionFileName[64];
    char m_missionFileName[64];
    char m_randomBackDropFileName[64];
    unsigned int m_teamModelHash[4];
    RedColor m_ambientColor;
    PblVector3 m_sunDirection;
    RedColor m_sunColor;
    bool m_firstUpdate;
    LoadStateT m_loadState;
    float m_teamModelOmega;
    float m_loadingTextBlinkRate;
    RedColor m_loadingColorPallete[16];
    int m_loadingColorPalleteSize;
    RedColor m_LEDColor;
    unsigned int m_LEDTextureHash;
    float m_totalProgressTime;
    float m_LEDHeight;
    float m_LEDWidth;
    float m_LEDSpacing;
    unsigned int m_numLEDs;
    RedCamera* m_redCamera;
    PblMatrix m_cameraMat;
    RedInterfaceScreen m_loadingScreen;
    RedTextElement m_textLoading;
    RedTextElement m_textMissionName;
    RedTextElement m_textModeName;
    Red3DModelElement m_modelTeamIcon[4];
    RedScreenGroupElement m_groupLoadingTips;
    RedTextElement m_textLevelTips;
    RedBorderElement m_tipsBox;
    ProgressIndicator m_progressBar;
    RedScreenGroupElement m_groupTopLeft;
    RedScreenGroupElement m_groupTopRight;
    RedScreenGroupElement m_groupBottomLeft;
    RedScreenGroupElement m_groupBottomRight;
    RedTextElement m_textCampaignTitle;
    RedTextElement m_textCampaignBriefing;
    RedScreenGroupElement m_groupCampaign;
    RedScreenGroupElement m_groupTitleBar;
    RedTextElement m_textBuildNameNumber;
    RedTextElement m_textBuildTime;
    unsigned int m_backDropHash;
    bool m_randomBackDrop;
    float m_timePerTip;
    float m_tipTimer;
    float m_totalTime;
    float m_curTime;
    float m_titleBarTimer;
    RedModel* m_models[10];
    RedTexture* m_textures[50];
    RedSkeleton* m_skeletons[10];
    int m_numModels;
    int m_numTextures;
    int m_numSkeletons;
public:
    static int m_ShowCount;
};

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
