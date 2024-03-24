//
// Created by zjcszn on 2023/7/10.
//

#include "zlibmem.h"
#include "lwmem/lwmem.h"

/* zlib alloc & free port */
voidpf zlib_alloc(voidpf opaque, uInt items, uInt size) {
    (void)opaque;
    return lwmem_calloc(items, size);
}

void zlib_free(voidpf opaque, voidpf address) {
    (void)opaque;
    lwmem_free(address);
}
