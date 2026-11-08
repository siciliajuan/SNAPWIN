/*
 * MODTEST.C - Regression test for the 1.0 "missing Control Panel icon" bug.
 *
 * With SNAPWIN.EXE running, loads SYSTEM\SNAPCFG.CPL and checks that the
 * module returned really exports CPlApplet. In 1.0 the applet was called
 * SNAPWIN.CPL: Windows matched it to the already loaded SNAPWIN module
 * (the EXE), so CPlApplet was not found and Control Panel silently skipped
 * the applet.
 *
 * Result is written with OutputDebugString:
 *   "PASS: running=1 module=<handle> CPlApplet found"   (expected)
 *   "FAIL: ..."
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#define STRICT
#include <windows.h>

int PASCAL WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR c, int n)
{
    HINSTANCE hLib;
    FARPROC   p;
    MSG       m;
    DWORD     t;
    BOOL      running;
    char      sys[144], path[160], buf[120];
    (void)hi; (void)hp; (void)c; (void)n;

    WinExec("SNAPWIN.EXE", SW_SHOWNORMAL);
    t = GetTickCount();
    while (GetTickCount() - t < 2000)
        if (PeekMessage(&m, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&m);

    GetSystemDirectory(sys, sizeof(sys));
    wsprintf(path, "%s\\SNAPCFG.CPL", (LPSTR)sys);
    running = FindWindow("SnapWinHidden", NULL) != NULL;
    hLib    = LoadLibrary(path);
    p       = ((UINT)hLib >= 32) ? GetProcAddress(hLib, "CPlApplet") : NULL;

    wsprintf(buf, "%s: running=%d module=%u CPlApplet %s",
             (LPSTR)((running && p) ? "PASS" : "FAIL"),
             running, (UINT)hLib, (LPSTR)(p ? "found" : "NOT found"));
    OutputDebugString(buf);
    return (running && p) ? 0 : 1;
}
