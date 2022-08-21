#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include <netinet/in.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string>
#include <functional>
#include <atomic>
#include <map>
#include "Poller.h"

namespace xistor {

typedef std::function<int (int client_fd)> MessageCallBack; // return readed byte number


class TcpServer {

public:
    TcpServer(struct sockaddr_in listen_addr, const std::string& nameArg);

    void start();
    void stop();

    void setMessageCallBack(const MessageCallBack &cb) { _msgCallBack = cb; }

private:
    int handlePollerEvent(int fd);

    void loop();

    std::string             _name;
    int                     _server_fd;
    struct sockaddr_in      _listen_addr;
    std::atomic<bool>       _runing;
    MessageCallBack         _msgCallBack;
    Poller                  _poller;

};

}   // namespace xistor

#endif // _TCPSERVER_H_