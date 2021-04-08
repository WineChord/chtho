// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "base/CurrentThread.h"
#include "base/noncopyable.h"
#include <atomic>

namespace chtho
{
class EventLoop : noncopyable
{
private:
  pid_t threadID; // each thread can only own one loop 
  std::atomic<bool> looping;
public:
  EventLoop();
  ~EventLoop();
  void loop(); 
  void assertInLoopThread()
  {
    if(!isInLoopThread())
      abortNotInLoopThread();
  }
  bool isInLoopThread() const { return threadID == CurrentThread::tid(); }
  void abortNotInLoopThread(); 
};

EventLoop::EventLoop()
{
}

EventLoop::~EventLoop()
{
}

} // namespace chtho
