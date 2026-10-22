#include "Fsm.h"

// FUNCTION: SWBF2 0x0052e210
Fsm::Fsm()
{
    mCurState = 0;
    mNextState = 0;
}

// FUNCTION: SWBF2 0x0052e260
Fsm::~Fsm()
{
}

// SYNTHETIC: SWBF2 0x0052e230
// Fsm::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052e270
void Fsm::Kill()
{
    ExitState();
    mCurState = 100;
}

// FUNCTION: SWBF2 0x0052e290
bool Fsm::IsDone() const
{
    return mCurState == 100;
}

// FUNCTION: SWBF2 0x0052e2a0
void Fsm::FSMUpdate(float dt, bool b)
{
    if (b)
    {
        if (mCurState != 0 && mCurState != 100)
        {
            UpdateState(dt);
        }
        return;
    }

    int iCount = 0;
    do
    {
        while (mNextState != 0)
        {
            ExitState();
            mCurState = mNextState;
            mNextState = 0;
            EnterState();
            if (++iCount > 40)
            {
                return;
            }
        }

        if (mCurState != 0 && mCurState != 100)
        {
            UpdateState(dt);
            dt = 0.000001f;
        }
    } while (mNextState != 0);
}

// FUNCTION: SWBF2 0x00478620
void Fsm::SetNextState(int state)
{
    mNextState = state;
}

// FUNCTION: SWBF2 0x0052e350
HelperFsm::HelperFsm(int iCurState, int iNextState)
{
    mCurState = iCurState;
    mNextState = iNextState;
}

// SYNTHETIC: SWBF2 0x0052e1e0
// HelperFsm::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0052e370
void HelperFsm::SetNextState(int state)
{
    mNextState = state;
}

// FUNCTION: SWBF2 0x0052e380
void HelperFsm::FSMChangeState()
{
    while (mNextState != 0)
    {
        ExitState();
        mCurState = mNextState;
        mNextState = 0;
        EnterState();
    }
}
