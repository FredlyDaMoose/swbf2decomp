#ifndef GF_STEAM_AUTH_H
#define GF_STEAM_AUTH_H

namespace gf
{

class SteamAuthProvider
{
public:
    // FUNCTION: SWBF2 0x00411c70
    SteamAuthProvider() : m_eResult(0), m_bInitialized(false) {}

    bool Init(unsigned int uiAppId, bool& bRestarting);
    void Shutdown();
    int GetEncryptedAppTicket(char* pTicket, int iSize);
    const char* GetPersonaName();

    // FUNCTION: SWBF2 0x00411c80
    bool IsInitialized() { return m_bInitialized; }

    // SYNTHETIC: SWBF2 0x00411c90
    // gf::SteamAuthProvider::operator=

private:
    int m_eResult;
    bool m_bInitialized;
};

SteamAuthProvider* _sap();

}

#endif
