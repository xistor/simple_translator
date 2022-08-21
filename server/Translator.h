#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_


#include <curl/curl.h>
#include <string>

namespace xistor {


class Translator {

public: 
    Translator(){}
    Translator(std::string url, std::string appid, std::string secret_key);

    void set_engine(std::string url, std::string appid, std::string secret_key){ _url = url; _appid = appid; _secret_key = secret_key; }

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


    Translator(const Translator& t) = delete;
    
};

}   // namespace xistor

#endif // _TRANSLATOR_H_