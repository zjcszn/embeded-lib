import argparse
import os
import struct
import time
import crcmod

from Crypto.Signature import pss
from Crypto.Hash import SHA256
from Crypto.PublicKey import RSA

parser = argparse.ArgumentParser(description="used for test")
parser.add_argument('--kpath', '-k', required=True, help=('Rsa private key (PEM format) path.'), type=str)
parser.add_argument('--fpath', '-f', required=True, help=('Firmware path.'), type=str)
args = parser.parse_args()

crc32 = crcmod.predefined.Crc('crc32')
ftype = b"mbl"
ctime = os.path.getctime(args.fpath).__int__()
fname = os.path.basename(args.fpath)
ppath = os.path.splitext(args.fpath)[0] + '.mbl'
raw_size = os.path.getsize(args.fpath)

with open(args.kpath, "rb") as rsa_skey:
    skey = RSA.importKey(rsa_skey.read())
with open(args.fpath, "rb") as firmware:
    fhash = SHA256.new(firmware.read())
    fsign = pss.new(skey).sign(fhash)
    falgo = 0x11
    foffset = 256
    firmware.seek(32)
    fversion = int(firmware.read(4).hex(), 16)
    com_size = raw_size
    aes_iv = bytes(16)

    fhead = struct.pack("4sLLL16s128sLLL", ftype, ctime, fversion, falgo,\
                         aes_iv, fsign, raw_size, com_size, foffset)
    crc32.update(fhead)

    print(fhash.hexdigest())
    print(fsign.hex())
    print(crc32.hexdigest())
    padding = bytes(80)
    fcrc = struct.pack("L80s", int(crc32.hexdigest(), 16), padding)
    print(len(fhead)+len(fcrc))
    with open(ppath, "wb") as package:
        package.write(fhead)
        package.write(fcrc)
        firmware.seek(0)
        package.write(firmware.read())



