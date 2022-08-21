#include "TcpServer.h"
#include <netinet/in.h>

xistor::TcpServer::TcpServer(struct sockaddr_in listen_addr, const std::string& nameArg):
_listen_addr(listen_addr),
_name(nameArg)
{
    _runing = true;
    _poller.setEventCallbask(std::bind(&xistor::TcpServer::handlePollerEvent, this, std::placeholders::_1));
}


void xistor::TcpServer::start() {
    int opt = 1;

    // Creating socket file descriptor
    if ((_server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP))
        == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(_server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (bind(_server_fd, (struct sockaddr*)&_listen_addr,
             sizeof(_listen_addr))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // listen 
    if (listen(_server_fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    _poller.push_back({_server_fd, POLLIN, 0});

    loop();
}

void xistor::TcpServer::stop() {
    _runing = false;
}

int xistor::TcpServer::handlePollerEvent(int fd) {
    int rc = 0, new_conn;
    bool close_conn = false;

    if (fd == _server_fd)
    {
        // accept new client connect
        do
        {
            new_conn = accept(_server_fd, NULL, NULL);
            if (new_conn < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                perror("  accept() failed");
                }
                break;
            }

            if(fcntl(new_conn, F_SETFL, fcntl(new_conn, F_GETFL) | O_NONBLOCK) < 0) {
                perror(" set O_NONBLOCK failed\n");
            }

            printf("  New incoming connection - %d\n", new_conn);

            _poller.push_back({new_conn, POLLIN, 0});

        } while (0);

        return 1;
    } else {

        printf("  Descriptor %d is readable\n", fd);

        rc = 1;
        if(_msgCallBack)
            rc = _msgCallBack(fd);

        if (rc < 0)
        {
            if (errno != EWOULDBLOCK)
            {
                perror("  recv() failed");
                close_conn = true;
            }
        }

        if(rc == 0){
            close_conn = true;    
        }

        if(close_conn) {
            // connetion close
            close(fd);
            _poller.erase(fd);
        }

        return rc;
    }


    return 1;
}

void xistor::TcpServer::loop() {
    int rc = 0, current_size;
    int new_conn;

    while(_runing) {
        _poller.poll();
    }
}


