/*
 * SNAPHOOK.C - DLL holding SnapWin's system-wide hooks.
 *
 * On Windows 3.1x, system-wide hooks must live in a DLL. A Win16 DLL has a
 * single data segment shared by every task, so the globals below are the
 * shared state between all the applications the hooks run in.
 *
 *  - WH_KEYBOARD   : detects the configured shortcuts and notifies SNAPWIN.EXE.
 *  - WH_CALLWNDPROC: detects the start of a drag (WM_SYSCOMMAND/SC_MOVE) and
 *                    its end (WM_WINDOWPOSCHANGED or WM_EXITSIZEMOVE). If the
 *                    cursor is in a corner or on the left/right edge when the
 *                    window is dropped, it notifies SNAPWIN.EXE.
 *
 * The hooks never move windows themselves: they post WM_SNAP_DO to the hidden
 * window of SNAPWIN.EXE, which does the work outside the hooked application.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#include "snapwin.h"

/* What a WH_CALLWNDPROC hook receives in lParam on Win16 */
typedef struct tagCWPSTRUCT16 {
    LPARAM lParam;
    WPARAM wParam;
    UINT   message;
    HWND   hwnd;
} CWPSTRUCT16;
typedef CWPSTRUCT16 FAR *LPCWPSTRUCT16;

/* Not sent by Windows 3.1 itself, but by some environments (e.g. full-window
   drag); not defined in the Win16 headers. */
#ifndef WM_ENTERSIZEMOVE
#define WM_ENTERSIZEMOVE 0x0231
#endif
#ifndef WM_EXITSIZEMOVE
#define WM_EXITSIZEMOVE  0x0232
#endif

static HINSTANCE g_hInst;
static HHOOK     g_hKbd;
static HHOOK     g_hCwp;
static HWND      g_hNotify;   /* hidden window of SNAPWIN.EXE */
static HWND      g_hMoving;   /* window currently being dragged */
static BOOL      g_bLoopMsgs; /* the system sends WM_ENTER/EXITSIZEMOVE */
static SNAPCFG   g_cfg;

LRESULT __export FAR PASCAL SnapKbdProc(int code, WPARAM wParam, LPARAM lParam);
LRESULT __export FAR PASCAL SnapCwpProc(int code, WPARAM wParam, LPARAM lParam);

int FAR PASCAL LibMain(HINSTANCE hInst, WORD wDataSeg, WORD cbHeap, LPSTR lpCmd)
{
    (void)wDataSeg; (void)cbHeap; (void)lpCmd;
    g_hInst = hInst;
    return 1;
}

int __export FAR PASCAL WEP(int nExit)
{
    (void)nExit;
    return 1;
}

/* ---------------------------------------------------------------- */

static UINT CurrentMods(void)
{
    UINT m = 0;
    if (GetAsyncKeyState(VK_SHIFT)   & 0x8000) m |= SMOD_SHIFT;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) m |= SMOD_CTRL;
    if (GetAsyncKeyState(VK_MENU)    & 0x8000) m |= SMOD_ALT;
    return m;
}

/* Returns the SNAP_* position of the shortcut being pressed, or 0 */
static int MatchHotkey(UINT vk)
{
    int  i;
    UINT mods = 0;
    BOOL got  = FALSE;
    for (i = 0; i < SNAP_COUNT; i++) {
        if (g_cfg.key[i] && g_cfg.key[i] == vk) {
            if (!got) { mods = CurrentMods(); got = TRUE; }
            if (mods == g_cfg.mods[i])
                return i + 1;
        }
    }
    return 0;
}

