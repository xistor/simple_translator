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

#define PORT 9010
#define MAX_CLIENTS 100
#define BUF_SIZE 4096

unsigned int g_client_num = 0;
std::mutex g_mut;



int main(int argc, char const* argv[])
{
    int server_fd, new_socket = 0;
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
  
    // Set socket to be nonblocking.
    opt = opt | O_NONBLOCK;
    fcntl(server_fd, F_SETFL, opt);

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

    memset(fds, 0 , sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while(true) {
        std::cout << " waiting on poll() ... " << std::endl;
        rc = poll(fds, nfds, -1);

        if (rc < 0)
        {
            perror("  poll() failed");
            break;
        }

        current_size = nfds;

        for (int i = 0; i < current_size; i++)
        {
            // check if there are event
            if(fds[i].revents == 0)
                continue;

            if(fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
                break;

            }

            if (fds[i].fd == server_fd)
            {

                do
                {

                    new_socket = accept(server_fd, NULL, NULL);
                    if (new_socket < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                        perror("  accept() failed");
                        }
                        break;
                    }

                    if(fcntl(new_socket, F_SETFL, fcntl(new_socket, F_GETFL) | O_NONBLOCK) < 0) {
                        perror(" set O_NONBLOCK failed\n");
                    }

                    printf("  New incoming connection - %d\n", new_socket);
                    fds[nfds].fd = new_socket;
                    fds[nfds].events = POLLIN;
                    nfds++;

                } while (new_socket != -1);
            } else
            {
                printf("  Descriptor %d is readable\n", fds[i].fd);
                close_conn = false;

                do
                {

                    rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                        perror("  recv() failed");
                        close_conn = true;
                        }
                        break;
                    }


                    if (rc == 0)
                    {
                        printf("  Connection closed\n");
                        close_conn = true;
                        break;
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
                    std::string res = tr.translate(req["from"], req["to"], req["words"]);

                    std::cout << "res " << res << std::endl;
                    send(fds[i].fd, res.c_str(), res.length(), 0);
                    sleep(1);   // for baidu api limit

                } while(rc);

                if (close_conn)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = true;
                }


            }
        }

        if (compress_array)
        {
            compress_array = false;
            for (int i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(int j = i; j < nfds-1; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }

    }


  // closing the connected socket
    close(new_socket);
  // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);
    return 0;
}