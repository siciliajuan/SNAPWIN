/*
 * HOOKTEST.C - Test window for the snapping logic.
 *
 * Opens a resizable window titled "HookTest", starts SNAPWIN.EXE and then
 * installs the SNAPHOOK.DLL hooks again *from this task*, pointing at
 * SnapWin's hidden window. The title changes to "HookTest OK" (or
 * "HookTest FAILED") once that is done.
 *
 * Why: Wine does not implement system-wide Win16 hooks (they only fire in
 * the task that installed them), so under Wine the hooks must be installed
 * in the same task as the window being tested. On real Windows 3.1 this
 * extra step is harmless; there you can test with any application.
 *
 * Run from the directory that holds SNAPWIN.EXE and SNAPHOOK.DLL.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#define STRICT
#include <windows.h>

typedef BOOL (FAR PASCAL *SNAPINSTALLPROC)(HWND);

LRESULT __export FAR PASCAL WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(h, m, w, l);
}

int PASCAL WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR c, int n)
{
    WNDCLASS wc = {0};
    MSG      msg;
    HWND     h, hSnap = NULL;
    DWORD    t;
    (void)hp; (void)c;

    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hi;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "HookTest";
    RegisterClass(&wc);

    h = CreateWindow("HookTest", "HookTest", WS_OVERLAPPEDWINDOW,
                     200, 150, 300, 200, NULL, NULL, hi, NULL);
    ShowWindow(h, n);
    UpdateWindow(h);

    WinExec("SNAPWIN.EXE", SW_SHOWNORMAL);

    /* Wait up to 5 s for SnapWin's hidden window */
    t = GetTickCount();
    while (!hSnap && GetTickCount() - t < 5000) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&msg);
        hSnap = FindWindow("SnapWinHidden", NULL);
    }
    if (hSnap) {
        HINSTANCE       hLib  = LoadLibrary("SNAPHOOK.DLL");
        SNAPINSTALLPROC pInst = (UINT)hLib > 32
            ? (SNAPINSTALLPROC)GetProcAddress(hLib, "SNAPINSTALL") : NULL;
        SetWindowText(h, (pInst && pInst(hSnap)) ? "HookTest OK" : "HookTest FAILED");
    } else
        SetWindowText(h, "HookTest FAILED (SnapWin not running)");

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
