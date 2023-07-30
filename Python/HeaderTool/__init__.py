from zlib import crc32
from sys import argv, exit
import urllib.request
from os.path import exists
from os import mkdir
import argparse

if __name__ != "__main__": raise Exception("Not Supported!")

def main(argv):
    if len(argv) <2 or argv[1]=='':             # Arg validation
        print("no file specified.")
        exit()

    with open(argv[1], "rb") as f:              # Arg2 access
        rom = f.read()
        if len(rom)&0x10: rom = rom[16:]        # remove header if present
            
        checksum = crc32(rom)                   # calculate headerless ROm

    header = urllib.request.urlopen(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}")
    if header.status != 200:                    # retrieve file, and if succesful
        print("No header for this ROM.")
        exit()
    
    header = header.read()
    if header[0] == 0xff: print("This ROM has no valid header, however the ROM raises no checksum errors.")
    elif header[0] != 0x00: print("Currently there is no added support for multi-header ROMs.")
    else:
        if not exists("./output"): mkdir("output")  # create output dir if not present
        with open(f"./output/{header[17:].decode('utf-8')}", "wb") as f:
            f.write(header[1:17] + rom)              # write renamed headered ROM with no-intro good name
        
main(argv)                                      #Support drag and drop