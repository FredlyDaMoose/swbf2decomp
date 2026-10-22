#include <stdio.h>

#include "QOSDisplay.h"
#include "GameLoop.h"
#include "NetComm.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Util/RedCommandConsole.h"

class RedConsole;

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
void RemoveInterfaceElement(RedScreenGroupElement* pElement);
}

extern int netCurMaxPlayers;
extern float netSecondsPerTurn;
extern unsigned int clientTurnNumber;
extern int netTurnsPerSecond;
extern int clientAhead;
extern int clientHostTurnNumber;
extern int clientUpdateCount;
extern bool netIsClient;

// GLOBAL: SWBF2 0x01eaeff6

bool QosEnable;
// GLOBAL: SWBF2 0x01eaeff5

bool QosTextEnable;
// GLOBAL: SWBF2 0x007e903b
unsigned char QosIconAlpha = 0x80;

// GLOBAL: SWBF2 0x007e9050
int QosValue[3][2] = { { 4, 8 }, { 6, 6 }, { 8, 4 } };
// GLOBAL: SWBF2 0x0078b084

const int QosValue20[3] = { 8, 6, 4 };

// GLOBAL: SWBF2 0x01faa210
QOSDisplayThread* displayThread;

// GLOBAL: SWBF2 0x01f39d90
// SYNTHETIC: SWBF2 0x00406100
// `dynamic initializer for 'msgGroup''
// SYNTHETIC: SWBF2 0x007693f0
// `dynamic atexit destructor for 'msgGroup''
RedScreenGroupElement msgGroup;
// GLOBAL: SWBF2 0x01f39e30
// SYNTHETIC: SWBF2 0x00406120
// `dynamic initializer for 'hostQosText''
// SYNTHETIC: SWBF2 0x00769400
// `dynamic atexit destructor for 'hostQosText''
RedTextElement hostQosText;
// GLOBAL: SWBF2 0x01f39fb0
// SYNTHETIC: SWBF2 0x00406140
// `dynamic initializer for 'clientQosText1''
// SYNTHETIC: SWBF2 0x00769410
// `dynamic atexit destructor for 'clientQosText1''
RedTextElement clientQosText1;
// GLOBAL: SWBF2 0x01f39ef0
// SYNTHETIC: SWBF2 0x00406160
// `dynamic initializer for 'clientQosText2''
// SYNTHETIC: SWBF2 0x00769420
// `dynamic atexit destructor for 'clientQosText2''
RedTextElement clientQosText2;
// GLOBAL: SWBF2 0x01f3a070
// SYNTHETIC: SWBF2 0x00406180
// `dynamic initializer for 'QosIcon''
// SYNTHETIC: SWBF2 0x00769430
// `dynamic atexit destructor for 'QosIcon''
RedBitmapElement QosIcon;

// FUNCTION: SWBF2 0x0062e270
int ToggleQosText(RedConsole* pConsole, unsigned int uArgc, const char* pcArgs)
{
    QosTextEnable = !QosTextEnable;
    return 0;
}

// SYNTHETIC: SWBF2 0x004061a0
// `dynamic initializer for 'qosText''
RedCommandConsole qosText("net.toggleqostext", ToggleQosText);
// SYNTHETIC: SWBF2 0x004061c0
// `dynamic initializer for 'qosTextDbg''
RedCommandConsole qosTextDbg("debugmenu.ToggleQosDisplay", ToggleQosText);

// FUNCTION: SWBF2 0x0062da10
QOSDisplayThread::QOSDisplayThread()
{
    timer = 0.0f;
    lastClientUpdateCount = clientUpdateCount;
    for (int i = 0; i < 64; ++i)
    {
        lastUpdates[i] = 0;
        lastFills[i] = 0;
    }
    lastInBytes = 0;
    lastOutBytes = 0;
}

// FUNCTION: SWBF2 0x0062daa0
void QOSDisplay::Create()
{
    displayThread = 0;
}

void QOSDisplay::Destroy()
{
}

