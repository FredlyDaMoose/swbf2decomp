#include <string.h>

#include "lgLcd.h"
#include "RedEngineFL/Memory/engineAlloc.h"

// GLOBAL: SWBF2 0x01e2fd04
int dword_1E2FD04;

// GLOBAL: SWBF2 0x01fac3b0
lgLcdInterface* dword_1FAC3B0;

HMODULE hLibModule;
// GLOBAL: SWBF2 0x01e2fbfc
void* dword_1E2FBFC;

char* lpSubKey =
    "SOFTWARE\\Classes\\CLSID\\{FE750200-B72E-11d9-829B-0050DA1A72D3}\\ServerBinary";

char Data[0x104];

char ProcName[] = "GetInterface";

typedef lgLcdInterface* (WINAPI *GetInterfaceFn)(int iVersion);

// FUNCTION: SWBF2 0x004012d4
BOOL sub_4012D4()
{
    BOOL bOk = FALSE;

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        DWORD dwType;
        DWORD cbData = sizeof(Data);
        if (RegQueryValueExA(hKey, 0, 0, &dwType, (LPBYTE)Data, &cbData) == ERROR_SUCCESS)
        {
            hLibModule = LoadLibraryA(Data);
            if (hLibModule)
            {
                dword_1E2FBFC = GetProcAddress(hLibModule, ProcName);
                if (dword_1E2FBFC)
                {
                    dword_1FAC3B0 = ((GetInterfaceFn)dword_1E2FBFC)(1);
                    bOk = (dword_1FAC3B0 != 0);
                }

                if (!bOk)
                {
                    sub_40136C();
                }
            }
        }
        RegCloseKey(hKey);
    }

    return bOk;
}

// FUNCTION: SWBF2 0x0040136c
BOOL sub_40136C()
{
    if (hLibModule)
    {
        FreeLibrary(hLibModule);
        hLibModule = 0;
    }
    dword_1FAC3B0 = 0;

    return TRUE;
}

// FUNCTION: SWBF2 0x004012a5
DWORD WINAPI lgLcdInit()
{
    if (dword_1E2FD04)
    {
        return LGLCD_E_ALREADY_INITIALIZED;
    }

    if (!sub_4012D4())
    {
        return LGLCD_E_SERVER_UNAVAILABLE;
    }

    dword_1E2FD04 = 1;
    return dword_1FAC3B0->Init();
}

// FUNCTION: SWBF2 0x004013b5
DWORD WINAPI lgLcdConnectA(lgLcdConnectContextA* pCtx)
{
    if (!dword_1E2FD04)
    {
        return LGLCD_E_SERVICE_NOT_ACTIVE;
    }
    return dword_1FAC3B0->Connect(pCtx);
}

// FUNCTION: SWBF2 0x004013f3
DWORD WINAPI lgLcdEnumerate(int iConnection, int iIndex, lgLcdDeviceDesc* pDesc)
{
    if (!dword_1E2FD04)
    {
        return LGLCD_E_SERVICE_NOT_ACTIVE;
    }
    return dword_1FAC3B0->Enumerate(iConnection, iIndex, pDesc);
}

// FUNCTION: SWBF2 0x0040141a
DWORD WINAPI lgLcdOpen(lgLcdOpenContext* pCtx)
{
    if (!dword_1E2FD04)
    {
        return LGLCD_E_SERVICE_NOT_ACTIVE;
    }
    return dword_1FAC3B0->Open(pCtx);
}

// FUNCTION: SWBF2 0x00401458
DWORD WINAPI lgLcdReadSoftButtons(int iDevice, DWORD* pdwButtons)
{
    if (!dword_1E2FD04)
    {
        return LGLCD_E_SERVICE_NOT_ACTIVE;
    }
    return dword_1FAC3B0->ReadSoftButtons(iDevice, pdwButtons);
}

// FUNCTION: SWBF2 0x0040147b
DWORD WINAPI lgLcdUpdateBitmap(int iDevice, void const* pBitmap, DWORD dwPriority)
{
    if (!dword_1E2FD04)
    {
        return LGLCD_E_SERVICE_NOT_ACTIVE;
    }
    return dword_1FAC3B0->UpdateBitmap(iDevice, pBitmap, dwPriority);
}

// GLOBAL: SWBF2 0x0099e768
// SYNTHETIC: SWBF2 0x00408600
// `dynamic initializer for 'gLCDPanels''
// SYNTHETIC: SWBF2 0x007699f0
// `dynamic atexit destructor for 'gLCDPanels''

RedLogitechLCD gLCDPanels[1];

// FUNCTION: SWBF2 0x006eff40
RedLogitechLCD::RedLogitechLCD()
{
    mOpened = false;
    mConnected = false;
    mpBitmapInfo = 0;
    mpBitmap = 0;
    mhBitmap = 0;
    mhOldBitmap = 0;
    mhDC = 0;
    mhFont = 0;
    mSoftButtons = 0;
    mPrevSoftButtons = 0;
}

