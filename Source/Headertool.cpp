/*

    HeaderTool by Brette

*/

#include <string>                               // name handling
#include <vector>                               // file manip
#include <iostream>                             // error reporting
#include <fstream>                              // for local FS and writing
#include <filesystem>                           // for directory rerouting
#include <thread>                               // for faster downloading
#include <atomic>                               // memory atomoticy
#include <mutex>                                // memory safety

#include <curl/curl.h>                          // file downloading
#include <zlib.h>                               // checksum evaluation
#include <nlohmann/json.hpp>                    // json handling


#ifdef _WIN32
#include <windows.h> // For Windows-specific code
#else
#include <cstdlib>   // For Unix/Linux-specific code
#endif


using namespace std;
using namespace nlohmann;
namespace fs = filesystem;

// global constants
const string indexurl = "https://raw.githubusercontent.com/brette-0/HeaderTool/main/index.json";
const string headerurlprefix = "https://raw.githubusercontent.com/brette-0/HeaderTool/main/headers/";
const string github = "https://github.com/brette-0/HeaderTool";
const string discord = "https://discord.gg/EwfEWcVtzp";

/* exit codes

ID: Name                    Description

 0: Success                 The app completed all tasks without error
 1: Parital Success         The app completed some tasks with some errors
 2: Local Success           The app completed all tasks without error through resorting to local database
 3: Local Partial Sucesss   The app completed some tasks with some errors through resorting to local database
 4: Network Success         The app completed all tasks without error through resorting to online database
 5: Network Partial Success The app completed some tasks with some errors through resorting to online database
 6: Total Failure           No operations were successful
 7: No Operations           No operations were found
-1: No Operation            The user provided no operations for the app
-2: Network Failure         The user attempted a network only operation which lead to failure
-3: Misused Arguement       The user attempted to use a positional arguement after a dynamic arguement
-4: Opposing Arguements     The user attempted to use at least two arguements which oppose in functionality
-5: Cancelling Arguements   The user attempted to use at least two arguements that would cancel all operations 
-6: No Entrance Point       The user attempted to process files without specify an entrance point
-7: Malformed arguement     An arguement provided by the user was illegible
-8: Unrecognized arguement  An arguement provided by the user was unknown
*/

// globals
bool verbose = false, renamerom = true, headerrom = true, clean = false, preferlocal = false, specified = false, nonet = false, fellback = false;
bool noskip = false; bool defaulttoNES = true; // if false, default to FDS
unsigned short int missing = 0, retrieved = 0;

atomic<unsigned short int> success = 0;

struct job{fs::path source, target;};


vector<job> jobs;
/*

verbose     : flag for verbose mode
renamerom   : flag for renaming roms
headerrom   : flag for headering roms
clean       : flag for cleaning roms
preferlocal : flag to prefer local database
specified   : flag to indicate specified output parent folder
nonet       : flag to indicate network failure
fellback    : flag to indiciate a successful fallback precaution

success     : the amount of tasks successfully completed
missing     : the amount of files found on network unfound on local database
retrieved   : the amount of missing files successfully downloaded

jobs        : the files to process
*/

//defines
auto clearScreen = []() {
#ifdef _WIN32
    // For Windows
    system("cls");
#else
    // For Unix/Linux
    system("clear");
#endif
};

//functions
void completejob(job thisjob);
void getjob(fs::path source, fs::path target);
void getdb(bool download = false, fs::path parentdir = fs::current_path());

vector<char> getheader(unsigned int checksum, fs::path source);
vector<char> downloadheader(unsigned int checksum);
vector<char> fromlocalFS(unsigned int checksum);


size_t JsonWriteCallback(void* contents, size_t size, size_t nmemb, string* output);
size_t HeaderWriteCallback(void* contents, size_t size, size_t nmemb, vector<char>* response);


