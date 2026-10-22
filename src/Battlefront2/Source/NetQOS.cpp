#include "NetGame.h"
#include "NetComm.h"

// GLOBAL: SWBF2 0x01fa88ec
int state;
// GLOBAL: SWBF2 0x01fa88a7
bool saveChanged;

// GLOBAL: SWBF2 0x01fa88a6
bool isShell;

extern int netGameCount;

// GLOBAL: SWBF2 0x01ead364
int netThrottle;
// GLOBAL: SWBF2 0x01ead46c

int gAutoNetPlayers;

void DoThrottle();

// FUNCTION: SWBF2 0x005dae90
void NetQOS::Init()
{
    state = 0;
    saveChanged = false;
}

// FUNCTION: SWBF2 0x005daeb0
void NetQOS::Create(bool bMission)
{
    isShell = !bMission;
    if (isShell)
    {
        return;
    }

    if (NetGame::IsHost() && netGameCount == 1)
    {
        DoThrottle();
    }
}

// FUNCTION: SWBF2 0x005daf10
void NetQOS::EndShell()
{
}

// FUNCTION: SWBF2 0x005daf00
void NetQOS::BeginShell()
{
}

// GLOBAL: SWBF2 0x01ead468
int netGameThrottle;

// FUNCTION: SWBF2 0x005daee0
void NetQOS::Reset()
{
    if (isShell)
    {
        return;
    }

    state = 0;
    netGameThrottle = 0;
}

// FUNCTION: SWBF2 0x005daf50
void DoThrottle()
{
    int iUp, iDown;
    if (netThrottle)
    {
        NetComm::GetBandwidth(64, iUp, iDown);
        NetComm::SetBandwidth(64, netThrottle * 1024, iDown);
    }
    else if (NetComm::IsLAN())
    {
        NetComm::SetBandwidth(64, 0x500000, 0x500000);
    }
    else if (gAutoNetPlayers)
    {
        NetComm::GetBandwidth(64, iUp, iDown);
        NetComm::SetBandwidth(64, gAutoNetPlayers * 0xa000, iDown);
    }
}
