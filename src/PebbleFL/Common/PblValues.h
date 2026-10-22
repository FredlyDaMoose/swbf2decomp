#ifndef PBLVALUES_H
#define PBLVALUES_H

enum PblValuesType
{
    PVT_FULL = 0,
    PVT_MOVES_LOW = 1,
    PVT_MOVES_HIGH = 2
};

class PblValues
{
public:

    // TEMPLATE: SWBF2 0x004fab50
    // PblValues::Record<bool>
    template <class T>
    static void Record(const T& value, PblValuesType type)
    {
        (void)value;
        (void)type;
    }
};

#endif
