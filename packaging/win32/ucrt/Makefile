# SPDX-License-Identifier: AGPL-3.0-or-later
# Native Windows UCRT (MSVC) build: <pkg>_ucrt-<arch>-<ver>.zip + .nupkg
#
# Expects a Visual Studio / Build Tools environment (vsdevcmd / GHA msvc-dev-cmd).

include ../../lib/common.mk

KIND    := ucrt
OUTDIR  := $(CURDIR)/out
STAGE   := $(CURDIR)/stage
BUILD   := $(CURDIR)/build
# arch: x64 | arm64 (override: make ARCH=arm64)
ARCH    ?= x64
RID     ?= $(if $(filter arm64,$(ARCH)),win-arm64,win-x64)
ARTIFACT := $(OUTDIR)/$(NAME)_ucrt-$(ARCH)-$(VERSION).zip
NUPKG   := $(OUTDIR)/$(NAME).$(VERSION).nupkg
PACK_NUGET := $(SRCDIR)/scripts/ci/pack-nuget.py

.PHONY: all local stage nuget clean

all:
	"$(HOST_SH)" run $(KIND) -- $(MAKE) -C "$(CURDIR)" local

local: $(ARTIFACT) $(NUPKG)

stage:
	rm -rf "$(STAGE)" "$(BUILD)"
	mkdir -p "$(STAGE)" "$(BUILD)"
	@set -e; \
	cd "$(SRCDIR)"; \
	if [ -z "$${OS:-}" ] || [ "$${OS}" != "Windows_NT" ]; then \
		echo "ucrt: native MSVC/UCRT build requires Windows" >&2; \
		exit 1; \
	fi; \
	meson setup "$(BUILD)" --prefix=/ --buildtype=release \
		--backend=ninja; \
	meson compile -C "$(BUILD)"; \
	DESTDIR="$(STAGE)" meson install -C "$(BUILD)"

$(ARTIFACT): stage
	mkdir -p "$(OUTDIR)"
	@set -e; \
	if [ ! -d "$(STAGE)" ] || [ -z "$$(find "$(STAGE)" -type f 2>/dev/null | head -1)" ]; then \
		echo "ucrt: empty stage" >&2; \
		exit 1; \
	fi; \
	rm -f "$(ARTIFACT)"; \
	( cd "$(STAGE)" && zip -r -9 "$(ARTIFACT)" . ); \
	echo "$(ARTIFACT)"

$(NUPKG): stage
	mkdir -p "$(OUTDIR)"
	@set -e; \
	if [ ! -f "$(PACK_NUGET)" ]; then \
		echo "ucrt: missing $(PACK_NUGET)" >&2; \
		exit 1; \
	fi; \
	python3 "$(PACK_NUGET)" \
		--id "$(NAME)" \
		--version "$(VERSION)" \
		--stage "$(STAGE)" \
		--out "$(NUPKG)" \
		--rid "$(RID)" \
		--description "$(NAME) native Windows UCRT ($(ARCH))"

nuget: $(NUPKG)

clean:
	rm -rf "$(OUTDIR)" "$(STAGE)" "$(BUILD)"
