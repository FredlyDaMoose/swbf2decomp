#pragma once

class CommandPost;

class CommandPostItor
{
public:
    operator bool();
    CommandPost* operator*();
    const CommandPostItor& operator++();

    int mIndex;
};
 

class CommandPostManager
{
public:

    CommandPostManager();

    static void Create();

    static void Destroy();

    int GetCount() const;
    CommandPostItor GetItor() const;
    CommandPost* FindPostFromIndex(int iIndex) const;
    CommandPost* FindPostFromID(unsigned int uID) const;
    CommandPost* FindPost(const char* pName) const;
    CommandPost* FindPostClosestToEnemy(int iTeam) const;

    // FUNCTION: SWBF2 0x00439b40
    static CommandPostManager* Instance() { return sInstance; }

    static CommandPostManager* sInstance;
};

 
