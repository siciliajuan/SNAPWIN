/*
 * CFG.C - Reads and writes SNAPWIN.INI (linked into both SNAPHOOK.DLL
 *         and SNAPCFG.CPL)
 *
 * [SnapWin]
 * DragHalves=1          drop on the left/right edge -> half screen
 * DragCorners=1         drop in a corner            -> quarter screen
 * <Pos>Key=<vk>         virtual-key code, 0 = no shortcut
 * <Pos>Mods=<flags>     SMOD_* flags (1 = Shift, 2 = Ctrl, 4 = Alt)
 *   where <Pos> is Left, Right, TopLeft, TopRight, BottomLeft, BottomRight
 *
 * Releases up to 2.1 used Spanish key names. When an English key is
 * missing, the Spanish one is used instead, so settings survive an
 * upgrade; the old keys are removed the next time the settings are saved.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#include "snapwin.h"

static const char *g_keyNames[SNAP_COUNT] = {
    "LeftKey", "RightKey", "TopLeftKey", "TopRightKey", "BottomLeftKey", "BottomRightKey"
};
static const char *g_modNames[SNAP_COUNT] = {
    "LeftMods", "RightMods", "TopLeftMods", "TopRightMods", "BottomLeftMods", "BottomRightMods"
};

/* Key names used by releases up to 2.1 */
static const char *g_oldKeyNames[SNAP_COUNT] = {
    "IzqTecla", "DerTecla", "SupIzqTecla", "SupDerTecla", "InfIzqTecla", "InfDerTecla"
};
static const char *g_oldModNames[SNAP_COUNT] = {
    "IzqMods", "DerMods", "SupIzqMods", "SupDerMods", "InfIzqMods", "InfDerMods"
};
#define OLD_DRAG     "Arrastrar"
#define OLD_CORNERS  "Esquinas"

/* Defaults: Ctrl+Alt+arrows for the halves and, for the quarters, the
   Home/PgUp/End/PgDn block, whose layout mirrors the screen (and matches
   7/9/1/3 on the numeric keypad with Num Lock off). */
static const UINT g_defKeys[SNAP_COUNT] = {
    VK_LEFT, VK_RIGHT, VK_HOME, VK_PRIOR, VK_END, VK_NEXT
};
#define DEF_MODS  (SMOD_CTRL | SMOD_ALT)

/* New key if present, otherwise legacy key, otherwise the default */
static UINT ReadInt(LPCSTR key, LPCSTR oldKey, UINT def)
{
    UINT v = GetPrivateProfileInt(SNAPWIN_SECTION, oldKey, def, SNAPWIN_INI);
    return GetPrivateProfileInt(SNAPWIN_SECTION, key, v, SNAPWIN_INI);
}

void SnapReadConfig(LPSNAPCFG c)
{
    int i;
    c->bDrag    = ReadInt("DragHalves",  OLD_DRAG,    1) != 0;
    c->bCorners = ReadInt("DragCorners", OLD_CORNERS, 1) != 0;
    for (i = 0; i < SNAP_COUNT; i++) {
        c->key[i]  = ReadInt(g_keyNames[i], g_oldKeyNames[i], g_defKeys[i]);
        c->mods[i] = ReadInt(g_modNames[i], g_oldModNames[i], DEF_MODS);
    }
}

static void WriteInt(LPCSTR key, UINT v)
{
    char buf[8];
    wsprintf(buf, "%u", v);
    WritePrivateProfileString(SNAPWIN_SECTION, key, buf, SNAPWIN_INI);
}

/* Passing NULL as the value deletes the key */
static void DeleteKey(LPCSTR key)
{
    WritePrivateProfileString(SNAPWIN_SECTION, key, NULL, SNAPWIN_INI);
}

void SnapWriteConfig(const SNAPCFG FAR *c)
{
    int i;
    WriteInt("DragHalves",  c->bDrag ? 1 : 0);
    WriteInt("DragCorners", c->bCorners ? 1 : 0);
    DeleteKey(OLD_DRAG);
    DeleteKey(OLD_CORNERS);
    for (i = 0; i < SNAP_COUNT; i++) {
        WriteInt(g_keyNames[i], c->key[i]);
        WriteInt(g_modNames[i], c->mods[i]);
        DeleteKey(g_oldKeyNames[i]);
        DeleteKey(g_oldModNames[i]);
    }
}
