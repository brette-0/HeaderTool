from zlib import crc32 
from sys import exit, argv
from pathlib import PurePosixPath
from os.path import exists, isdir
from os import listdir, mkdir
import urllib.request
import json

preferlocal = noskip = defaultoNES = specified = clean = verbose = False
rename = headerrom = defaulttoNES = True

class HeaderTool:
    def main(*argv):
        if argv[1] in {"-h", "--help"}:
            print("")#help message 
            return
        
        elif argv[1] in {"-g", "--get"}:
            HeaderTool.getdb()
            return

        argo = 1
        for arg in argv:

            if not argv[0].startswith("-"): break 
            argo += 1

            if argv in {"-v", "--verbose"}:
                global verbose
                verbose = True
                continue

            elif argv in {"-l", "--local"}:
                global preferlocal 
                preferlocal = True 
                continue

            elif argv in {"-o", "--output"}:
                global specified
                specified = True
                continue

            elif argv in {"-ns", "--noskip"}:
                global noskip 
                noskip = True 
                continue

            elif argv in {"-c", "--clean"}:
                if not headerrom:
                    print("Cannot clean header if preserving")
                    return
                
                elif not rename:
                    print("No operation")
                    return 
                
                global clean 
                clean = True 
                continue 

            elif argv in {"-nh", "--noheader"}:
                if clean:
                    print("Cannot clean header if preserving")
                    return 
                
                elif not rename: 
                    print("No operation")
                    return

                global headerrom 
                headerrom = False 
                continue

            elif argv in {"-nr", "--norename"}:
                if not headerrom:
                    print("No Operation")
                    return 
                
                global rename 
                rename = False
                continue
            
            else: 
                print("Unknown Command")
                return

        if argo + specified >= len(argv):
            print("No Input specified")
            return 
        
        output = (argv[argo] if exists(argv[argo]) else "./output") if specified else "./output"
        for arg in argv[argo + specified:]:
            HeaderTool.romheader(PurePosixPath(arg), PurePosixPath(output))
            
    def romheader(arg : PurePosixPath, output : PurePosixPath): 
        global noskip, header, clean, rename, verbose, defaulttoNES
        if isdir(arg):
            for path in listdir(arg):
                HeaderTool.romheader(f"{arg}/{path}", f"{output}/{arg.name}")
        elif arg.name.endswith(".nes") or (noskip and defaulttoNES):
            with open(arg, "rb") as f:
                rom = f.read()

            if headerrom or clean:
                rom = rom[len(rom) & 0x18:]
                if not clean:
                    header = HeaderTool.getheader(crc32(rom))
                    rom = header[:16]+rom 
                    print("Headerd ROM")
            
            goodname = str(header[16:]) if rename else arg.name
            with open(f"{output}/{arg.name}", "wb") as f:
                f.write(rom)

    def getdb(self): 
        try:
            index = urllib.request.urlopen("path/to/json").response.read()
            index = json.loads(index)

            for checksum in index:
                try:
                    with open(f"./headers/{checksum}", "wb") as f:
                        f.write(HeaderTool.downloadheader(checksum))

                except Exception as e:
                    print(f"Error downloading file: {e}")

        except Exception as e:
            print(f"Error downloading file: {e}")

    def getheader(checksum : int | str):
        global nonet
        if preferlocal or nonet:
            header = HeaderTool.fromlocalFS(checksum)
            if not header:
                if not nonet:
                    header = HeaderTool.downloadheader(checksum)
                    if not header:
                        print("Could not locate header for this ROM")
                        return 
                    else: return header
                else:
                    print("Could not locate header for this ROM")
                    return
            else: return header
        else:
            header = HeaderTool.downloadheader(checksum)
            if not header:
                if exists("./headers"):
                    header = HeaderTool.fromlocalFS(checksum)
                    if not header:
                        print("Could not locate header for this ROM")
                        return 
                    else: return header
                else:
                    print("Could not locate header for this ROM")
                    return
            else: return header

    def downloadheader(checksum : int | str) -> None:
        try:
            return urllib.request.urlopen(f"path/{checksum}").response.read()
        except Exception as e:
            print(f"Error downloading file: {e}")
        return None
    
    def fromlocalFS(checksum : int | str()):
        if not exists("./headers"): 
            print("Could not find local database")
            return 
        
        if not exists(f"./headers/{checksum}"):
            print("No header located for this file")
            return 
        
        with open(f"./headers/{checksum}", "rb") as f:
            return f.read()
        

if __name__ == "__main__": HeaderTool.main(*argv)