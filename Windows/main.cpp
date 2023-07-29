/*
    Criteria:
        Minimal Filesize
        Total error handling
        Drag & Drop functionality
        header and rename ROM in relative output folder
        minimal library inclusion
*/

#include <string>                                               // filename

#include <zlib.h>                                               // crc32 function
#include <vector>                                               // big data types
#include <stdint.h>                                             // data types

#include <iostream>                                             // error reporting
#include <cerrno>                                               // error handling

#include <fstream>                                              // file handling
#include <sys/stat.h>                                           // folder handling
#include <curl/curl.h>                                          // access files

#include <locale>
#include <codecvt>


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<uint8_t>* data) {
    /*
        Function to accept each byte from libcurl file retrieval
    */
    size_t totalSize = size * nmemb;                            // what is nmemb
    uint8_t* byteContents = static_cast<uint8_t*>(contents);    // how does this work
    data->insert(data->begin(), byteContents, byteContents + totalSize);
    return totalSize;                                           // why return this
}
std::vector<uint8_t> getHDR(std::string const url){
    /*
        Method to retrieve file from repo
    */
    std::vector<uint8_t> header;                                // create header object
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());           // not sure
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &header);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
    std::cerr << "Access Failure, likely bad Dump" << curl_easy_strerror(res) << std::endl;
    return {};}                                                 // return empty vector
    
    return header;                                              // return header contents
}

int main(const int argc, const char* argv[]){                   // accept sys args
    if (argc < 2) {                                             // validate args
        std::cerr << "No file path provided." << std::endl;
        return 1;
    }

    std::ifstream ROMbuffer(argv[1], std::ios::binary);         // create binary ifstraem
    if (!ROMbuffer.is_open()) {                                 // check if file open failed
        std::cerr << "Failed to open ROM";
        return 1;
    }

    // read ROM file contents from PATH into ROM vector
    std::vector<uint8_t> ROM = std::vector<uint8_t>(std::istreambuf_iterator<char>(ROMbuffer), std::istreambuf_iterator<char>());
    ROMbuffer.close();                                          // close ifstream

    // calculate file checksu as string, for filename.
    std::string romChecksum = std::to_string(crc32(0, ROM.data(), ROM.size()));
    if (mkdir("./output") != 0 && errno != EEXIST) {}           // ensure that ouput dir exists
    std::vector<uint8_t> header = getHDR("https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/" + romChecksum);
    std::string headerstr(header.begin(), header.end());        // convert uint vector to str
    std::string ROMstr(ROM.begin(), ROM.end());                 // convert char vector to stsd
    std::ofstream outbuffer(headerstr.substr(16));              // create outbuffer 

    
    if (!outbuffer.is_open()){                                  // handle unknown error
        std::cerr << "Failed to create File";
        return 1;
    }
    outbuffer << (headerstr.substr(0,16) + ROMstr);             // write contents
    outbuffer.close();                                          // operations are finished
    return 0;                                                   // success
}