
#define LOG_LVL  ELOG_LVL_DEBUG
#define LOG_TAG "MBEDTLS"


#include "rsa_pss_verify.h"

#include "mbedtls/platform.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"

#include "mbedtls/pk.h"

#include "elog.h"
#include "ff.h"


static const unsigned char rsa_pbkey_n[]   = RSA_PBKEY_N;
static const unsigned char rsa_pbkey_e[]   = RSA_PBKEY_E;

static const unsigned char rsa_pbkey_pem[] = RSA_PBKEY_PEM;

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
bool firmware_calc_hash_on_flash(const unsigned char *fw_addr, unsigned long fw_size, unsigned char *hash)
{
    if (!fw_addr || !fw_size || !hash) return false;

    mbedtls_sha256_context *hash_ctx;
    hash_ctx = mbedtls_malloc(sizeof(mbedtls_sha256_context));
    if (!hash_ctx) return false;
    
    log_i("Start caculate SHA-256 hash...");
    mbedtls_sha256_init(hash_ctx);
    mbedtls_sha256_starts(hash_ctx, 0);
    mbedtls_sha256_update(hash_ctx, fw_addr, fw_size);
    mbedtls_sha256_finish(hash_ctx, hash);
    elog_hexdump("SHA-256 hash", 32, hash, 32);
    mbedtls_sha256_free(hash_ctx);
    mbedtls_free(hash_ctx);

    return true;
}

/**
 * @brief Caculate hash code by file io
 * 
 * @param fpath file path
 * 
 * @param hash  The buffer holding the message digest. This must be a readable buffer of 32 Bytes.
 *
 * @return      true on pass, false on failed. 
 */
bool firmware_calc_hash_on_file(const char *fpath, unsigned char *hash)
{
    if (!fpath || !hash) return false;

    unsigned int  br;
    bool ret = false;

    FIL *file_obj;
    unsigned char *read_buf;
    mbedtls_sha256_context *hash_ctx;

    file_obj = mbedtls_malloc(sizeof(FIL));
    if (!file_obj) goto __exit;

    read_buf = mbedtls_malloc(sizeof(unsigned char) * 512);
    if (!read_buf) goto __exit;

    hash_ctx = mbedtls_malloc(sizeof(mbedtls_sha256_context));
    if (!hash_ctx) goto __exit;

    log_i("Start caculate SHA-256 hash...");
    mbedtls_sha256_init(hash_ctx);
    mbedtls_sha256_starts(hash_ctx, 0);
    if (f_open(file_obj, fpath, FA_READ | FA_OPEN_EXISTING) == FR_OK) 
    {
        f_lseek(file_obj, 0);
        while ((f_read(file_obj, read_buf, 512, &br) == FR_OK) && br)
        {
            mbedtls_sha256_update(hash_ctx, read_buf, br);
        }
        f_close(file_obj);
        mbedtls_sha256_finish(hash_ctx, hash);
        elog_hexdump("SHA-256 hash", 32, hash, 32);
        ret = true;
    }
    log_i("Caculate hash code %s...", ret ? "success" : "failed");
    mbedtls_sha256_free(hash_ctx);
    mbedtls_free(hash_ctx);

__exit:
    if (read_buf) mbedtls_free(read_buf);
    if (file_obj) mbedtls_free(file_obj);

    return ret;
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
bool firmware_verify(const unsigned char *hash, const unsigned char *sig)
{
    mbedtls_mpi rsa_n;
    mbedtls_mpi rsa_e;
    mbedtls_rsa_context  *rsa_ctx;
    int ret;

    rsa_ctx = (mbedtls_rsa_context *)mbedtls_malloc(sizeof(mbedtls_rsa_context));
    if (rsa_ctx == NULL) return false;

    log_i("Start verify firmware RSA signature...");
    elog_hexdump("RSA pubkey N", 32, rsa_pbkey_n, sizeof(rsa_pbkey_n));
    elog_hexdump("RSA pubkey E", 32, rsa_pbkey_e, sizeof(rsa_pbkey_e));

    mbedtls_mpi_init(&rsa_n);
    mbedtls_mpi_init(&rsa_e);
    mbedtls_rsa_init(rsa_ctx);
    mbedtls_rsa_set_padding(rsa_ctx, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    mbedtls_mpi_read_binary(&rsa_n, rsa_pbkey_n, sizeof(rsa_pbkey_n));
    mbedtls_mpi_read_binary(&rsa_e, rsa_pbkey_e, sizeof(rsa_pbkey_e));
    mbedtls_rsa_import(rsa_ctx, &rsa_n, NULL, NULL, NULL, &rsa_e);

    log_i("RSA pubkey size: %u", mbedtls_mpi_bitlen(&rsa_ctx->private_N));

    assert(mbedtls_rsa_check_pubkey(rsa_ctx));

    ret = mbedtls_rsa_rsassa_pss_verify(rsa_ctx, MBEDTLS_MD_SHA256, 32, hash, sig);
    log_i("Firmware RSA signature verify %s [%X]", ret ? "failed" : "pass", ret < 0 ? -ret : ret);

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
bool firmware_verify_by_pem(const unsigned char *hash, const unsigned char *sig)
{
    int ret;
    mbedtls_pk_context rsa_pk_ctx;
    mbedtls_pk_rsassa_pss_options rsa_pk_opt;

    rsa_pk_opt.mgf1_hash_id = MBEDTLS_MD_SHA256;
    rsa_pk_opt.expected_salt_len = MBEDTLS_RSA_SALT_LEN_ANY;
    
    mbedtls_pk_init(&rsa_pk_ctx);
    ret = mbedtls_pk_parse_public_key(&rsa_pk_ctx, rsa_pbkey_pem, sizeof(rsa_pbkey_pem));
    log_i("Parse public key %s, [%X]", ret ? "failed" : "success", ret < 0 ? -ret : ret);
    ret = mbedtls_pk_can_do(&rsa_pk_ctx, MBEDTLS_PK_RSA);
    log_i("The key is %s rsa key", ret ? "a" : "not a");
    
    mbedtls_rsa_set_padding(mbedtls_pk_rsa(rsa_pk_ctx), MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    ret = mbedtls_pk_verify_ext(MBEDTLS_PK_RSASSA_PSS, &rsa_pk_opt, &rsa_pk_ctx, MBEDTLS_MD_SHA256, hash, 32, sig, 128);
    log_i("Firmware RSA signature verify %s [%X]", ret ? "failed" : "pass", ret < 0 ? -ret : ret);

    mbedtls_pk_free(&rsa_pk_ctx);

    return ret ? false : true;
}
