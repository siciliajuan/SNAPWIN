# Changelog

## 2.5

- Ctrl+Alt+F now only maximizes (in 2.4 it also restored a maximized
  window).
- New shortcuts: Ctrl+Alt+R restores a maximized or minimized window, and
  Ctrl+Alt+Down Arrow minimizes. Restore does nothing on a window that is
  already restored.
- Applet: nine shortcut rows, in a more compact layout that still fits
  a 640x480 screen.

## 2.4

- Maximize: drop a window on the top edge of the screen (away from the
  corners, which still give the top quarters), or press Ctrl+Alt+F. The
  shortcut restores a window that is already maximized.
- The applet has a new option to turn top-edge maximizing on or off, and
  a seventh shortcut row.
- Only windows with a maximize button are maximized.

## 2.3

- Internationalisation: the user interface is available in English
  (default) and Spanish. The language follows the Windows setting
  (Control Panel > International) and can be chosen in the applet.
- Spanish readme (`LEEME.TXT`) alongside `README.TXT`.
- All user-visible text moved to one resource file per language
  (`src/lang/`); adding a language needs no C code changes beyond
  registering it in `lang.h`.
- New `langtest.exe` checks every translation for missing and truncated
  strings.
- The project URL replaces the author's email address.

## 2.2

- Everything translated to English: user interface, installer, end-user
  readme, source comments and identifiers.
- Installer renamed from `INSTALAR.EXE` to `SETUP.EXE`.
- `SNAPWIN.EXE /QUIT` replaces `/SALIR`.
- `SNAPWIN.INI` uses English key names. Settings from earlier releases are
  read automatically and converted when next saved.
- Hidden window class renamed to `SnapWinHidden`. Setup also stops
  instances of earlier releases.
- Setup now waits until the running instance has really exited (and
  `SNAPHOOK.DLL` is unloaded) before overwriting or deleting files.
- New portable `makefile` for Open Watcom `wmake` (DOS, Windows, OS/2,
  Linux) replaces the Linux-only build script.
- Developer documentation (`README.md`) and test programs.
- Released as free software under the MIT License. Every binary carries
  version, author and license information (`VERSIONINFO`); the applet and
  Setup show the copyright notice.

## 2.1

- Applet: removed the line separating the half-screen shortcuts from the
  quarter-screen ones.

## 2.0

- Quarters: drop a window in a corner, or use four new shortcuts
  (Ctrl+Alt+Home / Page Up / End / Page Down by default).
- Applet shows six shortcuts, and snapping to halves and to quarters can be
  turned on or off separately.
- More robust end-of-drag detection with full-window drag.

## 1.1

- The applet is now `SNAPCFG.CPL`. In 1.0 it was `SNAPWIN.CPL` and, with
  SnapWin running, Windows mistook it for `SNAPWIN.EXE` (same module name),
  so its icon did not appear in Control Panel. Setup deletes the old file.

## 1.0

- First release: snap to the left or right half by dragging to an edge or
  with Ctrl+Alt+Left / Right Arrow; Control Panel applet; installer.
