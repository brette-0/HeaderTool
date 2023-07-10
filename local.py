#assume ./headers stores [checksum].bin (16 bytes)
#use github repo file system to access header (use httplib)

#make cpp executable
#make cpp gui


from zlib import crc32
from sys import argv
from os import listdir

if __name__ == "__main__":
    if len(argv) <2 or argv[1]=='':
        print("no file specified.")
        exit()

    with open(argv[1], "rb") as f:
        rom = f.read()
        if len(rom)&0x10:
            rom = rom[16:]
            
        checksum = hex(crc32(rom))
        if checksum not in listdir("./headers"):
            print("ROM is unrecognized, likely a bad dump.")
            exit()

    with open(f"./headers/{checksum}.bin") as f:
        payload = f.read()
        with open((payload[16:]).decode("utf-8"), "wb") as f:
            f.write(payload[:16]+rom)

    
