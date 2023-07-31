from zlib import crc32
from sys import argv, exit
import urllib.request
from os.path import exists, isdir
from os import mkdir, system, listdir
import argparse

if __name__ != "__main__": raise Exception("Not Supported!")

jobs = 0
def recursedir(argv : str)-> int:
    """
        Function to recurse through a given folder with depth limit limit depth
    """
    
    global jobs
    errors = 0                                  # intialize error count
    for arg in listdir(argv):                   # loop list
        if isdir(f"./{argv}/{arg}"):                          # recurse and header roms in subdirectory
            errors += recursedir(f"./{argv}/{arg}")
        else:                                   # elsewise header current ROM
            errors += getheader(f"./{argv}/{arg}")
            jobs += 1
    return errors                               # return error count

def getheader(arg):
    with open(arg, "rb") as f:              # Arg2 access
        rom = f.read()
        if len(rom)&0x10: rom = rom[16:]        # remove header if present
                
        checksum = crc32(rom)                   # calculate headerless ROm

    try: header = urllib.request.urlopen(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}")
    except urllib.error.HTTPError: print("Failed to reteive header information"); return 1
    header = header.read()

    if not exists("./output"): mkdir("output")  # create output dir if not present
    with open(f"./output/{header[16:].decode('utf-8')}", "wb") as f:
        if header[:16] != b"\x00"*16:
            f.write(header[:16] + rom)              # write renamed headered ROM with no-intro good name
    return 0

def main(argv):
    global jobs
    if len(argv) <2 or argv[1]=='':                 # Arg validation
        print("no file specified.")
        exit()
    errors = 0
    for arg in argv[1:]:
        errors += recursedir(arg)
    print(f"Headering finished with {errors} fails out of {jobs}.")
    system("pause")
        
        
main(argv)                                      #Support drag and drop