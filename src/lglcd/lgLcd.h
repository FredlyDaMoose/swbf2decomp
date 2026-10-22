#pragma once

#include <windows.h>

#define LGLCD_E_SERVICE_NOT_ACTIVE  ERROR_SERVICE_NOT_ACTIVE
#define LGLCD_E_ALREADY_INITIALIZED ERROR_ALREADY_INITIALIZED
#define LGLCD_E_SERVER_UNAVAILABLE  RPC_S_SERVER_UNAVAILABLE

#define LGLCD_BMP_FORMAT_160x43x1   0x00000001

#define LGLCD_ASYNC_UPDATE_NORMAL   0x80000080

#define LGLCD_SYNC_UPDATE_NORMAL    0x00000080
#define LGLCD_ASYNC_UPDATE_ALERT    0x800000FF

struct lgLcdConnectContextA
{
    char const* appFriendlyName;
    BOOL isPersistent;
    BOOL isAutostartable;
    void* configCallback;
    void* configContext;
    int connection;
};
 

struct lgLcdOpenContext
{
    int connection;
    int index;
    void* softbuttonsChangedCallback;
    void* softbuttonsChangedContext;
    int device;
};
 

struct lgLcdDeviceDesc
{
    DWORD Width;
    DWORD Height;
    DWORD Bpp;
    DWORD NumSoftButtons;
};
 

struct lgLcdBitmap160x43x1
{
    DWORD Format;
    BYTE pixels[160 * 43];
};
 

struct lgLcdInterface
{
    DWORD (WINAPI* Init)();
    DWORD (WINAPI* DeInit)();
    void* mUnknown0008;
    DWORD (WINAPI* Connect)(lgLcdConnectContextA* pCtx);
    DWORD (WINAPI* Disconnect)(int iConnection);
    DWORD (WINAPI* Enumerate)(int iConnection, int iIndex,
                              lgLcdDeviceDesc* pDesc);
    DWORD (WINAPI* Open)(lgLcdOpenContext* pCtx);
    void* mUnknown001C;
    DWORD (WINAPI* ReadSoftButtons)(int iDevice, DWORD* pdwButtons);
    DWORD (WINAPI* UpdateBitmap)(int iDevice, void const* pBitmap,
                                 DWORD dwPriority);
};

DWORD WINAPI lgLcdInit();
DWORD WINAPI lgLcdConnectA(lgLcdConnectContextA* pCtx);
DWORD WINAPI lgLcdEnumerate(int iConnection, int iIndex, lgLcdDeviceDesc* pDesc);
DWORD WINAPI lgLcdOpen(lgLcdOpenContext* pCtx);

DWORD WINAPI lgLcdReadSoftButtons(int iDevice, DWORD* pdwButtons);
DWORD WINAPI lgLcdUpdateBitmap(int iDevice, void const* pBitmap, DWORD dwPriority);

DWORD WINAPI lgLcdDeInit();
DWORD WINAPI lgLcdDisconnect(int iConnection);
DWORD WINAPI lgLcdClose(int iDevice);

BOOL sub_4012D4();
BOOL sub_40136C();

extern int dword_1E2FD04;
extern lgLcdInterface* dword_1FAC3B0;
extern HMODULE hLibModule;
extern void* dword_1E2FBFC;

// VTABLE: SWBF2 0x00783e40
class RedLogitechLCD
{
public:

    RedLogitechLCD();
    virtual ~RedLogitechLCD();

    unsigned char mPortIndex;
    bool mOpened;
    bool mConnected;
    unsigned char mUnknown0007;
    lgLcdConnectContextA mConnectCtx;
    lgLcdOpenContext mOpenCtx;
    lgLcdBitmap160x43x1* mpBitmap;
    BITMAPINFO* mpBitmapInfo;
    HDC mhDC;
    HBITMAP mhBitmap;

    HGDIOBJ mhOldBitmap;
    void* mpBits;
    HFONT mhFont;
    int mWidth;
    int mHeight;
    DWORD mSoftButtons;
    DWORD mPrevSoftButtons;
};
 

extern RedLogitechLCD gLCDPanels[1];

void ConnectLCDPanel(RedLogitechLCD* pThis, unsigned char ucPortIndex);
void OpenLCDPanel(RedLogitechLCD* pThis);

void ClearLCD(RedLogitechLCD* pThis);
void PrepareLCDDrawing(RedLogitechLCD* pThis);
void DrawLCDText(RedLogitechLCD* pThis, char const* lpString, int x, int y);
void PresentLCD(RedLogitechLCD* pThis);
void UpdateLCDPanel(RedLogitechLCD* pThis, float fPriority);

void CloseLCDPanel(RedLogitechLCD* pThis);
void DisconnectLCDPanel(RedLogitechLCD* pThis);

extern char* lpString2;
extern int dword_7E143C;
