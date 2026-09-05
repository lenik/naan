/*
 * Copyright (C) 2026 Lenik <naan@bodz.net>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define _POSIX_C_SOURCE 200809L

#include "name_number.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>

int naan_parse_u64(const char *s, uint64_t *out) {
    char *end;
    unsigned long long v;
    unsigned long long mul = 1;

    if (s == NULL || *s == '\0' || out == NULL) {
        return -1;
    }

    errno = 0;
    v = strtoull(s, &end, 0);
    if (errno != 0 || end == s) {
        return -1;
    }

    if (*end != '\0') {
        switch (*end) {
        case 'k':
        case 'K':
            mul = 1024ULL;
            break;
        case 'm':
        case 'M':
            mul = 1024ULL * 1024ULL;
            break;
        case 'g':
        case 'G':
            mul = 1024ULL * 1024ULL * 1024ULL;
            break;
        default:
            return -1;
        }
        end++;
        if (*end != '\0') {
            return -1;
        }
    }

    if (mul != 1 && v > ULLONG_MAX / mul) {
        return -1;
    }

    *out = (uint64_t)(v * mul);
    return 0;
}

int naan_apply_profile(const char *name, uint64_t *div, uint64_t *bias) {
    if (name == NULL || div == NULL || bias == NULL) {
        return -1;
    }

    if (strcmp(name, "b") == 0) {
        *div = 256;
        *bias = 0;
        return 0;
    }
    if (strcmp(name, "w") == 0) {
        *div = 65536;
        *bias = 0;
        return 0;
    }
    if (strcmp(name, "dw") == 0) {
        *div = 4294967296ULL; /* 4G */
        *bias = 0;
        return 0;
    }
    if (strcmp(name, "wm") == 0) {
        *div = 2000;
        *bias = 2000;
        return 0;
    }
    return -1;
}

static uint16_t crc16_ccitt_false(const unsigned char *data, size_t len) {
    uint16_t crc = 0xFFFF;
    size_t i;
    int bit;

    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (bit = 0; bit < 8; bit++) {
            if (crc & 0x8000u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

static uint32_t crc32_iso_hdlc(const unsigned char *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    size_t i;
    int bit;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int)(crc & 1u));
        }
    }
    return ~crc;
}

static void store_be16(unsigned char *out, uint16_t v) {
    out[0] = (unsigned char)(v >> 8);
    out[1] = (unsigned char)v;
}

static void store_be32(unsigned char *out, uint32_t v) {
    out[0] = (unsigned char)(v >> 24);
    out[1] = (unsigned char)(v >> 16);
    out[2] = (unsigned char)(v >> 8);
    out[3] = (unsigned char)v;
}

int naan_digest(const void *data, size_t len, naan_algo algo,
                unsigned char *out, unsigned int *out_len) {
    const EVP_MD *md = NULL;
    EVP_MD_CTX *ctx;
    int ok;
    const unsigned char *bytes = data;

    if (out == NULL || out_len == NULL || (data == NULL && len != 0)) {
        return -1;
    }

    switch (algo) {
    case NAAN_ALGO_CRC16:
        store_be16(out, crc16_ccitt_false(bytes, len));
        *out_len = 2;
        return 0;
    case NAAN_ALGO_CRC32:
        store_be32(out, crc32_iso_hdlc(bytes, len));
        *out_len = 4;
        return 0;
    case NAAN_ALGO_MD5:
        md = EVP_md5();
        break;
    case NAAN_ALGO_SHA1:
        md = EVP_sha1();
        break;
    case NAAN_ALGO_SHA224:
        md = EVP_sha224();
        break;
    case NAAN_ALGO_SHA256:
        md = EVP_sha256();
        break;
    case NAAN_ALGO_SHA384:
        md = EVP_sha384();
        break;
    case NAAN_ALGO_SHA512:
        md = EVP_sha512();
        break;
    case NAAN_ALGO_SHA3_256:
        md = EVP_sha3_256();
        break;
    case NAAN_ALGO_SHA3_512:
        md = EVP_sha3_512();
        break;
    case NAAN_ALGO_BLAKE2S:
        md = EVP_blake2s256();
        break;
    case NAAN_ALGO_BLAKE2B:
        md = EVP_blake2b512();
        break;
    case NAAN_ALGO_RIPEMD160:
        md = EVP_ripemd160();
        break;
    default:
        return -1;
    }

    if (md == NULL) {
        return -1;
    }

    ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return -1;
    }

    ok = EVP_DigestInit_ex(ctx, md, NULL) == 1 &&
         EVP_DigestUpdate(ctx, data, len) == 1 &&
         EVP_DigestFinal_ex(ctx, out, out_len) == 1;
    EVP_MD_CTX_free(ctx);
    return ok ? 0 : -1;
}

uint64_t naan_reduce(const unsigned char *digest, unsigned int len, uint64_t mod) {
    unsigned __int128 acc = 0;
    unsigned int i;

    if (mod == 0 || digest == NULL) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        acc = (acc * 256 + digest[i]) % mod;
    }
    return (uint64_t)acc;
}

int naan_number(const void *data, size_t len, naan_algo algo,
                uint64_t div, uint64_t bias, uint64_t *out) {
    unsigned char digest[NAAN_DIGEST_MAX];
    unsigned int dlen = 0;

    if (out == NULL || div == 0) {
        return -1;
    }

    if (naan_digest(data, len, algo, digest, &dlen) != 0) {
        return -1;
    }

    *out = naan_reduce(digest, dlen, div) + bias;
    return 0;
}
