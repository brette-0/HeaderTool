/*
    Drag and Drop, should perform checksum check on given ROM
    Then it will use that checksum to download data from the githuub repo
    Finally it will header the rom in the outputs folder and rename it.
*/

#include <string>   //filename
#include <sstream>  //modify naming scheme to reduce executable size

#include <zlib.h>   //crc32 function
#include <vector>   //big data types
#include <stdint.h> //data types

#include <iostream> //error reporting

std::string CalculateChecksum(const std::vector<uint8_t>& buffer){
    uint32_t numeric = crc32(0L, Z_NULL, 0);    //reseach how this works
    numeric = crc32(numeric, buffer.data(), buffer.size());
    std::stringstream retval;
    retval << "0x" << numeric;
    return retval.str();
}

int main(int argc, char* argv[]){ 
    if (argc < 2) {
        std::cerr << "No file path provided." << std::endl;
        return 1;
    }

    
    return 0;
}