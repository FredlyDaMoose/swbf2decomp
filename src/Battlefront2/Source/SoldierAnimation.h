#pragma once

class MemoryPool;
class ZephyrAnim;
class SoldierAnimationData;
template <int N> class ZephyrPoseDyn;

class SoldierAnimation
{
public:

    enum SCOPE
    {
        INVALID = 0,
        UPPER_BODY = 1,
        LOWER_BODY = 2,
        FULL_BODY = 3
    };

    const ZephyrAnim* GetAnimation() const { return m_pZephyrAnim; }

    unsigned short GetAnimationNumFrames() const;

    unsigned char GetAnimIndex() const;
    unsigned char GetSyncType() const;
    float GetBlendInTime() const;
    float GetBlendTime(const SoldierAnimation* pNext) const;

    SoldierAnimation(const ZephyrAnim* pAnim, const SoldierAnimationData* pAnimData, SCOPE eScope);

    void Init(const ZephyrAnim* pAnim, const SoldierAnimationData* pAnimData, SCOPE eScope);

    bool IsLooping() const;

    static void* operator new(unsigned int uiSize);

    void SetupDynamicPose(ZephyrPoseDyn<32>& pose, float fTime) const;

    void SetVelocity(float fVelocityX, float fVelocityZ);

    float GetVelocityX() const { return m_iVelocityX * 0.01f; }
    float GetVelocityZ() const { return m_iVelocityZ * 0.01f; }

    bool HasLowerBody() const { return (m_uiScope & LOWER_BODY) != 0; }

    static MemoryPool sMemoryPool;

    ZephyrAnim* m_pZephyrAnim;
    union
    {
        struct
        {
            int m_iVelocityZ : 15;
            int m_iVelocityX : 15;
            unsigned int m_uiScope : 2;
        };
        unsigned int m_uiData;
    };
    SoldierAnimationData* m_pAnimData;
    char* mName;
};
 
