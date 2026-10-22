#pragma once

class RedConsole
{
public:

    // FUNCTION: SWBF2 0x00538760
    static RedConsole* Instance() { return sInstance; }

    void PrintMsg(char const* pcText);

    void PrintfMsg(char const* pcFormat, ...);
    void SetMode(unsigned int uiMode);

    static RedConsole* sInstance;
};
