from zlib import crc32
from sys import argv, exit
from urllib.request import urlopen
from urllib.error import HTTPError
from os.path import exists, isdir
from os import mkdir, system, listdir
import argparse

if __name__ != "__main__": raise Exception("Not Supported!")

jobs = 0                                        # global number of jobs
def recursedir(path : str)-> int:
    """
        Function to recurse through a given folder with depth limit limit depth
    """
    
    global jobs
    errors = 0                                  # intialize error count
    for subpath in listdir(path):               # loop list
        if isdir(f"./{path}/{subpath}"):        # recurse and header roms in subdirectory
            errors += recursedir(f"./{path}/{subpath}")
        else:                                   # elsewise header current ROM
            errors += getheader(f"./{path}/{subpath}")
            jobs += 1
    return errors                               # return error count

def getheader(arg):
    with open(arg, "rb") as f:                  # Arg2 access
        rom = f.read()
        if len(rom)&0x10: rom = rom[16:]        # remove header if present
                
        checksum = crc32(rom)                   # calculate headerless ROm

    try: header = urlopen(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}")
    except HTTPError: print("Failed to reteive header information"); return 1
    header = header.read()

    if not exists("./output"): mkdir("output")  # create output dir if not present
    with open(f"./output/{header[16:].decode('utf-8')}", "wb") as f:
        if header[:16] != b"\x00"*16:
            f.write(header[:16] + rom)          # write renamed headered ROM with no-intro good name
    return 0

def main(argv):
    global jobs
    if len(argv) <2 or argv[1]=='':             # Arg validation
        print("no file specified.")
        exit()
    errors = 0
    for arg in argv[1:]:
        if isdir(arg): errors += recursedir(arg)
        else: getheader(arg); jobs += 1
    print(f"Headering finished with {errors} fails out of {jobs}.")
    system("pause")
        
        
main(argv)                                      #Support drag and drop