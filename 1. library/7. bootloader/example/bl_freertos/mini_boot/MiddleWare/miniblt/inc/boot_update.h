#ifndef __BOOT_UPDATE_H__
#define __BOOT_UPDATE_H__

#include <stdbool.h>
#include <stdint.h>


void firmware_update(void);
bool firmware_update_on_flash(void);
bool firmware_update_on_file(const char *fpath, const char *part_name);



#endif