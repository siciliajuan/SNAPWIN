/*
 * SNAPWIN.H - Shared definitions for SnapWin (Windows 3.1x window snapping)
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#ifndef SNAPWIN_H
#define SNAPWIN_H

#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include "version.h"   /* SNAPWIN_VERSION, SNAPWIN_COPYRIGHT */
#include "lang.h"      /* LANG_*, IDS_*, SnapLoadString */

#define SNAPWIN_INI       "SNAPWIN.INI"   /* lives in the Windows directory */
#define SNAPWIN_SECTION   "SnapWin"
#define SNAPWIN_CLASS     "SnapWinHidden" /* class of SNAPWIN.EXE's hidden window */
#define SNAPWIN_EXE       "SNAPWIN.EXE"
#define SNAPWIN_DLL       "SNAPHOOK.DLL"
#define SNAPWIN_CPL       "SNAPCFG.CPL"

/*
 * Names used by older releases, only needed by SETUP.EXE when upgrading.
 * Versions up to 2.1 were in Spanish and used a different window class.
 * Version 1.0 shipped the applet as SNAPWIN.CPL, which clashed with the
 * module name of SNAPWIN.EXE (see README.md, "Win16 pitfalls").
 */
#define SNAPWIN_CLASS_OLD "SnapWinOculta"
#define SNAPWIN_CPL_OLD   "SNAPWIN.CPL"

/* Messages posted to SNAPWIN.EXE's hidden window */
#define WM_SNAP_DO        (WM_USER + 100) /* wParam = SNAP_* position,
                                             LOWORD(lParam) = hwnd (0 = active window) */
#define WM_SNAP_RELOAD    (WM_USER + 101) /* re-read SNAPWIN.INI */

/* Snap positions (wParam of WM_SNAP_DO). Index into SNAPCFG = position - 1 */
#define SNAP_LEFT         1   /* left half */
#define SNAP_RIGHT        2   /* right half */
#define SNAP_TOPLEFT      3   /* top-left quarter */
#define SNAP_TOPRIGHT     4   /* top-right quarter */
#define SNAP_BOTLEFT      5   /* bottom-left quarter */
#define SNAP_BOTRIGHT     6   /* bottom-right quarter */
#define SNAP_COUNT        6

/* Shortcut modifier flags */
#define SMOD_SHIFT        0x0001
#define SMOD_CTRL         0x0002
#define SMOD_ALT          0x0004

/* How close to the screen edge (in pixels) the cursor must be to count as "on the edge" */
#define SNAP_EDGE         2
/* Size of the corner zone: 1/SNAP_CORNER_DIV of the screen, at least SNAP_CORNER_MIN px */
#define SNAP_CORNER_DIV   8
#define SNAP_CORNER_MIN   24

typedef struct tagSNAPCFG {
    BOOL bDrag;              /* halves: drop on the left/right edge */
    BOOL bCorners;           /* quarters: drop in a corner */
    UINT key[SNAP_COUNT];    /* virtual-key code of each shortcut (0 = none) */
    UINT mods[SNAP_COUNT];   /* SMOD_* flags of each shortcut */
} SNAPCFG;
typedef SNAPCFG FAR *LPSNAPCFG;

/* cfg.c */
void SnapReadConfig(LPSNAPCFG lpCfg);
void SnapWriteConfig(const SNAPCFG FAR *lpCfg);

/* Exported by SNAPHOOK.DLL */
BOOL FAR PASCAL SnapInstall(HWND hwndNotify);
void FAR PASCAL SnapRemove(void);
void FAR PASCAL SnapReload(void);

#endif
