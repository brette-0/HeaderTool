#include "backend.cpp"                                                      // include core functionality
#include <Windows.h>                                                        // include system handling

void get_latest(){
    /*
        Routine to download and update current binary
    */
    std::vector<char> binary;
    CURL* bincurl = curl_easy_init();

    if (bincurl) {
        curl_easy_setopt(bincurl, CURLOPT_URL, "Latest version");
        curl_easy_setopt(bincurl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(bincurl, CURLOPT_WRITEDATA, &binary);

        CURLcode res = curl_easy_perform(bincurl);
        if (res != CURLE_OK) {
            std::cerr << "Error downloading the URL: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(bincurl);
    }
    curl_easy_cleanup(bincurl);
    std::ofstream outbuffer("c:/users/programme files/headertool/headertool.exe", std::ios::binary);
    outbuffer.write(binary.data(), binary.size());
    outbuffer.close();
    return;
}


int main(){
    /*
        Main routine, handles args and updating | sys handles. May also use GUI (to be decided)
    */
    if (mkdir("c:/program files/headertool/") && errno != EEXIST){}         // create dir in prog files 
    get_latest();                                                           // download portable binary
                                                                            // write in /prog files/headertool
    // add binary to PATH
    // add "Header with Headertool" to thingy
    // schedulse selt to delete
    // also we need to prepare for unins000.exe
    // Should also be able to uninstall program from Control.exe
    return 0;
}