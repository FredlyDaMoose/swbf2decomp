#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedMaterial.h"

#include "NetGame.h"
#include "PebbleFL/Common/PblTime.h"
#include "PebbleFL/Common/PblJournal.h"
#include "GameVoiceChat.h"
#include "FLInputManager.h"
#include "RedEngineFL/input/RedKeyboard.h"

// GLOBAL: SWBF2 0x01ec0bdc
// SYNTHETIC: SWBF2 0x00403b60
// `dynamic initializer for '_frameUpdateTimer''
PblTime _frameUpdateTimer;

// GLOBAL: SWBF2 0x01e55f90
unsigned __int64 _prevTickCount;
// GLOBAL: SWBF2 0x01e55f98
unsigned __int64 _prevTickCountReal;

namespace FrameUpdate
{

void Create();
void Update();

// GLOBAL: SWBF2 0x01e55f80
unsigned __int64 tickFramePeriod;
// GLOBAL: SWBF2 0x01e55f88
unsigned __int64 tickFramePeriodReal;
// GLOBAL: SWBF2 0x01e55f7c
float fOneOverTicksPerSecond;

// FUNCTION: SWBF2 0x0052d480
void Create()
{
    tickFramePeriod = PblTime::GetTicksPerSecond() / (NetGame::IsEnabled() ? 15 : 30);
    tickFramePeriodReal = PblTime::GetTicksPerSecond() / 30;
    fOneOverTicksPerSecond = 1.0f / (float)PblTime::GetTicksPerSecond();
}

}

// FUNCTION: SWBF2 0x00458950
template void PblJournal::Record<char>(char& value, PblJournalType type);

// FUNCTION: SWBF2 0x0052d510
void FrameUpdate::Update()
{
    const unsigned __int64 tickCount = PblTime::GetTickCount();

    const unsigned int elapsed = (unsigned int)(tickCount > _prevTickCount
        ? tickCount - _prevTickCount
        : ~_prevTickCount + tickCount);
    const unsigned int elapsedReal = (unsigned int)(tickCount > _prevTickCountReal
        ? tickCount - _prevTickCountReal
        : ~_prevTickCountReal + tickCount);

    char update = 0;
    if (elapsed > tickFramePeriod)
    {
        update |= 1;
    }
    if (elapsedReal > tickFramePeriodReal)
    {
        update |= 2;
    }

    if (update & 2)
    {
        _prevTickCountReal = tickCount;
        const float fDeltaTime = (float)elapsedReal * fOneOverTicksPerSecond;

        GameVoiceChat::Update(fDeltaTime);
        NetGame::Update();

        RedKeyboard* pKeyboard = FLInputManager::s_instance.GetPlayerJoystick(0)->GetKeyboard();
        if (pKeyboard)
        {
            pKeyboard->Update(fDeltaTime);
        }
    }

    if (update & 1)
    {
        _prevTickCount = tickCount;
    }
}

// FUNCTION: SWBF2 0x0052d620
template void PblJournal::Record<float>(float& value, PblJournalType type);

// GLOBAL: SWBF2 0x01ec0be0
// SYNTHETIC: SWBF2 0x00403b70
// `dynamic initializer for 'gNormalizedMagentaColor''
RedColorValue gNormalizedMagentaColor(
    RedColor::MAGENTA.r * (1.0f / 255.0f),
    RedColor::MAGENTA.g * (1.0f / 255.0f),
    RedColor::MAGENTA.b * (1.0f / 255.0f),
    RedColor::MAGENTA.a * (1.0f / 255.0f));
