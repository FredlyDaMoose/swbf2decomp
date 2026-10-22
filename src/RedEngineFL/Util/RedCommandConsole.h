#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"

class RedConsole;

struct MultiVar
{

    enum VAR_TYPE
    {
        VAR_TYPE_NULL = 0,
        VAR_TYPE_BOOL = 1,
        VAR_TYPE_CHAR = 2,
        VAR_TYPE_INT16 = 3,
        VAR_TYPE_INT32 = 4,
        VAR_TYPE_INT64 = 5,
        VAR_TYPE_FLOAT32 = 6,
        VAR_TYPE_FLOAT64 = 7,
        VAR_TYPE_STRING = 8,
        VAR_TYPE_CONSOLE_FUNCTION = 9,
    };

    MultiVar()
    {
        m_uiVarType = 0;
        m_uiVarLength = 0;
        m_pVar = 0;
    }

    MultiVar(bool* pVar)
    {
        m_uiVarType = VAR_TYPE_BOOL;
        m_uiVarLength = sizeof(bool);
        m_pVar_BOOL = pVar;
    }

    MultiVar(int (__cdecl* pFunc)(RedConsole*, unsigned int, char const*)) { InitFunction(pFunc); }

    void Set(char const* pcValue);
    void Run(RedConsole* pConsole, char const* pcName, unsigned int uiID, char const* pcArgs);

private:
    void InitFunction(int (__cdecl* pFunc)(RedConsole*, unsigned int, char const*))
    {
        m_uiVarType = VAR_TYPE_CONSOLE_FUNCTION;
        m_uiVarLength = 0;
        m_pVar_CONSOLE_FUNCTION = pFunc;
    }

public:
    unsigned int m_uiVarType : 4;
    unsigned int m_uiVarLength : 28;
    union
    {
        void* m_pVar;
        char* m_pVar_STRING;
        bool* m_pVar_BOOL;
        char* m_pVar_CHAR;
        short* m_pVar_INT16;
        int* m_pVar_INT32;
        __int64* m_pVar_INT64;
        float* m_pVar_FLOAT32;

        float* m_pVar_FLOAT64;

        int (__cdecl* m_pVar_CONSOLE_FUNCTION)(RedConsole*, unsigned int, char const*);
    };
};
 

class RedCommandConsole
{
public:

    class Command : public PblList<Command>::Node
    {
    public:

        Command() {}

        const char* GetName() const { return mName; }
        void Run(RedConsole* pConsole, char const* pcArgs);

        char mName[32];
        unsigned int mID;
        MultiVar mVar;
    };

    class Group
    {
    public:
        class List : public PblList<Group>
        {
        public:
            void Add(Group* pGroup);
            Group* Find(unsigned int uiID);
            Group* Find(char const* pcName);
        };

        Group();
        Group(char const* pcName, Group* pParent);

        const PblList<Command>& GetCommandList() const { return mCommandList; }
        Command* FindCommand(unsigned int uiID);
        Command* FindCommand(char const* pcName);

        unsigned int mID;
        char mName[32];
        PblList<Command> mCommandList;
        Group* mParent;
        List mChildList;
        PblList<Group>::Node _Node;
    };

    RedCommandConsole(char const* pcName, MultiVar const& var) { AddVariable(pcName, var); }

    // FUNCTION: SWBF2 0x00483200
    RedCommandConsole(char const* pcName, int (__cdecl* pFunc)(RedConsole*, unsigned int, char const*))
    {
        AddCommand(pcName, pFunc);
    }

    static int __cdecl AddVariable(char const* pcName, MultiVar const& var);
    static int __cdecl AddCommand(char const* pcName,
                                  int (__cdecl* pFunc)(RedConsole*, unsigned int, char const*));

    static Group* sRootNode;

    static int __cdecl Process(RedConsole* pConsole, char const* pcLine);
    static Command* __cdecl _FindCommand(char const* pcPath);
    static Group* __cdecl _FindGroup(char const* pcPath, bool bLeafIsCommand);
    static const Group* FindGroup(char const* pcName) { return _FindGroup(pcName, false); }

private:
    static Command* __cdecl _AddCommand(char const* pcPath);
    static Group* __cdecl _AddGroup(char const* pcPath, bool bLeafIsCommand);
};

 
 
 
 
 
 
 
 
 
 
 
 
 
 
