This is the API for GNSS.

It also contains two Python scripts (run each with `-h` for help):
- [u_gnss_ucenter_ubx.py](u_gnss_ucenter_ubx.py): you can give this script the log output from `ubxlib` and it will find the UBX messages in it and write them to a file (or on Linux a PTY, or on Linux and Windows a serial port) which you can be opened by the u-blox [uCenter tool](https://www.u-blox.com/en/product/u-center).  This script requires the Python module `pyserial` to be installed, which can be done with `pip3 -r ./requirements.txt`.  If you want to run a "live" streamed connection to uCenter, a Windows tool, you will need to install something like `com0com` or  Eltima's [Virtual Serial Port](https://www.eltima.com/products/vspdxp) on Windows to create a pair of looped-back serial ports; if you have, say `ubxlib` log output coming in on `COM1`, and `COM9`/`COM10` are your pair of looped-back serial ports, you would do `python u_gnss_ucenter_ubx.py COM1 COM9` and connect uCenter to `COM10`.
- [u_gnss_cfg_val_key.py](u_gnss_cfg_val_key.py): this script should be executed if the enums in [u_gnss_cfg_val_key.h](u_gnss_cfg_val_key.h) have been updated; it will re-write the header file to include a set of key ID macros that can be used by the application.