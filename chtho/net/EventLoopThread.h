// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_EVENTLOOPTHREAD_H
#define CHTHO_NET_EVENTLOOPTHREAD_H

#include "base/noncopyable.h"
#include "threads/Thread.h"
#include "threads/MutexLock.h"
#include "Callbacks.h"

#include <functional>

namespace chtho
{
namespace net
{
class EventLoop;

// EventLoopThread is a thread wrapper of EventLoop object
class EventLoopThread : noncopyable
{
private:
  EventLoop* loop_;
  bool exiting_;
  ThreadInitCB cb_;

  Thread thread_;
  MutexLock mutex_;
  Condition cond_;

  void func();
public:
  EventLoopThread(const ThreadInitCB& cb = ThreadInitCB(),
    const std::string& name = std::string());
  ~EventLoopThread();

  EventLoop* startLoop();
};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_EVENTLOOPTHREAD_H