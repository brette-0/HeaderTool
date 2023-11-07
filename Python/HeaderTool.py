from zlib import crc32 
from sys import argv
from pathlib import PurePosixPath
from os.path import exists, isdir
from os import listdir, mkdir
import urllib.request
import json

preferlocal = noskip = defaultoNES = specified = clean = verbose = False
rename = headerrom = defaulttoNES = True

class HeaderTool:
    @staticmethod
    def main(*argv):
        global preferlocal, noskip, defaultoNES, specified, clean, verbose, rename, headerrom, defaulttoNES
        
        if argv[1] in {"-h", "--help"}:
            print("")  # help message
            return
        
        elif argv[1] in {"-g", "--get"}:
            HeaderTool.getdb()
            return

        argo = 1
        for arg in argv:

            if not arg.startswith("-"): break 
            argo += 1

            if arg in {"-v", "--verbose"}:
                verbose = True
                continue

            elif arg in {"-l", "--local"}:
                preferlocal = True 
                continue

            elif arg in {"-o", "--output"}:
                specified = True
                continue

            elif arg in {"-ns", "--noskip"}:
                noskip = True 
                continue

            elif arg in {"-c", "--clean"}:
                if not headerrom:
                    print("Cannot clean header if preserving")
                    return
                
                elif not rename:
                    print("No operation")
                    return 
                
                clean = True 
                continue 

            elif arg in {"-nh", "--noheader"}:
                if clean:
                    print("Cannot clean header if preserving")
                    return 
                
                elif not rename: 
                    print("No operation")
                    return

                headerrom = False 
                continue

            elif arg in {"-nr", "--norename"}:
                if not headerrom:
                    print("No Operation")
                    return 
                
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
            
    @staticmethod
    def romheader(arg, output): 
        global noskip, headerrom, clean, rename, defaulttoNES
        if not exists(output): mkdir(output)
        if isdir(arg):
            for path in listdir(arg):
                HeaderTool.romheader(PurePosixPath(f"{arg}/{path}"), PurePosixPath(f"{output}/{arg.name}"))
        elif arg.name.endswith(".nes") or (noskip and defaulttoNES):
            with open(arg, "rb") as f:
                rom = f.read()

            if headerrom or clean:
                rom = rom[len(rom) & 0x18:]
                if not clean:
                    header = HeaderTool.getheader(crc32(rom))
                    rom = header[:16]+rom 
                    print("Headered ROM")
            
            goodname = str(header[16:]) if rename else arg.name
            with open(PurePosixPath(f"{output}/{goodname}"), "wb") as f:
                f.write(rom)

    @staticmethod
    def getdb(): 
        try:
            index = urllib.request.urlopen("path/to/json").read()
            index = json.loads(index)

            for checksum in index:
                try:
                    with open(f"./headers/{checksum}", "wb") as f:
                        f.write(HeaderTool.downloadheader(checksum))

                except Exception as e:
                    print(f"Error downloading file: {e}")

        except Exception as e:
            print(f"Error downloading file: {e}")

    @staticmethod
    def getheader(checksum):
        global nonet, header, rename, clean
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

    @staticmethod
    def downloadheader(checksum) -> None:
        try:
            return urllib.request.urlopen(f"path/{checksum}").read()
        except Exception as e:
            print(f"Error downloading file: {e}")
            return None
    
    @staticmethod
    def fromlocalFS(checksum):
        if not exists("./headers"): 
            print("Could not find local database")
            return 
        
        if not exists(f"./headers/{checksum}"):
            print("No header located for this file")
            return 
        
        with open(f"./headers/{checksum}", "rb") as f:
            return f.read()
        

if __name__ == "__main__": 
    HeaderTool.main(*argv)
