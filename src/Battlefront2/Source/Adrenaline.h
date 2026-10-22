#pragma once

enum AdrenalineLevel
{
    ADRENALINE_LOW = 0,
    ADRENALINE_MEDIUM = 1,
    ADRENALINE_HIGH = 2
};

class Adrenaline
{
public:
    // FUNCTION: SWBF2 0x00438020
    Adrenaline();

    void GetHit(float fAmount);
    void AddHit(float fAmount);

    void Update(float fDeltaTime, const class GameObject* pObject);

    void AddKill();
    void AddDied(const class GameObject* pObject);
    void NearbyDeath(bool bAlly);

    AdrenalineLevel GetCurrentLevel();

    float mLevel;

private:

    void AdjustLevel(float fAmount);

    void NearbyEnemy(float fDeltaTime);
};

 
