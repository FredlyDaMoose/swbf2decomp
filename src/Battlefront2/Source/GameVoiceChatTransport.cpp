#include "GameVoiceChatTransport.h"
#include "NetGame.h"

extern int netCurMaxPlayers;
namespace FriendBuddyList { bool __fastcall IsFriendBuddy(int ignored, int iPlayer); }

namespace GameVoiceChat
{

// FUNCTION: SWBF2 0x0053dcc0
void Transport::SetSendEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bKeepCurrent)
{
    SetSendReceiveEnableUsingMask(node, mask, bEnable, true, bKeepCurrent);
}

// FUNCTION: SWBF2 0x0053dce0
void Transport::SetReceiveEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bKeepCurrent)
{
    SetSendReceiveEnableUsingMask(node, mask, bEnable, false, bKeepCurrent);
}

// FUNCTION: SWBF2 0x0053dd00
void Transport::SetSendReceiveEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bSend,
                                              bool bKeepCurrent)
{
    if (mask == mask_TeamFriend || mask == mask_TeamEnemy)
    {
        NetPlayerTeamInfo players[64];
        int ownTeam = -1;
        const unsigned int count = BuildPlayerTeamInfo(players, 64, node, &ownTeam);
        for (unsigned int i = 0; i < count; ++i)
        {
            const int other = players[i].player;
            if (other < 0 || other == node)
            {
                continue;
            }
            const bool selected = mask == mask_TeamFriend ? players[i].team == ownTeam
                                                         : players[i].team != ownTeam;
            if (!selected && bKeepCurrent)
            {
                continue;
            }
            bool enable = bEnable && selected;
            if (bKeepCurrent && bEnable)
            {
                enable = bSend ? GetSendEnable(node, (unsigned char)other)
                               : GetReceiveEnable(node, (unsigned char)other);
            }
            if (bSend)
            {
                SetSendEnable(node, (unsigned char)other, enable, false);
            }
            else
            {
                SetReceiveEnable(node, (unsigned char)other, enable, false);
            }
        }
        return;
    }

    if (mask == mask_FriendsBuddies)
    {
        for (unsigned int other = 0; other < mNumNodes; ++other)
        {
            if (other == node)
            {
                continue;
            }
            const bool selected = FriendBuddyList::IsFriendBuddy(0, other);
            if (!selected && bKeepCurrent)
            {
                continue;
            }
            bool enable = bEnable && selected;
            if (bKeepCurrent && bEnable)
            {
                enable = bSend ? GetSendEnable(node, (unsigned char)other)
                               : GetReceiveEnable(node, (unsigned char)other);
            }
            if (bSend)
            {
                SetSendEnable(node, (unsigned char)other, enable, false);
            }
            else
            {
                SetReceiveEnable(node, (unsigned char)other, enable, false);
            }
        }
        return;
    }

    if (mask == mask_Everyone || mask == mask_Noone)
    {
        const bool selected = mask == mask_Everyone ? bEnable : !bEnable;
        for (unsigned int other = 0; other < mNumNodes; ++other)
        {
            if (other == node)
            {
                continue;
            }
            bool enable = selected;
            if (bKeepCurrent && selected)
            {
                enable = bSend ? GetSendEnable(node, (unsigned char)other)
                               : GetReceiveEnable(node, (unsigned char)other);
            }
            if (bSend)
            {
                SetSendEnable(node, (unsigned char)other, enable, false);
            }
            else
            {
                SetReceiveEnable(node, (unsigned char)other, enable, false);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0053e040
unsigned int Transport::BuildPlayerTeamInfo(NetPlayerTeamInfo* pInfo, unsigned int uMax, int iNode,
                                            int* pTeam) const
{
    (void)uMax;
    unsigned int uCount = 0;
    for (int iPlayer = 0; iPlayer < netCurMaxPlayers && uCount < 64; ++iPlayer, ++uCount)
    {
        pInfo[uCount].player = iPlayer;
        const int iTeam = NetGame::GetStartTeam(iPlayer);
        pInfo[uCount].team = iTeam;
        if (pInfo[uCount].player == iNode && pTeam)
        {
            *pTeam = iTeam;
        }
    }
    return uCount;
}

}