static BOOL MainButtonDown(void)
{
    int vk = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

/*
 * Looks at where the window was dropped and requests the matching snap:
 *  - Corner (on the left/right edge near the top/bottom, or on the
 *    top/bottom edge near the left/right)  -> quarter
 *  - Anywhere else on the left/right edge  -> half
 */
static void CheckEdgeAndSnap(HWND hwnd)
{
    POINT pt;
    int   cx = GetSystemMetrics(SM_CXSCREEN);
    int   cy = GetSystemMetrics(SM_CYSCREEN);
    int   zx = cx / SNAP_CORNER_DIV, zy = cy / SNAP_CORNER_DIV;
    BOOL  atL, atR, atT, atB, nearT, nearB, nearL, nearR;
    int   pos = 0;

    if (zx < SNAP_CORNER_MIN) zx = SNAP_CORNER_MIN;
    if (zy < SNAP_CORNER_MIN) zy = SNAP_CORNER_MIN;
    GetCursorPos(&pt);

    atL = pt.x <= SNAP_EDGE;   atR = pt.x >= cx - 1 - SNAP_EDGE;
    atT = pt.y <= SNAP_EDGE;   atB = pt.y >= cy - 1 - SNAP_EDGE;
    nearL = pt.x < zx;         nearR = pt.x >= cx - zx;
    nearT = pt.y < zy;         nearB = pt.y >= cy - zy;

    if (g_cfg.bCorners) {
        if      ((atL && nearT) || (atT && nearL)) pos = SNAP_TOPLEFT;
        else if ((atR && nearT) || (atT && nearR)) pos = SNAP_TOPRIGHT;
        else if ((atL && nearB) || (atB && nearL)) pos = SNAP_BOTLEFT;
        else if ((atR && nearB) || (atB && nearR)) pos = SNAP_BOTRIGHT;
    }
    if (!pos && g_cfg.bDrag) {
        if (atL)      pos = SNAP_LEFT;
        else if (atR) pos = SNAP_RIGHT;
    }
    if (pos)
        PostMessage(g_hNotify, WM_SNAP_DO, (WPARAM)pos, MAKELPARAM((UINT)hwnd, 0));
}

/* ---------------------------------------------------------------- */

LRESULT __export FAR PASCAL SnapKbdProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= 0 && g_hNotify) {
        int pos = MatchHotkey((UINT)wParam);
        if (pos) {
            /* bit 31 = key released, bit 30 = key was already down (auto-repeat) */
            if (code == HC_ACTION && !(lParam & 0x80000000L) && !(lParam & 0x40000000L)) {
                /* Get the active window here, in the context of the task that
                   receives the key; SNAPWIN.EXE would see its own state. */
                HWND hAct = GetActiveWindow();
                PostMessage(g_hNotify, WM_SNAP_DO, (WPARAM)pos, MAKELPARAM((UINT)hAct, 0));
            }
            return 1L;   /* swallow the key: the application never sees it */
        }
    }
    return CallNextHookEx(g_hKbd, code, wParam, lParam);
}

LRESULT __export FAR PASCAL SnapCwpProc(int code, WPARAM wParam, LPARAM lParam)
{
    if (code >= 0 && g_hNotify && (g_cfg.bDrag || g_cfg.bCorners) && lParam) {
        LPCWPSTRUCT16 p = (LPCWPSTRUCT16)lParam;

        if (p->message == WM_SYSCOMMAND && (p->wParam & 0xFFF0) == SC_MOVE) {
            /* A move starts (title bar drag, or Alt+Space, M) */
            if (!(GetWindowLong(p->hwnd, GWL_STYLE) & WS_CHILD)) {
                g_hMoving   = p->hwnd;
                g_bLoopMsgs = FALSE;
            }
        }
        else if (p->message == WM_ENTERSIZEMOVE && p->hwnd == g_hMoving && g_hMoving) {
            /* This system reports the end with WM_EXITSIZEMOVE, so the
               intermediate WM_WINDOWPOSCHANGED messages must be ignored */
            g_bLoopMsgs = TRUE;
        }
        else if (p->message == WM_WINDOWPOSCHANGED && p->hwnd == g_hMoving && g_hMoving
                 && !g_bLoopMsgs) {
            /* Windows 3.1 (outline drag): a single WM_WINDOWPOSCHANGED when the
               window is dropped. If the button is still down, it is an
               intermediate move. */
            const WINDOWPOS FAR *wp = (const WINDOWPOS FAR *)p->lParam;
            if (!MainButtonDown()) {
                g_hMoving = NULL;
                if (wp && !(wp->flags & SWP_NOMOVE))
                    CheckEdgeAndSnap(p->hwnd);
            }
        }
        else if (p->message == WM_EXITSIZEMOVE && p->hwnd == g_hMoving && g_hMoving) {
            /* Environments that send it (full-window drag) */
            g_hMoving = NULL;
            CheckEdgeAndSnap(p->hwnd);
        }
        else if (p->message == WM_DESTROY && p->hwnd == g_hMoving) {
            g_hMoving = NULL;
        }
    }
    return CallNextHookEx(g_hCwp, code, wParam, lParam);
}

/* ---------------------------------------------------------------- */

BOOL __export FAR PASCAL SnapInstall(HWND hwndNotify)
{
    if (g_hKbd || g_hCwp)
        SnapRemove();

    SnapReadConfig(&g_cfg);
    g_hMoving = NULL;
    g_hNotify = hwndNotify;

    g_hKbd = SetWindowsHookEx(WH_KEYBOARD,    (HOOKPROC)SnapKbdProc, g_hInst, NULL);
    g_hCwp = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)SnapCwpProc, g_hInst, NULL);

    if (!g_hKbd || !g_hCwp) {
        SnapRemove();
        return FALSE;
    }
    return TRUE;
}

void __export FAR PASCAL SnapRemove(void)
{
    if (g_hKbd) { UnhookWindowsHookEx(g_hKbd); g_hKbd = NULL; }
    if (g_hCwp) { UnhookWindowsHookEx(g_hCwp); g_hCwp = NULL; }
    g_hNotify = NULL;
    g_hMoving = NULL;
}

void __export FAR PASCAL SnapReload(void)
{
    SnapReadConfig(&g_cfg);
}
