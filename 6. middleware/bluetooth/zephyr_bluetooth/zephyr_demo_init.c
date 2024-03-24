#include <zephyr.h>

#define LOG_MODULE_NAME demo
#include <common/log.h>

struct bluetooth_demo {
    const char *name;
    void (*init)(void);
};

extern void demo_beacon(void);
extern void demo_central(void);
extern void demo_central_hr(void);
extern void demo_central_ht(void);
extern void demo_central_iso(void);
extern void demo_central_multilink(void);
extern void demo_central_past(void);
extern void demo_direction_finding_connectionless_rx(void);
extern void demo_direction_finding_connectionless_tx(void);
extern void demo_eddystone(void);
extern void demo_handsfree(void);
extern void demo_hci_pwr_ctrl(void);
extern void demo_ibeacon(void);
extern void demo_iso_broadcast(void);
extern void demo_iso_broadcast_benchmark(void);
extern void demo_iso_connected_benchmark(void);
extern void demo_iso_receive(void);
extern void demo_mesh(void);
extern void demo_mesh_demo(void);
extern void demo_mesh_provisioner(void);
extern void demo_periodic_adv(void);
extern void demo_periodic_sync(void);
extern void demo_peripheral(void);
extern void demo_peripheral_csc(void);
extern void demo_peripheral_dis(void);
extern void demo_peripheral_esp(void);
extern void demo_peripheral_hids(void);
extern void demo_peripheral_hr(void);
extern void demo_peripheral_ht(void);
extern void demo_peripheral_identity(void);
extern void demo_peripheral_iso(void);
extern void demo_peripheral_past(void);
extern void demo_peripheral_sc_only(void);
extern void demo_scan_adv(void);
extern void demo_st_ble_sensor(void);

struct bluetooth_demo demo_table[] = {
    { .name = "beacon",
      .init = demo_beacon },
    { .name = "central",
      .init = demo_central },
    { .name = "central_hr",
      .init = demo_central_hr },
    { .name = "central_ht",
      .init = demo_central_ht },
    { .name = "central_iso",
      .init = NULL },
    { .name = "central_multilink",
      .init = demo_central_multilink },
    { .name = "central_past",
      .init = NULL },
    { .name = "direction_finding_connectionless_rx",
      .init = NULL },
    { .name = "direction_finding_connectionless_tx",
      .init = NULL },
    { .name = "eddystone",
      .init = NULL },
    { .name = "handsfree",
      .init = NULL },
    { .name = "hci_pwr_ctrl",
      .init = NULL },
    { .name = "ibeacon",
      .init = demo_ibeacon },
    { .name = "iso_broadcast",
      .init = NULL },
    { .name = "iso_broadcast_benchmark",
      .init = NULL },
    { .name = "iso_connected_benchmark",
      .init = NULL },
    { .name = "iso_receive",
      .init = NULL },
    { .name = "mesh",
      .init = NULL },
    { .name = "mesh_demo",
      .init = NULL },
    { .name = "mesh_provisioner",
      .init = NULL },
    { .name = "periodic_adv",
      .init = NULL },
    { .name = "periodic_sync",
      .init = NULL },
    { .name = "peripheral",
      .init = demo_peripheral },
    { .name = "peripheral_csc",
      .init = demo_peripheral_csc },
    { .name = "peripheral_dis",
      .init = demo_peripheral_dis },
    { .name = "peripheral_esp",
      .init = demo_peripheral_esp },
    { .name = "peripheral_hids",
      .init = demo_peripheral_hids },
    { .name = "peripheral_hr",
      .init = demo_peripheral_hr },
    { .name = "peripheral_ht",
      .init = NULL },
    { .name = "peripheral_identity",
      .init = demo_peripheral_identity },
    { .name = "peripheral_iso",
      .init = NULL },
    { .name = "peripheral_ots",
      .init = NULL },
    { .name = "peripheral_past",
      .init = NULL },
    { .name = "peripheral_sc_only",
      .init = demo_peripheral_sc_only },
    { .name = "scan_adv",
      .init = demo_scan_adv },
    { .name = "st_ble_sensor",
      .init = NULL },
};

void zephyr_bluetooth_thread_entry(void *parameter)
{
    const char *name = (const char *)parameter;

    printk("select bluetooth demo %s\r\n", name);

    for (uint32_t i = 0; i < sizeof(demo_table) / sizeof(struct bluetooth_demo); i++) {
        if (strncmp(demo_table[i].name, name, 32) == 0) {
            if (demo_table[i].init) {
                demo_table[i].init();
            } else {
                BT_ERR("do not support this demo");
            }
        }
    }

    k_thread_abort(NULL);
}

k_thread_stack_t ble_demo_stack[2048];

int bluetooth_demo_init(int argc, char **argv)
{
    const char *name;

    if (argc != 2) {
        BT_ERR("please select bluetooth demo");
        for (uint32_t i = 0; i < sizeof(demo_table) / sizeof(struct bluetooth_demo); i++) {
            printk("%s\r\n", demo_table[i].name);
        }
        return 0;
    }
    extern void zephyr_init(void);
    zephyr_init();

    name = argv[1];

    k_thread_create("bledemo", ble_demo_stack, sizeof(ble_demo_stack), zephyr_bluetooth_thread_entry, (void *)name, 10);

    return 0;
}
