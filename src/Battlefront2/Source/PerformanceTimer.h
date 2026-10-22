#pragma once

class PblPerformanceTimer
{
public:
    virtual ~PblPerformanceTimer();
    virtual void Start();
    virtual void Stop();

    static unsigned __int64 _GetCycleCounter();

    unsigned __int64 _iValue;
    unsigned __int64 _iAccum;

    static float _fToSeconds;
};

 
