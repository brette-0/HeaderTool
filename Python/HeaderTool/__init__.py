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

    if not exists("./output"): mkdir("output")  # create output dir if not present
    with open(f"./output/{header[16:].decode('utf-8')}", "wb") as f:
        if header[:16] != b"\x00"*16:
            f.write(header[:16] + rom)              # write renamed headered ROM with no-intro good name
        
main(argv)                                      #Support drag and drop