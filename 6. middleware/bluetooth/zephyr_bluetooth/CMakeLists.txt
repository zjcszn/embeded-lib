add_library(zephyr_bluetooth STATIC)

set(CONFIG_BT_BROADCASTER 1)
set(CONFIG_BT_OBSERVER 1)
set(CONFIG_BT_HOST_CRYPTO 1)
set(CONFIG_BT_CONN 1)
set(CONFIG_BT_ECC 1)
set(CONFIG_BT_SMP 1)
set(CONFIG_BT_HOST_CCM 1)
set(CONFIG_BT_RPA 1)

# set(CONFIG_BT_BREDR 1)
# set(CONFIG_BT_RFCOMM 1)
# set(CONFIG_BT_HFP_HF 1)

set(CONFIG_BT_DIS 1)
set(CONFIG_BT_BAS 1)
set(CONFIG_BT_HRS 1)
set(CONFIG_BT_TPS 1)
# set(CONFIG_BT_OTS 1)

target_include_directories(zephyr_bluetooth PUBLIC
core/include 
core/src 
core/src/tinycrypt/include
osal/freertos 
drivers
.
)

target_sources(zephyr_bluetooth PRIVATE
osal/freertos/k_os.c
osal/freertos/k_queue.c
osal/freertos/k_work_q.c
osal/freertos/k_mem_slab.c
osal/freertos/k_poll.c
)

# tinycrypt, including all sources
target_sources(zephyr_bluetooth PRIVATE
core/src/tinycrypt/source/aes_decrypt.c
core/src/tinycrypt/source/aes_encrypt.c
core/src/tinycrypt/source/cbc_mode.c
core/src/tinycrypt/source/ctr_mode.c
core/src/tinycrypt/source/ccm_mode.c
core/src/tinycrypt/source/cmac_mode.c
core/src/tinycrypt/source/ctr_prng.c
core/src/tinycrypt/source/ecc_dh.c
core/src/tinycrypt/source/ecc.c
core/src/tinycrypt/source/ecc_dsa.c
core/src/tinycrypt/source/hmac.c
core/src/tinycrypt/source/hmac_prng.c
core/src/tinycrypt/source/sha256.c
core/src/tinycrypt/source/utils.c
)

target_sources(zephyr_bluetooth PRIVATE
core/src/utils/buf.c
core/src/utils/atomic_c.c
)

# common
target_sources(zephyr_bluetooth PRIVATE
core/src/common/log.c
# core/src/common/dummy.c
)

if(CONFIG_BT_RPA)
target_sources(zephyr_bluetooth PRIVATE
core/src/common/rpa.c
)
endif()

# host
if(CONFIG_BT_ECC)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/hci_ecc.c
)
endif()

if(CONFIG_BT_A2DP)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/a2dp.c
)
endif()

if(CONFIG_BT_AVDTP)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/avdtp.c
)
endif()

if(CONFIG_BT_RFCOMM)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/rfcomm.c
)
endif()

if(CONFIG_BT_SETTINGS)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/settings.c
)
endif()

if(CONFIG_BT_HOST_CCM)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/aes_ccm.c
)
endif()

if(CONFIG_BT_BREDR)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/br.c
core/src/host/keys_br.c
core/src/host/l2cap_br.c
core/src/host/sdp.c
core/src/host/ssp.c
)
endif()

if(CONFIG_BT_HFP_HF)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/hfp_hf.c
core/src/host/at.c
)
endif()

target_sources(zephyr_bluetooth PRIVATE
core/src/host/uuid.c
core/src/host/addr.c
core/src/host/buf.c
core/src/host/hci_core.c
core/src/host/hci_common.c
core/src/host/id.c
)

if(CONFIG_BT_BROADCASTER)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/adv.c
)
endif()

if(CONFIG_BT_OBSERVER)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/scan.c
)
endif()

if(CONFIG_BT_HOST_CRYPTO)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/crypto.c
)
endif()

if(CONFIG_BT_ECC)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/ecc.c
)
endif()

if(CONFIG_BT_CONN)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/conn.c
core/src/host/l2cap.c
core/src/host/att.c
core/src/host/gatt.c
)
endif()

if(CONFIG_BT_SMP)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/smp.c
core/src/host/keys.c
)
else()
target_sources(zephyr_bluetooth PRIVATE
core/src/host/smp_null.c
)
endif()

if(CONFIG_BT_ISO)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/iso.c
core/src/host/conn.c
)
endif()

if(CONFIG_BT_DF)
target_sources(zephyr_bluetooth PRIVATE
core/src/host/direction.c
)
endif()

if(CONFIG_BT_DIS)
target_sources(zephyr_bluetooth PRIVATE
core/src/services/dis.c
)
endif()

if(CONFIG_BT_BAS)
target_sources(zephyr_bluetooth PRIVATE
core/src/services/bas.c
)
endif()

if(CONFIG_BT_HRS)
target_sources(zephyr_bluetooth PRIVATE
core/src/services/hrs.c
)
endif()

if(CONFIG_BT_TPS)
target_sources(zephyr_bluetooth PRIVATE
core/src/services/tps.c
)
endif()

if(CONFIG_BT_OTS)
target_sources(zephyr_bluetooth PRIVATE
core/src/services/ots/ots.c
core/src/services/ots/ots_l2cap.c
core/src/services/ots/ots_obj_manager.c
core/src/services/ots/ots_oacp.c
core/src/services/ots/ots_olcp.c
)
endif()

if(CONFIG_BT_OTS_DIR_LIST_OBJ)
target_sources(zephyr_bluetooth PRIVATE
core/src/services/ots_dir_list.c
)
endif()

target_sources(zephyr_bluetooth PRIVATE zephyr_init.c zephyr_demo_init.c)
