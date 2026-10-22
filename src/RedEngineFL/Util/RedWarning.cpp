#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "RedWarning.h"
#include "RedConsole.h"

#include "PebbleFL/Common/PblFile.h"

// GLOBAL: SWBF2 0x007df210
bool gBreakOnWarning = true;

// FUNCTION: SWBF2 0x006cccb0
bool GetBreakOnWarning()
{
    return gBreakOnWarning;
}

static char* kapcSEVERITYSHORT[6] = { "*", "?", "#", "$", "%", "!" };
static char* kapcSEVERITYLONG[6] = { "VERBOSE", "INFORM", "WARNING", "ERROR", "BUG", "FATAL" };

namespace RedWarning { bool g_bOverwriteLogFile = true; }

static char kpcNullFileName[7] = "[NULL]";
static char kpcNullDate[7] = "[NULL]";
static char kpcNullTime[7] = "[NULL]";

namespace RedWarning
{

RED_ERROR_SEVERITY g_eSeverityMin;
RED_ERROR_SEVERITY g_aeSeverityToDestination[6];
unsigned int g_auiFormatToDestination[5];
void(__cdecl* g_fnLogCallback)(char const*, ...);
RED_ERROR_SEVERITY g_eSeverity;
char* g_pcFileName;
char* g_pcCompileDate;
char* g_pcCompileTime;
int g_iLineNumber;
bool g_bFormatted;
bool g_bValidLogFile;
bool g_bInitialized;

char g_pcLogFileName[64];

}

// FUNCTION: SWBF2 0x006f6af0
void RedWarning::DialogBoxMessage(char const* pcFormat, ...)
{

    char* apcDialogText[2];
    char acMessage[1024] = "";
    va_list args;

    va_start(args, pcFormat);
    vsnprintf(acMessage, sizeof(acMessage), pcFormat, args);
    va_end(args);

    apcDialogText[0] = kapcSEVERITYLONG[g_eSeverity];
    apcDialogText[1] = acMessage;

    DialogBoxParamA(0, MAKEINTRESOURCEA(101), 0, (DLGPROC)DlgProc, (LPARAM)apcDialogText);
}

// FUNCTION: SWBF2 0x006f6b80
int __stdcall RedWarning::DlgProc(HWND__* hDlg, unsigned int uiMessage, unsigned int uiWParam,
                                  long lParam)
{
    switch (uiMessage)
    {
    case WM_INITDIALOG:
        {
            char** apcDialogText = (char**)lParam;
            SetWindowTextA(hDlg, apcDialogText[0]);
            SetWindowTextA(GetDlgItem(hDlg, 1001), apcDialogText[1]);
        }
        return 1;

    case WM_COMMAND:
        if (LOWORD(uiWParam) == IDOK)
        {

            EndDialog(hDlg, uiWParam);
            return 1;
        }
        break;
    }

    return 0;
}

// FUNCTION: SWBF2 0x006f6bf0
unsigned int RedWarning::Format(char* pcBuffer, unsigned int uiSize, char* pcMessage,
                                unsigned int uiFormat, char cSeparator)
{
    char* pcAt = pcBuffer;
    char* pcEnd = pcBuffer + uiSize;

    if (uiFormat & 0x00010000)
    {
        pcAt += _snprintf(pcAt, pcEnd - pcAt, "\n");

        if (uiFormat & 0x02)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "Message Severity: %d\n", g_eSeverity);
        }
        else if (uiFormat & 0x01)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "Message Severity: %s\n",
                              kapcSEVERITYLONG[g_eSeverity]);
        }

        if (uiFormat & 0x0C)
        {
            if (uiFormat & 0x04)
            {
                pcAt += _snprintf(pcAt, pcEnd - pcAt, "%s", g_pcFileName);
            }
            if (uiFormat & 0x08)
            {
                pcAt += _snprintf(pcAt, pcEnd - pcAt, "(%d)", g_iLineNumber);
            }
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "\n");
        }

        if (uiFormat & 0x30)
        {
            if (uiFormat & 0x10)
            {
                pcAt += _snprintf(pcAt, pcEnd - pcAt, g_pcCompileTime);
            }
            if (uiFormat & 0x20)
            {
                pcAt += _snprintf(pcAt, pcEnd - pcAt, g_pcCompileDate);
            }
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "\n");
        }

        if (uiFormat & 0x40)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "%s\n", pcMessage);
        }

        if (pcAt < pcEnd)
        {
            return (unsigned int)(pcAt - pcBuffer);
        }

        pcAt = pcEnd - 2;
    }
    else
    {

        if (uiFormat & 0x04)
        {
            unsigned int uiLength = (unsigned int)_snprintf(pcAt, pcEnd - pcAt, "%s", g_pcFileName);
            while (uiLength < 16 && pcAt + 1 + uiLength < pcEnd)
            {
                pcAt[uiLength++] = ' ';
            }
            pcAt += uiLength;
            *pcAt = '\0';
        }

        if (uiFormat & 0x08)
        {
            unsigned int uiLength =
                (unsigned int)_snprintf(pcAt, pcEnd - pcAt, "(%d) :", g_iLineNumber);
            while (uiLength < 9 && pcAt + 1 + uiLength < pcEnd)
            {
                pcAt[uiLength++] = ' ';
            }
            pcAt += uiLength;
            *pcAt = '\0';
        }

        if (uiFormat & 0x02)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "[%s]", kapcSEVERITYLONG[g_eSeverity]);
        }
        else if (uiFormat & 0x01)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "[%s]", kapcSEVERITYSHORT[g_eSeverity]);
        }

        if (uiFormat & 0x10)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "%c%s", cSeparator, g_pcCompileTime);
        }
        if (uiFormat & 0x20)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "%c%s", cSeparator, g_pcCompileDate);
        }
        if (uiFormat & 0x40)
        {
            pcAt += _snprintf(pcAt, pcEnd - pcAt, "%c%s", cSeparator, pcMessage);
        }

        if (pcAt > pcEnd - 2)
        {
            pcAt = pcEnd - 2;
        }
    }

    pcAt += _snprintf(pcAt, pcEnd - pcAt, "\n");
    return (unsigned int)(pcAt - pcBuffer);
}

