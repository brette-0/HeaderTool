/*

    HeaderTool by Brette

*/

#include <string>                               // name handling
#include <vector>                               // file manip
#include <iostream>                             // error reporting
#include <fstream>                              // for local FS and writing
#include <filesystem>                           // for directory rerouting

#include <curl/curl.h>                          // file downloading
#include <zlib.h>                               // checksum evaluation
#include <nlohmann/json.hpp>                    // json handling


using namespace std;
using namespace nlohmann;
namespace fs = filesystem;

// global constants
const string indexurl = "https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/index.json";
const string headerurlprefix = "https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/";
const string github = "https://github.com/BrettefromNesUniverse/HeaderTool";
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
unsigned short int jobs = 0, success = 0, missing = 0, retrieved = 0;

/*

verbose     : flag for verbose mode
renamerom   : flag for renaming roms
headerrom   : flag for headering roms
clean       : flag for cleaning roms
preferlocal : flag to prefer local database
specified   : flag to indicate specified output parent folder
nonet       : flag to indicate network failure
fellback    : flag to indiciate a successful fallback precaution

jobs        : the amount of tasks found
success     : the amount of tasks successfully completed
missing     : the amount of files found on network unfound on local database
retrieved   : the amount of missing files successfully downloaded

*/

//functions
void romheader(fs::path source, fs::path target);
void getdb(bool download = false, fs::path parentdir = fs::current_path());

vector<char> getheader(unsigned int checksum, fs::path source);
vector<char> downloadheader(unsigned int checksum);
vector<char> fromlocalFS(unsigned int checksum);


size_t JsonWriteCallback(void* contents, size_t size, size_t nmemb, string* output);
size_t HeaderWriteCallback(void* contents, size_t size, size_t nmemb, vector<char>* response);


