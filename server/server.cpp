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
#include "nlohmann/json.hpp"
#include "translator.h"

#define PORT 9010
#define MAX_CLIENTS 10
#define BUF_SIZE 4096

unsigned int g_client_num = 0;
std::mutex g_mut;


void trans_thread(int socket_fd, xistor::translator *tr) {
    char buffer[BUF_SIZE] = { 0 };
    int readN = 0;

    while(readN = read(socket_fd, buffer, 4096)) {

        std::cout << " read " << readN << " char " << std::endl;
        if(readN) {
            buffer[readN] = '\0';
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
            std::string res = tr->translate(req["from"], req["to"], req["words"]);

            std::cout << "res " << res << std::endl;
            send(socket_fd, res.c_str(), res.length(), 0);
            sleep(1);   // for baidu api limit
        }
    }

    close(socket_fd);
}

int main(int argc, char const* argv[])
{
    int server_fd, new_socket = 0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    std::string config_path;

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

    xistor::translator tr(engines[0]["url"],
                        engines[0]["appid"], 
                        engines[0]["secret_key"]);
  
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
  
    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        std::cout << " waiting accept " << std::endl;
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        std::cout << " accepted " << std::endl;

        if (g_client_num < MAX_CLIENTS)
        {
            // create new thread
            std::thread t(trans_thread, new_socket, &tr);
            t.detach();
        }
        else
        {
            perror("too many cliens");
        }

    }
    

  // closing the connected socket
    close(new_socket);
  // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}