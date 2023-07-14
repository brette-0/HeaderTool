/*
    Drag and Drop, should perform checksum check on given ROM
    Then it will use that checksum to download data from the githuub repo
    Finally it will header the rom in the outputs folder and rename it.
*/

#include <string>   //filename
#include <sstream>  //modify naming scheme to reduce executable size

#include <zlib.h>                                               // crc32 function
#include <vector>                                               // big data types
#include <stdint.h>                                             // data types

#include <iostream>                                             // error reporting

std::string CalculateChecksum(const std::vector<uint8_t>& buffer){
    uint32_t numeric = crc32(0L, Z_NULL, 0);                    // reseach how this works
    numeric = crc32(numeric, buffer.data(), buffer.size());     // calculate crc32 obj
    std::stringstream retval;                                   // create sstream
    retval << "0x" << numeric;                                  // stream contents into it
    return retval.str();                                        // return string filetype (redundant?)
}

int main(int argc, char* argv[]){                               // accept sys args
    if (argc < 2) {                                             // validate args
        std::cerr << "No file path provided." << std::endl;
        return 1;
    }

    // check if arg2 is filepath
    // we then need to open the filepath and store its contents into some vector
    std::string rom = CalculateChecksum(buffer);
    // here we need use curl to acccess the header and good name
    // here we need modify our buffer to insert the header
    // here we write the modified buffer to output/goodname.nes

    return 0;
}