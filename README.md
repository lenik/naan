# naan

`naan` (name as a number) hashes a name and prints a number.

```bash
naan [OPTION]... NAME...
```

Each argument prints one line:

```
number = (digest(NAME) mod DIV) + BIAS
```

The digest is treated as a big-endian integer.  The default is profile
`w` (SHA-256, `DIV=65536`, `BIAS=0`).

## Options

| Option | Meaning |
|--------|---------|
| `-5`, `--md5` | MD5 |
| `-1`, `--sha1` | SHA-1 |
| `--sha224` | SHA-224 |
| `-2`, `--sha256` | SHA-256 (default) |
| `--sha384` | SHA-384 |
| `-8`, `--sha512` | SHA-512 |
| `-3`, `--sha3-256` | SHA3-256 |
| `--sha3-512` | SHA3-512 |
| `--blake2s` | BLAKE2s |
| `--blake2b` | BLAKE2b |
| `--ripemd160` | RIPEMD-160 |
| `--crc16` | CRC-16/CCITT-FALSE |
| `-c`, `--crc32` | CRC-32/ISO-HDLC |
| `-m`, `--mod DIV` | modulus (default 65536) |
| `-b`, `--bias BIAS` | added after reduction (default 0) |
| `-p`, `--profile NAME` | preset hash, `DIV`, and `BIAS` |

`DIV` and `BIAS` accept decimal or `0x` hex, with optional `k`/`m`/`g`
(powers of 1024).

Help text and man pages are translated for L3 locales (Tier I–III).

## Profiles

| Profile | Hash | DIV | BIAS |
|---------|------|-----|------|
| `b` | SHA-256 | 256 | 0 |
| `w` (default) | SHA-256 | 65536 | 0 |
| `dw` | SHA-256 | 4G (4294967296) | 0 |
| `wm` | SHA-1 | 2000 | 2000 |

## Examples

```bash
naan hello
naan -5 -p b alice bob
naan -p wm service-name
```

## Repository layout

- `src/` - application sources (`naan.c`) and helpers (`name_number.c`)
- `tests/` - unit and CLI tests
- `docs/` - AsciiDoc man page sources
- `meson.build` - top-level build definition

## Build

```bash
sudo apt install meson ninja-build gcc pkg-config libssl-dev gettext asciidoctor
meson setup /build
ninja -C /build
meson test -C /build
```

## License

Copyright (C) 2026 Lenik <naan@bodz.net>

Licensed under **AGPL-3.0-or-later**.
See `LICENSE` for the full text and supplemental project terms.