// FUNCTION: SWBF2 0x0062dab0
void QOSDisplay::InitClient()
{
    if (!NetGame::IsEnabled())
    {
        return;
    }

    for (int i = 0; i < 3; ++i)
    {
        QosValue[i][1] = (int)(QosValue20[i] * 0.05f / netSecondsPerTurn);
    }

    displayThread = new QOSDisplayThread;
    displayThread->ActivateThread(0, PER_FRAME);

    msgGroup.Enable(false);
    msgGroup.SetPosition(0.0f, 0.0f);

    hostQosText.Enable(false);
    hostQosText.SetFont(RedFont::GAMEFONT_MEDIUM);
    hostQosText.SetIntercharacterSpacing(0);
    hostQosText.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
    hostQosText.SetTextAlign(RedTextElement::TEXT_HALIGN_RIGHT, RedTextElement::TEXT_VALIGN_BOTTOM);
    msgGroup.AddElement(&hostQosText);

    clientQosText1.Enable(false);
    clientQosText1.SetFont(RedFont::GAMEFONT_MEDIUM);
    clientQosText1.SetIntercharacterSpacing(0);
    clientQosText1.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
    clientQosText1.SetTextAlign(RedTextElement::TEXT_HALIGN_RIGHT, RedTextElement::TEXT_VALIGN_BOTTOM);
    msgGroup.AddElement(&clientQosText1);

    clientQosText2.Enable(false);
    clientQosText2.SetFont(RedFont::GAMEFONT_MEDIUM);
    clientQosText2.SetIntercharacterSpacing(0);
    clientQosText2.m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
    clientQosText2.SetTextAlign(RedTextElement::TEXT_HALIGN_RIGHT, RedTextElement::TEXT_VALIGN_BOTTOM);
    msgGroup.AddElement(&clientQosText2);

    QosIcon.Enable(false);
    QosIcon.m_eStyle = RedBitmapElement::BITMAP_STYLE_NORMAL;
    RedColor white = { 0xff, 0xff, 0xff, 0xff };
    QosIcon.m_color = white;
    QosIcon.SetPosition(-110.0f, -100.0f);
    QosIcon.SetRect(64.0f, 64.0f, RedBitmapElement::BITMAP_HALIGN_LEFT,
                    RedBitmapElement::BITMAP_VALIGN_TOP);
    QosIcon.m_color.a = QosIconAlpha;
    QosIcon.SetTexture(0x298058FEu);
    msgGroup.AddElement(&QosIcon);

    FLRenderer::AddInterfaceElement(&msgGroup, 1.0f, 1.0f, false);
}

