#include "backend.cpp"                                          // include core functionality         
#include <nlohmann/json.hpp>                                    // include json reading

namespace nl = nlohmann;                                        // tidy namespace for long name

nl::json getindex() {
    std::string index;
    CURL* indexcurl = curl_easy_init();

    if (indexcurl) {
        curl_easy_setopt(indexcurl, CURLOPT_URL, "https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/index.json");
        curl_easy_setopt(indexcurl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(indexcurl, CURLOPT_WRITEDATA, &index);

        CURLcode res = curl_easy_perform(indexcurl);
        if (res != CURLE_OK) {
            std::cerr << "Error downloading the URL: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(indexcurl);
    }

    return nl::json::parse(index);
}

void dumprepo(){
    /*
        dumps repo from on-repo index json
    */

    if (mkdir("stored headers") && errno != EEXIST) {
        std::cerr << "Failed to create output folder, aborted" << std::endl;
        return ;
    }
    nl::json index = getindex();
    for (const auto& item : index) {
        std::ofstream outbuffer("./stored headers/" + std::string(item), std::ios::binary);
        std::vector<char> header = getHDR("https://raw.githubusercontent.com/BrettefromNesUniverse/HeaderTool/main/headers/" + std::string(item));
        outbuffer.write(header.data(),header.size());
        outbuffer.close();
        }
}

int main(const int argc, const char* argv[]) {
    /*
        Main GUI script and frontend endpoint
    */

    /*
        Needs to have "Open file(s) / folder(s)"
        Provide information about dumping, checksum, no-intro, piracy etc...
        Base lighting of sys mode (if not then default dark)
        Provide text display of success rate
        Use dialouge boxes on completion or fatal error
        allow local/network priority to be user assigned (default = network/local)
        Light advertisement ;) Patchy was a god and frankly unappreciated
    */
}