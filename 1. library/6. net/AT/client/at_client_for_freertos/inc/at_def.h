#ifndef __AT_DEF_H__
#define __AT_DEF_H__

#include <stdint.h>

/* boolean type definitions */
#define AT_TRUE                         1               /**< boolean true  */
#define AT_FALSE                        0               /**< boolean fails */


/* RT-Thread error code definitions */
#define AT_EOK                          0               /**< There is no error */
#define AT_ERROR                        1               /**< A generic error happens */
#define AT_ETIMEOUT                     2               /**< Timed out */
#define AT_EFULL                        3               /**< The resource is full */
#define AT_EEMPTY                       4               /**< The resource is empty */
#define AT_ENOMEM                       5               /**< No memory */
#define AT_ENOSYS                       6               /**< No system */
#define AT_EBUSY                        7               /**< Busy */
#define AT_EIO                          8               /**< IO error */
#define AT_EINTR                        9               /**< Interrupted system call */
#define AT_EINVAL                       10              /**< Invalid argument */
#define AT_ETRAP                        11              /**< Trap event */
#define AT_ENOENT                       12              /**< No entry */
#define AT_ENOSPC                       13              /**< No space left */



#endif