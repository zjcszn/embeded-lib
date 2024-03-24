#include "boot_osal.h"
#include "ff.h"

#define LOG_LVL  ELOG_LVL_INFO
#define LOG_TAG "BOOT"

#include "boot.h"
#include "components.h"
#include "libcrc.h"

#include "mbedtls/platform.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/pk.h"


static const unsigned char rsa_pbkey_n[] = RSA_PBKEY_N;
static const unsigned char rsa_pbkey_e[] = RSA_PBKEY_E;
static const unsigned char rsa_pbkey_pem[] = RSA_PBKEY_PEM;

/**
 * @brief verify firmware file
 *
 * @param fpath file path
 *
 * @param hash  The buffer holding the message digest. This must be a readable buffer of 32 Bytes.
 *
 * @return      true on pass, false on failed.
 */
bool firmware_verify_on_file(const char *fpath) {
    if (!fpath) return false;

    bool ret = false;
    unsigned int br;
    unsigned long read_crc;
    unsigned long calc_crc;
    unsigned char hash[32];
    FIL *fw = NULL;
    unsigned char *read_buf = NULL;
    mbedtls_sha256_context *hash_ctx = NULL;

    if (((fw = rtos_malloc(sizeof(FIL))) == NULL) ||
        ((read_buf = rtos_malloc(sizeof(unsigned char) * 512)) == NULL) ||
        ((hash_ctx = rtos_malloc(sizeof(mbedtls_sha256_context))) == NULL)) {
        goto __exit;
    }

    if (f_open(fw, fpath, FA_READ | FA_OPEN_EXISTING) == FR_OK) {
        if (f_read(fw, read_buf, sizeof(struct mbl_header), &br) == FR_OK) {
            read_crc = ((mbl_header_t) read_buf)->head_crc;
            calc_crc = crc32(read_buf, MBL_HEADER_HEADCRC_OFFSET);
            if (read_crc == calc_crc) {
                /* calculate firmware sha-256 */
                f_lseek(fw, ((mbl_header_t) read_buf)->fw_offset);
                log_d("Start caculate SHA-256 hash...");
                mbedtls_sha256_init(hash_ctx);
                mbedtls_sha256_starts(hash_ctx, 0);
                while ((f_read(fw, read_buf, 512, &br) == FR_OK) && br) {
                    mbedtls_sha256_update(hash_ctx, read_buf, br);
                }
                mbedtls_sha256_finish(hash_ctx, hash);
                mbedtls_sha256_free(hash_ctx);

                /* read rsa signature && verify */
                f_lseek(fw, MBL_HEADER_FW_SIGN_OFFSET);
                if (f_read(fw, read_buf, FIRMWARE_SIGN_LENGTH, &br) == FR_OK &&
                    firmware_sign_verify(hash, read_buf)) {
                    ret = true;
                }
#if LOG_LVL >= ELOG_LVL_DEBUG
                elog_hexdump("SHA-256 hash", 32, hash, 32);
#endif
            }
        }
        f_close(fw);
    } else {
        log_e("open \"%s\" failed", fpath);
    }
    log_d("verify firmware on file \"%s\": [%s]", fpath, ret ? "success" : "failed");

    __exit:
    if (hash_ctx) rtos_free(hash_ctx);
    if (read_buf) rtos_free(read_buf);
    if (fw) rtos_free(fw);

    return ret;
}

bool firmware_verify_on_flash(const char *part_name) {
    bool ret = false;
    unsigned char *fw_sign;
    unsigned char fw_hash[32];
    unsigned long read_crc;
    unsigned long calc_crc;
    unsigned long part_base;
    fal_partition_t part;
    mbl_header_t fw_header;

    part = fal_partition_find(part_name);
    if (part != NULL) {
        part_base = FLASH_BASE + part->offset;
        fw_header = (mbl_header_t) part_base;

        read_crc = fw_header->head_crc;
        calc_crc = crc32((uint8_t *) fw_header, MBL_HEADER_HEADCRC_OFFSET);
        log_d("app header read crc: %X", read_crc);
        log_d("app header calc crc: %X", calc_crc);

        if (read_crc == calc_crc) {
            fw_sign = fw_header->fw_sign;
            if (firmware_hash_calc_on_flash((const unsigned char *) (part_base + fw_header->fw_offset),
                                            fw_header->raw_size, fw_hash)
                && firmware_sign_verify(fw_hash, fw_sign)) {
                ret = true;
            }
        }
    }
    log_i("verify firmware on [%s] partition [%s]", part_name, ret ? "pass" : "fail");
    return ret;
}

/**
 * @brief Caculate hash code on flash
 * 
 * @param fw_addr firmware addr
 * 
 * @param fw_size firmware size
 * 
 * @param hash  The buffer holding the message digest. This must be a readable buffer of 32 Bytes.
 *
 * @return      true on pass, false on failed. 
 */