int main(int argc, char* argr[]){
    // confirm that at least one arguement is provided
    if (argc < 2){
        cerr << "Error 1 : No Operations Provided\n";
        return -1;  // no operation error
    }

    // dump args into header (quite possibly redundant)
    vector<string> argv = {"HeaderTool"};

    for(int i = 1; i < argc; ++i) {
        argv.push_back(string(argr[i]));
    }

    if ((argv[1] == "-h" || argv[1] == "--help") && argc == 2){
        // generic help message (not really sure what I should put here)
        cout << "HeaderTool 1.5 (x(x" << (sizeof(void*) == 8) << ") [Windows]\n"
             << "MIT LICENCSE      :" << fs::absolute("LICENSE") << '\n'
             << "GITHUB REPOSITORY : "<< github << '\n'
             << "DISCORD SERVER    : "<< discord << '\n';
        return 0;
        
    } else if ((argv[1] == "-g" || argv[1] == "--get") && argc <= 3){
        // database pull command, simple as
        getdb();
        if (nonet){
            cerr << "Error 2 : repo is unreachable, check network configuration or visit repo: " << github << '\n';
            return -2;
        }
    
        // elect parent directory for header directory
        fs::path parentdir;
        if (argc == 3){
            parentdir = fs::path(argv[2]);
            if (!fs::exists(argv[2])) parentdir = fs::current_path();
        } else parentdir = fs::current_path();
        
        getdb(true, parentdir);
        return 0;
    }

    getdb();
    if (nonet && !fs::exists("./headers/")){
        return -9; // impossible to operate
    }
    int argx = 1;                                       // define here for later use
    for (; argx < argc; ++argx){                        // for all potential key word arguements
        if (argv[argx][0] != '-') break;                // leave if not a key word arguement

        // innapropriate use of these keywords
        if (argv[argx] == "-h" || argv[argx] == "--help"){
            cerr << "Error 3 : Broken use of help arguement\n";
            return -3;  // broken use of positional arguement
        } else if (argv[argx] == "-g" || argv[argx] == "--get"){
            cerr << "Error 3 : Broken use of get arguement\n";
            return -3;
        }

        else if (argv[argx] == "-v" || argv[argx] == "--verbose") verbose = true;
        else if (argv[argx] == "-l" || argv[argx] == "--local") preferlocal = true;
        else if (argv[argx] == "-o" || argv[argx] == "--output") specified = true;
        else if (argv[argx] == "-ns" || argv[argx] == "--noskip") noskip = true;
        else if (argv[argx] == "-c" || argv[argx] == "--clean"){
            if (!headerrom){
                cerr << "Error 4 : Cannot clean header if retaining current header information\n";
                return -4; // contradicting options
            }
            clean = true;
        }
        else if (argv[argx] == "-nh" || argv[argx] == "--noheader"){
            if (clean){
                cerr << "Error 4 : Cannot clean header if retaining current header information\n";
                return -4; // contradicting operations
            } else if (!renamerom){
                cerr << "Error 5 : Cannot retain current header without renaming\n";
                return -5; // arguements cancel all operation
            }
            headerrom = false;
        }
        else if (argv[argx] == "-nr" || argv[argx] == "--norename"){
            if (!headerrom){
                cerr << "Error 5 : Cannot retain current header without renaming\n";
                return -5; // arguements cancel all operation
            }
            renamerom = false;
        } else if (argv[argx].size() > 1){
            if (argv[argx][0] == ' '){
                cerr << "Error 7 : Malformed optional arguement\n";
                return -7;
            } else {
                cerr << "Error 8 : Unrecognized optional argument\n";
                return -8;
            }
        } else {
            cerr << "Error 7 : Malformed optional arguement\n";
            return -7;
        }
    }
    if (argc == argx){
        cerr << "Error 6 : No input directory specified\n";
        return -6; // no entrance point
    }

    fs::path outdir;
    if (specified){                 // if the `-o | --output` key word was used
        outdir = argv[argx];
        ++argx;
    } else outdir = "./output/";    // or not, use default output naming scheme

    if (argc == argx){              // not all arguements should be key word arguements
        cerr << "Error 6 : No input directory specified\n";
        return -6;
    }

    cout << "Recursing input paths...\n";
    for (; argx < argc; ++argx){    // the remaining should be valid paths
        if (fs::exists(argv[argx])) getjob(argv[argx], outdir);
        else cerr << "No file or folder exists : " << argv[argx] << '\n';
    }


    atomic<float> progress = distance(fs::directory_iterator(outdir), fs::directory_iterator{});
    mutex mtx;

    auto completejob = [&](job thisjob){
        string percentage = to_string((static_cast<unsigned char>((progress.load() / jobs.size()) * 100)));
        string success_str;
        progress.store(progress.load() + 1.0);
        

        ifstream inROMbuffer(thisjob.source, ios::binary);
        if (!inROMbuffer) {
            cerr << "Error opening file.\n";
        }
        vector<char> rom = std::vector<char>(std::istreambuf_iterator<char>(inROMbuffer), std::istreambuf_iterator<char>());
        inROMbuffer.close();

        vector<char> header;                                        // preserve scope
        if (headerrom || clean){                                    // manipulate header if tasked to
            rom.erase(rom.begin(), rom.begin() + (rom.size() & 0x18));
            if (!clean){
                header = getheader(crc32(0, (const Bytef *)rom.data(), rom.size()), thisjob.source);
                if (header.size() == 1) return; // rewrite maybe?
                rom.insert(rom.begin(), header.begin(), header.begin() + 16);
                success_str =  percentage + "% Headered:            " + thisjob.source.string();
            } else {
                success_str =  percentage + "% UnHeadered:            " + thisjob.source.string();
            }
        }

        // rename ROM if tasked to with header information from database
        fs::path goodname = thisjob.source;
        if (renamerom){
            goodname = fs::path(string(header.begin()+16, header.end()));
            success_str +=  " and renamed to " + string(header.begin()+16, header.end()) + '\n';
        } else success_str += '\n';

        ofstream outROMbuffer(thisjob.target/goodname, ios::binary);
        if (!outROMbuffer){
            cout << percentage << "% Failed :            " << thisjob.source << '\n';
            return;
        } else cout << success_str;
        outROMbuffer.write(rom.data(), rom.size());
        success.store(success.load() + 1.0);                              // register success if no failure
        std::lock_guard<std::mutex> lock(mtx);
        return;
    };

    vector<std::thread> jobthreads;
    for (const job& thisjob : jobs) jobthreads.emplace_back([&]() { completejob(thisjob); });
    for (std::thread& thread : jobthreads) thread.join();

    clearScreen();
    cout << "100% Jobs complete\n";

    cout << "HeadertTool finished " << jobs.size() << " tasks with " << success << " passing files\n";
    if (success){

        if (!fellback){
            if (jobs.size() == success){
                cout << "Total Success : No requirement for secondary use database\n";
                return 0;
                // report total success
            } else {
                cout << "Partial Success : Unable to use secondary use database\n";
                return 1;
                // report partial success
            }
        }

        switch ((preferlocal << 1) | (success == jobs.size())){
            case 0:         // prefer online, partial success
                cout << "Partial success : Relied on network databse\n";
                cout << "Found " << missing << " files and succesfully updated database with " << retrieved << " of them.\n";
                return 3;
            
            case 1:         // prefer online, total success
                cout << "Full success : Relied on network databse\n";
                cout << "Found " << missing << " files and succesfully updated database with " << retrieved << " of them.\n";
                return 2;
            
            case 2:         // prefer offline, partial success
                cout << "Partial success : Relied on local databse\n";
                return 5;

            case 3:         // prefer offline, total success
                cout << "Total success : Relied on local databse\n";
                return 4;
        }
    } else if (!jobs.size()){
        clog << "No Operations : Nothing could be found\n";
        return 7;
    } else {
        clog << "Total Failure : No operations were successful\n";
        return 6;
    }
}

