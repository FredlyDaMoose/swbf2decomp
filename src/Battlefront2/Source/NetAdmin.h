#pragma once

struct Admin
{
    unsigned int addr;
    unsigned int port;
    float lastRecvTime;
    bool admin;
    char pad[3];
};
 

extern Admin adminDests[3];
extern int adminKeyCount;
extern float adminKeyNextTime;
extern char adminKey[2];

void __fastcall HeartBeat(const unsigned __int64* pPeer);
int __fastcall FindAdminSource(int iDest);
