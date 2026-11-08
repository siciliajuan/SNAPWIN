# SnapWin makefile for Open Watcom wmake (1.9 or 2.0).
#
# Copyright (c) 2026 Juan Sicilia
# SPDX-License-Identifier: MIT
#
# Works on any host Open Watcom runs on (DOS, Windows, OS/2, Linux):
#   set WATCOM and INCLUDE (see README.md), then from this directory run
#
#   wmake            build everything into dist/
#   wmake tests      also build the test programs into build/
#   wmake clean      delete build/ contents and dist/ contents
#
# Intermediate files go to build/, distributable files to dist/.

SRC  = src
TST  = tests
OBJ  = build
OUT  = dist

# Common compiler options: 16-bit Windows target, large model, Windows
# prologs, optimise for size, no stack checks, warnings are errors.
COPTS = -q -bt=windows -ml -zw -os -s -wx -we -i=$(SRC)
# DLLs additionally need -bd (DLL) and -zu (SS != DS: the stack belongs to
# whichever task calls into the DLL).
CDLL  = $(COPTS) -bd -zu
CEXE  = $(COPTS)

LIBENTRY = $(%WATCOM)/lib286/win/libentry.obj

# Every binary embeds a VERSIONINFO resource built from these
VERRC = $(SRC)/version.rc $(SRC)/version.h
# ...and every user-visible text comes from these (one file per language)
LANGRC = $(SRC)/lang.h $(SRC)/lang/en.rc $(SRC)/lang/es.rc

.ERASE
.EXTENSIONS:
.EXTENSIONS: .exe .dll .cpl .lib .obj .res .c .rc

all: dirs $(OUT)/snaphook.dll $(OUT)/snapwin.exe $(OUT)/snapcfg.cpl $(OUT)/setup.exe $(OUT)/readme.txt $(OUT)/leeme.txt $(OUT)/license.txt .SYMBOLIC

dirs: .SYMBOLIC
    -@mkdir $(OBJ)
    -@mkdir $(OUT)

# ------------------------------------------------------------------
# SNAPHOOK.DLL - system-wide hooks
# cfg.c is compiled once with DLL options and linked into both DLLs.

$(OBJ)/snaphook.obj: $(SRC)/snaphook.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h
    wcc $(CDLL) -fo=$@ $(SRC)/snaphook.c

$(OBJ)/cfg.obj: $(SRC)/cfg.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h
    wcc $(CDLL) -fo=$@ $(SRC)/cfg.c

$(OBJ)/snaphook.res: $(SRC)/snaphook.rc $(VERRC)
    wrc -q -r -bt=windows -i=$(SRC) -fo=$@ $(SRC)/snaphook.rc

$(OUT)/snaphook.dll: $(OBJ)/snaphook.obj $(OBJ)/cfg.obj $(OBJ)/snaphook.res
    %create $(OBJ)/snaphook.lnk
    %append $(OBJ)/snaphook.lnk system windows dll
    %append $(OBJ)/snaphook.lnk name $@
    %append $(OBJ)/snaphook.lnk file $(OBJ)/snaphook.obj, $(OBJ)/cfg.obj
    %append $(OBJ)/snaphook.lnk libfile $(LIBENTRY)
    %append $(OBJ)/snaphook.lnk option oneautodata, heapsize=1024, description 'SnapWin hooks'
    %append $(OBJ)/snaphook.lnk segment type code preload fixed
    %append $(OBJ)/snaphook.lnk segment type data preload fixed
    %append $(OBJ)/snaphook.lnk option resource=$(OBJ)/snaphook.res
    wlink @$(OBJ)/snaphook.lnk
    wlib -q -n $(OBJ)/snaphook.lib +$@

# Import library, created as a side effect of linking the DLL
$(OBJ)/snaphook.lib: $(OUT)/snaphook.dll
    @%null

# ------------------------------------------------------------------
# lang.c - language selection; compiled once for EXEs and once for DLLs

$(OBJ)/langexe.obj: $(SRC)/lang.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h
    wcc $(CEXE) -fo=$@ $(SRC)/lang.c

