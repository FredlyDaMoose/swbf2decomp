#pragma once

#include <stddef.h>

namespace Snd
{

class GSM
{
public:
    void Decode(unsigned char (*pFrame)[33], short (*pSamples)[160]);
    void Encode(short (*pSamples)[160], unsigned char (*pFrame)[33]);
    void Init();

    short dp0[280];
    short z1;
    int L_z2;
    int mp;
    short u[8];
    short LARpp[2][8];
    short j;
    short ltp_cut;
    short nrp;
    short v[9];
    short msr;
    char verbose;
    char fast;
    char wav_fmt;
    unsigned char frame_index;
    unsigned char frame_chain;
};

 
 
 
 
 
 
 
 
 

}
