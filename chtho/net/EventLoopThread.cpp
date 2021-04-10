// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EventLoopThread.h"

#include "EventLoop.h"
#include "threads/MutexLockGuard.h"

namespace chtho
{
namespace net
{
EventLoopThread::EventLoopThread(const ThreadInitCB& cb,
  const std::string& name)  
  : loop_(nullptr),
    exiting_(false),
    cb_(cb),
    thread_([this](){this->func();}, name),
    mutex_(),
    cond_(mutex_)
{}

EventLoopThread::~EventLoopThread()
{
  exiting_ = true;
  // not guaranteed to be thread-safe
  // func could be running cb_ 
  if(loop_ != nullptr)
  {
    // if func exits just now
    // there is a chance that loop_ is nullptr
    // here, but when EventLoopThread destructs,
    // usually the whole program ends
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop()
{
  assert(!thread_.started());
  thread_.start();
  EventLoop* loop = nullptr;
  {
    // wait until the thread function 
    // has truly started 
    MutexLockGuard lock(mutex_);
    while(loop_ == nullptr) cond_.wait();
    loop = loop_;
  }
  return loop;
}

// thread function. it will create a EventLoop object
// and starts to loop 
void EventLoopThread::func()
{
  EventLoop loop;
  // cb_ is thread initialization function 
  if(cb_) cb_(&loop);
  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    // notify the creator that the thread function
    // has truly started 
    cond_.notify(); 
  }
  loop.loop();
  MutexLockGuard lock(mutex_);
  loop_ = nullptr;
}
} // namespace net
} // namespace chtho