$(OBJ)/langdll.obj: $(SRC)/lang.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h
    wcc $(CDLL) -fo=$@ $(SRC)/lang.c

# ------------------------------------------------------------------
# SNAPWIN.EXE - resident program, no visible window

$(OBJ)/snapwin.obj: $(SRC)/snapwin.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h
    wcc $(CEXE) -fo=$@ $(SRC)/snapwin.c

$(OBJ)/snapwin.res: $(SRC)/snapwin.rc $(SRC)/snapwin.ico $(VERRC) $(LANGRC)
    wrc -q -r -bt=windows -i=$(SRC) -fo=$@ $(SRC)/snapwin.rc

$(OUT)/snapwin.exe: $(OBJ)/snapwin.obj $(OBJ)/langexe.obj $(OBJ)/snapwin.res $(OBJ)/snaphook.lib
    %create $(OBJ)/snapwin.lnk
    %append $(OBJ)/snapwin.lnk system windows
    %append $(OBJ)/snapwin.lnk name $@
    %append $(OBJ)/snapwin.lnk file $(OBJ)/snapwin.obj, $(OBJ)/langexe.obj
    %append $(OBJ)/snapwin.lnk library $(OBJ)/snaphook.lib
    %append $(OBJ)/snapwin.lnk option heapsize=1024, stack=8192, description 'SnapWin'
    %append $(OBJ)/snapwin.lnk option resource=$(OBJ)/snapwin.res
    wlink @$(OBJ)/snapwin.lnk

# ------------------------------------------------------------------
# SNAPCFG.CPL - Control Panel applet
# Its module name must differ from SNAPWIN (see README.md, "Win16 pitfalls").

$(OBJ)/snapcpl.obj: $(SRC)/snapcpl.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h $(SRC)/resource.h
    wcc $(CDLL) -fo=$@ $(SRC)/snapcpl.c

$(OBJ)/snapcpl.res: $(SRC)/snapcpl.rc $(SRC)/resource.h $(SRC)/snapwin.ico $(VERRC) $(LANGRC)
    wrc -q -r -bt=windows -i=$(SRC) -fo=$@ $(SRC)/snapcpl.rc

$(OUT)/snapcfg.cpl: $(OBJ)/snapcpl.obj $(OBJ)/cfg.obj $(OBJ)/langdll.obj $(OBJ)/snapcpl.res
    %create $(OBJ)/snapcpl.lnk
    %append $(OBJ)/snapcpl.lnk system windows dll
    %append $(OBJ)/snapcpl.lnk name $@
    %append $(OBJ)/snapcpl.lnk file $(OBJ)/snapcpl.obj, $(OBJ)/cfg.obj, $(OBJ)/langdll.obj
    %append $(OBJ)/snapcpl.lnk libfile $(LIBENTRY)
    %append $(OBJ)/snapcpl.lnk option oneautodata, heapsize=1024, modname=SNAPCFG
    %append $(OBJ)/snapcpl.lnk option description 'SnapWin Control Panel applet'
    %append $(OBJ)/snapcpl.lnk option resource=$(OBJ)/snapcpl.res
    wlink @$(OBJ)/snapcpl.lnk

# ------------------------------------------------------------------
# SETUP.EXE - installer / uninstaller

$(OBJ)/setup.obj: $(SRC)/setup.c $(SRC)/snapwin.h $(SRC)/version.h $(SRC)/lang.h
    wcc $(CEXE) -fo=$@ $(SRC)/setup.c

$(OBJ)/setup.res: $(SRC)/setup.rc $(SRC)/snapwin.ico $(VERRC) $(LANGRC)
    wrc -q -r -bt=windows -i=$(SRC) -fo=$@ $(SRC)/setup.rc

