/*
 * Copyright (C) 2026 Lenik <naan@bodz.net>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200809L

#include "name_number.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void expect_ok(const char *name, int rc) {
    if (rc != 0) {
        fprintf(stderr, "FAIL %s: unexpected error\n", name);
        failures++;
    }
}

static void expect_err(const char *name, int rc) {
    if (rc == 0) {
        fprintf(stderr, "FAIL %s: expected error\n", name);
        failures++;
    }
}

static void expect_eq_u64(const char *name, uint64_t got, uint64_t want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s: got %" PRIu64 " want %" PRIu64 "\n", name, got, want);
        failures++;
    }
}

static void check_number(const char *title, const char *name, naan_algo algo,
                         uint64_t div, uint64_t bias, uint64_t want) {
    uint64_t got = 0;
    if (naan_number(name, strlen(name), algo, div, bias, &got) != 0) {
        fprintf(stderr, "FAIL %s: compute error\n", title);
        failures++;
        return;
    }
    expect_eq_u64(title, got, want);
}

int main(void) {
    uint64_t v = 0;
    uint64_t div = 0;
    uint64_t bias = 0;
    naan_algo algo = NAAN_ALGO_MD5;

    expect_ok("parse 256", naan_parse_u64("256", &v));
    expect_eq_u64("parse 256 value", v, 256);
    expect_ok("parse 4G", naan_parse_u64("4G", &v));
    expect_eq_u64("parse 4G value", v, 4294967296ULL);
    expect_ok("parse 64k", naan_parse_u64("64k", &v));
    expect_eq_u64("parse 64k value", v, 65536);
    expect_ok("parse 1M", naan_parse_u64("1M", &v));
    expect_eq_u64("parse 1M value", v, 1048576);
    expect_ok("parse 0x100", naan_parse_u64("0x100", &v));
    expect_eq_u64("parse 0x100 value", v, 256);
    expect_err("parse empty", naan_parse_u64("", &v));
    expect_err("parse junk", naan_parse_u64("12x", &v));

    expect_ok("profile b", naan_apply_profile("b", &algo, &div, &bias));
    expect_eq_u64("profile b algo", (uint64_t)algo, NAAN_ALGO_SHA256);
    expect_eq_u64("profile b div", div, 256);
    expect_eq_u64("profile b bias", bias, 0);
    expect_ok("profile w", naan_apply_profile("w", &algo, &div, &bias));
    expect_eq_u64("profile w algo", (uint64_t)algo, NAAN_ALGO_SHA256);
    expect_eq_u64("profile w div", div, 65536);
    expect_eq_u64("profile w bias", bias, 0);
    expect_ok("profile dw", naan_apply_profile("dw", &algo, &div, &bias));
    expect_eq_u64("profile dw algo", (uint64_t)algo, NAAN_ALGO_SHA256);
    expect_eq_u64("profile dw div", div, 4294967296ULL);
    expect_eq_u64("profile dw bias", bias, 0);
    expect_ok("profile wm", naan_apply_profile("wm", &algo, &div, &bias));
    expect_eq_u64("profile wm algo", (uint64_t)algo, NAAN_ALGO_SHA1);
    expect_eq_u64("profile wm div", div, 2000);
    expect_eq_u64("profile wm bias", bias, 0);
    expect_err("profile unknown", naan_apply_profile("xx", &algo, &div, &bias));

    check_number("hello sha256 w", "hello", NAAN_ALGO_SHA256, 65536, 0, 38948);
    check_number("hello sha256 b", "hello", NAAN_ALGO_SHA256, 256, 0, 36);
    check_number("hello sha256 dw", "hello", NAAN_ALGO_SHA256, 4294967296ULL, 0, 2475399204ULL);
    check_number("hello sha1 wm", "hello", NAAN_ALGO_SHA1, 2000, 0, 1181);
    check_number("hello md5 w", "hello", NAAN_ALGO_MD5, 65536, 0, 50578);
    check_number("hello sha1 w", "hello", NAAN_ALGO_SHA1, 65536, 0, 17229);
    check_number("world sha256 w", "world", NAAN_ALGO_SHA256, 65536, 0, 47271);
    check_number("empty sha256 w", "", NAAN_ALGO_SHA256, 65536, 0, 47189);
    check_number("naan md5 1M", "naan", NAAN_ALGO_MD5, 1048576, 0, 134446);
    check_number("hello crc16 w", "hello", NAAN_ALGO_CRC16, 65536, 0, 53870);
    check_number("hello crc32 w", "hello", NAAN_ALGO_CRC32, 65536, 0, 42630);
    check_number("hello sha512 w", "hello", NAAN_ALGO_SHA512, 65536, 0, 49219);
    check_number("hello sha3-256 w", "hello", NAAN_ALGO_SHA3_256, 65536, 0, 62354);
    check_number("hello blake2s w", "hello", NAAN_ALGO_BLAKE2S, 65536, 0, 51749);
    check_number("hello ripemd160 w", "hello", NAAN_ALGO_RIPEMD160, 65536, 0, 23245);
    check_number("empty crc32 w", "", NAAN_ALGO_CRC32, 65536, 0, 0);

    expect_err("div zero", naan_number("x", 1, NAAN_ALGO_SHA256, 0, 0, &v));
    check_number("div==bias", "hello", NAAN_ALGO_SHA256, 10, 10, 10);

    return failures == 0 ? 0 : 1;
}