void getjob(fs::path source, fs::path target){
    /*
        recursive function to interpret NES file or folder
    */
    if (!fs::exists(target)) fs::create_directory(target);          // create target parent if needed
    if (fs::is_directory(source)){                                  // recurse directory if needed
        for (fs::path subdir : fs::directory_iterator(source)){
            getjob(subdir, target/(source.filename()));
        }
    } else if (source.extension() == ".nes" || noskip){             // only header NES extension (unless not)
        jobs.push_back({source, target});
    } else if (verbose){
        clog << "Skipping: " << source << " Not a NES file\n";
    }
}


void getdb(bool download, fs::path parentdir){
    /*
    
        downloads database to memory, or begins download headers, depends how it was called.
    
    */
    CURL* curl = curl_easy_init();
    if (!curl) {
        nonet = true;                   // disable network operations
        return;
    }

    string headersbuffer;

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, indexurl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, JsonWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &headersbuffer);
    
    

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (!download && res == CURLE_OK) return;
    if (res != CURLE_OK){
        nonet = true;                                       // disable network operations
        return;
    }

    fs::path targetdir = parentdir/fs::path("headers/");
    if (!fs::exists(targetdir)) fs::create_directory(targetdir);

    auto checksums = json::parse(headersbuffer);
    atomic<float> progress = distance(fs::directory_iterator(targetdir), fs::directory_iterator{});
    mutex mtx;

    auto downloadHeaderThread = [&](unsigned int checksum) {
        vector<char> header = downloadheader(checksum);
        string percentage = to_string((static_cast<unsigned char>((progress.load() / checksums.size()) * 100)));
        if (header.size() == 1){
            if (header[0] == -1){
                cerr << percentage << "% Unknown Network Error: " << checksum << '\n';
            } else if (header[0] == -2){
                cerr << percentage << "% Missing header:        " << checksum << '\n';
            }
        } else {
            progress.store(progress.load() + 1.0);
            std::lock_guard<std::mutex> lock(mtx);
            ofstream headerbuffer(targetdir/to_string(checksum), ios::binary);
            headerbuffer.write(header.data(), header.size());
            cout << percentage << "% Downloaded:            " << string(header.begin()+16, header.end()) << '\n';
        }
    };

    // Create a vector of threads to download headers concurrently
    vector<thread> threads;
    for (unsigned int checksum : checksums) {
        if (!fs::exists(targetdir / fs::path(to_string(checksum)))) {
            threads.emplace_back(downloadHeaderThread, checksum);
        }
    }

    // Join all threads to ensure they complete before moving on
    for (auto& thread : threads) {
        thread.join();
    }

    clearScreen();
    cout << "100% Downloaded, use -l or --local to use local database\n";
}