// FUNCTION: SWBF2 0x006eff90
RedLogitechLCD::~RedLogitechLCD()
{
}

// SYNTHETIC: SWBF2 0x006effa0
// RedLogitechLCD::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006f03c0
void ConnectLCDPanel(RedLogitechLCD* pThis, unsigned char ucPortIndex)
{
    pThis->mPortIndex = ucPortIndex;

    DWORD dwResult = lgLcdInit();
    pThis->mConnected = (dwResult == 0 || dwResult == LGLCD_E_ALREADY_INITIALIZED);

    if (pThis->mConnected)
    {

        pThis->mConnectCtx.appFriendlyName = "Star Wars Battlefront II";
        pThis->mConnectCtx.isPersistent = 1;
        pThis->mConnectCtx.isAutostartable = 0;
        pThis->mConnectCtx.configCallback = 0;
        pThis->mConnectCtx.configContext = 0;
        pThis->mConnectCtx.connection = -1;

        if (lgLcdConnectA(&pThis->mConnectCtx) != 0)
        {
            pThis->mConnected = false;
        }
    }
}

// FUNCTION: SWBF2 0x006f0430
void OpenLCDPanel(RedLogitechLCD* pThis)
{
    if (pThis->mOpened)
    {
        return;
    }
    if (!pThis->mConnected)
    {
        return;
    }

    pThis->mpBitmapInfo = 0;
    pThis->mpBitmap = 0;
    pThis->mhFont = 0;
    pThis->mSoftButtons = 0;
    pThis->mPrevSoftButtons = 0;

    lgLcdDeviceDesc desc;
    if (lgLcdEnumerate(pThis->mConnectCtx.connection, pThis->mPortIndex, &desc) != 0)
    {
        return;
    }

    pThis->mOpenCtx.connection = pThis->mConnectCtx.connection;
    pThis->mOpenCtx.index = pThis->mPortIndex;
    pThis->mOpenCtx.softbuttonsChangedCallback = 0;
    pThis->mOpenCtx.softbuttonsChangedContext = 0;
    pThis->mOpenCtx.device = -1;
    if (lgLcdOpen(&pThis->mOpenCtx) != 0)
    {
        return;
    }

    pThis->mpBitmap = new lgLcdBitmap160x43x1;
    pThis->mpBitmap->Format = LGLCD_BMP_FORMAT_160x43x1;
    lgLcdUpdateBitmap(pThis->mOpenCtx.device, pThis->mpBitmap, LGLCD_ASYNC_UPDATE_NORMAL);

    pThis->mhDC = CreateCompatibleDC(0);

    const int iInfoSize = sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD);
    pThis->mpBitmapInfo = (BITMAPINFO*)new BYTE[iInfoSize];
    memset(pThis->mpBitmapInfo, 0, iInfoSize);

    pThis->mWidth = 160;
    pThis->mHeight = 43;

    BITMAPINFO* pInfo = pThis->mpBitmapInfo;
    pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pInfo->bmiHeader.biWidth = pThis->mWidth;

    pInfo->bmiHeader.biHeight = -pThis->mHeight;
    pInfo->bmiHeader.biPlanes = 1;
    pInfo->bmiHeader.biBitCount = 8;
    pInfo->bmiHeader.biCompression = BI_RGB;

    pInfo->bmiHeader.biSizeImage = pInfo->bmiHeader.biBitCount
                                   * pInfo->bmiHeader.biHeight
                                   * pInfo->bmiHeader.biWidth / 8;
    pInfo->bmiHeader.biXPelsPerMeter = 3200;
    pInfo->bmiHeader.biYPelsPerMeter = 3200;
    pInfo->bmiHeader.biClrUsed = 256;
    pInfo->bmiHeader.biClrImportant = 256;

    for (int i = 0; i < 256; i++)
    {
        BYTE cLevel = (i <= 128) ? 0 : 0xFF;
        pInfo->bmiColors[i].rgbRed = cLevel;
        pInfo->bmiColors[i].rgbGreen = cLevel;
        pInfo->bmiColors[i].rgbBlue = cLevel;
        pInfo->bmiColors[i].rgbReserved = 0;
    }

    pThis->mhBitmap = CreateDIBSection(pThis->mhDC, pThis->mpBitmapInfo, DIB_RGB_COLORS,
                                       &pThis->mpBits, 0, 0);
    pThis->mhOldBitmap = 0;
    pThis->mOpened = true;
}

char* lpString2 = "Arial";

// GLOBAL: SWBF2 0x007e143c
int dword_7E143C = 11;

// FUNCTION: SWBF2 0x006f00f0
void ClearLCD(RedLogitechLCD* pThis)
{
    if (!pThis->mOpened)
    {
        return;
    }
    if (!pThis->mConnected)
    {
        return;
    }
    if (!pThis->mpBits)
    {
        return;
    }

    memset(pThis->mpBits, 0, pThis->mHeight * pThis->mWidth);
}

