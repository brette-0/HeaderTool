from zlib import crc32
from sys import argv
import requests
from os.path import exists
from os import mkdir

if __name__ == "__main__":
    if len(argv) <2 or argv[1]=='':
        print("no file specified.")
        exit()

    with open(argv[1], "rb") as f:
        rom = f.read()
        if len(rom)&0x10:
            rom = rom[16:]
            
        checksum = hex(crc32(rom))

    header = requests.get(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}.bin")
    if header.status_code != 200:
        print("No header for this ROM.")
        exit()
    
    headers = [header,]
    addend = 1
    while not headers[-1] is None:
        headers.append(requests.get(f"https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/{checksum}_{addend}.bin"))
        if headers[-1].status_code != 200: headers[-1] = None 
        addend += 1
    headers = headers[:-1]

    if len(headers)-1:
        print(*[f"\n{n+1}: {h.content[16:]}" for n, h in enumerate(iter(headers))])
        choice = None 
        while choice is None:
            choice = input("Select which header to use: ")
            try: choice = int(choice-1) 
            except: choice = None

            if choice < 0: choice = None
            header = headers[choice]
    header = header.content

    if not exists("./output"): mkdir("output")
        
    with open(f"./output/{header[16:].decode('utf-8')}", "wb") as f:
        f.write(header[:16] + rom)
        
