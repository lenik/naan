# SPDX-License-Identifier: AGPL-3.0-or-later
# Portable MinGW-w64 build: <pkg>_mingw-<ver>.exe
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
MINGW_CC := $(shell command -v x86_64-w64-mingw32-gcc 2>/dev/null)

.PHONY: all local stage exe clean

# Debian does not ship OpenSSL for mingw-w64. Skip instead of failing
# `zfr release` when the cross compiler is present but libcrypto is not.
MINGW_PKG_CONFIG := $(shell command -v x86_64-w64-mingw32-pkg-config 2>/dev/null)
HAVE_MINGW_CRYPTO := $(shell \
	if [ -n "$(MINGW_PKG_CONFIG)" ] && "$(MINGW_PKG_CONFIG)" --exists libcrypto; then \
		echo yes; \
	fi)

all:
	"$(HOST_SH)" run $(KIND) -- $(MAKE) -C "$(CURDIR)" local

local:
	@set -e; \
	if [ -n "$$OS" ] && [ "$$OS" = Windows_NT ]; then \
		$(MAKE) "$(ARTIFACT)"; \
	elif [ -n "$(MINGW_CC)" ] && [ "$(HAVE_MINGW_CRYPTO)" = yes ]; then \
		$(MAKE) "$(ARTIFACT)"; \
	elif [ -n "$(MINGW_CC)" ]; then \
		echo "mingw: skip portable exe (no cross libcrypto)" >&2; \
	else \
		echo "mingw: need MSYS/MinGW or x86_64-w64-mingw32-gcc" >&2; \
		exit 1; \
	fi

stage:
	rm -rf "$(STAGE)" "$(BUILD)"
	mkdir -p "$(STAGE)" "$(BUILD)"
	@set -e; \
	cd "$(SRCDIR)"; \
	if [ -n "$$OS" ] && [ "$$OS" = Windows_NT ]; then \
		meson setup "$(BUILD)" --prefix=/ --buildtype=release; \
	elif [ -n "$(MINGW_CC)" ] && [ "$(HAVE_MINGW_CRYPTO)" = yes ]; then \
		meson setup "$(BUILD)" --cross-file "$(CROSS)" --prefix=/ --buildtype=release; \
	else \
		echo "mingw: need MSYS/MinGW with libcrypto, or x86_64-w64-mingw32-gcc + OpenSSL" >&2; \
		exit 1; \
	fi
	meson compile -C "$(BUILD)"
	DESTDIR="$(STAGE)" meson install -C "$(BUILD)"

$(ARTIFACT): stage
	mkdir -p "$(OUTDIR)"
	@set -e; \
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
	fi
	@echo "$(ARTIFACT)"

exe: $(ARTIFACT)

clean:
	rm -rf "$(OUTDIR)" "$(STAGE)" "$(BUILD)"
