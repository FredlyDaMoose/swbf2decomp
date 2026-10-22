#pragma once

#include <stddef.h>

struct SpawnCameraFloatPair
{
    float mFirstValue;
    float mSecondValue;
};
 

void ParseCommandLine(char const* pcAppName, char const* pcCmdLine);

void ParseCommandLine(int iArgc, char** const apcArgv);

extern char gGamespyAutoName[32];
extern char gGamespyAutoConnectAddr[64];
extern char gGamespyAutoPassStr[128];
extern char gGamespyAutoEmail[64];
extern char gGamespyAutoPPass[32];
extern unsigned int gGamespyAutoPass;
extern bool gBootLagger;
extern bool gShowLoadSpam;
extern bool gLogFinalScore;
extern int gBootLaggerPing;

namespace PlayerPrefs { extern int gDefaultDifficulty; }
namespace PlayerPrefs { extern int gDefaultTeamNum; }
