#ifndef __BOOT_CMD_H__
#define __BOOT_CMD_H__

void system_reset_to_app(void);
void fs_mount(void);
void fs_unmount(void);
void update_by_sdcard(const char *fpath);


#endif
