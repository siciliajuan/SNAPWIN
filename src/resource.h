/*
 * RESOURCE.H - Icon and dialog control IDs for SNAPCFG.CPL.
 *              String and dialog IDs are in lang.h.
 *
 * Copyright (c) 2026 Juan Sicilia
 * SPDX-License-Identifier: MIT
 */
#define IDI_SNAPWIN      1

#define IDC_DRAG         101   /* halves when dropping on an edge */
#define IDC_CORNERS      102   /* quarters when dropping in a corner */
#define IDC_LANG         103   /* language drop-down list */
#define IDC_STATUS       130
#define IDC_COPYRIGHT    131   /* author / license line */

/* Shortcut row i (i = 0..5, same order as SNAP_*):
   IDC_ROW(i)+0 Ctrl, +1 Alt, +2 Shift, +3 key drop-down list */
#define IDC_ROW_BASE     200
#define IDC_ROW(i)       (IDC_ROW_BASE + (i) * 10)
#define IDC_CTRL(i)      (IDC_ROW(i) + 0)
#define IDC_ALT(i)       (IDC_ROW(i) + 1)
#define IDC_SHIFT(i)     (IDC_ROW(i) + 2)
#define IDC_KEY(i)       (IDC_ROW(i) + 3)
