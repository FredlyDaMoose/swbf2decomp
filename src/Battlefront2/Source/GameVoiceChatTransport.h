#ifndef GAMEVOICECHATTRANSPORT_H
#define GAMEVOICECHATTRANSPORT_H

#include "VoiceChatTransport.h"

namespace GameVoiceChat
{
class Transport : public VoiceChat::Transport
{
public:

    enum Mask
    {
        mask_Invalid = 0,
        mask_TeamFriend = 1,
        mask_TeamEnemy = 2,
        mask_FriendsBuddies = 3,
        mask_Everyone = 4,
        mask_Noone = 5,
        mask_Custom = 6,
        mask_Max = 7
    };

    struct NetPlayerTeamInfo
    {
        int player;
        int team;
    };

    Transport() {}
    ~Transport();

    void SetSendEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bKeepCurrent);
    void SetReceiveEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bKeepCurrent);

protected:

    void SetSendReceiveEnableUsingMask(unsigned char node, Mask mask, bool bEnable, bool bSend,
                                       bool bKeepCurrent);

private:

    unsigned int BuildPlayerTeamInfo(NetPlayerTeamInfo* pInfo, unsigned int uMax, int iNode,
                                     int* pTeam) const;
};

extern Transport gTransport;
}

#endif
