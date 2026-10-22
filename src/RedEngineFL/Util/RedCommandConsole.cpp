#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "RedCommandConsole.h"
#include "RedConsole.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"

// GLOBAL: SWBF2 0x009abab4

RedCommandConsole::Group* RedCommandConsole::sRootNode;

// FUNCTION: SWBF2 0x006ec960
void RedCommandConsole::Group::List::Add(Group* pGroup)
{
    Node* pAt = _head._pNext;
    while (pAt->_pObject)
    {
        int iOrder = _stricmp(pGroup->mName, pAt->_pObject->mName);
        if (iOrder == 0)
        {
            return;
        }
        if (iOrder < 0)
        {
            break;
        }
        pAt = pAt->_pNext;
    }
    AddBefore(pAt, &pGroup->_Node, pGroup);
}

// FUNCTION: SWBF2 0x006eca20
RedCommandConsole::Group* RedCommandConsole::Group::List::Find(unsigned int uiID)
{
    for (Node* pNode = _head._pNext; pNode->_pObject; pNode = pNode->_pNext)
    {
        if (pNode->_pObject->mID == uiID)
        {
            return pNode->_pObject;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006eca50
RedCommandConsole::Group* RedCommandConsole::Group::List::Find(char const* pcName)
{
    return Find(PblHash(pcName));
}

// FUNCTION: SWBF2 0x006ec8f0
RedCommandConsole::Group::Group()
{
    mID = 0;
    mName[0] = 0;
    mParent = 0;
}

// FUNCTION: SWBF2 0x006ec840
RedCommandConsole::Group::Group(char const* pcName, Group* pParent)
{
    strlcpy(mName, pcName, sizeof(mName));

    mName[sizeof(mName) - 1] = 0;
    mID = PblHash(mName);
    if (pParent)
    {
        pParent->mChildList.Add(this);
        mParent = pParent;
    }
    else
    {
        mParent = 0;
    }
}

// FUNCTION: SWBF2 0x006ecfb0
RedCommandConsole::Group* RedCommandConsole::_AddGroup(char const* pcPath, bool bLeafIsCommand)
{
    char acName[32];

    if (!sRootNode)
    {
        sRootNode = new Group;
    }

    if (!*pcPath || isspace(*pcPath))
    {
        return bLeafIsCommand ? 0 : sRootNode;
    }

    Group* pGroup = sRootNode;
    char* pcOut = acName;

    while (*pcPath == '.')
    {
        ++pcPath;
    }

    while (*pcPath && !isspace(*pcPath))
    {
        if (*pcPath == '.')
        {
            *pcOut = 0;
            Group* pChild = pGroup->mChildList.Find(acName);
            if (pChild)
            {
                pGroup = pChild;
            }
            else
            {
                pGroup = new Group(acName, pGroup);
            }

            pcOut = acName;
            acName[0] = 0;
            while (*pcPath == '.')
            {
                ++pcPath;
            }
        }
        else
        {

            if (pcOut == acName + sizeof(acName) - 1)
            {
                return 0;
            }
            *pcOut++ = *pcPath++;
        }
    }

    *pcOut = 0;
    if (!bLeafIsCommand && acName[0])
    {
        Group* pChild = pGroup ? pGroup->mChildList.Find(acName) : 0;
        if (pChild)
        {
            return pChild;
        }
        pGroup = new Group(acName, pGroup);
    }
    return pGroup;
}

// FUNCTION: SWBF2 0x006ece20
RedCommandConsole::Command* RedCommandConsole::_AddCommand(char const* pcPath)
{
    char acName[32];

    if (!pcPath || !*pcPath)
    {
        return 0;
    }

    Group* pGroup = _AddGroup(pcPath, true);
    if (!pGroup)
    {
        return 0;
    }

    char const* pcName = 0;
    char const* pcEnd = pcPath;
    while (*pcEnd && !isspace(*pcEnd))
    {
        if (*pcEnd == '.')
        {
            pcName = pcEnd;
        }
        ++pcEnd;
    }
    if (pcName)
    {
        ++pcName;
    }
    else
    {
        pcName = pcPath;
    }

    if (!*pcName || isspace(*pcName))
    {
        return 0;
    }

    if (*pcEnd || pcEnd - pcName > (int)sizeof(acName) - 1)
    {
        strlcpy(acName, pcName, sizeof(acName));
        pcName = acName;
    }

    PblList<Command>::Itor itor(pGroup->mCommandList);
    while (itor)
    {
        int iOrder = _stricmp(pcName, itor->mName);
        if (iOrder == 0)
        {
            return 0;
        }
        if (iOrder < 0)
        {
            break;
        }
        ++itor;
    }

    Command* pCommand = new Command;
    strlcpy(pCommand->mName, pcName, sizeof(pCommand->mName));
    pCommand->mID = PblHash(pCommand->mName);
    pGroup->mCommandList.AddBefore(itor.GetNode(), pCommand, pCommand);
    return pCommand;
}

// FUNCTION: SWBF2 0x006ec9f0
int RedCommandConsole::AddVariable(char const* pcName, MultiVar const& var)
{
    Command* pCommand = _AddCommand(pcName);
    if (!pCommand)
    {
        return 0;
    }
    pCommand->mVar = var;
    return 1;
}

// FUNCTION: SWBF2 0x006ec9c0
int RedCommandConsole::AddCommand(char const* pcName,
                                  int (__cdecl* pFunc)(RedConsole*, unsigned int, char const*))
{
    Command* pCommand = _AddCommand(pcName);
    if (!pCommand)
    {
        return 0;
    }
    pCommand->mVar = MultiVar(pFunc);
    return 1;
}

// FUNCTION: SWBF2 0x006ecd20
void MultiVar::Set(char const* pcValue)
{
    switch (m_uiVarType)
    {
    case VAR_TYPE_BOOL:
        *m_pVar_BOOL = atoi(pcValue) != 0;
        break;
    case VAR_TYPE_CHAR:
        *m_pVar_CHAR = (char)atoi(pcValue);
        break;
    case VAR_TYPE_INT16:
        *m_pVar_INT16 = (short)atoi(pcValue);
        break;
    case VAR_TYPE_INT32:
        *m_pVar_INT32 = atoi(pcValue);
        break;
    case VAR_TYPE_INT64:
        *m_pVar_INT64 = _atoi64(pcValue);
        break;
    case VAR_TYPE_FLOAT32:
        *m_pVar_FLOAT32 = (float)atof(pcValue);
        break;
    case VAR_TYPE_FLOAT64:
        *(double*)m_pVar_FLOAT64 = atof(pcValue);
        break;
    case VAR_TYPE_STRING:
        strlcpy(m_pVar_STRING, pcValue, m_uiVarLength);
        break;
    }
}

// FUNCTION: SWBF2 0x006ecb50
void MultiVar::Run(RedConsole* pConsole, char const* pcName, unsigned int uiID, char const* pcArgs)
{
    if (!m_pVar)
    {
        return;
    }

    if (m_uiVarType == VAR_TYPE_CONSOLE_FUNCTION)
    {
        while (isspace(*pcArgs))
        {
            ++pcArgs;
        }
        m_pVar_CONSOLE_FUNCTION(pConsole, uiID, *pcArgs ? pcArgs : 0);
        return;
    }

    while (isspace(*pcArgs))
    {
        ++pcArgs;
    }
    if (*pcArgs)
    {
        Set(pcArgs);
        return;
    }

    switch (m_uiVarType)
    {
    case VAR_TYPE_BOOL:
        *m_pVar_BOOL = !*m_pVar_BOOL;
        pConsole->PrintfMsg("%s = %i\n", pcName, *m_pVar_BOOL);
        break;
    case VAR_TYPE_CHAR:
        pConsole->PrintfMsg("%s = %c\n", pcName, *m_pVar_CHAR);
        break;
    case VAR_TYPE_INT16:
        pConsole->PrintfMsg("%s = %i\n", pcName, *m_pVar_INT16);
        break;
    case VAR_TYPE_INT32:
        pConsole->PrintfMsg("%s = %i\n", pcName, *m_pVar_INT32);
        break;
    case VAR_TYPE_INT64:
        pConsole->PrintfMsg("%s = %i\n", pcName, *m_pVar_INT64);
        break;
    case VAR_TYPE_FLOAT32:
        pConsole->PrintfMsg("%s = %f\n", pcName, *m_pVar_FLOAT32);
        break;
    case VAR_TYPE_FLOAT64:
        pConsole->PrintfMsg("%s = %f\n", pcName, *(double*)m_pVar_FLOAT64);
        break;
    case VAR_TYPE_STRING:
        pConsole->PrintfMsg("%s = %s\n", pcName, m_pVar_STRING);
        break;
    default:
        pConsole->PrintfMsg("Unknown VarType: %d\n", m_uiVarType);
        break;
    }
}
