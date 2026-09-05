/*
 * Copyright (C) 2026 Lenik <naan@bodz.net>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define _GNU_SOURCE

#include "name_number.h"
#include "config.h"

#include <getopt.h>
#include <inttypes.h>
#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _(s) gettext(s)

static const char *progname(const char *argv0) {
    const char *base = strrchr(argv0, '/');
    return base ? base + 1 : argv0;
}

static void print_version(const char *prog) {
    printf("%s %s\n", prog, PROJECT_VERSION);
    printf("Copyright (C) %d %s <%s>\n", PROJECT_YEAR, PROJECT_AUTHOR, PROJECT_EMAIL);
    printf("%s\n", _("License AGPL-3.0-or-later: GNU AGPL version 3 or later"));
}

static void print_help(const char *prog) {
    printf(_("Usage: %s [OPTION]... NAME...\n"), prog);
    printf("%s\n", _("Compute a number from each name."));
    printf("\n");
    printf("%s\n", _("  -5, --md5           hash with MD5"));
    printf("%s\n", _("  -1, --sha1          hash with SHA-1"));
    printf("%s\n", _("      --sha224        hash with SHA-224"));
    printf("%s\n", _("  -2, --sha256        hash with SHA-256 (default)"));
    printf("%s\n", _("      --sha384        hash with SHA-384"));
    printf("%s\n", _("  -8, --sha512        hash with SHA-512"));
    printf("%s\n", _("  -3, --sha3-256      hash with SHA3-256"));
    printf("%s\n", _("      --sha3-512      hash with SHA3-512"));
    printf("%s\n", _("      --blake2s       hash with BLAKE2s"));
    printf("%s\n", _("      --blake2b       hash with BLAKE2b"));
    printf("%s\n", _("      --ripemd160     hash with RIPEMD-160"));
    printf("%s\n", _("      --crc16         digest with CRC-16"));
    printf("%s\n", _("  -c, --crc32         digest with CRC-32"));
    printf("%s\n", _("  -m, --mod DIV       modulus (default: 65536)"));
    printf("%s\n", _("  -b, --bias BIAS     added after reduction (default: 0)"));
    printf("%s\n", _("  -p, --profile NAME  hash and range profile (default: w)"));
    printf("%s\n", _("  -h, --help          display this help and exit"));
    printf("%s\n", _("      --version       output version information and exit"));
    printf("\n");
    printf("%s\n", _("Each output line is:"));
    printf("  (digest(NAME) mod DIV) + BIAS\n");
    printf("\n");
    printf("%s\n", _("Profiles:"));
    printf("  b    sha256 DIV=256      BIAS=0\n");
    printf("  w    sha256 DIV=65536    BIAS=0     (%s)\n", _("default"));
    printf("  dw   sha256 DIV=4G       BIAS=0\n");
    printf("  wm   sha1   DIV=2000     BIAS=2000\n");
    printf("\n");
    printf("%s\n", _("DIV and BIAS accept decimal or 0x hex, with optional k/m/g suffix (1024)."));
}

int main(int argc, char **argv) {
    static const struct option long_opts[] = {
        {"md5", no_argument, NULL, '5'},
        {"sha1", no_argument, NULL, '1'},
        {"sha224", no_argument, NULL, 1001},
        {"sha256", no_argument, NULL, '2'},
        {"sha384", no_argument, NULL, 1002},
        {"sha512", no_argument, NULL, '8'},
        {"sha3-256", no_argument, NULL, '3'},
        {"sha3-512", no_argument, NULL, 1003},
        {"blake2s", no_argument, NULL, 1004},
        {"blake2b", no_argument, NULL, 1005},
        {"ripemd160", no_argument, NULL, 1006},
        {"crc16", no_argument, NULL, 1007},
        {"crc32", no_argument, NULL, 'c'},
        {"mod", required_argument, NULL, 'm'},
        {"bias", required_argument, NULL, 'b'},
        {"profile", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'V'},
        {NULL, 0, NULL, 0},
    };

    const char *prog = progname(argv[0]);
    naan_algo algo = NAAN_ALGO_SHA256;

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);
    uint64_t div = 65536;
    uint64_t bias = 0;
    int c;
    int i;
    int status = 0;

    while ((c = getopt_long(argc, argv, "51283cm:b:p:h", long_opts, NULL)) != -1) {
        switch (c) {
        case '5':
            algo = NAAN_ALGO_MD5;
            break;
        case '1':
            algo = NAAN_ALGO_SHA1;
            break;
        case 1001:
            algo = NAAN_ALGO_SHA224;
            break;
        case '2':
            algo = NAAN_ALGO_SHA256;
            break;
        case 1002:
            algo = NAAN_ALGO_SHA384;
            break;
        case '8':
            algo = NAAN_ALGO_SHA512;
            break;
        case '3':
            algo = NAAN_ALGO_SHA3_256;
            break;
        case 1003:
            algo = NAAN_ALGO_SHA3_512;
            break;
        case 1004:
            algo = NAAN_ALGO_BLAKE2S;
            break;
        case 1005:
            algo = NAAN_ALGO_BLAKE2B;
            break;
        case 1006:
            algo = NAAN_ALGO_RIPEMD160;
            break;
        case 1007:
            algo = NAAN_ALGO_CRC16;
            break;
        case 'c':
            algo = NAAN_ALGO_CRC32;
            break;
        case 'm':
            if (naan_parse_u64(optarg, &div) != 0) {
                fprintf(stderr, _("%s: invalid DIV: %s\n"), prog, optarg);
                return 1;
            }
            break;
        case 'b':
            if (naan_parse_u64(optarg, &bias) != 0) {
                fprintf(stderr, _("%s: invalid BIAS: %s\n"), prog, optarg);
                return 1;
            }
            break;
        case 'p':
            if (naan_apply_profile(optarg, &algo, &div, &bias) != 0) {
                fprintf(stderr, _("%s: unknown profile: %s\n"), prog, optarg);
                return 1;
            }
            break;
        case 'h':
            print_help(prog);
            return 0;
        case 'V':
            print_version(prog);
            return 0;
        default:
            fprintf(stderr, _("Try '%s --help' for more information.\n"), prog);
            return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, _("%s: missing name\n"), prog);
        fprintf(stderr, _("Try '%s --help' for more information.\n"), prog);
        return 1;
    }

    if (div == 0) {
        fprintf(stderr, _("%s: DIV must be greater than 0\n"), prog);
        return 1;
    }

    for (i = optind; i < argc; i++) {
        uint64_t n;
        const char *name = argv[i];

        if (naan_number(name, strlen(name), algo, div, bias, &n) != 0) {
            fprintf(stderr, _("%s: failed to compute number for '%s'\n"), prog, name);
            status = 1;
            continue;
        }
        if (printf("%" PRIu64 "\n", n) < 0) {
            fprintf(stderr, _("%s: write error\n"), prog);
            return 1;
        }
    }

    if (fflush(stdout) != 0) {
        fprintf(stderr, _("%s: write error\n"), prog);
        return 1;
    }
    return status;
}
