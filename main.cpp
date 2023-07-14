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
#include <cerrno>                                               // error handling

#include <fstream>                                              // file handling
#include <sys/stat.h>                                           // folder handling

std::string CalculateChecksum(const std::vector<uint8_t>& contents){
    uint32_t numeric = crc32(0L, Z_NULL, 0);                    // reseach how this works
    numeric = crc32(numeric, contents.data(), contents.size());     // calculate crc32 obj
    std::stringstream retval;                                   // create sstream
    retval << "0x" << numeric;                                  // stream contents into it
    return retval.str();                                        // return string filetype (redundant?)
}

int main(int argc, char* argv[]){                               // accept sys args
    if (argc < 2) {                                             // validate args
        std::cerr << "No file path provided." << std::endl;
        return 1;
    }

    std::string path = argv[1];
    std::ifstream ROMbuffer(path, std::ios::binary);            // research this

    if (ROMbuffer.is_open()) {                                  // check if file opened
        std::vector<char> ROM(std::istreambuf_iterator<char>(ROMbuffer), {});   // research this
        ROMbuffer.close();                                      // close buffer (speed in cpp)
    } else {
        std::cerr << "Failed to open ROM";
        return 1;
    }

    std::string rom = CalculateChecksum(*ROM);                  // Calculate Filename
    // here we need use curl to acccess the header and good name
    // here we need modify our buffer to insert the header
    // here we write the modified buffer to output/goodname.nes

    if (mkdir("./output", 0777) != 0 && errno != EEXIST) {}     // ensure that ouput dir exists

    std::string goodname = "";                                  // extract goodname from payload
    std::ofstream outbuffer(goodname);                          // create outbuffer 

    // add check for file exists
    if (outbuffer.is_open()){

    } else {
        std::cerr << "Failed to create File";                    // Throw unknown error
        return 1;
    }
    outbuffer << header << ROM;                                 // write contents
    return 0;
}