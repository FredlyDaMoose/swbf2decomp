#include "RedEngineFL/Graphics/RedCamera.h"

#include "GameLoop.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "CameraManager.h"

unsigned int Hash16(unsigned short const* pusString);

// GLOBAL: SWBF2 0x01e2fd5c
namespace AIDebugR { bool mBulletTimeEnabled; }
// GLOBAL: SWBF2 0x01e2fd5d

bool gTestSpawnTakeoverPending;

// GLOBAL: SWBF2 0x01e2fd5e

bool gTestSpawnForced;
// GLOBAL: SWBF2 0x01e2fd5f

bool gTestSpawnEnabled;

// GLOBAL: SWBF2 0x007e5885

bool gTestSpawnOptionEnabled = true;

extern bool gNoAIRespawn;

// GLOBAL: SWBF2 0x01e2fd60
namespace AIDebugR { float mBulletTimeScale; }
// GLOBAL: SWBF2 0x01e2fd64
namespace AIDebugR { float mBulletTimeStart; }
// GLOBAL: SWBF2 0x01e2fd68
namespace AIDebugR { float mBulletTimeUntil; }

namespace AIDebug { namespace TestSpawn { void Create(); } }

namespace AIDebug {

// FUNCTION: SWBF2 0x00438a00
bool GetTestSpawnMode()
{
    return false;
}

}

namespace AIDebugR {

// FUNCTION: SWBF2 0x004385d0
void Create()
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
    const unsigned int uTail = Hash16(pProfile->GetName() + 1);
    AIDebugR::mBulletTimeEnabled = Hash16(pProfile->GetName()) - uTail == 0x60081D20;
}

// FUNCTION: SWBF2 0x00438610
void StartBulletTime(float fFirst, float fSecond, float fThird)
{
    if (!AIDebugR::mBulletTimeEnabled)
    {
        return;
    }
    float fTime = GameLoop::GetMissionTime() + fFirst;
    AIDebugR::mBulletTimeStart = fTime;
    AIDebugR::mBulletTimeUntil = fTime + fThird;
    AIDebugR::mBulletTimeScale = fSecond;
}

}

namespace AIDebug {

// FUNCTION: SWBF2 0x00438660
void Create()
{
    AIDebugR::Create();
    TestSpawn::Create();
}

// FUNCTION: SWBF2 0x004386b0
void StartBulletTime(float fFirst, float fSecond, float fThird)
{
    if (!NetGame::IsEnabled() && CameraManager::Instance()->GetNumCameras() == 1)
    {
        AIDebugR::StartBulletTime(fFirst, fSecond, fThird);
    }
}

// FUNCTION: SWBF2 0x004386e0
void TestSpawn::Create()
{
    if (!gTestSpawnForced)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
        const unsigned int uTail = Hash16(pProfile->GetName() + 1);
        const unsigned int uWhole = Hash16(pProfile->GetName());
        gTestSpawnEnabled = false;
        if (uWhole - uTail == 0xC2ADE824)
        {
            gTestSpawnEnabled = true;
        }
    }
    else
    {
        gTestSpawnEnabled = true;
    }
    if (GetTestSpawnMode())
    {
        gTestSpawnTakeoverPending = false;
        gTestSpawnOptionEnabled = true;
        gNoAIRespawn = true;
    }
}

}

// FUNCTION: SWBF2 0x00438a20
bool __cdecl IsCoverHintFilteringEnabled()
{
    return false;
}

namespace AIDebug {

// FUNCTION: SWBF2 0x00438680
bool OnlyOneAI()
{
    return false;
}

}

static const PblVector3 CYL_CENTER(-74.0f, 12.0f, 20.0f);

namespace AIDebug
{
namespace TestSpawn
{

// GLOBAL: SWBF2 0x01eb0250
// SYNTHETIC: SWBF2 0x004021a0
// `dynamic initializer for 'AIDebug::TestSpawn::mCamera''
// SYNTHETIC: SWBF2 0x00766d30
// `dynamic atexit destructor for 'AIDebug::TestSpawn::mCamera''
RedCamera mCamera;

// GLOBAL: SWBF2 0x01eaffe4
// SYNTHETIC: SWBF2 0x004021c0
// `dynamic initializer for 'AIDebug::TestSpawn::camDir''
PblVector3 camDir = CYL_CENTER;

}
}
