/*
 * LANG.C - Chooses the user interface language and loads strings in it.
 *          Linked into SNAPWIN.EXE, SNAPCFG.CPL and SETUP.EXE.
 *
 * The language is taken from SNAPWIN.INI ([SnapWin] Language=en|es) or,
 * when that is missing or "auto", from the Windows regional setting
 * (WIN.INI [intl] sLanguage, e.g. "enu", "esp", "esn"). Anything that is
 * not a supported language falls back to English.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#include "snapwin.h"

static const char *g_codes[LANG_COUNT] = LANG_CODES;

/* Index of the language whose code starts s (2 letters, any case), or -1 */
static int CodeToLang(LPCSTR s)
{
    int  i;
    char c[3];
    if (!s[0] || !s[1])
        return -1;
    c[0] = s[0]; c[1] = s[1]; c[2] = 0;
    AnsiLower(c);
    for (i = 0; i < LANG_COUNT; i++)
        if (g_codes[i][0] == c[0] && g_codes[i][1] == c[1])
            return i;
    return -1;
}

int SnapGetLangSetting(void)
{
    char buf[8];
    GetPrivateProfileString(SNAPWIN_SECTION, "Language", "auto", buf, sizeof(buf), SNAPWIN_INI);
    return CodeToLang(buf);   /* "auto" (or anything unknown) -> -1 */
}

void SnapSetLangSetting(int lang)
{
    WritePrivateProfileString(SNAPWIN_SECTION, "Language",
        (lang >= 0 && lang < LANG_COUNT) ? g_codes[lang] : "auto", SNAPWIN_INI);
}

int SnapGetLanguage(void)
{
    char buf[8];
    int  lang = SnapGetLangSetting();
    if (lang >= 0)
        return lang;
    GetProfileString("intl", "sLanguage", "enu", buf, sizeof(buf));
    lang = CodeToLang(buf);
    return lang >= 0 ? lang : LANG_EN;
}

/* LoadString in the current language, falling back to English */
int SnapLoadString(HINSTANCE hInst, UINT id, LPSTR buf, int cb)
{
    int lang = SnapGetLanguage();
    int n    = 0;
    if (lang != LANG_EN)
        n = LoadString(hInst, id + lang * LANG_STRIDE, buf, cb);
    if (n == 0)
        n = LoadString(hInst, id, buf, cb);
    if (n == 0 && cb > 0)
        buf[0] = 0;
    return n;
}
