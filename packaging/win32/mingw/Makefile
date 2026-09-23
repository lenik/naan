# SPDX-License-Identifier: AGPL-3.0-or-later
# Portable MinGW-w64 build: <pkg>_mingw-<ver>.exe + <pkg>.<ver>.nupkg
#
# Native Windows/MSYS: meson without a cross file.
# Linux: meson --cross-file meson-cross.ini (mingw-w64).

include ../../lib/common.mk

KIND    := mingw
OUTDIR  := $(CURDIR)/out
STAGE   := $(CURDIR)/stage
BUILD   := $(CURDIR)/build
CROSS   := $(CURDIR)/meson-cross.ini
ARTIFACT := $(OUTDIR)/$(NAME)_mingw-$(VERSION).exe
NUPKG   := $(OUTDIR)/$(NAME).gnu.$(VERSION).nupkg
MINGW_CC := $(shell command -v x86_64-w64-mingw32-gcc 2>/dev/null)
PACK_NUGET := $(SRCDIR)/scripts/ci/pack-nuget.py
RID     ?= win-x64

.PHONY: all local stage exe nuget clean

all:
	"$(HOST_SH)" run $(KIND) -- $(MAKE) -C "$(CURDIR)" local

local: $(ARTIFACT) $(NUPKG)

# Cross builds need mingw pkg-config deps (glib, …). Without them, skip
# cleanly so `zfr publish` can still ship deb/rpm on Linux hosts.
MINGW_PKG_CONFIG := $(shell command -v x86_64-w64-mingw32-pkg-config 2>/dev/null || command -v i686-w64-mingw32-pkg-config 2>/dev/null)
MINGW_HAS_GLIB := $(shell if [ -n "$(MINGW_PKG_CONFIG)" ] && "$(MINGW_PKG_CONFIG)" --exists glib-2.0 2>/dev/null; then echo 1; fi)

stage:
	rm -rf "$(STAGE)" "$(BUILD)"
	mkdir -p "$(STAGE)" "$(BUILD)"
	@set -e; \
	cd "$(SRCDIR)"; \
	if [ -n "$$OS" ] && [ "$$OS" = Windows_NT ]; then \
		meson setup "$(BUILD)" --prefix=/ --buildtype=release; \
	elif [ -n "$(MINGW_CC)" ]; then \
		if [ -z "$(MINGW_HAS_GLIB)" ]; then \
			echo "mingw: skip stage (no mingw glib-2.0 via pkg-config)" >&2; \
			exit 0; \
		fi; \
		meson setup "$(BUILD)" --cross-file "$(CROSS)" --prefix=/ --buildtype=release; \
	else \
		echo "mingw: need MSYS/MinGW or x86_64-w64-mingw32-gcc" >&2; \
		exit 1; \
	fi; \
	meson compile -C "$(BUILD)"; \
	DESTDIR="$(STAGE)" meson install -C "$(BUILD)"

$(ARTIFACT): stage
	mkdir -p "$(OUTDIR)"
	@set -e; \
	if [ ! -d "$(STAGE)" ] || [ -z "$$(find "$(STAGE)" -type f 2>/dev/null | head -1)" ]; then \
		echo "mingw: skip artifact (empty stage)" >&2; \
		exit 0; \
	fi; \
	exes=$$(find "$(STAGE)" -type f -name '*.exe' | sort); \
	n=$$(printf '%s\n' "$$exes" | sed '/^$$/d' | wc -l); \
	if [ "$$n" -eq 1 ]; then \
		cp "$$exes" "$(ARTIFACT)"; \
	elif [ -n "$$exes" ] && [ -f "$(STAGE)/bin/$(NAME).exe" ]; then \
		cp "$(STAGE)/bin/$(NAME).exe" "$(ARTIFACT)"; \
	elif command -v 7z >/dev/null 2>&1; then \
		rm -f "$(ARTIFACT)"; \
		( cd "$(STAGE)" && 7z a -sfx "$(ARTIFACT)" . ); \
	elif command -v zip >/dev/null 2>&1; then \
		rm -f "$(ARTIFACT)"; \
		( cd "$(STAGE)" && zip -r -9 "$(ARTIFACT)" . ); \
		echo "mingw: packed stage as zip-format $(ARTIFACT)" >&2; \
	else \
		echo "mingw: no .exe in stage and no 7z/zip to pack" >&2; \
		exit 1; \
	fi; \
	echo "$(ARTIFACT)"

$(NUPKG): stage
	mkdir -p "$(OUTDIR)"
	@set -e; \
	if [ ! -d "$(STAGE)" ] || [ -z "$$(find "$(STAGE)" -type f 2>/dev/null | head -1)" ]; then \
		echo "mingw: skip nuget (empty stage)" >&2; \
		exit 0; \
	fi; \
	if [ ! -f "$(PACK_NUGET)" ]; then \
		echo "mingw: missing $(PACK_NUGET)" >&2; \
		exit 1; \
	fi; \
	python3 "$(PACK_NUGET)" \
		--id "$(NAME).gnu" \
		--version "$(VERSION)" \
		--stage "$(STAGE)" \
		--out "$(NUPKG)" \
		--rid "$(RID)" \
		--description "$(NAME) MinGW-w64 (-gnu) native binaries"

exe: $(ARTIFACT)
nuget: $(NUPKG)

clean:
	rm -rf "$(OUTDIR)" "$(STAGE)" "$(BUILD)"