// FUNCTION: SWBF2 0x006f6e90
RedWarning::RED_ERROR_SEVERITY RedWarning::GetSeverityToDestination(RED_LOG_DESTINATION_INDEX eIndex)
{
    return g_aeSeverityToDestination[eIndex];
}

// FUNCTION: SWBF2 0x006f6ea0
void RedWarning::Init(void)
{
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_CONSOLE] = RED_SEVERITY_INFORM;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_TRACE] = RED_SEVERITY_VERBOSE;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_FILE] = RED_SEVERITY_NONE;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_CALLBACK] = RED_SEVERITY_NONE;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_BREAK] = RED_SEVERITY_ERROR;
    g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_OPEN_CONSOLE] = RED_SEVERITY_WARNING;
    g_eSeverityMin = RED_SEVERITY_VERBOSE;

    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_CONSOLE] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_TRACE] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_FILE] = 0x0001004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_CALLBACK] = 0x0000004e;
    g_auiFormatToDestination[RED_LOG_DESTINATION_INDEX_BREAK] = 0x0000004e;

    g_fnLogCallback = 0;
    g_eSeverity = RED_SEVERITY_BUG;
    g_pcFileName = kpcNullFileName;
    g_pcCompileDate = kpcNullDate;
    g_pcCompileTime = kpcNullTime;
    g_iLineNumber = 0;
    g_bFormatted = true;

    if (g_bOverwriteLogFile)
    {
        _snprintf(g_pcLogFileName, sizeof(g_pcLogFileName), "%s.%s", "BFront2", "log");
    }
    else
    {

        int i = 0;
        do
        {
            _snprintf(g_pcLogFileName, sizeof(g_pcLogFileName), "%s_%d.%s", "BFront2", i, "log");
            if (!PblFile::Exists(g_pcLogFileName))
            {
                break;
            }
            ++i;
        } while (i < 10);

        if (i >= 10)
        {
            memset(g_pcLogFileName, 0, sizeof(g_pcLogFileName));
        }
    }

    g_bInitialized = true;
}