bool firmware_hash_calc_on_flash(const unsigned char *fw_addr, unsigned long fw_size, unsigned char *hash) {
    if (!fw_addr || !fw_size || !hash) return false;

    mbedtls_sha256_context *hash_ctx;
    hash_ctx = mbedtls_malloc(sizeof(mbedtls_sha256_context));
    if (!hash_ctx) return false;

    log_d("Start caculate SHA-256 hash...");
    mbedtls_sha256_init(hash_ctx);
    mbedtls_sha256_starts(hash_ctx, 0);
    mbedtls_sha256_update(hash_ctx, fw_addr, fw_size);
    mbedtls_sha256_finish(hash_ctx, hash);
    mbedtls_sha256_free(hash_ctx);
    mbedtls_free(hash_ctx);

#if LOG_LVL >= ELOG_LVL_DEBUG
    elog_hexdump("SHA-256 hash", 32, hash, 32);
#endif

    return true;
}

/**
 * @brief Verify firmware integrity and signature
 * 
 * @param hash The buffer holding the message digest or raw data. This must be a readable buffer of at 
 *             least hashlen Bytes.
 * @param sig  The buffer holding the signature. This must be a readable buffer of length ctx->len Bytes. 
 *             For example, 256 Bytes for an 2048-bit RSA modulus.
 * @return     true on pass, false on failed. 
 */
bool firmware_sign_verify(const unsigned char *hash, const unsigned char *sig) {
    mbedtls_mpi rsa_n;
    mbedtls_mpi rsa_e;
    mbedtls_rsa_context *rsa_ctx;
    int ret;

    rsa_ctx = (mbedtls_rsa_context *) mbedtls_malloc(sizeof(mbedtls_rsa_context));
    if (rsa_ctx == NULL) return false;

    log_d("start verify firmware rsa signature...");

#if LOG_LVL >= ELOG_LVL_DEBUG
    elog_hexdump("RSA pubkey N", 32, rsa_pbkey_n, sizeof(rsa_pbkey_n));
    elog_hexdump("RSA pubkey E", 32, rsa_pbkey_e, sizeof(rsa_pbkey_e));
#endif

    mbedtls_mpi_init(&rsa_n);
    mbedtls_mpi_init(&rsa_e);
    mbedtls_rsa_init(rsa_ctx);
    mbedtls_rsa_set_padding(rsa_ctx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    mbedtls_mpi_read_binary(&rsa_n, rsa_pbkey_n, sizeof(rsa_pbkey_n));
    mbedtls_mpi_read_binary(&rsa_e, rsa_pbkey_e, sizeof(rsa_pbkey_e));
    mbedtls_rsa_import(rsa_ctx, &rsa_n, NULL, NULL, NULL, &rsa_e);

    log_d("rsa pubkey size: %u", mbedtls_mpi_bitlen(&rsa_ctx->private_N));

    assert(mbedtls_rsa_check_pubkey(rsa_ctx));

    ret = mbedtls_rsa_rsassa_pss_verify(rsa_ctx, MBEDTLS_MD_SHA256, 32, hash, sig);
    log_d("verify firmware rsa signature [%s], code [%X]", ret ? "fail" : "pass", ret < 0 ? -ret : ret);

    mbedtls_rsa_free(rsa_ctx);
    mbedtls_free(rsa_ctx);

    return ret ? false : true;
}


/**
 * @brief Verify firmware integrity and signature by pem format rsa public key
 * 
 * @param hash The buffer holding the message digest or raw data. This must be a readable buffer of at 
 *             least hashlen Bytes.
 * @param sig  The buffer holding the signature. This must be a readable buffer of length ctx->len Bytes. 
 *             For example, 256 Bytes for an 2048-bit RSA modulus.
 * @return     true on pass, false on failed. 
 */
bool firmware_sign_verify_by_pem(const unsigned char *hash, const unsigned char *sig) {
    int ret;
    mbedtls_pk_context rsa_pk_ctx;
    mbedtls_pk_rsassa_pss_options rsa_pk_opt;

    rsa_pk_opt.mgf1_hash_id = MBEDTLS_MD_SHA256;
    rsa_pk_opt.expected_salt_len = MBEDTLS_RSA_SALT_LEN_ANY;

    mbedtls_pk_init(&rsa_pk_ctx);
    ret = mbedtls_pk_parse_public_key(&rsa_pk_ctx, rsa_pbkey_pem, sizeof(rsa_pbkey_pem));
    log_d("Parse public key %s, [%X]", ret ? "failed" : "success", ret < 0 ? -ret : ret);
    ret = mbedtls_pk_can_do(&rsa_pk_ctx, MBEDTLS_PK_RSA);
    log_d("The key is %s rsa key", ret ? "a" : "not a");

    mbedtls_rsa_set_padding(mbedtls_pk_rsa(rsa_pk_ctx), MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    ret = mbedtls_pk_verify_ext(MBEDTLS_PK_RSASSA_PSS, &rsa_pk_opt, &rsa_pk_ctx, MBEDTLS_MD_SHA256, hash, 32, sig, 128);
    log_d("Firmware RSA fw_sign verify %s [%X]", ret ? "failed" : "pass", ret < 0 ? -ret : ret);

    mbedtls_pk_free(&rsa_pk_ctx);

    return ret ? false : true;
}
