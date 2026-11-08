/*
 * SNAPCPL.C - "SnapWin" applet for the Windows 3.1x Control Panel.
 *
 * A .CPL file is a DLL that exports CPlApplet. Control Panel loads every
 * .CPL file in the Windows SYSTEM directory when it starts.
 *
 * NOTE: the file is SNAPCFG.CPL, not SNAPWIN.CPL. Win16 identifies loaded
 * modules by name, so SNAPWIN.CPL would be confused with the already
 * running SNAPWIN.EXE and the icon would silently not appear.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#include "snapwin.h"
#include "resource.h"

/* --- Win16 cpl.h (not shipped with Open Watcom) --- */
#define CPL_INIT        1
#define CPL_GETCOUNT    2
#define CPL_INQUIRE     3
#define CPL_SELECT      4
#define CPL_DBLCLK      5
#define CPL_STOP        6
#define CPL_EXIT        7

typedef struct tagCPLINFO {
    int  idIcon;
    int  idName;
    int  idInfo;
    LONG lData;
} CPLINFO;
typedef CPLINFO FAR *LPCPLINFO;

static HINSTANCE g_hInst;

/* Keys offered in the drop-down lists, in display order. Their names are
   string resources IDS_KEY_BASE + index (see the files in src/lang). Letters A-Z
   and digits 0-9 are added in code. */
static const UINT g_keys[] = {
    0,                                    /* (none) */
    VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
    VK_HOME, VK_END, VK_PRIOR, VK_NEXT,
    VK_INSERT, VK_DELETE,
    VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,
    VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
    VK_NUMPAD7, VK_NUMPAD9, VK_NUMPAD1, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD6
};
#define NUM_KEYS (sizeof(g_keys) / sizeof(g_keys[0]))

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

static LPSTR Str(UINT id, LPSTR buf, int cb)
{
    SnapLoadString(g_hInst, id, buf, cb);
    return buf;
}

static void FillKeyCombo(HWND hDlg, int id, UINT vkSel)
{
    HWND hCb = GetDlgItem(hDlg, id);
    char buf[4];
    int  i, idx, sel = -1;

    for (i = 0; i < NUM_KEYS; i++) {
        char name[32];
        Str(IDS_KEY_BASE + i, name, sizeof(name));
        idx = (int)SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)(LPSTR)name);
        SendMessage(hCb, CB_SETITEMDATA, idx, (LPARAM)g_keys[i]);
        if (g_keys[i] == vkSel) sel = idx;
    }
    /* Letters A-Z and digits 0-9 (their virtual-key code is the ASCII code) */
    for (i = 0; i < 36; i++) {
        UINT vk = (i < 26) ? ('A' + i) : ('0' + i - 26);
        buf[0] = (char)vk; buf[1] = 0;
        idx = (int)SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)(LPSTR)buf);
        SendMessage(hCb, CB_SETITEMDATA, idx, (LPARAM)vk);
        if (vk == vkSel) sel = idx;
    }
    SendMessage(hCb, CB_SETCURSEL, sel < 0 ? 0 : sel, 0L);
}

static UINT GetKeyCombo(HWND hDlg, int id)
{
    HWND hCb = GetDlgItem(hDlg, id);
    int  sel = (int)SendMessage(hCb, CB_GETCURSEL, 0, 0L);
    if (sel < 0) return 0;
    return (UINT)SendMessage(hCb, CB_GETITEMDATA, sel, 0L);
}

static void SetRow(HWND hDlg, int i, UINT key, UINT mods)
{
    CheckDlgButton(hDlg, IDC_CTRL(i),  (mods & SMOD_CTRL)  != 0);
    CheckDlgButton(hDlg, IDC_ALT(i),   (mods & SMOD_ALT)   != 0);
    CheckDlgButton(hDlg, IDC_SHIFT(i), (mods & SMOD_SHIFT) != 0);
    FillKeyCombo(hDlg, IDC_KEY(i), key);
}

static void GetRow(HWND hDlg, int i, UINT FAR *key, UINT FAR *mods)
{
    UINT m = 0;
    if (IsDlgButtonChecked(hDlg, IDC_CTRL(i)))  m |= SMOD_CTRL;
    if (IsDlgButtonChecked(hDlg, IDC_ALT(i)))   m |= SMOD_ALT;
    if (IsDlgButtonChecked(hDlg, IDC_SHIFT(i))) m |= SMOD_SHIFT;
    *mods = m;
    *key  = GetKeyCombo(hDlg, IDC_KEY(i));
}

static void UpdateStatus(HWND hDlg)
{
    char buf[128];
    SetDlgItemText(hDlg, IDC_STATUS,
        Str(FindWindow(SNAPWIN_CLASS, NULL) ? IDS_CPL_RUNNING : IDS_CPL_STOPPED, buf, sizeof(buf)));
}

/* Language list: "Automatic", then each language under its own name */
static void FillLangCombo(HWND hDlg)
{
    HWND hCb = GetDlgItem(hDlg, IDC_LANG);
    char buf[64];
    int  l;
    SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)(LPSTR)Str(IDS_CPL_LANG_AUTO, buf, sizeof(buf)));
    for (l = 0; l < LANG_COUNT; l++) {
        if (!LoadString(g_hInst, IDS_LANG_NAME + l * LANG_STRIDE, buf, sizeof(buf)))
            wsprintf(buf, "#%d", l);
        SendMessage(hCb, CB_ADDSTRING, 0, (LPARAM)(LPSTR)buf);
    }
    SendMessage(hCb, CB_SETCURSEL, SnapGetLangSetting() + 1, 0L);
}