$(OUT)/setup.exe: $(OBJ)/setup.obj $(OBJ)/langexe.obj $(OBJ)/setup.res
    %create $(OBJ)/setup.lnk
    %append $(OBJ)/setup.lnk system windows
    %append $(OBJ)/setup.lnk name $@
    %append $(OBJ)/setup.lnk file $(OBJ)/setup.obj, $(OBJ)/langexe.obj
    %append $(OBJ)/setup.lnk option heapsize=1024, stack=8192, description 'SnapWin Setup'
    %append $(OBJ)/setup.lnk option resource=$(OBJ)/setup.res
    wlink @$(OBJ)/setup.lnk

# ------------------------------------------------------------------
# End-user documentation and license (CRLF line endings, see .gitattributes)

$(OUT)/readme.txt: doc/readme.txt
    %copy doc/readme.txt $@

$(OUT)/leeme.txt: doc/leeme.txt
    %copy doc/leeme.txt $@

# The MIT License requires the license text to accompany every copy
$(OUT)/license.txt: LICENSE
    %copy LICENSE $@

# ------------------------------------------------------------------
# Test programs (see README.md, "Testing")

tests: all $(OBJ)/hooktest.exe $(OBJ)/cpltest.exe $(OBJ)/modtest.exe $(OBJ)/langtest.exe .SYMBOLIC

$(OBJ)/hooktest.obj: $(TST)/hooktest.c
    wcc $(CEXE) -fo=$@ $(TST)/hooktest.c

$(OBJ)/cpltest.obj: $(TST)/cpltest.c
    wcc $(CEXE) -fo=$@ $(TST)/cpltest.c

$(OBJ)/modtest.obj: $(TST)/modtest.c
    wcc $(CEXE) -fo=$@ $(TST)/modtest.c

$(OBJ)/langtest.obj: $(TST)/langtest.c $(SRC)/lang.h
    wcc $(CEXE) -fo=$@ $(TST)/langtest.c

$(OBJ)/langtest.exe: $(OBJ)/langtest.obj
    wlink system windows name $@ file $(OBJ)/langtest.obj option quiet, heapsize=1024, stack=8192

$(OBJ)/hooktest.exe: $(OBJ)/hooktest.obj
    wlink system windows name $@ file $(OBJ)/hooktest.obj option quiet, heapsize=1024, stack=8192

$(OBJ)/cpltest.exe: $(OBJ)/cpltest.obj
    wlink system windows name $@ file $(OBJ)/cpltest.obj option quiet, heapsize=1024, stack=8192

$(OBJ)/modtest.exe: $(OBJ)/modtest.obj
    wlink system windows name $@ file $(OBJ)/modtest.obj option quiet, heapsize=1024, stack=8192

# ------------------------------------------------------------------

clean: .SYMBOLIC
    -%erase $(OBJ)/snaphook.obj
    -%erase $(OBJ)/cfg.obj
    -%erase $(OBJ)/langexe.obj
    -%erase $(OBJ)/langdll.obj
    -%erase $(OBJ)/snapwin.obj
    -%erase $(OBJ)/snapcpl.obj
    -%erase $(OBJ)/setup.obj
    -%erase $(OBJ)/hooktest.obj
    -%erase $(OBJ)/cpltest.obj
    -%erase $(OBJ)/modtest.obj
    -%erase $(OBJ)/langtest.obj
    -%erase $(OBJ)/snaphook.res
    -%erase $(OBJ)/snapwin.res
    -%erase $(OBJ)/snapcpl.res
    -%erase $(OBJ)/setup.res
    -%erase $(OBJ)/snaphook.lnk
    -%erase $(OBJ)/snapwin.lnk
    -%erase $(OBJ)/snapcpl.lnk
    -%erase $(OBJ)/setup.lnk
    -%erase $(OBJ)/snaphook.lib
    -%erase $(OBJ)/hooktest.exe
    -%erase $(OBJ)/cpltest.exe
    -%erase $(OBJ)/modtest.exe
    -%erase $(OBJ)/langtest.exe
    -%erase $(OUT)/snaphook.dll
    -%erase $(OUT)/snapwin.exe
    -%erase $(OUT)/snapcfg.cpl
    -%erase $(OUT)/setup.exe
    -%erase $(OUT)/readme.txt
    -%erase $(OUT)/leeme.txt
    -%erase $(OUT)/license.txt
