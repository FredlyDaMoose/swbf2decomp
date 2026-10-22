#include <string.h>

#include "PblJournal.h"

// GLOBAL: SWBF2 0x009cefd0
PblJournal sJournalInstance;

// GLOBAL: SWBF2 0x009cefc0
namespace PblProfile { bool mPausedFlag; }

namespace PblProfile {

// FUNCTION: SWBF2 0x0072b310
bool IsPaused()
{
    return PblProfile::mPausedFlag;
}
}

namespace PblProfile {

// FUNCTION: SWBF2 0x0072b320
void SetPaused(bool bValue)
{
    PblProfile::mPausedFlag = bValue;
}
}

// FUNCTION: SWBF2 0x0072b340
PblJournal* PblJournal::Get(PblJournalType  )
{
    return &sJournalInstance;
}

// FUNCTION: SWBF2 0x005b4230
void PblJournal::RecordLow(void*  , int  )
{
}

// FUNCTION: SWBF2 0x0072b360
void JournalString(char* pcString, int iMax)
{
    int iLength = (int)strlen(pcString);
    if (iLength > iMax)
    {
        iLength = iMax;
    }
    pcString[iLength] = '\0';
}

// FUNCTION: SWBF2 0x00453680
void JournalRecordStub()
{
}

// FUNCTION: SWBF2 0x0072b350
void __cdecl JournalReceiveTraceStub()
{
}
