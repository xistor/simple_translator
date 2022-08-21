 
#ifndef _POLLER_H_
#define _POLLER_H_

#include <vector>
#include <map>
#include <poll.h>
#include <functional>
#include <unistd.h>

namespace xistor {

/*
paramter: 
    fd: which have event 
return value：
    0： will close connection
*/
typedef std::function<int (int fd)> EventCallBack;

class Poller {

    public:
        void push_back(struct pollfd);
        void erase(int fd);
        struct pollfd* get_data();
        size_t size();
        void poll();
        void setEventCallbask(const EventCallBack &cb);
    private:

        typedef std::vector<struct pollfd> PollFdList;

        PollFdList              _pollfds;
        std::map<int, int>      _fd_idx;    // fd's index in _pollfds
        EventCallBack           _event_callback;

};

}   // namespace xistor

#endif // _POLLER_H_