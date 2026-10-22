#pragma once

class PblTimeBase
{
public:
    PblTimeBase();

    void SetRate(float fRate);
    void AdvanceTicks(unsigned __int64 iTicks);
    void AdvanceTime(float fSeconds);

    float GetTime() const
    {
        return (float)(_iValue >> TICK_SHIFT) * _fSecondsPerTick;
    }

    void PblJournal_Record();

    static void PblJournal_RecordInit();

    class Stamp
    {
    public:

        // FUNCTION: SWBF2 0x00530db0
        Stamp()
        {
            _pTimeBase = 0;
        }

        explicit Stamp(PblTimeBase const* pTimeBase)
        {
            _pTimeBase = (PblTimeBase*)pTimeBase;
            _iValue = pTimeBase->_iValue;
        }

        void Mark()
        {
            _iValue = _pTimeBase->_iValue;
        }

        // FUNCTION: SWBF2 0x00530dc0
        void Mark(PblTimeBase const* pTimeBase)
        {
            _pTimeBase = (PblTimeBase*)pTimeBase;
            _iValue = pTimeBase->_iValue;
        }

        // FUNCTION: SWBF2 0x004199e0
        float GetElapsed() const
        {
            if (_pTimeBase && _pTimeBase->_iValue > _iValue)
            {
                unsigned __int64 iTicks = (_pTimeBase->_iValue - _iValue) >> TICK_SHIFT;
                if (iTicks == 0)
                {
                    return _fSecondsPerTick;
                }
                return (float)(unsigned int)iTicks * _fSecondsPerTick;
            }
            return 0.0f;
        }

        unsigned __int64 _iValue;
        PblTimeBase* _pTimeBase;
    };

    unsigned __int64 _iValue;
    __int64 _iIncrement;
    float _fRate;

    static int TICK_SHIFT;

    static const int FRAMES_PER_SECOND = 60;
    static unsigned __int64 _iTicksPerSecond;
    static unsigned __int64 _iTicksPerFrame;
    static float _fSecondsPerTick;

private:
    static void ComputeConstants();
};
 
 
