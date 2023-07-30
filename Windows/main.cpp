/*
    Criteria:
        Minimal Filesize
        Total error handling
        Drag & Drop functionality
        header and rename ROM in relative output folder
        minimal library inclusion
*/

#include <iostream>
#include <string>                                               // filename
#include <zlib.h>                                               // crc32 function
#include <vector>                                               // big data types
#include <fstream>                                              // file handling
#include <sys/stat.h>                                           // folder handling
#include <curl/curl.h>                                          // access files
#include <cstdlib>                                              // error display 

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* data) {
    /*
        Function to accept each byte from libcurl file retrieval
    */
    size_t totalSize = size * nmemb;                            // what is nmemb
    char* byteContents = static_cast<char*>(contents);          // how does this work
    data->insert(data->begin(), byteContents, byteContents + totalSize);
    return totalSize;                                           // why return this
}
std::vector<char> getHDR(std::string const url){
    /*
        Method to retrieve file from repo
    */
    std::vector<char> header;                                   // create header object
    CURL* curl = curl_easy_init();                              // research
    if (!curl) {                                                // if curl failed to initialize
        // report fatal error within programme
        std::cerr << "Failed to initialize libcurl." << std::endl;
        return {};                                              // return empty vector for error
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());           // research
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &header);

    CURLcode res = curl_easy_perform(curl);                     // research
    if (res != CURLE_OK) {
        long httpCode = 0;                                      // initialize status code variable
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (httpCode == 404) {                                  // if file not found
            std::cerr << "Access Failure, likely bad Dump: HeaderTool only supports NES roms ending `.nes`" << std::endl;
        } else {                                                // otherwise unknown network error
            std::cerr << "Access Failure, Unknown reason: Check your network connection status or troubleshoot" << std::endl;
        }
        curl_easy_cleanup(curl);
        return {};}                                             // return empty vector
    curl_easy_cleanup(curl);                                    // Cleanup libcurl handle
    return header;                                              // return header contents
}

int getheader(const std::string path){
    std::ifstream ROMbuffer(path, std::ios::binary);            // create binary ifstraem
    if (!ROMbuffer.is_open()) {                                 // check if file open failed
        std::cerr << "Failed to open ROM";                      // report file access error
        return 1;                                               // leave with exit code 1
    }

    // read ROM file contents from PATH into ROM vector
    std::vector<char> ROM = std::vector<char>(std::istreambuf_iterator<char>(ROMbuffer), std::istreambuf_iterator<char>());
    ROMbuffer.close();                                          // close ifstream

    // calculate file checksu as string, for filename.
    std::string romChecksum = std::to_string(crc32(0, reinterpret_cast<const Bytef*>(ROM.data()), ROM.size()));
    if (mkdir("./output") != 0 && errno != EEXIST) {}           // ensure that ouput dir exists
    std::vector<char> header = getHDR("https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/" + romChecksum);
    if (!header.size()){                                        // if we got emtpy results
        return 1;                                               // leave with exit code 1
    }
    std::string headerstr(header.begin(), header.end());        // convert uint vector to str
    std::string ROMstr(ROM.begin(), ROM.end());                 // convert char vector to stsd
    std::ofstream outbuffer("./output/" + headerstr.substr(16));// create outbuffer 

    
    if (!outbuffer.is_open()){                                  // handle unknown error
        std::cerr << "Failed to create File";                   // report fatal error
        return 1;
    }
    outbuffer << (headerstr.substr(0,16) + ROMstr);             // write contents
    outbuffer.close();                                          // operations are finished
    return 0;
}

int main(const int argc, const char* argv[]){                   // accept sys args
    if (argc < 2) {                                             // validate args
        std::cerr << "No file path provided." << std::endl;     // alert player of no PATH arg
        system("pause");                                        // wait for user input
        return 1;                                               // leave with exit code 1
    }
    long int errors = 0;
    for (int arg = 1; arg < argc; arg++){
        errors += getheader(argv[arg]);
    }
    // report quantity of failed and total jobs
    std::cout << "Headering finished with " << std::to_string(errors) << " fails out of " << std::to_string(argc - 1) << " jobs.";
    system("pause");                                            // wait for user input
    return 0;                                                   // leave with exit code 0
}
