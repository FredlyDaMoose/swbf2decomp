#pragma once

class Timer;

namespace MissionTimer
{

    enum Show
    {
        SHOW_DEFEAT = -1,
        SHOW_NEUTRAL = 0,
        SHOW_VICTORY = 1
    };

    void Init();
    void EnterClient();
    void LeaveClient();

    void Set(Timer* pTimer, Show eShow, int iTeam);
    Timer* GetTimer();
    Show GetShow();
    int GetTeam();
    Show GetTeamShow(int iTeam);

    inline void SetDefeat(Timer* pTimer, int iTeam) { Set(pTimer, SHOW_DEFEAT, iTeam); }
    inline void SetVictory(Timer* pTimer, int iTeam) { Set(pTimer, SHOW_VICTORY, iTeam); }
}
