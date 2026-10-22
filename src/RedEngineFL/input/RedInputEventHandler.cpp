#include "RedInputEventHandler.h"

// GLOBAL: SWBF2 0x007e1424
PblList<RedInputEventHandler> RedInputEventHandler::m_handlerList;

// FUNCTION: SWBF2 0x006efde0
RedInputEventHandler::RedInputEventHandler()
{
    m_node._pList = 0;
    m_node._pNext = 0;
    m_node._pPrev = 0;
    m_node._pObject = 0;
}

// FUNCTION: SWBF2 0x006efe10
RedInputEventHandler::~RedInputEventHandler()
{
    RemoveEventHandler(this);
}

// FUNCTION: SWBF2 0x006efe50
void RedInputEventHandler::AddEventHandler(RedInputEventHandler* pHandler)
{
    if (pHandler->m_node._pList)
    {
        pHandler->m_node._pList->Remove(&pHandler->m_node);
    }

    m_handlerList.AddLast(&pHandler->m_node, pHandler);
}

// FUNCTION: SWBF2 0x006eff00
void RedInputEventHandler::RemoveEventHandler(RedInputEventHandler* pHandler)
{
    if (pHandler->m_node._pList)
    {
        m_handlerList.Remove(&pHandler->m_node);
    }
}

// FUNCTION: SWBF2 0x006efeb0
void RedInputEventHandler::PostKeyboardEvent(RedInputEventType eType, RedKeyboard* pKeyboard,
                                             unsigned char ucKey, unsigned char ucAscii,
                                             unsigned short usFlags)
{
    for (PblList<RedInputEventHandler>::Itor it = m_handlerList.Begin(); it != m_handlerList.End();
         ++it)
    {
        it->KeyboardEvent(eType, pKeyboard, ucKey, ucAscii, usFlags);
    }
}
