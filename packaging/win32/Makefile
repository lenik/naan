# SPDX-License-Identifier: AGPL-3.0-or-later
# Windows (win32) packaging.
#
#   make -C packaging/win32           # ucrt + mingw + innosetup + wix
#   make -C packaging/win32 ucrt      # native MSVC/UCRT
#   make -C packaging/win32 mingw     # <pkg>_mingw-<ver>.exe + .gnu nupkg
#   make -C packaging/win32 innosetup  # <pkg>-<ver>.exe
#   make -C packaging/win32 wix        # <pkg>-<ver>.msi

include ../lib/common.mk

.PHONY: all ucrt mingw innosetup wix clean

all: ucrt mingw innosetup wix

ucrt:
	$(MAKE) -C ucrt

mingw:
	$(MAKE) -C mingw

innosetup:
	$(MAKE) -C innosetup

wix:
	$(MAKE) -C wix

clean:
	$(MAKE) -C ucrt clean
	$(MAKE) -C mingw clean
	$(MAKE) -C innosetup clean
	$(MAKE) -C wix clean