vector<char> downloadheader(unsigned int checksum){
    vector<char> header;

    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(!curl){
        return {-2};
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, (headerurlprefix + to_string(checksum)).c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HeaderWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &header);

        // Perform the request and capture the result
    res = curl_easy_perform(curl);
    if (string(header.data(), header.size()) == "404: Not Found"){
        return {-2};                    // unknown network error
    } else if (res != CURLE_OK){
        return {-1};                    // header does not exist
    } 

    curl_easy_cleanup(curl);
    return header;
}

vector<char> fromlocalFS(unsigned int checksum){
    if (!fs::exists("headers/")){
        cerr << "Local database is inaccessible!\n";
        return {-1};
    }

    ifstream headerbuffer(fs::path("headers/" + to_string(checksum)), ios::binary);
    if (!headerbuffer) {
        return {-2};
    }
    vector<char> header = std::vector<char>(std::istreambuf_iterator<char>(headerbuffer), std::istreambuf_iterator<char>());
    headerbuffer.close();

    return header;
}



vector<char> getheader(unsigned int checksum, fs::path source){
    vector<char> header;
    
    if ((preferlocal || nonet) && fs::exists("./headers/")){
        header = fromlocalFS(checksum);
        if (header.size() == 1){
            switch(header[0]){
                case -1:
                    cerr << "Local Database is inaccessible\n";
                    return {-1};

                case -2:
                    cerr << "Could not locate " << source << " in local database\n";
                    if (nonet) return {-1};
                    if (header.size() == 1){
                        switch(header[0]){
                            case -1:
                                if (verbose) cerr << "Could not establish a connection with database\n";
                                nonet = true;
                                return {-1};

                            case -2:
                                if (verbose) cerr << "No header for " << source << " on local database\n";
                                return {-1};

                            default:
                                if (verbose) cerr << "Unknown Error\n";
                                return {-3};
                        }
                    } else {
                        ++missing;
                        fellback = true;
                        ofstream updatebuffer("./headers/" + to_string(checksum), ios::binary);         // update local db with header if not present
                        if (!updatebuffer && verbose){
                            cerr << "Could not update local DB for " << source << '\n';
                            return {header};
                        } else if (updatebuffer){
                            updatebuffer.write(header.data(), header.size());
                            updatebuffer.close();
                            if (verbose) clog << "Added " << string(header.begin() + 16, header.end()) << " to local datbase\n";
                            ++retrieved;
                        }
                        return{header};
                    }
                
                default:
                    cerr << "Unknown Error\n";
                    return {-3};
            }
        } else return{header};
    } else {
        header = downloadheader(checksum);
        if (header.size() == 1){
            switch(header[0]){
                case -1:
                    if (verbose) cerr << "Could not establish a connection with database\n";
                    nonet = true;
                    break;

                case -2:
                    if (verbose) cerr << "No header for " << source << " on local database\n";
                    break;

                default:
                    if (verbose)cerr << "Unknown Error\n";
                    return {-3};
            }
            if (!fs::exists("./headers/")) return {-1};
            header = fromlocalFS(checksum);
            if (header.size() == 1){
                switch(header[0]){
                    case -1:
                        if (verbose) cerr << "Local Database is inacessible\n";
                        return {-1};

                    case -2:
                        if (verbose)cerr << "Could not locate " << source << " in local database\n";
                        return {-1};

                    default:
                        if (verbose)cerr << "Unknown Error\n";
                        return {-3};
                }
            } else {
                fellback = true;
                return header;
            }
        } else return {header};
    }
}

size_t JsonWriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    /*
        Dedicated Write Callback function for downloading database index
    */
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}
size_t HeaderWriteCallback(void* contents, size_t size, size_t nmemb, vector<char>* response) {
    size_t totalSize = size * nmemb;
    char* data = static_cast<char*>(contents);
    for(size_t i = 0; i < totalSize; ++i) {
        response->push_back(data[i]);
    }
    return totalSize;
}