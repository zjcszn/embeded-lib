#ifndef __RSA_PSS_VERIFY_H__
#define __RSA_PSS_VERIFY_H__

#include <stdbool.h>

/* RSA public key N param */
#define RSA_PBKEY_N                                                                                     \
{                                                                                                       \
    0x00, 0xa4, 0x26, 0x31, 0xa4, 0xfb, 0xff, 0x37, 0x55, 0x79, 0x90, 0x2c, 0x1a, 0x8b, 0x26, 0x73,     \
    0x9c, 0x53, 0x10, 0xda, 0x24, 0x9e, 0x3e, 0x33, 0x1d, 0x00, 0x77, 0x93, 0xc7, 0xb1, 0x15, 0xe3,     \
    0x2a, 0x6a, 0x4a, 0xa1, 0xc3, 0x5d, 0x27, 0xb6, 0xd9, 0x15, 0x8c, 0xb8, 0x99, 0x9f, 0x71, 0x20,     \
    0x02, 0xbc, 0x5f, 0xd4, 0x03, 0x70, 0xc3, 0x79, 0x51, 0xcc, 0x33, 0xe3, 0xe4, 0x8d, 0x33, 0xec,     \
    0x4e, 0xa9, 0xe6, 0x13, 0xa7, 0x05, 0xdb, 0x98, 0x25, 0x0a, 0x1f, 0xfe, 0xbd, 0xb4, 0x1e, 0x6e,     \
    0x03, 0x4a, 0x89, 0x53, 0xc1, 0xbb, 0x3b, 0x26, 0xde, 0x8c, 0x66, 0xab, 0xb1, 0x21, 0x0c, 0xfc,     \
    0xc6, 0xb3, 0xd8, 0x69, 0x15, 0x76, 0x61, 0x02, 0xc9, 0x91, 0x9c, 0xf7, 0x10, 0x60, 0xb5, 0xe7,     \
    0x56, 0xf4, 0xaa, 0x59, 0x54, 0x85, 0xc6, 0x92, 0xb9, 0x1d, 0x2f, 0x9a, 0x1c, 0x16, 0x93, 0xad,     \
    0xbd,                                                                                               \
}

/* RSA public key E param */
#define RSA_PBKEY_E                                                                                     \
{                                                                                                       \
    0x01, 0x00, 0x01,                                                                                   \
}

/* RSA PEM format public key */
#define RSA_PBKEY_PEM                                                                                   \
"-----BEGIN PUBLIC KEY-----\n\
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCkJjGk+/83VXmQLBqLJnOcUxDa\n\
JJ4+Mx0Ad5PHsRXjKmpKocNdJ7bZFYy4mZ9xIAK8X9QDcMN5Ucwz4+SNM+xOqeYT\n\
pwXbmCUKH/69tB5uA0qJU8G7OybejGarsSEM/Maz2GkVdmECyZGc9xBgtedW9KpZ\n\
VIXGkrkdL5ocFpOtvQIDAQAB\n\
-----END PUBLIC KEY-----"




bool firmware_calc_hash_on_flash(const unsigned char *fw_addr, unsigned long fw_size, unsigned char *hash);
bool firmware_calc_hash_on_file(const char *fpath, unsigned char *hash);
bool firmware_verify(const unsigned char *hash, const unsigned char *sig);
bool firmware_verify_by_pem(const unsigned char *hash, const unsigned char *sig);






#endif