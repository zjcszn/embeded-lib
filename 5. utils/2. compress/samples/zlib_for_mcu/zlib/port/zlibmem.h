//
// Created by zjcszn on 2023/7/10.
//

#ifndef USB_DEMO_USER_LIBRARY_ZLIB_PORT_ZLIB_PORT_H_
#define USB_DEMO_USER_LIBRARY_ZLIB_PORT_ZLIB_PORT_H_

#include "zlib.h"

voidpf zlib_alloc(voidpf opaque, uInt items, uInt size);

void zlib_free(voidpf opaque, voidpf address);

#endif //USB_DEMO_USER_LIBRARY_ZLIB_PORT_ZLIB_PORT_H_