// FUNCTION: SWBF2 0x0062dd10
void QOSDisplay::InitHost()
{
    if (NetGame::IsEnabled() && NetGame::IsDedicated() && displayThread)
    {
        displayThread->DeactivateThread();
        displayThread->ActivateThread(0, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x0062dd60
void QOSDisplay::Reset()
{
    if (displayThread)
    {
        delete displayThread;
        displayThread = 0;
        hostQosText.SetFont((const char*)0);
        clientQosText1.SetFont((const char*)0);
        clientQosText2.SetFont((const char*)0);
        QosIcon.Cleanup();
        msgGroup.RemoveAllElements();
        FLRenderer::RemoveInterfaceElement(&msgGroup);
    }
}

// FUNCTION: SWBF2 0x0062ddc0
bool QOSDisplayThread::Update(float fDeltaTime)
{
    QosIconAlpha -= 20;
    timer -= fDeltaTime;
    if (QosEnable)
    {
        QosIcon.m_color.a = QosIconAlpha;
    }

    if (timer < 0.0f)
    {
        timer += 1.0f;

        char text1[64];
        char text2[64];
        if (netIsClient)
        {
            const int iUpdates = clientUpdateCount - lastClientUpdateCount;
            lastClientUpdateCount = clientUpdateCount;
            int iLag = (int)(clientTurnNumber - clientHostTurnNumber);
            if (iLag < netTurnsPerSecond)
            {
                iLag = clientAhead;
            }

            if (NetGame::IsHost())
            {
                _snprintf(text1, sizeof(text1), "U-%d, L-%d", iUpdates, iLag);
            }
            else
            {
                unsigned int uIn;
                unsigned int uOut;
                NetComm::GetBytes(uIn, uOut);
                const int iIn = uIn - lastInBytes;
                lastInBytes = uIn;
                const int iOut = uOut - lastOutBytes;
                lastOutBytes = uOut;

                unsigned int uUpdates;
                unsigned int uFills;
                int iLoss;
                NetGame::GetPlayerQOS(NetComm::GetLocalIndex(0), uUpdates, uFills, iLoss);
                _snprintf(text1, sizeof(text1), "P-%d, IO-%d/%d, L-%d",
                          NetComm::GetPing(NetComm::GetLocalIndex(0)),
                          iIn * 8 / 1024, iOut * 8 / 1024, iLoss);
                _snprintf(text2, sizeof(text2), "U-%d, L-%d, TS-%.2f", iUpdates, iLag,
                          NetGame::GetTimeScale());
                DisplayIcon(fDeltaTime, iUpdates, iLag);
            }

            if (QosTextEnable)
            {
                clientQosText1.m_color = RedColor::RED;
                clientQosText1.m_color.a = 0xff;
                clientQosText1.SetText(text1, true);
                clientQosText1.SetPosition(-60.0f, -200.0f);
                clientQosText2.m_color = RedColor::RED;
                clientQosText2.m_color.a = 0xff;
                clientQosText2.SetText(text2, true);
                clientQosText2.SetPosition(-60.0f, -220.0f);
            }
            clientQosText1.Enable(QosTextEnable);
            clientQosText2.Enable(QosTextEnable);
        }

        if (NetGame::IsHost())
        {
            int iPlaying = 0;
            int iTotal = 0;
            for (int i = 0; i < netCurMaxPlayers; ++i)
            {
                if (NetGame::IsPlaying(i, false))
                {
                    ++iPlaying;
                    unsigned int uUpdates;
                    unsigned int uFills;
                    int iLoss;
                    NetGame::GetPlayerQOS(i, uUpdates, uFills, iLoss);
                    iTotal += uUpdates - lastUpdates[i];
                    lastUpdates[i] = uUpdates;
                    lastFills[i] = uFills;
                }
            }
            if (!iPlaying)
            {
                iPlaying = 1;
            }

            unsigned int uIn;
            unsigned int uOut;
            NetComm::GetBytes(uIn, uOut);
            const int iIn = uIn - lastInBytes;
            lastInBytes = uIn;
            const int iOut = uOut - lastOutBytes;
            lastOutBytes = uOut;

            const int iAverage = iTotal / iPlaying;
            _snprintf(text1, sizeof(text1), "U-%d, IO-%dK/%dK", iAverage,
                      iIn * 8 / 1024, iOut * 8 / 1024);
            DisplayIcon(fDeltaTime, iAverage, 0);

            if (QosTextEnable)
            {
                hostQosText.m_color = RedColor::RED;
                hostQosText.m_color.a = 0xff;
                hostQosText.SetText(text1, true);
                hostQosText.SetPosition(-170.0f, -95.0f);
            }
            hostQosText.Enable(QosTextEnable);
        }

        msgGroup.Enable(true);
    }
    return true;
}

// FUNCTION: SWBF2 0x0062e1e0
bool QOSDisplayThread::DisplayIcon(float fDeltaTime, int iUpdates, int iLag)
{
    QosEnable = false;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mOnlineOptions.mOnlineIcon && clientUpdateCount > 48
        && !GameLoop::IsGameOver()
        && (iUpdates < QosValue[0][0] || iLag > QosValue[0][1]))
    {
        QosEnable = true;
        QosIcon.SetTexture(0xF5BDCDDEu);
    }
    QosIcon.Enable(QosEnable);
    return true;
}
