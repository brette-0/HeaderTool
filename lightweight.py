from zlib import crc32
from sys import argv, exit
import requests
from os.path import exists
from os import mkdir

if __name__ == "__main__":
    if len(argv) <2 or argv[1]=='':         # Arg validation
        print("no file specified.")
        exit()

    with open(argv[1], "rb") as f:          # Arg2 access
        rom = f.read()
        if len(rom)&0x10: rom = rom[16:]    # remove header if present
            
        checksum = hex(crc32(rom))          # calculate headerless ROm

    header = requests.get(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}.bin")
    if header.status_code != 200:           # retrieve file, and if succesful
        print("No header for this ROM.")
        exit()
    
    headers = [header,]                     # setup multi-header
    addend = 1                              # scan for duplicates
    while not headers[-1] is None:          # until EOF declared
        headers.append(requests.get(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}_{addend}.bin"))
        if headers[-1].status_code != 200: headers[-1] = None   # wait until 404 (or other)
        addend += 1                                             # increment addend for search
    headers = headers[:-1]                                      # trim EOF

    if len(headers)-1:                                          # if multiple headers
        print(*[f"\n{n+1}: {h.content[16:]}" for n, h in enumerate(iter(headers))])
        choice = None                       # allow user to select one
        while choice is None:
            choice = input("Select which header to use: ")
            try: choice = int(choice-1) 
            except: choice = None

            if choice < 0: choice = None
            header = headers[choice]
    header = header.content

    if not exists("./output"): mkdir("output")                  # create output dir if not present
        
    with open(f"./output/{header[16:].decode('utf-8')}", "wb") as f:
        f.write(header[:16] + rom)          # write renamed headered ROM with no-intro good name
        
else: raise Exception("No support")