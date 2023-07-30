import os
from zlib import crc32
for file in os.listdir("./input"):
    with open(f"./input/{file}", "rb") as f:
        payload = f.read() 
    with open(f"./headers/{crc32(payload[16:])}", "wb") as f:
        if len(payload) & 0x10 == 16: 
            f.write(payload[:16]+bytes(file,"utf-8")) 
        else:
            f.write((b"\x00"*16)+bytes(f"No header for : {file}","utf-8"))