static BOOL Validate(HWND hDlg, const SNAPCFG FAR *c)
{
    char msg[256], fmt[160], row1[48], row2[48];
    int  i, j;

    for (i = 0; i < SNAP_COUNT; i++) {
        if (c->key[i] && !(c->mods[i] & (SMOD_CTRL | SMOD_ALT))) {
            Str(IDS_CPL_ERR_NOMOD, fmt, sizeof(fmt));
            wsprintf(msg, fmt, (LPSTR)Str(IDS_CPL_ROW_BASE + i, row1, sizeof(row1)));
            MessageBox(hDlg, msg, "SnapWin", MB_OK | MB_ICONEXCLAMATION);
            SetFocus(GetDlgItem(hDlg, IDC_CTRL(i)));
            return FALSE;
        }
    }
    for (i = 0; i < SNAP_COUNT; i++)
        for (j = i + 1; j < SNAP_COUNT; j++)
            if (c->key[i] && c->key[i] == c->key[j] && c->mods[i] == c->mods[j]) {
                Str(IDS_CPL_ERR_DUP, fmt, sizeof(fmt));
                wsprintf(msg, fmt, (LPSTR)Str(IDS_CPL_ROW_BASE + i, row1, sizeof(row1)),
                                   (LPSTR)Str(IDS_CPL_ROW_BASE + j, row2, sizeof(row2)));
                MessageBox(hDlg, msg, "SnapWin", MB_OK | MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hDlg, IDC_KEY(j)));
                return FALSE;
            }
    return TRUE;
}

BOOL __export FAR PASCAL ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SNAPCFG c;
    int     i;
    (void)lParam;

    switch (msg) {
    case WM_INITDIALOG: {
        char fmt[80], buf[120];
        wsprintf(buf, Str(IDS_CPL_CAPTION, fmt, sizeof(fmt)), (LPSTR)SNAPWIN_VERSION);
        SetWindowText(hDlg, buf);
        wsprintf(buf, Str(IDS_CPL_LICENSE, fmt, sizeof(fmt)), (LPSTR)SNAPWIN_COPYRIGHT);
        SetDlgItemText(hDlg, IDC_COPYRIGHT, buf);
        FillLangCombo(hDlg);
        SnapReadConfig(&c);
        CheckDlgButton(hDlg, IDC_DRAG,    c.bDrag);
        CheckDlgButton(hDlg, IDC_CORNERS, c.bCorners);
        for (i = 0; i < SNAP_COUNT; i++)
            SetRow(hDlg, i, c.key[i], c.mods[i]);
        UpdateStatus(hDlg);
        return TRUE;
    }

    case WM_COMMAND:
        switch (wParam) {
        case IDOK: {
            HWND hSnap;
            c.bDrag    = IsDlgButtonChecked(hDlg, IDC_DRAG);
            c.bCorners = IsDlgButtonChecked(hDlg, IDC_CORNERS);
            for (i = 0; i < SNAP_COUNT; i++)
                GetRow(hDlg, i, &c.key[i], &c.mods[i]);
            if (!Validate(hDlg, &c))
                return TRUE;
            SnapWriteConfig(&c);
            /* Takes effect the next time the applet or Setup is opened */
            SnapSetLangSetting((int)SendDlgItemMessage(hDlg, IDC_LANG, CB_GETCURSEL, 0, 0L) - 1);

            /* Apply immediately: ask the running instance to reload, or start it */
            hSnap = FindWindow(SNAPWIN_CLASS, NULL);
            if (hSnap)
                PostMessage(hSnap, WM_SNAP_RELOAD, 0, 0L);
            else
                WinExec(SNAPWIN_EXE, SW_SHOWNORMAL);
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* ---------------------------------------------------------------- */

LONG __export FAR PASCAL CPlApplet(HWND hwndCPl, UINT msg, LPARAM lParam1, LPARAM lParam2)
{
    switch (msg) {
    case CPL_INIT:
        return 1L;
    case CPL_GETCOUNT:
        return 1L;
    case CPL_INQUIRE: {
        /* Control Panel loads these strings itself, so give it the IDs of
           the current language */
        LPCPLINFO p   = (LPCPLINFO)lParam2;
        int       ofs = SnapGetLanguage() * LANG_STRIDE;
        p->idIcon = IDI_SNAPWIN;
        p->idName = IDS_CPL_NAME + ofs;
        p->idInfo = IDS_CPL_INFO + ofs;
        p->lData  = 0L;
        return 0L;
    }
    case CPL_DBLCLK: {
        UINT id = IDD_CONFIG + SnapGetLanguage();
        if (!FindResource(g_hInst, MAKEINTRESOURCE(id), RT_DIALOG))
            id = IDD_CONFIG;                       /* English fallback */
        DialogBox(g_hInst, MAKEINTRESOURCE(id), hwndCPl, (DLGPROC)ConfigDlgProc);
        return 0L;
    }
    }
    (void)lParam1;
    return 0L;
}
