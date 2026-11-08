/*
 * SNAPWIN.C - SnapWin's resident program (no user interface).
 *
 * Started automatically from the "load=" line of WIN.INI. It creates a
 * hidden window, installs the hooks in SNAPHOOK.DLL and moves windows when
 * the DLL asks it to.
 *
 *   SNAPWIN.EXE          start (does nothing if it is already running)
 *   SNAPWIN.EXE /QUIT    stop the running instance
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#include <string.h>
#include "snapwin.h"

LRESULT __export FAR PASCAL SnapWndProc(HWND, UINT, WPARAM, LPARAM);

/*
 * Moves the window to the requested position (SNAP_*).
 * Resizable window -> half or quarter of the screen.
 * Fixed-size window (dialogs, Calculator...) -> only pushed to that side/corner.
 */
static void SnapWindow(HWND hwnd, int pos)
{
    LONG style;
    RECT rc;
    int  cxScr, cyScr, x, y, w, h;
    BOOL right, bottom, quadrant;
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;

    if (pos < 1 || pos > SNAP_COUNT)
        return;
    if (!hwnd || !IsWindow(hwnd) || !IsWindowVisible(hwnd))
        return;
    style = GetWindowLong(hwnd, GWL_STYLE);
    if (style & WS_CHILD)            /* MDI children and controls are left alone */
        return;

    if (IsIconic(hwnd) || IsZoomed(hwnd))
        ShowWindow(hwnd, SW_RESTORE);

    cxScr    = GetSystemMetrics(SM_CXSCREEN);
    cyScr    = GetSystemMetrics(SM_CYSCREEN);
    quadrant = pos >= SNAP_TOPLEFT;
    right    = (pos == SNAP_RIGHT || pos == SNAP_TOPRIGHT || pos == SNAP_BOTRIGHT);
    bottom   = (pos == SNAP_BOTLEFT || pos == SNAP_BOTRIGHT);

    if (style & WS_THICKFRAME) {
        /* The right/bottom part gets the extra pixel when the size is odd */
        w = right  ? cxScr - cxScr / 2 : cxScr / 2;
        h = !quadrant ? cyScr : (bottom ? cyScr - cyScr / 2 : cyScr / 2);
        x = right  ? cxScr / 2 : 0;
        y = bottom ? cyScr / 2 : 0;
    } else {
        GetWindowRect(hwnd, &rc);
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
        x = right ? cxScr - w : 0;
        if (quadrant)
            y = bottom ? cyScr - h : 0;
        else {
            y = rc.top;                       /* half: keep the vertical position */
            if (y + h > cyScr) y = cyScr - h;
        }
        if (y < 0) y = 0;
        flags |= SWP_NOSIZE;
    }
    SetWindowPos(hwnd, NULL, x, y, w, h, flags);
}

LRESULT __export FAR PASCAL SnapWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_SNAP_DO: {
        HWND target = (HWND)LOWORD(lParam);
        if (!target)
            target = GetActiveWindow();
        if (target && target != hwnd)
            SnapWindow(target, (int)wParam);
        return 0;
    }
    case WM_SNAP_RELOAD:
        SnapReload();
        return 0;
    case WM_ENDSESSION:
        if (wParam)
            SnapRemove();
        return 0;
    case WM_DESTROY:
        SnapRemove();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static BOOL HasArg(LPSTR cmd, LPCSTR arg)
{
    char buf[64];
    lstrcpyn(buf, cmd, sizeof(buf));
    AnsiUpper(buf);
    return _fstrstr(buf, arg) != NULL;
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    WNDCLASS wc;
    HWND     hwnd;
    MSG      msg;
    (void)nShow;

    if (HasArg(lpCmd, "QUIT")) {
        hwnd = FindWindow(SNAPWIN_CLASS, NULL);
        if (hwnd)
            SendMessage(hwnd, WM_CLOSE, 0, 0L);
        return 0;
    }

    if (hPrev || FindWindow(SNAPWIN_CLASS, NULL))
        return 0;   /* already running */

    wc.style         = 0;
    wc.lpfnWndProc   = SnapWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(1));
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = SNAPWIN_CLASS;
    if (!RegisterClass(&wc))
        return 1;

    /* Never shown, so it gets no icon and does not appear in the Task List.
       (Programs started from "load=" are normally shown minimized.) */
    hwnd = CreateWindow(SNAPWIN_CLASS, "SnapWin", WS_OVERLAPPED,
                        0, 0, 0, 0, NULL, NULL, hInst, NULL);
    if (!hwnd)
        return 1;

    if (!SnapInstall(hwnd)) {
        char msg[200];
        SnapLoadString(hInst, IDS_SNAP_NOHOOKS, msg, sizeof(msg));
        MessageBox(NULL, msg, "SnapWin", MB_OK | MB_ICONSTOP);
        DestroyWindow(hwnd);
        return 1;
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
