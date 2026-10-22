#include "Battlefront2/Platform/Pc/pcSaveFile.h"
#include <ctype.h>

// FUNCTION: SWBF2 0x00631460
void SaveFile::StringToTime(const unsigned short* pusString, int& iYear, int& iMonth,
                            int& iDay, int& iHour, int& iMinute, int& iSecond)
{
    unsigned int value = 0;
    for (const unsigned short* p = pusString; *p; ++p)
    {
        unsigned int digit = *p;
        if (digit >= '0' && digit <= '9')
        {
            digit -= '0';
        }
        else
        {
            digit = (unsigned int)tolower(digit) - 'a' + 10;
        }
        value = value * 16 + digit;
    }

    iSecond = value % 60;
    iMinute = value / 60 % 60;
    unsigned int hours = value / 3600;
    iHour = hours % 24;
    unsigned int days = hours / 24;
    iDay = days & 31;
    unsigned int months = days / 32;
    iMonth = months % 13;
    unsigned int years = months / 13;
    iYear = years + 100 * (20 - years / 100);
}
