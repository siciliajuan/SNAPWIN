/*
 * CPLTEST.C - Opens the SnapWin applet the way Control Panel does.
 *
 * Starts SNAPWIN.EXE first (as it would be after Windows starts), then loads
 * SYSTEM\SNAPCFG.CPL and walks through the CPlApplet protocol:
 * CPL_INIT, CPL_GETCOUNT, CPL_INQUIRE, CPL_DBLCLK (shows the dialog), CPL_EXIT.
 * The applet's name and description are written with OutputDebugString
 * (visible with WINEDEBUG=+debugstr under Wine, or DBWIN on Windows 3.1).
 *
 * Useful under Wine, which has no Windows 3.1 Control Panel.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#define STRICT
#include <windows.h>

typedef LONG (FAR PASCAL *CPLPROC)(HWND, UINT, LPARAM, LPARAM);
typedef struct { int idIcon, idName, idInfo; LONG lData; } CPLINFO;

int PASCAL WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR c, int n)
{
    HINSTANCE hLib;
    CPLPROC   p;
    CPLINFO   ci;
    char      sys[144], path[160], name[64], info[128], buf[256];
    MSG       m;
    DWORD     t;
    (void)hi; (void)hp; (void)c; (void)n;

    WinExec("SNAPWIN.EXE", SW_SHOWNORMAL);
    t = GetTickCount();
    while (GetTickCount() - t < 2000)
        if (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&m);

    GetSystemDirectory(sys, sizeof(sys));
    wsprintf(path, "%s\\SNAPCFG.CPL", (LPSTR)sys);
    hLib = LoadLibrary(path);
    if ((UINT)hLib < 32) {
        wsprintf(buf, "LoadLibrary(%s) failed: %u", (LPSTR)path, (UINT)hLib);
        MessageBox(NULL, buf, "CplTest", MB_OK | MB_ICONSTOP);
        return 1;
    }
    p = (CPLPROC)GetProcAddress(hLib, "CPLAPPLET");
    if (!p) {
        MessageBox(NULL, "CPlApplet not found", "CplTest", MB_OK | MB_ICONSTOP);
        return 1;
    }
    if (!p(NULL, 1 /* CPL_INIT */, 0, 0))
        return 2;
    wsprintf(buf, "applets=%ld", p(NULL, 2 /* CPL_GETCOUNT */, 0, 0));
    OutputDebugString(buf);
    p(NULL, 3 /* CPL_INQUIRE */, 0, (LPARAM)(CPLINFO FAR *)&ci);
    LoadString(hLib, ci.idName, name, sizeof(name));
    LoadString(hLib, ci.idInfo, info, sizeof(info));
    OutputDebugString(name);
    OutputDebugString(info);
    p(NULL, 5 /* CPL_DBLCLK */, 0, ci.lData);
    p(NULL, 7 /* CPL_EXIT */, 0, 0);
    FreeLibrary(hLib);
    return 0;
}