// FUNCTION: SWBF2 0x006effd0
void PrepareLCDDrawing(RedLogitechLCD* pThis)
{
    if (!pThis->mOpened)
    {
        return;
    }
    if (!pThis->mConnected)
    {
        return;
    }
    if (pThis->mhOldBitmap)
    {
        return;
    }

    pThis->mhOldBitmap = SelectObject(pThis->mhDC, pThis->mhBitmap);
    SetTextColor(pThis->mhDC, 0x00FFFFFF);
    SetBkColor(pThis->mhDC, 0);

    LOGFONTA lf;
    lf.lfHeight = 0;
    if (pThis->mhFont)
    {
        memset(&lf, 0, sizeof(lf));
        GetObjectA(pThis->mhFont, sizeof(lf), &lf);
    }

    if (lf.lfHeight == -dword_7E143C)
    {
        return;
    }

    if (pThis->mhFont)
    {
        DeleteObject(pThis->mhFont);
    }

    lf.lfHeight = -dword_7E143C;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_NORMAL;
    lf.lfItalic = 0;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = 0;
    lf.lfOutPrecision = 0;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;
    lstrcpyA(lf.lfFaceName, lpString2);

    pThis->mhFont = CreateFontIndirectA(&lf);
}

// FUNCTION: SWBF2 0x006f0180
void DrawLCDText(RedLogitechLCD* pThis, char const* lpString, int x, int y)
{
    if (!pThis->mOpened)
    {
        return;
    }
    if (!pThis->mConnected)
    {
        return;
    }

    PrepareLCDDrawing(pThis);

    if (!pThis->mhFont)
    {
        return;
    }

    int iOldMapMode = SetMapMode(pThis->mhDC, MM_TEXT);
    int iOldBkMode = SetBkMode(pThis->mhDC, TRANSPARENT);
    HGDIOBJ hOldFont = SelectObject(pThis->mhDC, pThis->mhFont);

    TextOutA(pThis->mhDC, x, y, lpString, (int)strlen(lpString));

    SetMapMode(pThis->mhDC, iOldMapMode);
    SetBkMode(pThis->mhDC, iOldBkMode);
    SelectObject(pThis->mhDC, hOldFont);
}

// FUNCTION: SWBF2 0x006f03b0
void PresentLCD(RedLogitechLCD* pThis)
{
    UpdateLCDPanel(pThis, -1.0f);
}

// FUNCTION: SWBF2 0x006f06c0
void UpdateLCDPanel(RedLogitechLCD* pThis, float fPriority)
{
    if (!pThis->mOpened)
    {
        return;
    }
    if (!pThis->mConnected)
    {
        return;
    }

    if (fPriority > 0.0f)
    {
        pThis->mPrevSoftButtons = pThis->mSoftButtons;
        lgLcdReadSoftButtons(pThis->mOpenCtx.device, &pThis->mSoftButtons);
    }

    if (!pThis->mpBitmap)
    {
        return;
    }
    if (!pThis->mpBits)
    {
        return;
    }

    if (pThis->mhOldBitmap)
    {
        GdiFlush();
        SelectObject(pThis->mhDC, pThis->mhOldBitmap);
        pThis->mhOldBitmap = 0;
        memcpy(pThis->mpBitmap->pixels, pThis->mpBits,
               pThis->mWidth * pThis->mHeight);
        memset(pThis->mpBits, 0, pThis->mWidth * pThis->mHeight);
    }

    lgLcdUpdateBitmap(pThis->mOpenCtx.device, pThis->mpBitmap,
                      0.0f > fPriority ? LGLCD_ASYNC_UPDATE_ALERT
                                       : LGLCD_SYNC_UPDATE_NORMAL);
}

// FUNCTION: SWBF2 0x0040138e
DWORD WINAPI lgLcdDeInit()
{
    if (!dword_1E2FD04)
    {
        return 0;
    }
    DWORD dwResult = dword_1FAC3B0->DeInit();
    sub_40136C();
    dword_1E2FD04 = 0;
    return dwResult;
}

// FUNCTION: SWBF2 0x004013d4
DWORD WINAPI lgLcdDisconnect(int iConnection)
{
    return dword_1E2FD04 ? dword_1FAC3B0->Disconnect(iConnection) : LGLCD_E_SERVICE_NOT_ACTIVE;
}

// FUNCTION: SWBF2 0x00401439
DWORD WINAPI lgLcdClose(int iDevice)
{
    typedef DWORD (WINAPI* CloseFn)(int);
    return dword_1E2FD04 ? ((CloseFn)dword_1FAC3B0->mUnknown001C)(iDevice) : LGLCD_E_SERVICE_NOT_ACTIVE;
}
