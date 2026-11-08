/*
 * SETUP.C - SnapWin installer / uninstaller for Windows 3.1x.
 *
 * Install:   copies SNAPHOOK.DLL, SNAPWIN.EXE and SNAPCFG.CPL to the Windows
 *            SYSTEM directory, adds SNAPWIN.EXE to "load=" in WIN.INI and
 *            starts it.
 * Uninstall: stops SnapWin, removes it from "load=" and deletes the files.
 *            SNAPWIN.INI (the user's settings) is left in place.
 *
 * Upgrades from any earlier release are handled: the running instance is
 * stopped whatever its window class, and the obsolete SNAPWIN.CPL is deleted.
 * Settings in SNAPWIN.INI are migrated by cfg.c.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#include "snapwin.h"

#define BUFSZ  4096

static HINSTANCE g_hInst;
static char      g_srcDir[144];
static char      g_sysDir[144];
static char      g_title[64];

/* Message box with a string resource, optionally followed by a second one
   (idMore, 0 = none: Win16 strings are limited to 255 characters) and
   formatted with up to two string arguments */
static int Msg(UINT id, UINT idMore, UINT flags, LPCSTR a1, LPCSTR a2)
{
    char fmt[600], text[700];
    SnapLoadString(g_hInst, id, fmt, 300);
    if (idMore)
        SnapLoadString(g_hInst, idMore, fmt + lstrlen(fmt), 300);
    wsprintf(text, fmt, a1, a2);
    return MessageBox(NULL, text, g_title, flags);
}

static void MakePath(LPSTR out, LPCSTR dir, LPCSTR file)
{
    int n;
    lstrcpy(out, dir);
    n = lstrlen(out);
    if (n && out[n - 1] != '\\')
        lstrcat(out, "\\");
    lstrcat(out, file);
}

static BOOL CopyOne(LPCSTR name)
{
    static char buf[BUFSZ];
    char      src[160], dst[160];
    OFSTRUCT  of;
    HFILE     hIn, hOut;
    UINT      n;
    BOOL      ok = TRUE;

    MakePath(src, g_srcDir, name);
    MakePath(dst, g_sysDir, name);

    hIn = OpenFile(src, &of, OF_READ | OF_SHARE_DENY_WRITE);
    if (hIn == HFILE_ERROR)
        return FALSE;
    hOut = OpenFile(dst, &of, OF_CREATE | OF_WRITE);
    if (hOut == HFILE_ERROR) {
        _lclose(hIn);
        return FALSE;
    }
    while ((n = _lread(hIn, buf, BUFSZ)) > 0 && n != (UINT)HFILE_ERROR) {
        if (_lwrite(hOut, buf, n) != n) { ok = FALSE; break; }
    }
    if (n == (UINT)HFILE_ERROR) ok = FALSE;
    _lclose(hIn);
    _lclose(hOut);
    return ok;
}

/* TRUE if the file is gone (deleted, or was never there) */
static BOOL DeleteOne(LPCSTR name)
{
    char     path[160];
    OFSTRUCT of;
    MakePath(path, g_sysDir, name);
    if (OpenFile(path, &of, OF_EXIST) == HFILE_ERROR)
        return TRUE;
    return OpenFile(path, &of, OF_DELETE) != HFILE_ERROR;
}

/* TRUE if the "load=" token (a path) refers to SNAPWIN.EXE */
static BOOL IsSnapToken(LPSTR tok, int len)
{
    char  t[160];
    LPSTR base;
    int   i;
    if (len >= sizeof(t)) return FALSE;
    for (i = 0; i < len; i++) t[i] = tok[i];
    t[len] = 0;
    AnsiUpper(t);
    base = t;
    for (i = 0; t[i]; i++)
        if (t[i] == '\\' || t[i] == ':') base = t + i + 1;
    return lstrcmp(base, SNAPWIN_EXE) == 0 || lstrcmp(base, "SNAPWIN") == 0;
}

/* Rebuilds "load=" without SnapWin, keeping any other programs listed there;
   if add is TRUE, appends SnapWin at the end */
static void UpdateLoadLine(BOOL add)
{
    char  load[512], out[640], exe[160];
    LPSTR p = load;

    GetProfileString("windows", "load", "", load, sizeof(load));
    out[0] = 0;
    while (*p) {
        LPSTR start;
        int   len;
        while (*p == ' ' || *p == ',') p++;
        start = p;
        while (*p && *p != ' ' && *p != ',') p++;
        len = (int)(p - start);
        if (len && !IsSnapToken(start, len)) {
            int o = lstrlen(out);
            int i;
            if (o) out[o++] = ' ';
            for (i = 0; i < len; i++) out[o + i] = start[i];
            out[o + len] = 0;
        }
    }
    if (add) {
        MakePath(exe, g_sysDir, SNAPWIN_EXE);
        if (out[0]) lstrcat(out, " ");
        lstrcat(out, exe);
    }
    WriteProfileString("windows", "load", out);
}

