#include "HUDEventQueue.h"
#include "GameLoop.h"

namespace HUD {

// SYNTHETIC: SWBF2 0x0055e1a0
// HUD::DelayedEvent::DelayedEvent
// SYNTHETIC: SWBF2 0x0055e1b0
// HUD::DelayedEvent::~DelayedEvent

// GLOBAL: SWBF2 0x01ec6110
// SYNTHETIC: SWBF2 0x00404000
// `dynamic initializer for 'HUD::gEventList''
// SYNTHETIC: SWBF2 0x00768b90
// `dynamic atexit destructor for 'HUD::gEventList''
DelayedEvent gEventList[32];

// FUNCTION: SWBF2 0x0055e1c0
void RemoveAllEvents()
{
    for (int i = 0; i < 32; i++)
    {
        gEventList[i].activationTime = -1.0f;
    }
}

// FUNCTION: SWBF2 0x0055e1f0
void EventQueue::Create()
{
    RemoveAllEvents();
    sEventQueue.ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x0055e290
// HUD::EventQueue::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0055e2c0
EventQueue::~EventQueue()
{
    RemoveAllEvents();
}

// FUNCTION: SWBF2 0x0055e2d0
bool EventQueue::Update(float)
{
    for (int i = 0; i < 32; i++)
    {
        DelayedEvent& kEvent = gEventList[i];
        if (kEvent.activationTime > 0.0f
            && GameLoop::GetMissionTime() > kEvent.activationTime)
        {
            kEvent.e.Send();
            kEvent.activationTime = -1.0f;
        }
    }
    return true;
}

}

// FUNCTION: SWBF2 0x0055e210
void HUD::EventQueue::Destroy()
{
    RemoveAllEvents();
    sEventQueue.DeactivateThread();
}

// FUNCTION: SWBF2 0x0055e270
HUD::EventQueue::EventQueue()
{
    RemoveAllEvents();
}

// GLOBAL: SWBF2 0x01ec60f0
// SYNTHETIC: SWBF2 0x00404030
// `dynamic initializer for 'HUD::EventQueue::sEventQueue''
// SYNTHETIC: SWBF2 0x00768bb0
// `dynamic atexit destructor for 'HUD::EventQueue::sEventQueue''
HUD::EventQueue HUD::EventQueue::sEventQueue;
