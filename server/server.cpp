#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <fstream>
#include <poll.h>
#include <fcntl.h>
#include <list>
#include "nlohmann/json.hpp"
#include "Translator.h"
#include "TcpServer.h"

#define PORT 9010
#define MAX_CLIENTS 100
#define BUF_SIZE 4096


xistor::Translator g_tr;
                
int handle_request(int client_fd) {
    int rc;
    char buffer[4096];
    do
    {

        rc = recv(client_fd, buffer, sizeof(buffer), 0);
        if (rc <= 0)
        {
           return rc;
        }

        int len = rc;
        printf("  %d bytes received\n", len);

        // translate and send back
        buffer[len] = '\0';
        std::cout << "buffer " << buffer << std::endl;
        nlohmann::json req;
        try {
            req = nlohmann::json::parse(buffer);
        }
        catch (std::exception& e)
        {
            // output exception information
            std::cout << __FILE__ << " exception: " << e.what() << '\n';
        }

        std::cout << "req " << req.dump() << std::endl;
        std::string res = g_tr.translate(req["from"], req["to"], req["words"]);

        std::cout << "res " << res << std::endl;
        send(client_fd, res.c_str(), res.length(), 0);
        sleep(1);   // for baidu api limit

    } while(rc);

    return 1;
}

int main(int argc, char const* argv[])
{
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    std::string config_path;
    struct pollfd fds[MAX_CLIENTS + 1];
    int    nfds = 1, current_size = 0;
    char buffer[BUF_SIZE] = { 0 };
    bool compress_array = false, close_conn = false;
    int rc =0;

    if(argc < 2) {
        config_path = "config.json";
    } else {
        config_path = std::string(argv[1]);
    }

    std::ifstream f(config_path);

    if(!f.is_open())
    {
        std::cout << config_path << " dose not exist!" << std::endl;
        return 0;
    } else {
        std::cout << "use config : " << config_path << std::endl;
    }

    nlohmann::json config = nlohmann::json::parse(f);



    nlohmann::json engines = config["server"]["engine"];

    g_tr.set_engine(engines[0]["url"],
                    engines[0]["appid"], 
                    engines[0]["secret_key"]);


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  

    xistor::TcpServer transServer(address, "transServer");

    transServer.setMessageCallBack(handle_request);

    transServer.start();


    return 0;
}