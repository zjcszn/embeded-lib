CLRC663
=======

This is a library for NXP's [CLRC663][nxp_CLRC663] NFC and reader IC. Master mcu can be any.

Design
------
The library itself is written in C, it requires the chip to be connected to the SPI, UART, i2c interface, furthermore, it can only
deal with a single chip that's connected to the microcontroller. The library does *not* require time or delay functions
neither does it need an interrupt pin, all that is required is the SPI, UART, i2c interface. The handling of timeouts is done by using
one of the chip's timers.

The library is clearly structured, providing functionality from low level to higher level:

* Register interaction: The adresses for all registers are provided as define statements, functions for FIFO reading and writing are available, as well as reading and writing normal registers. For various important registers there are additional define statements available that make it easier to interact with these.

* Commands: The chip can be instructed to execute various commands, these functions provide wrappers for these and handle the arguments required for each command.

* ISO14443A: Provides the necessities from ISO14443a to interact with RFID tags: the REQA, WUPA and SELECT procedure (with collision handling) to determine the UID(s).

* MIFARE: Provides functions to authenticate, read and write blocks on MIFARE cards.

Documentation
-------------

License
-------
MIT License, see LICENSE.txt.

Copyright (c) 2021 Liamaev Mihkail (kenny5660)

Some parts of the code are taken from repo [iwanders/MFRC630][repo_iwanders/MFRC630]


[nxp_CLRC663]: https://www.nxp.com/products/rfid-nfc/nfc-hf/nfc-readers/clrc663-iplus-i-family-high-performance-nfc-frontends:CLRC66303HN
[repo_iwanders/MFRC630]: https://github.com/iwanders/MFRC630
html