int main(int argc, char* argr[]){
    if (argc < 2){
        cerr << "Error 1 : No Operations Provided" << endl;
        return -1;  // no operation error
    }

    vector<string> argv = {"HeaderTool"};

    for(int i = 1; i < argc; ++i) {
        argv.push_back(string(argr[i]));
    }

    if ((argv[1] == "-h" || argv[1] == "--help") && argc == 2){
        cout << "HeaderTool 1.5 (x64) [Windows]" << endl
             << "MIT LICENCSE      :" << fs::absolute("LICENSE") << endl
             << "GITHUB REPOSITORY : "<< github << endl
             << "DISCORD SERVER    : "<< discord << endl;
        return 0;
    } else if ((argv[1] == "-g" || argv[1] == "--get") && argc <= 3){
        getdb();
        if (nonet){
            cerr << "Error 2 : repo is unreachable, check network configuration or visit repo: " << github << endl;
            return -2;
        }
    
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
    for (; argx < argc; ++argx){
        if (argv[argx][0] != '-') break;
        if (argv[argx] == "-h" || argv[argx] == "--help"){
            cerr << "Error 3 : Broken use of help arguement" << endl;
            return -3;  // broken use of positional arguement
        } else if (argv[argx] == "-g" || argv[argx] == "--get"){
            cerr << "Error 3 : Broken use of get arguement" << endl;
            return -3;
        }

        else if (argv[argx] == "-v" || argv[argx] == "--verbose") verbose = true;// we actually have something to look at
        else if (argv[argx] == "-l" || argv[argx] == "--local") preferlocal = true;
        else if (argv[argx] == "-o" || argv[argx] == "--output") specified = true;
        else if (argv[argx] == "-c" || argv[argx] == "--clean"){
            if (!headerrom){
                cerr << "Error 4 : Cannot clean header if retaining current header information" << endl;
                return -4; // contradicting options
            }
            clean = true;
        }
        else if (argv[argx] == "-nh" || argv[argx] == "--noheader"){
            if (clean){
                cerr << "Error 4 : Cannot clean header if retaining current header information" << endl;
                return -4; // contradicting operations
            } else if (!renamerom){
                cerr << "Error 5 : Cannot retain current header without renaming" << endl;
                return -5; // arguements cancel all operation
            }
            headerrom = false;
        }
        else if (argv[argx] == "-nr" || argv[argx] == "--norename"){
            if (!headerrom){
                cerr << "Error 5 : Cannot retain current header without renaming" << endl;
                return -5; // arguements cancel all operation
            }
            renamerom = false;
        } else if (argv[argx].size() > 1){
            if (argv[argx][0] == ' '){
                cerr << "Error 7 : Malformed optional arguement" << endl;
                return -7;
            } else {
                cerr << "Error 8 : Unrecognized optional argument" << endl;
                return -8;
            }
        } else {
            cerr << "Error 7 : Malformed optional arguement" << endl;
            return -7;
        }
    }
    if (argc == argx){
        cerr << "Error 6 : No input directory specified" << endl;
        return -6; // no entrance point
    }

    fs::path outdir;
    
    if (specified){
        outdir = argv[argx];
        ++argx;
    } else outdir = "./output/";

    if (argc == argx){
        cerr << "Error 6 : No input directory specified" << endl;
        return -6;
    }

    for (; argx < argc; ++argx){
        if (fs::exists(argv[argx])) romheader(argv[argx], outdir);
        else cerr << "No file or folder exists : " << argv[argx] << endl;
    }

    // we may need to clean up the output folder for empty folders

    cout << "HeadertTool finished " << jobs << " tasks with " << success << " passing files" << endl;
    if (success){

        if (!fellback){
            if (jobs == success){
                cout << "Total Success : No requirement for secondary use database" << endl;
                return 0;
                // report total success
            } else {
                cout << "Partial Success : Unable to use secondary use database" << endl;
                return 1;
                // report partial success
            }
        }

        switch ((preferlocal << 1) | (success == jobs)){
            case 0:         // prefer online, partial success
                cout << "Partial success : Relied on network databse" << endl;
                cout << "Found " << missing << " files and succesfully updated database with " << retrieved << " of them." << endl;
                return 3;
            
            case 1:         // prefer online, total success
                cout << "Full success : Relied on network databse" << endl;
                cout << "Found " << missing << " files and succesfully updated database with " << retrieved << " of them." << endl;
                return 2;
            
            case 2:         // prefer offline, partial success
                cout << "Partial success : Relied on local databse" << endl;
                return 5;

            case 3:         // prefer offline, total success
                cout << "Total success : Relied on local databse" << endl;
                return 4;
        }
    } else if (!jobs){
        clog << "No Operations : Nothing could be found" << endl;
        return 7;
    } else {
        clog << "Total Failure : No operations were successful" << endl;
        return 6;
    }
}

void romheader(fs::path source, fs::path target){
    cout << source << endl << target << endl;
    if (!fs::exists(target)) fs::create_directory(target);
    if (fs::is_directory(source)){
        for (fs::path subdir : fs::directory_iterator(source)){
            romheader(subdir, target/(source.filename()));
        }
    } else if (source.extension() == ".nes"){
        ++jobs;
        ifstream inROMbuffer(source, ios::binary);
        vector<char> rom;
        if (!inROMbuffer) {
            cerr << "Error opening file." << endl;
        }
        inROMbuffer.seekg(0, ios::end);
        streampos fileSize = inROMbuffer.tellg();
        rom.resize(fileSize);
        inROMbuffer.seekg(0, ios::beg);
        inROMbuffer.read(rom.data(), fileSize);
        inROMbuffer.close();
        vector<char> header;

        if (headerrom || clean){ 
            rom.erase(rom.begin(), rom.begin() + (rom.size() & 0x1f));
            if (!clean){
                header = getheader(crc32(0, (const Bytef *)rom.data(), rom.size()), source);
                if (header.size() == 1) return; // rewrite maybe?
                rom.insert(rom.begin(), header.begin(), header.begin() + 16);
                cout << "Headered " << source;
            } else {
                cout << "Removed header from " << source;
            }
        }
        fs::path goodname = source;
        if (renamerom){
            goodname = fs::path(string(header.begin()+16, header.end()));
            cout << " and renamed to " << string(header.begin()+16, header.end()) << endl;
        } else cout << endl;

        ofstream outROMbuffer(target/goodname, ios::binary);
        if (!outROMbuffer){
            cerr << "Unknown error : Could not write to " << target/goodname << endl;
            return;
        }
        outROMbuffer.write(rom.data(), rom.size());
        ++success;
        return;
    } else if (verbose){
        clog << "Skipping: " << source << " Not a NES file" << endl;
    }
}

void getdb(bool download, fs::path parentdir){
    CURL* curl = curl_easy_init();
    if (!curl) {
        nonet = true;
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
        nonet = true;
        return;
    }

    fs::path targetdir = parentdir/fs::path("headers/");
    if (!fs::exists(targetdir)) fs::create_directory(targetdir);

    auto checksums = json::parse(headersbuffer);
    float progress = distance(fs::directory_iterator(targetdir), fs::directory_iterator{});
    string percentage;

    for (unsigned int checksum : checksums){
        if (!fs::exists(targetdir/fs::path(to_string(checksum)))){
            ++progress;
            vector<char> header = downloadheader(checksum);
            percentage = to_string((static_cast<unsigned char>((progress / checksums.size()) * 100)));
            if (header.size() == 1){
                if (header[0] == -1){
                    cerr << percentage << "% Unknown Network Error: " << checksum << endl;
                } else if (header[0] == -2){
                    cerr << percentage << "% Missing header:        " << checksum << endl;
                }
            } else {
                ofstream headerbuffer(targetdir/to_string(checksum), ios::binary);
                headerbuffer.write(header.data(), header.size());
                cout << percentage << "% Downloaded:            " << string(header.begin()+16, header.end()) << endl;
            }
        }
    }
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
        cerr << "Local database is inaccessible!" << endl;
        nonet = true;
        return {-1};
    }

    ifstream headerbuffer(fs::path("headers/" + to_string(checksum)), ios::binary);
    vector<char> header;
    if (!headerbuffer) {
        return {-2};
    }
    headerbuffer.seekg(0, ios::end);
    streampos fileSize = headerbuffer.tellg();
    header.resize(fileSize);
    headerbuffer.seekg(0, ios::beg);
    headerbuffer.read(header.data(), fileSize);
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
                    cerr << "Local Database is inaccessible" << endl;
                    return {-1};

                case -2:
                    cerr << "Could not locate " << source << " in local database" << endl;
                    if (nonet) return {-1};
                    if (header.size() == 1){
                        switch(header[0]){
                            case -1:
                                cerr << "Could not establish a connection with database" << endl;
                                nonet = true;
                                return {-1};

                            case -2:
                                cerr << "No header for " << source << " on local database" << endl;
                                return {-1};

                            default:
                                cerr << "Unknown Error" << endl;
                                return {-3};
                        }
                    } else {
                        ++missing;
                        fellback = true;
                        ofstream updatebuffer("./headers/" + to_string(checksum), ios::binary);         // update local db with header if not present
                        if (!updatebuffer && verbose){
                            cerr << "Could not update local DB for " << source << endl;
                            return {header};
                        } else if (updatebuffer){
                            updatebuffer.write(header.data(), header.size());
                            updatebuffer.close();
                            if (verbose) clog << "Added " << string(header.begin() + 16, header.end()) << " to local datbase" << endl;
                            ++retrieved;
                        }
                        return{header};
                    }
                
                default:
                    cerr << "Unknown Error" << endl;
                    return {-3};
            }
        } else return{header};
    } else {
        header = downloadheader(checksum);
        if (header.size() == 1){
            switch(header[0]){
                case -1:
                    cerr << "Could not establish a connection with database" << endl;
                    nonet = true;
                    break;

                case -2:
                    cerr << "No header for " << source << " on local database" << endl;
                    break;

                default:
                    cerr << "Unknown Error" << endl;
                    return {-3};
            }
            if (!fs::exists("./headers/")) return {-1};
            header = fromlocalFS(checksum);
            if (header.size() == 1){
                switch(header[0]){
                    case -1:
                        cerr << "Local Database is inacessible" << endl;
                        return {-1};

                    case -2:
                        cerr << "Could not locate " << source << " in local database" << endl;
                        return {-1};

                    default:
                        cerr << "Unknown Error" << endl;
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