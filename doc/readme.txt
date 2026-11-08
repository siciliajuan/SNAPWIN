SnapWin 2.3 - Window snapping for Windows 3.1 / 3.11
=====================================================

WHAT IT DOES
Drag a window and drop it:
- On the left or right edge of the screen: it fills that half.
- In a corner: it fills that quarter of the screen.
  Dropping on the left/right edge near the top or bottom, or on the
  top/bottom edge near a side, counts as a corner (the corner zone is
  1/8 of the screen).

From the keyboard (active window; configurable):
- Ctrl+Alt+Left Arrow ......... left half
- Ctrl+Alt+Right Arrow ........ right half
- Ctrl+Alt+Home ............... top-left quarter
- Ctrl+Alt+Page Up ............ top-right quarter
- Ctrl+Alt+End ................ bottom-left quarter
- Ctrl+Alt+Page Down .......... bottom-right quarter
  (Home, Page Up, End and Page Down form a square laid out like the
  quarters. With Num Lock off, keys 7, 9, 1 and 3 on the numeric keypad
  work too.)

Fixed-size windows (Calculator, dialog boxes) are not resized: they are
only moved to the matching side or corner.
MDI windows (documents inside an application) are not touched.

REQUIREMENTS
Windows 3.1, 3.11 or Windows for Workgroups. (Windows 3.0 is not supported.)

INSTALLING AND UPGRADING
1. Close Control Panel if it is open.
2. In Program Manager: File > Run > SETUP.EXE
3. If an earlier version is installed, choose "Yes" (Reinstall / upgrade).
   Your settings are kept, including those of the Spanish 1.x/2.x releases.

SETUP.EXE copies SNAPHOOK.DLL, SNAPWIN.EXE and SNAPCFG.CPL to the Windows
SYSTEM directory and adds SNAPWIN.EXE to the "load=" line of WIN.INI.
Any other programs on that line are kept.
SnapWin has no window, no icon and does not appear in the Task List.

SETTINGS
Control Panel > "SnapWin" icon:
- Turn snapping to halves (edges) and to quarters (corners) on or off
  separately.
- Choose the key and modifiers (Ctrl, Alt, Shift) of each of the six
  shortcuts, or "(none)" to disable one.
  Every shortcut must include Ctrl or Alt, and no two may be the same.
  Ctrl+Alt+key is recommended: Alt-only shortcuts can activate the
  application's menu bar.
- Choose the language: Automatic (follows the Windows language set in
  Control Panel > International), English or Español.
Changes take effect immediately (the language, the next time the applet
is opened). They are stored in WINDOWS\SNAPWIN.INI.

LANGUAGE
SnapWin speaks English and Spanish. It uses Spanish when Windows is set
to Spanish, and English otherwise. LEEME.TXT is this file in Spanish.

UNINSTALLING
Run SETUP.EXE again and choose "No" (Uninstall). SNAPWIN.INI is kept.
To stop SnapWin temporarily: File > Run > SNAPWIN.EXE /QUIT

FILES
SNAPHOOK.DLL  System hooks (keyboard and window messages)
SNAPWIN.EXE   Invisible resident program that moves the windows
SNAPCFG.CPL   Control Panel applet
SETUP.EXE     Installer / uninstaller
README.TXT    This file
LEEME.TXT     This file in Spanish
LICENSE.TXT   License (MIT)

Source code, history and updates:
https://github.com/<user>/snapwin

AUTHOR AND LICENSE
Copyright (c) 2026 Juan Sicilia

SnapWin is free software, released under the MIT License: you may use,
copy, modify and share it freely, including modified versions, as long
as this copyright notice and the license are kept.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
The full license text is in LICENSE.TXT.
