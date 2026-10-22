#pragma once

class Trigger
{
public:

    enum TriggerButtonState
    {
        BUTTONSTATE_UP = 0,
        BUTTONSTATE_DOWN = 1,
        BUTTONSTATE_RELEASED = 2,
        BUTTONSTATE_PRESSED = 3
    };

    enum TriggerAction
    {
        ACTION_NONE = 0,
        ACTION_TAP = 1,
        ACTION_HOLD = 2,
        ACTION_CLEAR = 3,
        ACTION_TAP_PRESS = 4,
        ACTION_DOUBLETAP = 5,
        ACTION_TAP_HOLD = 6,
        ACTION_CLEAR_UNTIL_PRESS = 7
    };

    enum TriggerActionState
    {
        ACTIONSTATE_UP = 0,
        ACTIONSTATE_DOWN = 1,
        ACTIONSTATE_TAP = 6,
        ACTIONSTATE_HOLD = 9,
        ACTIONSTATE_HOLD_RELEASE = 10,
        ACTIONSTATE_CLEAR = 12,
        ACTIONSTATEFLAG_DOUBLE = 16,
        ACTIONSTATE_TAP_PRESS = 19,
        ACTIONSTATE_CLEAR_UNTIL_PRESS = 28
    };

    Trigger()
    {
        m_uiActionState = 0;
        z_uiReserved0 = 0;
        m_uiTimer = 0;
        z_uiReserved1 = 0;
    }

    void Clear() { m_uiActionState = ACTIONSTATE_CLEAR; }

    bool IsPressed() const { return m_uiButtonState == BUTTONSTATE_PRESSED; }

    bool IsDown() const { return m_bDown != 0; }

    bool IsUp() const { return !m_bDown; }
    bool IsRelease() const { return m_uiButtonState == BUTTONSTATE_RELEASED; }

    bool Held() const { return (m_uiActionState & ~ACTIONSTATEFLAG_DOUBLE) == ACTIONSTATE_HOLD; }

    bool Tapped() const { return (m_uiActionState & ~ACTIONSTATEFLAG_DOUBLE) == ACTIONSTATE_TAP; }
    bool HeldAndReleased() const
    {
        return (m_uiActionState & ~ACTIONSTATEFLAG_DOUBLE) == ACTIONSTATE_HOLD_RELEASE;
    }
    bool TappedAndPressed() const { return m_uiActionState == ACTIONSTATE_TAP_PRESS; }

    bool DoubleTapped() const { return m_uiActionState == (ACTIONSTATE_TAP | ACTIONSTATEFLAG_DOUBLE); }

    void Update(float fDeltaTime, bool bDown);

    void Update(float fDeltaTime, float fValue) { Update(fDeltaTime, fValue != 0.0f); }
    void ClearUntilPressed()
    {
        m_uiActionState = ACTIONSTATE_CLEAR_UNTIL_PRESS;
        m_uiTimer = 0;
    }

    union
    {
        struct
        {
            unsigned int m_uiButtonState : 2;
            unsigned int m_uiAction : 3;
            unsigned int zz_uiSpace1 : 19;
            unsigned int m_uiTimerNet : 5;
            unsigned int zz_uiSpace2 : 3;
        };
        struct
        {
            unsigned int m_uiActionState : 5;
            unsigned int z_uiReserved0 : 3;
            unsigned int m_uiTimer : 21;
            unsigned int z_uiReserved1 : 3;
        };
        struct
        {
            unsigned int m_bDown : 1;
            unsigned int m_bEdge : 1;
            unsigned int zz_uiSpace0 : 30;
        };
    };
};
 

static const float TRIGGER_TIMER_NETTICKS_PER_SECOND = 30.0f;

static const float TRIGGER_TIMER_SECONDS_PER_NETTICK = 1.0f / TRIGGER_TIMER_NETTICKS_PER_SECOND;

static const float TRIGGER_TIMER_TICKS_PER_SECOND = TRIGGER_TIMER_NETTICKS_PER_SECOND * 65536.0f;
static const float TRIGGER_TIMER_SECONDS_PER_TICK = 1.0f / TRIGGER_TIMER_TICKS_PER_SECOND;
static const float MAX_TAP_DURATION = 0.3f;
static const float MAX_DOUBLETAP_INTERVAL = 0.25f;
static const unsigned int MAX_TAP_DURATION_TICKS =
    (unsigned int)(MAX_TAP_DURATION * TRIGGER_TIMER_TICKS_PER_SECOND + 0.5f);
static const unsigned int MAX_DOUBLETAP_INTERVAL_TICKS =
    (unsigned int)(MAX_DOUBLETAP_INTERVAL * TRIGGER_TIMER_TICKS_PER_SECOND + 0.5f);

// FUNCTION: SWBF2 0x0043a950
inline void Trigger::Update(float fDeltaTime, bool bDown)
{
    if (m_uiActionState == ACTIONSTATE_CLEAR_UNTIL_PRESS)
    {
        if (!bDown)
        {
            m_uiActionState = ACTIONSTATE_UP;
        }
        return;
    }
    if (m_uiActionState == ACTIONSTATE_CLEAR)
    {
        m_uiActionState = bDown ? ACTIONSTATE_DOWN : ACTIONSTATE_UP;
        return;
    }

    if (m_uiTimer < 0x1F0000)
    {
        unsigned int uiTimer = (unsigned int)(fDeltaTime * TRIGGER_TIMER_TICKS_PER_SECOND + m_uiTimer);
        m_uiTimer = uiTimer > 0x1F0000 ? 0x1F0000 : uiTimer;
    }

    if (bDown)
    {
        if (m_bDown)
        {
            m_uiButtonState = BUTTONSTATE_DOWN;
            if (m_uiTimer > MAX_TAP_DURATION_TICKS)
            {
                m_uiAction |= ACTION_HOLD;
            }
        }
        else
        {
            m_uiButtonState = BUTTONSTATE_PRESSED;
            if ((m_uiAction & ACTION_TAP) && m_uiTimer <= MAX_DOUBLETAP_INTERVAL_TICKS)
            {
                m_uiAction = ACTION_TAP_PRESS;
            }
            else
            {
                m_uiAction = ACTION_NONE;
            }
            m_uiTimer = 0;
        }
    }
    else if (m_bDown)
    {
        m_uiButtonState = BUTTONSTATE_RELEASED;
        if ((m_uiAction & ACTION_HOLD) || m_uiTimer > MAX_TAP_DURATION_TICKS)
        {
            m_uiAction |= ACTION_HOLD;
        }
        else
        {
            m_uiAction |= ACTION_TAP;
        }
        m_uiTimer = 0;
    }
    else
    {
        m_uiButtonState = BUTTONSTATE_UP;
    }
}
