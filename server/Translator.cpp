#include <curl/curl.h>
#include <string.h>
#include <openssl/md5.h>
#include "Translator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "nlohmann/json.hpp"

using namespace xistor;

size_t Translator::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{

    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Translator::Translator(std::string url, std::string appid, std::string secret_key):
_url(url),
_appid(appid),
_secret_key(secret_key)
{


}

std::string Translator::send_request(const std::string &from, const std::string &to, const std::string &word) {
  
  CURLcode res;
  std::string read_buffer;
  read_buffer = "";
  _curl = curl_easy_init();
  curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &read_buffer);
  if(_curl) {

    std::string req;
    std::string ss;
    std::string query_word;
    
    for (size_t i = 0; i < word.size(); ++i) {
      if (word[i] == ' ')
          query_word += "%20";
      else
          query_word += word[i];
    }

    int a = rand();
    std::string salt = std::to_string(a);

    ss += _appid + word + salt + _secret_key;

    unsigned char md[16];
    MD5(reinterpret_cast<const unsigned char*>(ss.data()), ss.size(),md);


    std::stringstream sign;
    for (int i = 0; i < 16; i++){
        sign <<  std::setfill('0') << std::setw(2)  << std::hex << (int)md[i];
    }
  
    req += _url + "appid=" + _appid + "&q=" + query_word + "&from=" + from + "&to=" + to + "&salt=" + salt + "&sign=" + sign.str();

    std::cout << "req: "<< req << std::endl;
    curl_easy_setopt(_curl, CURLOPT_URL, req.c_str());


    res = curl_easy_perform(_curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(_curl);
  }

  return read_buffer;
}

std::string Translator::translate(const std::string &from, const std::string &to, const std::string &query_word) {

  return send_request(from, to, query_word);
}