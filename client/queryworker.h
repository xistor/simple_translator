#ifndef _QUERYWORKER_H_
#define _QUERYWORKER_H_

#include "blockingqueue.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "nlohmann/json.hpp"
#include <unistd.h>
#include <thread>

#define BUFSIZE 4096

namespace xistor
{

class QueryMsg {

public:
    std::string from;
    std::string to;
    std::string words;

    QueryMsg(const std::string& from, const std::string& to, const std::string& words )
    :from(from),
    to(to),
    words(words)
    {}


    nlohmann::json toJson(){
        nlohmann::json j;
        j["from"] = from;
        j["to"] = to;
        j["words"] = words;
        return j;
    }

};

class ResponseMsg {
    
public:
    std::string error_code;
    std::string error_msg;
    std::string from;
    std::string to;
    std::string src;
    std::string dst;

    ResponseMsg(std::string response) : error_code{}
    {
        nlohmann::json res_json;
        try {
            res_json = nlohmann::json::parse(response);
        }
        catch (std::exception& e)
        {
            // output exception information
            std::cout<< __FILE__ << " message: " << e.what() << '\n';
                  
        }

        if(res_json.contains("error_code")){
            error_code = res_json["error_code"];
            error_msg = res_json["error_msg"];
        } else {
            error_code.clear();
            from = res_json["from"];
            to = res_json["to"];
            src = res_json["trans_result"][0]["src"];
            dst = res_json["trans_result"][0]["dst"];
        }
    }
};


class QueryWorker {

public:
    QueryWorker(){}

    QueryWorker(const std::string& srv_addr, const uint16_t srv_port, BlockingQueue<ResponseMsg>* res_queue_ptr):
    _running(true),
    _srv_addr(srv_addr),
    _srv_port(srv_port),
    _res_queue_ptr(res_queue_ptr),
    _query_thread{}
    {
        
    }


    int start(){
        // connect server
        if ((_sockt_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }
        
        memset(&_sock_addr, '0', sizeof(_sock_addr));
 
        _sock_addr.sin_family = AF_INET;
        _sock_addr.sin_port = htons(_srv_port);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if(inet_pton(AF_INET, _srv_addr.c_str(), &_sock_addr.sin_addr)<=0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }

        if (connect(_sockt_fd, (struct sockaddr *)&_sock_addr, sizeof(_sock_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            return -1;
        }

        // start query thread
        _query_thread = std::thread{&QueryWorker::run, this};

        return 0;
    }

    void stop(){
        _query_queue.put({"","",""});
        _running = false;
    }

    void set_srv(const std::string& srv_addr, const uint16_t srv_port) {
        _srv_addr = srv_addr;
        _srv_port = srv_port;

    }

    void query(const QueryMsg& qmsg) {
        std::cout << qmsg.from << " " << qmsg.to << " " << qmsg.words << std::endl;
        _query_queue.put(qmsg);
    }

    void run(){
        int readN;
        while(_running) {
            std::cout << "take " << std::endl;
            auto q = _query_queue.take();

            auto s = q.toJson().dump();
            std::cout << "query " << s << std::endl;

            send(_sockt_fd , s.c_str() , s.length(), 0 );
            std::cout << "send " << s << std::endl;
            readN = read(_sockt_fd , _buffer, BUFSIZE);
            if(readN) {
                _buffer[readN] = '\0';

                std::cout << "response " << _buffer << std::endl;
                ResponseMsg resMsg(_buffer);

                if(_res_queue_ptr != nullptr)
                    _res_queue_ptr->put(resMsg);
            }
            
        }
        std::cout << "stop " << std::endl;
    }

    ~QueryWorker(){
        if(_running){
            _running = false;
        }
        _query_thread.join();
    }

private:

    bool                        _running;
    std::string                 _srv_addr;
    uint16_t                    _srv_port;
    struct sockaddr_in          _sock_addr;
    int                         _sockt_fd;
    char                        _buffer[BUFSIZE];
    BlockingQueue<QueryMsg>     _query_queue;
    BlockingQueue<ResponseMsg>* _res_queue_ptr;
    std::thread                 _query_thread;
};

}  // namespace xistor

#endif  // _BLOCKINGQUEUE_H_