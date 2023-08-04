#include "backend.cpp"                                          // include core functionality


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
        else if (fs::path(argv[arg]).extension().string() == ".nes") {
            error = getheader(fs::path(argv[arg]));             // get success
            if (error) {                                        // report failure
                std::cerr << "Failed job: " << argv[arg] << std::endl;
            } else {                                            // report success
                std::cout << "Succeeded job: " << argv[arg] << std::endl;
            }
            errors += error;                                    // include current job in sum
        }                    
        else {                                                  // report invalid filetype
            std::clog << "Skipping File, does not have NES Excetion" << std::endl;
        }
        
    }
    // report quantity of failed and total jobs
    std::cout << "Headering finished with " << std::to_string(errors) << " fails out of " << std::to_string(jobs) << " jobs.";
    system("pause");                                            // wait for user input
    return 0;                                                   // leave with exit code 0
}
