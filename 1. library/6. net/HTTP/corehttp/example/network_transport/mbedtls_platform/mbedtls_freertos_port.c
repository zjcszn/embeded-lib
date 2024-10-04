/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "semphr.h"

/* mbed TLS includes. */
#ifdef   MBEDTLS_CONFIG_FILE
#include MBEDTLS_CONFIG_FILE
#else
#include "mbedtls/mbedtls_config.h"
#endif

#include "mbedtls/entropy.h"
#include "mbedtls_freertos_port.h"
#include "mbedtls/platform_time.h"
#include "mbedtls/platform.h"

#include "bsp_rng.h"
#include "bsp_rtc.h"

/*-----------------------------------------------------------*/

/**
 * @brief Allocates memory for an array of members.
 *
 * @param[in] nmemb Number of members that need to be allocated.
 * @param[in] size Size of each member.
 *
 * @return Pointer to the beginning of newly allocated memory.
 */
void *mbedtls_platform_calloc(size_t nmemb,
                              size_t size) {
    size_t totalSize = nmemb * size;
    void *pBuffer = NULL;
    
    /* Check that neither nmemb nor size were 0. */
    if (totalSize > 0) {
        /* Overflow check. */
        if ((totalSize / size) == nmemb) {
            pBuffer = pvPortMalloc(totalSize);
            
            if (pBuffer != NULL) {
                (void)memset(pBuffer, 0U, totalSize);
            }
        }
    }
    
    return pBuffer;
}

/*-----------------------------------------------------------*/

/**
 * @brief Frees the space previously allocated by calloc.
 *
 * @param[in] ptr Pointer to the memory to be freed.
 */
void mbedtls_platform_free(void *ptr) {
    if (ptr != NULL) {
        vPortFree(ptr);
    }
}

/*-----------------------------------------------------------*/

#if defined( MBEDTLS_THREADING_C )

/**
 * @brief Creates a mutex.
 *
 * @param[in, out] pMutex mbedtls mutex handle.
 */
static void mbedtls_platform_mutex_init(mbedtls_threading_mutex_t *pMutex) {
    configASSERT(pMutex != NULL);

#if(configSUPPORT_STATIC_ALLOCATION == 1)
    /* Create a statically-allocated FreeRTOS mutex. This should never fail as
    * storage is provided. */
    
    pMutex->mutexHandle = xSemaphoreCreateMutexStatic(&(pMutex->mutexStorage));
#elif(configSUPPORT_DYNAMIC_ALLOCATION == 1)
    pMutex->mutexHandle = xSemaphoreCreateMutex();
#endif
    
    configASSERT(pMutex->mutexHandle != NULL);
}

/*-----------------------------------------------------------*/

/**
 * @brief Frees a mutex.
 *
 * @param[in] pMutex mbedtls mutex handle.
 *
 * @note This function is an empty stub as nothing needs to be done to free
 * a statically allocated FreeRTOS mutex.
 */
static void mbedtls_platform_mutex_free(mbedtls_threading_mutex_t *pMutex) {
    vSemaphoreDelete(pMutex->mutexHandle);
    pMutex->mutexHandle = NULL;
}

/*-----------------------------------------------------------*/

/**
 * @brief Function to lock a mutex.
 *
 * @param[in] pMutex mbedtls mutex handle.
 *
 * @return 0 (success) is always returned as any other failure is asserted.
 */
static int mbedtls_platform_mutex_lock(mbedtls_threading_mutex_t *pMutex) {
    BaseType_t mutexStatus = 0;
    
    configASSERT(pMutex != NULL);
    configASSERT(pMutex->mutexHandle != NULL);
    
    /* mutexStatus is not used if asserts are disabled. */
    (void)mutexStatus;
    
    /* This function should never fail if the mutex is initialized. */
    mutexStatus = xSemaphoreTake(pMutex->mutexHandle, portMAX_DELAY);
    
    configASSERT(mutexStatus == pdTRUE);
    
    return 0;
}

/*-----------------------------------------------------------*/

/**
 * @brief Function to unlock a mutex.
 *
 * @param[in] pMutex mbedtls mutex handle.
 *
 * @return 0 is always returned as any other failure is asserted.
 */
static int mbedtls_platform_mutex_unlock(mbedtls_threading_mutex_t *pMutex) {
    BaseType_t mutexStatus = 0;
    
    configASSERT(pMutex != NULL);
    configASSERT(pMutex->mutexHandle != NULL);
    /* mutexStatus is not used if asserts are disabled. */
    (void)mutexStatus;
    
    /* This function should never fail if the mutex is initialized. */
    mutexStatus = xSemaphoreGive(pMutex->mutexHandle);
    configASSERT(mutexStatus == pdTRUE);
    
    return 0;
}

