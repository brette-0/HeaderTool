#include <filesystem>
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <vector>

using namespace std;
namespace fs = filesystem;

const char github[39] = "https://github.com/brette-0/HeaderTool";
const char discord[30] = "https://discord.gg/EwfEWcVtzp";

bool verbose = false, renamerom = true, headerrom = true, clean = false, specified = false;
bool noskip = false, defaulttoNES = true; // if false, default to FDS
unsigned short int jobs = 0, success = 0, missing = 0, retrieved = 0;

void romheader(fs::path source, fs::path target);
vector<char> fromlocalFS(unsigned int checksum);

int main(int argc, char* argr[]){
    // confirm that at least one arguement is provided
    if (argc < 2){
        cerr << "Error 1 : No Operations Provided" << endl;
        return -1;  // no operation error
    }

    // dump args into header (quite possibly redundant)
    vector<string> argv = {"HeaderTool"};

    for(int i = 1; i < argc; ++i) {
        argv.push_back(string(argr[i]));
    }

    if ((argv[1] == "-h" || argv[1] == "--help") && argc == 2){
        // generic help message (not really sure what I should put here)
        cout << "HeaderTool 1.5L (x64) [Windows]" << endl
             << "MIT LICENCSE      :" << fs::absolute("LICENSE") << endl
             << "GITHUB REPOSITORY : "<< github << endl
             << "DISCORD SERVER    : "<< discord << endl;
        return 0;
    }

    int argx = 1;                                       // define here for later use
    for (; argx < argc; ++argx){                        // for all potential key word arguements
        if (argv[argx][0] != '-') break;                // leave if not a key word arguement

        // innapropriate use of these keywords
        if (argv[argx] == "-h" || argv[argx] == "--help"){
            cerr << "Error 3 : Broken use of help arguement" << endl;
            return -3;  // broken use of positional arguement
        }

        else if (argv[argx] == "-v" || argv[argx] == "--verbose") verbose = true;
        else if (argv[argx] == "-o" || argv[argx] == "--output") specified = true;
        else if (argv[argx] == "-ns" || argv[argx] == "--noskip") noskip = true;
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
    if (specified){                 // if the `-o | --output` key word was used
        outdir = argv[argx];
        ++argx;
    } else outdir = "./output/";    // or not, use default output naming scheme

    if (argc == argx){              // not all arguements should be key word arguements
        cerr << "Error 6 : No input directory specified" << endl;
        return -6;
    }

    for (; argx < argc; ++argx){    // the remaining should be valid paths
        if (fs::exists(argv[argx])) romheader(argv[argx], outdir);
        else cerr << "No file or folder exists : " << argv[argx] << endl;
    }

    // we may need to clean up the output folder for empty folders

    cout << "HeaderTool finished " << jobs << " tasks with " << success << " passing files" << endl;
    if (!jobs){
        // report nothing to do
    } else if (!success){
        // report total failure
    } else if (success < jobs){
        // report partial success
    }
    // report total succecss
}

void romheader(fs::path source, fs::path target){
    /*
        recursive function to interpret NES file or folder
    */
    if (!fs::exists(target)) fs::create_directory(target);          // create target parent if needed
    if (fs::is_directory(source)){                                  // recurse directory if needed
        for (fs::path subdir : fs::directory_iterator(source)){
            romheader(subdir, target/(source.filename()));
        }
    } else if (source.extension() == ".nes" || noskip){             // only header NES extension (unless not)
        ++jobs;                                                     // record NES file as attempted
        ifstream inROMbuffer(source, ios::binary);
        if (!inROMbuffer) {
            cerr << "Error opening file." << endl;
        }
        vector<char> rom = std::vector<char>(std::istreambuf_iterator<char>(inROMbuffer), std::istreambuf_iterator<char>());
        inROMbuffer.close();

        vector<char> header = fromlocalFS(crc32(0, (const Bytef *)rom.data(), rom.size()));
        if (headerrom || clean){                                    // manipulate header if tasked to
            rom.erase(rom.begin(), rom.begin() + (rom.size() & 0x18));
            if (!clean){
                if (header.size() == 1) return;
                rom.insert(rom.begin(), header.begin(), header.begin() + 16);
                cout << "Headered " << source;
            } else {
                cout << "Removed header from " << source;
            }
        }

        // rename ROM if tasked to with header information from database
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
        ++success;                              // register success if no failure
        return;
    } else if (verbose){
        clog << "Skipping: " << source << " Not a NES file" << endl;
    }
}

vector<char> fromlocalFS(unsigned int checksum){
    if (!fs::exists("headers/")){
        cerr << "Local database is inaccessible!" << endl;
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