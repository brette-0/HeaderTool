/*
    Criteria:
        Graphical User Interface
*/

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
        std::cerr << "Access Failure, Unknown reason: Check your network connection status or troubleshoot" << std::endl;
        curl_easy_cleanup(curl);
        return {};}                                             // return empty vector
    curl_easy_cleanup(curl);                                    // Cleanup libcurl handle
    if (std::string (header.begin(),header.end()) == "404: Not Found"){
        std::cerr << "Access Failure, likely bad Dump: HeaderTool only supports NES roms ending `.nes`" << std::endl;
        return {};                                              // return empty vector
    }
    return header;                                              // return header contents
}

int getheader(const fs::path path){
    /*
        Focal function called on per-arg from main
    */

    jobs++;                                                     // increment job counter
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
    std::string goodname(header.begin()+16,header.end());    // convert char vector to str
    std::ofstream outbuffer("./output/" + goodname, std::ios::binary); 

    
    if (!outbuffer.is_open()){                                  // handle unknown error
        std::cerr << "Failed to create File" << std::endl;      // report fatal error
        return 1;
    }
    if (header[0]) {                                            // if header begins with non-zero
        outbuffer.write(header.data(), 16);                     // write header if applicable
    }
    else {
        // if header is zero, implied empty header means log no-header and apply goodname
        std::clog << "No header for this ROM, goodname will be applied";
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
        else {                                                  // otherwise if file
            error = getheader(entry.path());                    // header file
            if (error) {                                        // report failure
                std::cerr << "Failed job: " << entry << std::endl;
            } else {                                            // report success
                std::cout << "Succeeded job: " << entry << std::endl;
            }
            errors += error;                                    // include current job in sum
        }
    }
    return errors;                                              // return error count
}

int main(const int argc, const char* argv[]){                   // accept sys args
    /*
        Main Function to process sys args
    */
   
    if (argc < 2) {                                             // validate args
        std::cerr << "No file path provided." << std::endl;     // alert player of no PATH arg
        system("pause");                                        // wait for user input
        return 1;                                               // leave with exit code 1
    }
    long int errors = 0;                                        // track rate of failure and jobs
    for (int arg = 1; arg < argc; arg++){                       // for enum of args

        bool error;                                             // prepare access scope 
        if (fs::is_directory(fs::path(argv[arg]))){
            errors += recursedir(fs::path(argv[arg]));
        }
        else {
            error = getheader(fs::path(argv[arg]));             // get success
            if (error) {                                        // report failure
                std::cerr << "Failed job: " << argv[arg] << std::endl;
            } else {                                            // report success
                std::cout << "Succeeded job: " << argv[arg] << std::endl;
            }
            errors += error;                                    // include current job in sum
        }                    
        
    }
    // report quantity of failed and total jobs
    std::cout << "Headering finished with " << std::to_string(errors) << " fails out of " << std::to_string(jobs) << " jobs.";
    system("pause");                                            // wait for user input
    return 0;                                                   // leave with exit code 0
}
