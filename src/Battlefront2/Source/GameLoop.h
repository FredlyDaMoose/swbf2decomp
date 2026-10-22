#pragma once

namespace GameLoop
{

    void Update(float fDeltaTime, float fSystemDeltaTime);

    bool IsQuitRequested();

extern char mMissionScript[64];

extern unsigned int mMissionScriptHash;

extern char mWorldFileName[64];
extern char mTerrainFileName[64];

void SetNextMission(char const* pcName);

// FUNCTION: SWBF2 0x004882d0
inline char const* GetNextMission() { return mMissionScript; }

void SetWorldFilename(char const* pcName);
void SetTerrainFilename(char const* pcName);

void Pause();

void Resume();

int Init();
int PostLoad();

int Cleanup();

extern bool _bFirstPersonEnabled;

inline void EnableFirstPerson(bool bEnable) { _bFirstPersonEnabled = bEnable; }

inline bool FirstPersonEnabled() { return _bFirstPersonEnabled; }

extern bool sPauseMode;

extern bool gInvincible;

extern bool mbMetagameRules;

// FUNCTION: SWBF2 0x0044c570
inline bool AreMetagameRulesOn() { return mbMetagameRules; }

// FUNCTION: SWBF2 0x00596b60
inline void SetMetagameRulesOn(bool bOn) { mbMetagameRules = bOn; }

extern unsigned int mNumCameras;

// FUNCTION: SWBF2 0x004a9cb0
inline unsigned int GetNumCameras() { return mNumCameras; }

inline void SetNumCameras(unsigned int uNum) { mNumCameras = uNum; }

extern int mMetagameSide[4];

// FUNCTION: SWBF2 0x005b40d0
inline int GetMetagameSide(int iIndex) { return mMetagameSide[iIndex]; }

inline void SetMetagameSide(int iIndex, int iSide) { mMetagameSide[iIndex] = iSide; }

extern int mLastGameWinner;
inline int GetLastGameWinner() { return mLastGameWinner; }
inline void SetLastGameWinner(int iWinner) { mLastGameWinner = iWinner; }

float GetMissionTime();

bool IsGameOver();

int GetJoystickYFlip();

void SetJoystickYFlip(int iFlip);

extern bool sGameOver;

extern bool mShowTeamPoints[2];
inline void SetShowTeamPoints(int iTeam, bool bShow) { mShowTeamPoints[iTeam - 1] = bShow; }

}

namespace GameLoop { extern float sDeltaTime; }

namespace GameLoop
{

// FUNCTION: SWBF2 0x004439e0
inline float GetDeltaTime() { return GameLoop::sDeltaTime; }
}

namespace GameLoop { void SetCanSwitchSides(bool bFlag); }
