#pragma once

#include <stddef.h>

class PblVector3;
namespace Snd
{
class VoiceVirtual;
}

class GameSound
{
public:

    enum Type
    {
        type_invalid = 0,
        type_sound = 1,
        type_soundLayered = 2,
        type_soundStream = 3,
        type_ID = 4
    };

    GameSound(bool bDeferred = false);

    ~GameSound();

    void SetID(unsigned int uID, bool, bool);

    void SetID(const char* pName, bool, bool);

    GameSound& operator=(const GameSound& other);

    bool CanPlay() const;
    void Play(const PblVector3* pPos, const PblVector3* pVel, bool bArg3,
              bool bArg4) const;
    const void* GetProperties() const;
    Type GetPropertiesType() const;
    unsigned int GetPropertiesHashID() const;
    float GetMuteDistance() const;
    float GetGain() const;
    float GetPitch() const;

    static void AddDeferred(GameSound* const pSound);

    static void SetupDeferred();

    static void InitDeferred();

    static GameSound** sDeferredSounds;
    static unsigned int sNumDeferredSounds;

    char* mSoundDescription;
    unsigned char mType;
};
 
 
 

class GameSoundControllable
{
public:

    enum Flags
    {
        flags_playing = 1,
        flags_looping = 2
    };

    GameSoundControllable();
    ~GameSoundControllable();

    void Play(const GameSound* pSound, const PblVector3* pPos,
              const PblVector3* pVel, bool bArg);
    void Stop(bool bArg);

    void Update(const GameSound* pSound, const PblVector3* pPos,
                const PblVector3* pVel);
    Snd::VoiceVirtual* GetVoiceVirtual() const;

    bool GetPlaying() const { return (mFlags & flags_playing) != 0; }

private:
    Snd::VoiceVirtual* ReleaseVoice();
    static void StolenCallback(Snd::VoiceVirtual* pVoice, void* pData);

public:

    unsigned short mVoiceVirtualHandle;
    unsigned char mFlags;
};
 
