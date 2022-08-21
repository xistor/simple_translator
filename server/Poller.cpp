#include "Poller.h"


void xistor::Poller::push_back(struct pollfd pfd) {

    _pollfds.push_back(pfd);
    _fd_idx[pfd.fd] = _pollfds.size() - 1;

}

void xistor::Poller::erase(int fd) {
    if(_fd_idx[fd] == _pollfds.size() - 1){
        _pollfds.pop_back();
    }
    else {
        iter_swap(_pollfds.begin() + _fd_idx[fd], _pollfds.end() - 1);
        _pollfds.pop_back();
    }

    // _fd_idx.erase(fd);
}

struct pollfd* xistor::Poller::get_data(){
    return &*_pollfds.begin();
}

size_t xistor::Poller::size() {
    return _pollfds.size();
}

void xistor::Poller::setEventCallbask(const xistor::EventCallBack &cb){
    _event_callback = cb;
}

void xistor::Poller::poll() {
    int numEvents = ::poll(&*_pollfds.begin(), _pollfds.size(), -1);
    if (numEvents > 0)
    {
        for (size_t i = 0; i < _pollfds.size() && numEvents > 0; i++)
        {
            if (_pollfds[i].revents > 0)
            {
                --numEvents;
                if(_pollfds[i].revents != POLLIN)
                {
                    printf("  Error! revents = %d\n", _pollfds[i].revents);
                    break;

                }

                int rc = 0;
                if(_event_callback) {
                    rc = _event_callback(_pollfds[i].fd);

                }
                   
            }
        }
    }
}