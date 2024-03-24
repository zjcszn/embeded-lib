#ifndef __RSA_PSS_VERIFY_H__
#define __RSA_PSS_VERIFY_H__

#include <stdbool.h>



bool firmware_verify_on_flash(const char *part_name);
bool firmware_verify_on_file(const char *fpath);
bool firmware_hash_calc_on_flash(const unsigned char *fw_addr, unsigned long fw_size, unsigned char *hash);
bool firmware_sign_verify(const unsigned char *hash, const unsigned char *sig);
bool firmware_sign_verify_by_pem(const unsigned char *hash, const unsigned char *sig);






#endif