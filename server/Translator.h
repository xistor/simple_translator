#include <curl/curl.h>
#include <string>

namespace xistor {


class translator {

public: 
    translator(std::string url, std::string appid, std::string secret_key);

    // send query request to baidu/google server, return raw response
    std::string send_request(const std::string &from, const std::string &to, const std::string &word);

    // translate, return translated word
    std::string translate(const std::string &from, const std::string &to, const std::string &word);
private:
    // curl write data callback
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

    CURL *_curl;
    std::string _url;
    std::string _appid;
    std::string _secret_key;


    translator(const translator& t) = delete;
    
};

}