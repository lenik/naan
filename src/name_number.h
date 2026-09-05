#ifndef NAME_NUMBER_H
#define NAME_NUMBER_H

#include <stddef.h>
#include <stdint.h>

#define NAAN_DIGEST_MAX 64

typedef enum {
    NAAN_ALGO_MD5 = 0,
    NAAN_ALGO_SHA1,
    NAAN_ALGO_SHA224,
    NAAN_ALGO_SHA256,
    NAAN_ALGO_SHA384,
    NAAN_ALGO_SHA512,
    NAAN_ALGO_SHA3_256,
    NAAN_ALGO_SHA3_512,
    NAAN_ALGO_BLAKE2S,
    NAAN_ALGO_BLAKE2B,
    NAAN_ALGO_RIPEMD160,
    NAAN_ALGO_CRC16,
    NAAN_ALGO_CRC32
} naan_algo;

int naan_parse_u64(const char *s, uint64_t *out);
int naan_apply_profile(const char *name, naan_algo *algo, uint64_t *div,
                       uint64_t *bias);
int naan_digest(const void *data, size_t len, naan_algo algo,
                unsigned char *out, unsigned int *out_len);
uint64_t naan_reduce(const unsigned char *digest, unsigned int len, uint64_t mod);
int naan_number(const void *data, size_t len, naan_algo algo,
                uint64_t div, uint64_t bias, uint64_t *out);

#endif /* NAME_NUMBER_H */
