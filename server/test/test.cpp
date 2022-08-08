#include<iostream>
#include<string>
#include"translator.h"

using namespace xistor;



int main() {
    std::string url = "http://api.fanyi.baidu.com/api/trans/vip/translate?";
    std::string appid = "20170520000048515";    //replace myAppid with your own appid
    std::string secret_key = "sERmjSSdTNowRYvEABOr";

    translator tr(url, appid, secret_key);
    std::string res = tr.translate("en", "zh", "One is using JSON for something like a configuration file. In this case, we care primarily about the code being as simple and readable as possible. In this situation, benchmarks (for one example) are basically irrelevant—configuration files tend to be fairly small, and we’re typically only parsing the configuration file once during startup, so taking an extra 17 microseconds (or even 17 milliseconds) simply doesn’t matter. Having code that’s clean and readable matters a lot.");

    std::cout << res << std::endl;
}