#pragma once

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class Character;
class GameObject;
class RedColor;
class Team;
namespace HUD { class ElementGroup; }

// VTABLE: SWBF2 0x007af260
class SpaceAssault : public Thread
{
public:
    enum { MAX_CRITICAL_SYSTEMS = 32 };

    struct CriticalSystemData
    {
        unsigned int nameHashId;
        short pointValue;
        float hudPosX;
        float hudPosY;
        int team;
        float lastHealth;
        float flashTimer;
        bool bShowMarker;
        CriticalSystemData* pNextLinkedSystem;
        GameObject* pGameObject;
        unsigned int isAliveTurnNumber;
        bool isAlive;
    };

    class ViewportData
    {
    public:
        ViewportData();
        ~ViewportData();

        static void SetupMemoryPool(unsigned int uCount, unsigned int uSize, unsigned int uGrow)
        {
            sMemoryPool.Setup(uCount, uSize, uGrow);
        }
        static void operator delete(void* p) { sMemoryPool.Free(p); }

        int team;
        bool isVisible;
        RedBitmapElement elemShipFriendly;
        RedBitmapElement elemShipEnemy;
        RedBitmapElement elemShieldFriendly;
        RedBitmapElement elemShieldEnemy;
        RedBitmapElement elemCriticalSystem[MAX_CRITICAL_SYSTEMS];

        static MemoryPool sMemoryPool;
    };

    SpaceAssault();
    virtual ~SpaceAssault();
    virtual bool Update(float fDeltaTime);

    static void Create();
    static void CreateClient();
    static void Destroy();

    // FUNCTION: SWBF2 0x00438ba0
    static SpaceAssault* Instance() { return mInstance; }

    // FUNCTION: SWBF2 0x00438bb0
    bool IsEnabled() const { return mbEnabled; }

    void Enable(bool bEnable);
    void AddCriticalSystem(const char* pName, short pointValue, float hudPosX, float hudPosY,
                           bool bShowMarker);
    void LinkCriticalSystems(unsigned int* const pNameHashIds, int iCount);
    void SetTeamNumbers(int iTeamATT, int iTeamDEF);
    void SetHUDBitmaps(unsigned int uShipATT, unsigned int uShipDEF, unsigned int uShieldATT,
                       unsigned int uShieldDEF, unsigned int uCriticalSystemATT,
                       unsigned int uCriticalSystemDEF);
    bool IsCriticalSystem(GameObject* pObject) const;
    unsigned int GetIsAliveTurnNumber(GameObject* pObject) const;
    short GetPointValue(unsigned int uNameHashId, Character* pCharacter);
    GameObject* GetRandomCritSys(const Team* pTeam, float fBias) const;
    static bool IsOkForRandom(const CriticalSystemData& data, int iTeam);

protected:
    void InitBitmaps(int iViewport);
    float CalcHealthPercentage(CriticalSystemData& data);
    RedColor CalcDamageColor(float fHealth);
    CriticalSystemData* FindCriticalSystemData(unsigned int uNameHashId);

public:
    CriticalSystemData mCriticalSystems[MAX_CRITICAL_SYSTEMS];
    ViewportData* mpViewportData;
    int mNumCriticalSystems;
    bool mAreCriticalSystemsDirty;
    bool mbEnabled;
    int mTeamATT;
    int mTeamDEF;
    unsigned int mShipBitmapATTHashId;
    unsigned int mShipBitmapDEFHashId;
    unsigned int mShieldBitmapATTHashId;
    unsigned int mShieldBitmapDEFHashId;
    unsigned int mCriticalSystemBitmapATTHashId;
    unsigned int mCriticalSystemBitmapDEFHashId;
    float mShieldFlashTimerATT;
    float mShieldFlashTimerDEF;
    float mLastShieldATT;
    float mLastShieldDEF;
    float mWidth;
    float mHeight;
    HUD::ElementGroup* mpGroupElement;

    static SpaceAssault* mInstance;
};
 
 
 
 
 
 
 