/*-----------------------------------------------------------*/

#if defined( MBEDTLS_THREADING_ALT )

int mbedtls_platform_threading_init(void) {
    mbedtls_threading_set_alt(mbedtls_platform_mutex_init,
                              mbedtls_platform_mutex_free,
                              mbedtls_platform_mutex_lock,
                              mbedtls_platform_mutex_unlock);
    return 0;
}

#else /* !MBEDTLS_THREADING_ALT */

void (* mbedtls_mutex_init)( mbedtls_threading_mutex_t * mutex ) = mbedtls_platform_mutex_init;
void (* mbedtls_mutex_free)( mbedtls_threading_mutex_t * mutex ) = mbedtls_platform_mutex_free;
int (* mbedtls_mutex_lock)( mbedtls_threading_mutex_t * mutex ) = mbedtls_platform_mutex_lock;
int (* mbedtls_mutex_unlock)( mbedtls_threading_mutex_t * mutex ) = mbedtls_platform_mutex_unlock;

#endif /* !MBEDTLS_THREADING_ALT */

#endif /* MBEDTLS_THREADING_C */
/*-----------------------------------------------------------*/


/**
 * @brief Function to generate a random number.
 *
 * @param[in] data Callback context.
 * @param[out] output The address of the buffer that receives the random number.
 * @param[in] len Maximum size of the random number to be generated.
 * @param[out] olen The size, in bytes, of the #output buffer.
 *
 * @return 0 if no critical failures occurred,
 * MBEDTLS_ERR_ENTROPY_SOURCE_FAILED otherwise.
 */
int mbedtls_platform_entropy_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
    configASSERT(output != NULL);
    configASSERT(olen != NULL);
    
    /* Context is not used by this function. */
    (void)data;
    
    /* TLS requires a secure random number generator; use the RNG provided
     * by Windows. This function MUST be re-implemented for other platforms. */
    
    for (size_t bytes = 0; bytes < len; bytes++) {
        output[bytes] = (unsigned char)(bsp_rng_read() & 0xffUL); /* Note a secure method - replace _RB_ */
    }
    *olen = len;
    
    return 0;
}

/*-----------------------------------------------------------*/

mbedtls_time_t mbedtls_platform_time(mbedtls_time_t *timer) {
    mbedtls_time_t now;
    if (timer == NULL) {
        now = rtc_time_get();
    } else {
        now = *timer;
        rtc_time_set(now);
    }
    return now;
}

/*-----------------------------------------------------------*/

/**
 * \brief      Platform-specific implementation of gmtime_r()
 *
 *             The function is a thread-safe abstraction that behaves
 *             similarly to the gmtime_r() function from Unix/POSIX.
 *
 *             Mbed TLS will try to identify the underlying platform and
 *             make use of an appropriate underlying implementation (e.g.
 *             gmtime_r() for POSIX and gmtime_s() for Windows). If this is
 *             not possible, then gmtime() will be used. In this case, calls
 *             from the library to gmtime() will be guarded by the mutex
 *             mbedtls_threading_gmtime_mutex if MBEDTLS_THREADING_C is
 *             enabled. It is recommended that calls from outside the library
 *             are also guarded by this mutex.
 *
 *             If MBEDTLS_PLATFORM_GMTIME_R_ALT is defined, then Mbed TLS will
 *             unconditionally use the alternative implementation for
 *             mbedtls_platform_gmtime_r() supplied by the user at compile time.
 *
 * \param tt     Pointer to an object containing time (in seconds) since the
 *               epoch to be converted
 * \param tm_buf Pointer to an object where the results will be stored
 *
 * \return      Pointer to an object of type struct tm on success, otherwise
 *              NULL
 */
struct tm *mbedtls_platform_gmtime_r(const mbedtls_time_t *tt,
                                     struct tm *tm_buf) {
    return localtime_r(tt, tm_buf);
}

/*-----------------------------------------------------------*/

void mbedtls_platform_init(void) {
    /* Set calloc free func */
    mbedtls_platform_set_calloc_free(mbedtls_platform_calloc, mbedtls_platform_free);
    
    /* Set time function */
    mbedtls_platform_set_time(mbedtls_platform_time);

#if defined( MBEDTLS_THREADING_ALT )
    /* Set the mutex functions for mbed TLS thread safety. */
    mbedtls_platform_threading_init();
#endif
}

/*-----------------------------------------------------------*/

void mbedtls_platform_debug(void *ctx, int level, const char *file, int line,
                            const char *str) {
    const char *p, *basename;
    (void)ctx;
    
    /* Extract basename from file */
    for (p = basename = file; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            basename = p + 1;
        }
    }
    
    mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
}

/*-----------------------------------------------------------*/

