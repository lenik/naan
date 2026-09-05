# naan

`naan`（name as a number）把名字哈希成一个数字。

```bash
naan [OPTION]... NAME...
```

每个参数输出一行：

```
number = (digest(NAME) mod DIV) + BIAS
```

摘要按大端整数解释。默认是 profile `w`（SHA-256，`DIV=65536`，`BIAS=0`）。

## 选项

| 选项 | 含义 |
|------|------|
| `-5`, `--md5` | MD5 |
| `-1`, `--sha1` | SHA-1 |
| `--sha224` | SHA-224 |
| `-2`, `--sha256` | SHA-256（默认） |
| `--sha384` | SHA-384 |
| `-8`, `--sha512` | SHA-512 |
| `-3`, `--sha3-256` | SHA3-256 |
| `--sha3-512` | SHA3-512 |
| `--blake2s` | BLAKE2s |
| `--blake2b` | BLAKE2b |
| `--ripemd160` | RIPEMD-160 |
| `--crc16` | CRC-16/CCITT-FALSE |
| `-c`, `--crc32` | CRC-32/ISO-HDLC |
| `-m`, `--mod DIV` | 模数（默认 65536） |
| `-b`, `--bias BIAS` | 约简后加上的偏置（默认 0） |
| `-p`, `--profile NAME` | 预设哈希、`DIV` 与 `BIAS` |

`DIV` 和 `BIAS` 可以是十进制或 `0x` 十六进制，并可带 `k`/`m`/`g`
后缀（1024 的幂）。

帮助文本与 man 页已覆盖 L3 语言（第一至第三档）。

## 预置 profile

| Profile | 哈希 | DIV | BIAS |
|---------|------|-----|------|
| `b` | SHA-256 | 256 | 0 |
| `w`（默认） | SHA-256 | 65536 | 0 |
| `dw` | SHA-256 | 4G（4294967296） | 0 |
| `wm` | SHA-1 | 2000 | 2000 |

## 示例

```bash
naan hello
naan -5 -p b alice bob
naan -p wm service-name
```

## 仓库结构

- `src/` - 应用源码（`naan.c`）与辅助模块（`name_number.c`）
- `tests/` - 单元测试与命令行测试
- `docs/` - AsciiDoc man 页源文件
- `meson.build` - 顶层构建定义

## 构建

```bash
sudo apt install meson ninja-build gcc pkg-config libssl-dev gettext asciidoctor
meson setup /build
ninja -C /build
meson test -C /build
```

## 许可证

Copyright (C) 2026 Lenik <naan@bodz.net>

采用 **AGPL-3.0-or-later** 许可。完整文本见 `LICENSE`。
