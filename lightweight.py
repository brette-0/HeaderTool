from zlib import crc32
from sys import argv
from urllib.request import urlretrieve

if __name__ == "__main__":
    if len(argv) <2 or argv[1]=='':
        print("no file specified.")
        exit()

    with open(argv[1], "rb") as f:
        rom = f.read()
        if len(rom)&0x10:
            rom = rom[16:]
            
        checksum = hex(crc32(rom))
	try:
		header = urlretrieve("https://github.com/BrettefromNesUniverse/HeaderTool/tree/main/headers", checksum)
		with open((header[16:]).decode("utf-8"), "wb") as f:
            		f.write(header[:16]+rom)
	except: 
		print("No header for this ROM.")
            	exit()
	        
