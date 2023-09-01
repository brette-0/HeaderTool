#include <iostream>
#include <string>                                               // filename
#include <zlib.h>                                               // crc32 function
#include <vector>                                               // big data types
#include <fstream>                                              // file handling
#include <sys/stat.h>                                           // folder handling
#include <curl/curl.h>                                          // access files
#include <cstdlib>                                              // error display 
#include <filesystem>                                           // recurse dir

namespace fs = std::filesystem;
long int jobs = 0;                                              // create global job count

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* data) {
    /*
        Function to accept each byte from libcurl file retrieval
    */
    size_t totalSize = size * nmemb;                            // what is nmemb
    char* byteContents = static_cast<char*>(contents);          // how does this work
    data->insert(data->begin(), byteContents, byteContents + totalSize);
    return totalSize;                                           // why return this
}
std::vector<char> getHDR(std::string const checksum){
    /*
        Method to retrieve file from repo
    */
    std::vector<char> header;                                   // create header object
    CURL* headercurl = curl_easy_init();                        // curl initialization
    curl_easy_setopt(headercurl, CURLOPT_URL, ("https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/" + checksum).c_str());
    curl_easy_setopt(headercurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(headercurl, CURLOPT_WRITEDATA, &header);

    CURLcode res = curl_easy_perform(headercurl);               // Cleanup libcurl handle          
    if (res != CURLE_OK) {
        std::cerr << "Access Failure, Unknown reason: Check your network connection status or troubleshoot" << std::endl;
        curl_easy_cleanup(headercurl);
        return {};}                                             // return empty vector
    curl_easy_cleanup(headercurl);                              // Cleanup libcurl handle
    if (std::string (header.begin(),header.end()) == "404: Not Found"){
        std::cerr << "Access Failure, likely bad Dump: HeaderTool only supports NES roms ending `.nes`" << std::endl;
        return {};                                              // return empty vector
    }
    return header;                                              // return header contents
}

std::vector<char> localheader(std::string checksum) {
    std::ifstream headerbuffer("./headers/" + checksum, std::ios::binary);
    if (!headerbuffer.is_open()) {                          // check if file open failed
        std::cerr << "Failed to open header";               // report file access error
        return {};                                          // leave with empty header
    }

    // read ROM file contents from PATH into ROM vector
    std::vector<char> header = std::vector<char>(std::istreambuf_iterator<char>(headerbuffer), std::istreambuf_iterator<char>());
    headerbuffer.close();
    return header
}

int getheader(const fs::path path, bool prefer_network == true, bool failsafe == true, headerflag == true, 
    bool rename == true){
    /*
        Focal function called on per-arg from main
    */

    jobs++;                                                     // increment job counter
    std::ifstream ROMbuffer(path, std::ios::binary);            // create binary ifstream
    if (!ROMbuffer.is_open()) {                                 // check if file open failed
        std::cerr << "Failed to open ROM";                      // report file access error
        return 1;                                               // leave with exit code 1
    }

    // read ROM file contents from PATH into ROM vector
    std::vector<char> ROM = std::vector<char>(std::istreambuf_iterator<char>(ROMbuffer), std::istreambuf_iterator<char>());
    ROMbuffer.close();                                          // close ifstream

    if (ROM.size() & 0x1fe7){                                   // read for illegal filesize
        std::cerr << "ROM filesize indicates severe corruption!" << std::endl; 
        return 1;
    }

    if (ROM.size() & 0x18){                                     // remove bad header if present
        ROM.erase(ROM.begin(), ROM.begin()+(ROM.size() & 0x18));
    }

    // calculate file checksum as string, for filename.
    std::string romChecksum = std::to_string(crc32(0, reinterpret_cast<const Bytef*>(ROM.data()), ROM.size()));
    if (mkdir("./output") != 0 && errno != EEXIST) {}           // ensure that ouput dir exists
    if (prefer_network) {
        std::vector<char> header = getHDR(romChecksum);
        if (!header.size()) {                                   // if we got emtpy results
            if (failsafe) {
                std::cout << "Could not find server stored header info, searching local FS" << std::endl;
                if (fs::exists("./headers/" + romChecksum) {
                    header = localheader(romChecksum);          // get header from local FS
                    if (!header.size()) {                       // if failed to find header info
                        return 1;                               // leave with exti code 1
                    }
                } else {
                    return 1;                                   // leave with exit code 1
                }

            }
            else {
                return 1;                                       // leave with exit code 1
            }
        }
    } else {
        header = localheader(romChecksum);                      // get header from local FS
        if (!header.size()) {                                   // if failed to find header info
            return 1;                                           // leave with exti code 1
        }
    } 
    if (rename) {
        // use correted file name from header file
        std::string newfilename(header.begin() + 16, header.end());
    } else {
        std::string newfilename = path.filename().string();
    }
    std::ofstream outbuffer("./output/" + newfilename, std::ios::binary); 

    
    if (!outbuffer.is_open()){                                  // handle unknown error
        std::cerr << "Failed to create File" << std::endl;      // report fatal error
        return 1;
    }
    if (headerflag) {
        if (header[0]) {                                            // if header begins with non-zero
            outbuffer.write(header.data(), 16);                     // write header if applicable
        }
        else {
            // if header is zero, implied empty header means log no-header and apply goodname
            std::clog << "No header for this ROM, goodname will be applied";
        }
    }
    outbuffer.write(ROM.data(), ROM.size());                    // followed by ROM
    outbuffer.close();                                          // operations are finished
    return 0;
}

long int recursedir(const fs::path& path){
    /*
        recursive file directory mapping
    */

    long int errors = 0;                                        // store error count
    bool error;                                                 // initalize error
    for (const auto& entry : fs::directory_iterator(path)) {    // for path in path
        if (fs::is_directory(entry)) {                          // if subpath is subdir
            errors += recursedir(entry.path());                 // map subdir
        }
        else if (entry.path().extension().string() == ".nes") {                                                  // otherwise if file
            error = getheader(entry.path());                    // header file
            if (error) {                                        // report failure
                std::cerr << "Failed job: " << entry << std::endl;
            } else {                                            // report success
                std::cout << "Succeeded job: " << entry << std::endl;
            }
            errors += error;                                    // include current job in sum
        }
        else {                                                  // report invalid filetype
            std::clog << "Skipping File, does not have NES Excetion" << std::endl;
        }
    }
    return errors;                                              // return error count
}