// FUNCTION: SWBF2 0x006f6ff0
void RedWarning::LogMessage(char const* pcFormat, ...)
{
    if (g_eSeverity < g_eSeverityMin)
    {
        return;
    }

    char acFormatted[512];
    char acMessage[1024] = "";
    va_list args;

    va_start(args, pcFormat);
    vsnprintf(acMessage, sizeof(acMessage), pcFormat, args);
    va_end(args);

    char* pcText = acFormatted;
    bool bBreak = false;

    for (int i = 0; i < RED_LOG_NUM_WRITE_DESTINATIONS; ++i)
    {
        if (g_eSeverity < g_aeSeverityToDestination[i])
        {
            continue;
        }

        if (g_bFormatted)
        {
            Format(acFormatted, sizeof(acFormatted), acMessage, g_auiFormatToDestination[i], '|');
        }
        else
        {
            pcText = acMessage;
        }

        RED_ERROR_SEVERITY eBreakAt = GetBreakOnWarning() ? RED_SEVERITY_WARNING : RED_SEVERITY_ERROR;
        if (g_bFormatted && !bBreak && g_eSeverity >= eBreakAt)
        {
            bBreak = true;
        }

        switch (i)
        {
        case RED_LOG_DESTINATION_INDEX_CONSOLE:
            if (RedConsole::sInstance)
            {
                if (g_eSeverity >= g_aeSeverityToDestination[RED_LOG_DESTINATION_INDEX_OPEN_CONSOLE])
                {
                    RedConsole::sInstance->SetMode(1);
                }
                RedConsole::sInstance->PrintMsg(pcText);
            }
            break;

        case RED_LOG_DESTINATION_INDEX_FILE:
            if (g_bValidLogFile)
            {
                FILE* pFile = fopen(g_pcLogFileName, "a");
                if (pFile)
                {
                    fprintf(pFile, pcText);
                    fclose(pFile);
                }
            }
            break;

        case RED_LOG_DESTINATION_INDEX_CALLBACK:
            if (g_fnLogCallback)
            {
                g_fnLogCallback(pcText);
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006f7180
void RedWarning::SetLogData(RED_ERROR_SEVERITY eSeverity)
{
    g_eSeverity = eSeverity;
    g_bFormatted = false;
}

// FUNCTION: SWBF2 0x006f71a0
void RedWarning::SetLogData(RED_ERROR_SEVERITY eSeverity, char const* pcFileName, int iLineNumber,
                            char const* pcCompileDate, char const* pcCompileTime)
{
    g_eSeverity = eSeverity;
    g_bFormatted = true;

    if (pcFileName)
    {
        g_pcFileName = (char*)pcFileName;
        g_pcCompileDate = (char*)pcCompileDate;
        g_pcCompileTime = (char*)pcCompileTime;
        g_iLineNumber = iLineNumber;
    }
    else
    {
        g_pcFileName = kpcNullFileName;
        g_pcCompileDate = kpcNullDate;
        g_pcCompileTime = kpcNullTime;
        g_iLineNumber = 0;
    }
}

// FUNCTION: SWBF2 0x006f7210
void RedWarning::SetLogFileOverwrite(bool bOverwrite)
{
    g_bOverwriteLogFile = bOverwrite;
}

// FUNCTION: SWBF2 0x006f7220
void RedWarning::SetSeverityToDestination(unsigned int uiDestinationMask, RED_ERROR_SEVERITY eSeverity)
{
    g_eSeverityMin = RED_SEVERITY_NONE;

    unsigned int uiBit = 1;
    for (int i = 0; i < RED_LOG_NUM_DESTINATIONS; ++i)
    {
        if (uiDestinationMask & uiBit)
        {
            SetSeverityToDestination((RED_LOG_DESTINATION_INDEX)i, eSeverity);
        }

        if (g_aeSeverityToDestination[i] < g_eSeverityMin)
        {
            g_eSeverityMin = g_aeSeverityToDestination[i];
        }

        uiBit <<= 1;
    }
}

// FUNCTION: SWBF2 0x006f7280
void RedWarning::SetSeverityToDestination(RED_LOG_DESTINATION_INDEX eIndex,
                                          RED_ERROR_SEVERITY eSeverity)
{
    if (g_eSeverityMin > eSeverity)
    {
        g_eSeverityMin = eSeverity;
    }
    else if (g_eSeverityMin == g_aeSeverityToDestination[eIndex])
    {

        g_eSeverityMin = eSeverity;
        for (int i = 0; i < RED_LOG_NUM_DESTINATIONS; ++i)
        {
            if (i == eIndex)
            {
                continue;
            }
            if (g_aeSeverityToDestination[i] < g_eSeverityMin)
            {
                g_eSeverityMin = g_aeSeverityToDestination[i];
            }
        }
    }

    g_aeSeverityToDestination[eIndex] = eSeverity;

    if (!g_bValidLogFile && g_pcLogFileName[0] != '\0'
        && eIndex == RED_LOG_DESTINATION_INDEX_FILE && eSeverity < RED_SEVERITY_NONE)
    {
        FILE* pFile = fopen(g_pcLogFileName, "w");
        g_bValidLogFile = (pFile != 0);

        if (g_bValidLogFile)
        {
            char acTime[256];
            memset(acTime, 0, sizeof(acTime));

            __time64_t tNow = _time64(0);
            strftime(acTime, sizeof(acTime) - 2, " %Y-%m-%d %H%M", _localtime64(&tNow));

            fprintf(pFile, "Opened logfile %s %s\n", g_pcLogFileName, acTime);
            fclose(pFile);
        }
    }
}
