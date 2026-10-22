#pragma once

enum RED_LOG_DESTINATION_INDEX
{
    RED_LOG_DESTINATION_INDEX_CONSOLE = 0,
    RED_LOG_DESTINATION_INDEX_TRACE = 1,
    RED_LOG_DESTINATION_INDEX_FILE = 2,
    RED_LOG_DESTINATION_INDEX_CALLBACK = 3,
    RED_LOG_DESTINATION_INDEX_BREAK = 4,
    RED_LOG_DESTINATION_INDEX_OPEN_CONSOLE = 5,
    RED_LOG_NUM_WRITE_DESTINATIONS = 5,
    RED_LOG_NUM_DESTINATIONS = 6
};

enum RED_LOG_DESTINATION
{
    RED_LOG_DESTINATION_NONE = 0,
    RED_LOG_DESTINATION_CONSOLE = 1,
    RED_LOG_DESTINATION_TRACE = 2,
    RED_LOG_DESTINATION_FILE = 4,
    RED_LOG_DESTINATION_CALLBACK = 8,
    RED_LOG_DESTINATION_BREAK = 16,
    RED_LOG_DESTINATION_OPEN_CONSOLE = 32,
    RED_LOG_DESTINATION_ALL_WRITE = 31,
    RED_LOG_DESTINATION_ALL = 63
};

struct HWND__;

namespace RedWarning
{

enum RED_ERROR_SEVERITY
{
    RED_SEVERITY_VERBOSE = 0,
    RED_SEVERITY_INFORM = 1,
    RED_SEVERITY_WARNING = 2,
    RED_SEVERITY_ERROR = 3,
    RED_SEVERITY_BUG = 4,
    RED_SEVERITY_FATAL = 5,
    RED_SEVERITY_NONE = 6
};

void __cdecl Init(void);

void __cdecl SetLogData(RED_ERROR_SEVERITY eSeverity);
void __cdecl SetLogData(RED_ERROR_SEVERITY eSeverity, char const* pcFileName, int iLineNumber,
                        char const* pcCompileDate, char const* pcCompileTime);

void __cdecl SetLogFileOverwrite(bool bOverwrite);

void __cdecl SetSeverityToDestination(unsigned int uiDestinationMask, RED_ERROR_SEVERITY eSeverity);
void __cdecl SetSeverityToDestination(RED_LOG_DESTINATION_INDEX eIndex, RED_ERROR_SEVERITY eSeverity);
RED_ERROR_SEVERITY __cdecl GetSeverityToDestination(RED_LOG_DESTINATION_INDEX eIndex);

unsigned int __cdecl Format(char* pcBuffer, unsigned int uiSize, char* pcMessage,
                            unsigned int uiFormat, char cSeparator);
void __cdecl LogMessage(char const* pcFormat, ...);

void __cdecl DialogBoxMessage(char const* pcFormat, ...);
int __stdcall DlgProc(HWND__* hDlg, unsigned int uiMessage, unsigned int uiWParam, long lParam);

void __cdecl Cleanup(void);
unsigned int __cdecl GetFormatToDestination(RED_LOG_DESTINATION_INDEX eIndex);
void __cdecl SetFormatToDestination(RED_LOG_DESTINATION_INDEX eIndex, unsigned int uiFormat);
void __cdecl SetFormatToDestination(unsigned int uiDestinationMask, unsigned int uiFormat);
void __cdecl SetLogCallback(void(__cdecl* pFunc)(char const*, ...));
void __cdecl SetDestination(unsigned int uiDestinationMask);
void __cdecl SetSeverityToBreak(RED_ERROR_SEVERITY eSeverity);

}
