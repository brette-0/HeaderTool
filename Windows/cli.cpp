#include "backend.cpp"                                          // include core functionality


int main(const int argc, const char* argv[]){                   // accept sys args
    /*
        Main Function to process sys args
    */

    bool failsafe = true;
    bool prefer_network = true;
    bool rename == true;
    bool headerflag == true;

    void help_message() {
        std::cout << "Headertool version 1.7" << std::endl
            << "BrettefromNesUniverse : {github link}" << std::endl
            << "Brette's Domain : {discord link}" << std::endl << std::endl
            << "-h | --help  : Help commands to learn more about HeaderTool" << std::endl
            << "-d | --dump  : Dump header repo to c:/users/Documents/HeaderTool" << std::endl
            << "-r | --repo  : do not use local files, only use repo (unreccomended)" << std::endl
            << "-l | --local : prefer local files, then use repo (unreccomended)" << std::endl
            << "-n | --name  : do not rename files, only output headers" << std::endl
            << "-H | --header: only rename files, do not header ROMs (unreccomended)" << std::endl;
        return;
    }

    if (argc < 2) {                                             // validate args
        help_message();
        return 1;
    }
    int args = 1;
    while (argv[args][0] == "-" or args == argc - 1) {         // scale all non-path args except binary
        if (argv[args] == "-h" or argv[args] == "--help") {
            help_message();
            return 0;
        }

        if (argv[args] == "-d" or argv[args] == "--dump") {
            CURL* headercurl = curl_easy_init();
            std::string json_data;
            curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/index.json".c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json_data);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Access Failure, Unknown reason: Check your network connection status or troubleshoot" << std::endl;
            }
            else {
                // delete previous database in preperation to update it
                if (fs::exists("./headers")) { fs::remove_all("./headers"); }
                json parsed_json = json::parse(json_data);

                for (const auto& header : parsed_json) {
                    std::ofstream outbuffer(header.string(), std::ios::binary);
                    outbuffer.write(getHDR(header));
                    outbuffer.close();
                }
            }

            // Clean up
            curl_easy_cleanup(curl);
        }

        if (argv[args] == "-r" or argv[args] == "--repo") {
            failsafe = false;                                   // disable fallback local 
            return 0;
        }

        if (argv[args] == "-l" or argv[args] == "--local") {
            if (!fs::exists("./headers")) {
                std::cerr << "[-l | --local] cannot be used if local header database is not present" << std::endl;
            } else {
                prefer_network = false;                              // re-order operations
            }
        }

        if (argv[args] == "-n" or argv[args] == "--name") {
            if (not headerflag) {
                std::cerr << "[-H | --header] cannot be used alongside [-n | --name]" << std::endl;
                return 1;
            }
            rename = false                                     // unset rename flag
            return 0;
        }

        if (argv[args] == "-H" or argv[args] == "--Header") {
            if (not rename) {
                std::cerr << "[-H | --header] cannot be used alongside [-n | --name]" << std::endl; 
                return 1;
            }
            headerflag = false;                                // unset headering flag
            return 0;
        }
    }

    long int errors = 0;                                        // track rate of failure and jobs
    for (int arg = args; arg < argc; arg++){                    // for enum of path args

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

