/*
 * LANG.H - User interface languages and string resource IDs.
 *
 * Win16 resources have no LANGUAGE attribute (that arrived with Win32), so
 * each language gets its own block of resource IDs:
 *
 *   string ID = base ID + language * LANG_STRIDE
 *   dialog ID = base ID + language
 *
 * The texts of each language live in src/lang/<code>.rc. English is the
 * default and the fallback for any missing string. Each string can be at
 * most 255 characters long (a Win16 limit). See README.md,
 * "Adding a translation".
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#ifndef LANG_H
#define LANG_H

#define LANG_EN        0
#define LANG_ES        1
#define LANG_COUNT     2   /* add new languages before this, in order */
#define LANG_STRIDE    1000

/* SNAPWIN.INI [SnapWin] Language= value; anything else means "auto" */
#define LANG_CODES     { "en", "es" }

/* ---- String IDs (English base; add language * LANG_STRIDE) ---- */

/* Every language file defines its own name, in that language */
#define IDS_LANG_NAME        3

/* SNAPCFG.CPL */
#define IDS_CPL_NAME         1
#define IDS_CPL_INFO         2
#define IDS_CPL_CAPTION      10   /* %s = version */
#define IDS_CPL_LICENSE      11   /* %s = copyright notice */
#define IDS_CPL_RUNNING      12
#define IDS_CPL_STOPPED      13
#define IDS_CPL_ERR_NOMOD    14   /* %s = row name */
#define IDS_CPL_ERR_DUP      15   /* %s, %s = row names */
#define IDS_CPL_LANG_AUTO    16
#define IDS_CPL_ROW_BASE     20   /* 20..25: row names used in messages */
#define IDS_KEY_BASE         100  /* 100..: key names, same order as g_keys */

/* SETUP.EXE */
#define IDS_SETUP_TITLE      300
#define IDS_SETUP_NEEDWIN31  301
#define IDS_SETUP_ASK        302  /* %s version, %s copyright */
#define IDS_SETUP_ALREADY    303  /* %s version */
#define IDS_SETUP_COPYFAIL   304
#define IDS_SETUP_DONE       305
#define IDS_SETUP_REMOVED    306
#define IDS_SETUP_PARTIAL    307
#define IDS_SETUP_DONE2      308  /* second half of IDS_SETUP_DONE */

/* SNAPWIN.EXE */
#define IDS_SNAP_NOHOOKS     400

/* ---- Dialog IDs (add language) ---- */
#define IDD_CONFIG           100

#ifndef RC_INVOKED
/* lang.c */
int  SnapGetLangSetting(void);   /* -1 = auto, otherwise LANG_* */
void SnapSetLangSetting(int lang);
int  SnapGetLanguage(void);      /* effective language: LANG_* */
int  SnapLoadString(HINSTANCE hInst, UINT id, LPSTR buf, int cb);
#endif

#endif