/* Stops the running instance (current or pre-2.2 window class), which also
   releases SNAPHOOK.DLL so it can be overwritten or deleted */
static void StopSnapWin(void)
{
    HWND  h;
    MSG   msg;
    DWORD t;

    h = FindWindow(SNAPWIN_CLASS, NULL);
    if (h)
        SendMessage(h, WM_CLOSE, 0, 0L);
    h = FindWindow(SNAPWIN_CLASS_OLD, NULL);
    if (h)
        SendMessage(h, WM_CLOSE, 0, 0L);

    /* Windows 3.1 multitasks cooperatively: the closed task only exits (and
       SNAPHOOK.DLL is only unloaded) once it gets to run. Yield until the DLL
       is gone, so it is never overwritten while loaded. Give up after 5 s. */
    t = GetTickCount();
    while (GetModuleHandle("SNAPHOOK") && GetTickCount() - t < 5000) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Yield();
    }
}

static BOOL IsInstalled(void)
{
    char     path[160];
    OFSTRUCT of;
    MakePath(path, g_sysDir, SNAPWIN_EXE);
    return OpenFile(path, &of, OF_EXIST) != HFILE_ERROR;
}

static void Install(void)
{
    char exe[160];

    StopSnapWin();
    DeleteOne(SNAPWIN_CPL_OLD);   /* 1.0 applet */

    if (!CopyOne(SNAPWIN_DLL) || !CopyOne(SNAPWIN_EXE) || !CopyOne(SNAPWIN_CPL)) {
        Msg(IDS_SETUP_COPYFAIL, 0, MB_OK | MB_ICONSTOP, NULL, NULL);
        return;
    }
    UpdateLoadLine(TRUE);

    MakePath(exe, g_sysDir, SNAPWIN_EXE);
    WinExec(exe, SW_SHOWNORMAL);

    Msg(IDS_SETUP_DONE, IDS_SETUP_DONE2, MB_OK | MB_ICONINFORMATION, NULL, NULL);
}

static void Uninstall(void)
{
    BOOL ok;
    StopSnapWin();
    UpdateLoadLine(FALSE);
    ok  = DeleteOne(SNAPWIN_EXE);
    ok &= DeleteOne(SNAPWIN_DLL);
    ok &= DeleteOne(SNAPWIN_CPL);
    ok &= DeleteOne(SNAPWIN_CPL_OLD);
    Msg(ok ? IDS_SETUP_REMOVED : IDS_SETUP_PARTIAL, 0, MB_OK | MB_ICONINFORMATION, NULL, NULL);
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    int i, n;
    (void)hPrev; (void)lpCmd; (void)nShow;

    g_hInst = hInst;
    SnapLoadString(hInst, IDS_SETUP_TITLE, g_title, sizeof(g_title));

    {
        WORD v     = LOWORD(GetVersion());   /* low byte = major, high byte = minor */
        BYTE major = LOBYTE(v), minor = HIBYTE(v);
        if (major < 3 || (major == 3 && minor < 10)) {
            Msg(IDS_SETUP_NEEDWIN31, 0, MB_OK | MB_ICONSTOP, NULL, NULL);
            return 1;
        }
    }

    /* Source directory = directory SETUP.EXE was run from */
    n = GetModuleFileName(hInst, g_srcDir, sizeof(g_srcDir));
    for (i = n - 1; i >= 0 && g_srcDir[i] != '\\'; i--) ;
    g_srcDir[i < 0 ? 0 : i] = 0;
    GetSystemDirectory(g_sysDir, sizeof(g_sysDir));

    if (IsInstalled()) {
        int r = Msg(IDS_SETUP_ALREADY, 0, MB_YESNOCANCEL | MB_ICONQUESTION, SNAPWIN_VERSION, NULL);
        if (r == IDYES) Install();
        else if (r == IDNO) Uninstall();
    } else {
        if (Msg(IDS_SETUP_ASK, 0, MB_YESNO | MB_ICONQUESTION, SNAPWIN_VERSION, SNAPWIN_COPYRIGHT) == IDYES)
            Install();
    }
    return 0;
}
