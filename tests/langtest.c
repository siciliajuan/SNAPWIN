/*
 * LANGTEST.C - Checks the translations compiled into the binaries.
 *
 * For each of SNAPCFG.CPL, SETUP.EXE and SNAPWIN.EXE (in the current
 * directory) and each language, loads every user-visible string and reports:
 *   MISSING    the English string exists but this language has none
 *              (users would see English);
 *   TRUNCATED  the string is exactly 255 characters long, the Win16
 *              maximum, so the resource compiler most likely cut it.
 * Also checks that each language has its own applet dialog.
 *
 * Output goes to OutputDebugString (WINEDEBUG=+debugstr under Wine, DBWIN
 * on Windows 3.1), ending with "PASS" or "FAIL: <n> problem(s)".
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#define STRICT
#include <windows.h>
#include "../src/lang.h"

static int g_problems;

static void Report(LPCSTR what, LPCSTR file, int lang, UINT id)
{
    char buf[100];
    wsprintf(buf, "%s: %s lang=%d id=%u", what, file, lang, id);
    OutputDebugString(buf);
    g_problems++;
}

static void CheckRange(HINSTANCE h, LPCSTR file, UINT first, UINT last)
{
    char buf[300];
    UINT id;
    int  lang;
    for (id = first; id <= last; id++) {
        if (!LoadString(h, id, buf, sizeof(buf)))
            continue;                          /* not a string ID */
        for (lang = 0; lang < LANG_COUNT; lang++) {
            int n = LoadString(h, id + lang * LANG_STRIDE, buf, sizeof(buf));
            if (n == 0)
                Report("MISSING", file, lang, id);
            else if (n >= 255)
                Report("TRUNCATED", file, lang, id);
        }
    }
}

static HINSTANCE Open(LPCSTR file)
{
    /* Load as a library only to read its resources */
    HINSTANCE h = LoadLibrary(file);
    if ((UINT)h < 32) {
        Report("CANNOT LOAD", file, 0, 0);
        return NULL;
    }
    return h;
}

int PASCAL WinMain(HINSTANCE hi, HINSTANCE hp, LPSTR c, int n)
{
    HINSTANCE h;
    char      buf[60];
    int       lang;
    (void)hi; (void)hp; (void)c; (void)n;

    if ((h = Open("SNAPCFG.CPL")) != NULL) {
        CheckRange(h, "SNAPCFG.CPL", 1, 199);
        for (lang = 0; lang < LANG_COUNT; lang++)
            if (!FindResource(h, MAKEINTRESOURCE(IDD_CONFIG + lang), RT_DIALOG))
                Report("MISSING DIALOG", "SNAPCFG.CPL", lang, IDD_CONFIG + lang);
        FreeLibrary(h);
    }
    if ((h = Open("SETUP.EXE")) != NULL) {
        CheckRange(h, "SETUP.EXE", 1, 399);
        FreeLibrary(h);
    }
    if ((h = Open("SNAPWIN.EXE")) != NULL) {
        CheckRange(h, "SNAPWIN.EXE", 1, 499);
        FreeLibrary(h);
    }

    if (g_problems)
        wsprintf(buf, "FAIL: %d problem(s)", g_problems);
    else
        lstrcpy(buf, "PASS");
    OutputDebugString(buf);
    return g_problems ? 1 : 0;
}
