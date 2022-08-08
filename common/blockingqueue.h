// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

// from muduo
#ifndef _BLOCKINGQUEUE_H_
#define _BLOCKINGQUEUE_H_

#include <condition_variable>
#include<mutex>

#include <deque>
#include <assert.h>

namespace xistor
{

template<typename T>
class BlockingQueue
{
 public:
  using queue_type = std::deque<T>;

  BlockingQueue()
    : _mutex(),
      _notEmpty(),
      _queue()
  {
  }

  void put(const T& x)
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push_back(x);
    }

    _notEmpty.notify_one(); // wait morphing saves us
    // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
  }

  void put(T&& x)
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _queue.push_back(std::move(x));
    }

    _notEmpty.notify_one();
  }

  T take()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if(_queue.empty())
      _notEmpty.wait(lock, [this]{return !_queue.empty();});

    assert(!_queue.empty());
    T front(std::move(_queue.front()));
    _queue.pop_front();
    return front;
  }

  queue_type drain()
  {
    std::deque<T> queue;
    {
      std::lock_guard<std::mutex> lock(_mutex);
      queue = std::move(_queue);
      assert(_queue.empty());
    }
    return queue;
  }

  size_t size() const
  {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.size();
  }

 private:

  mutable std::mutex        _mutex;
  std::condition_variable   _notEmpty;
  queue_type                _queue;
};

}  // namespace xistor

#endif  // _BLOCKINGQUEUE_H_
