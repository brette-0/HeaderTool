#include "backend.cpp"                                          // include core functionality         
#include <nlohmann/json.hpp>                                    // include json reading

namespace nl = nlohmann;                                        // tidy namespace for long name

nl::json getindex(const std::string& url) {
    std::string index;
    CURL* indexcurl = curl_easy_init();

    if (indexcurl) {
        curl_easy_setopt(indexcurl, CURLOPT_URL, url.c_str());
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
    nl::json index = getindex();
}