#pragma once

namespace Snd
{

class Bus;
class SourceTransport;
class Source;

class SourceBus
{
public:
    SourceBus();

    // FUNCTION: SWBF2 0x00738370
    ~SourceBus() {}

    void Stop(SourceTransport* pTransport);

    float GetGain() const;

    void SetGain(Source* pSource, float fGain);

    void SetBus(Bus* pBus);

    void Play(SourceTransport* pTransport);

    // FUNCTION: SWBF2 0x00738380
    Bus* GetBus() const { return mBus; }

protected:

    void Update(Source* pSource, SourceTransport* pTransport);

public:

    Bus* mBus;
    float mGain;
    unsigned char mPlayState;
};

 

}
