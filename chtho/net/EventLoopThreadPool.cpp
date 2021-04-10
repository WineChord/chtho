// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <assert.h>

namespace chtho
{
namespace net
{
EventLoopThreadPool::EventLoopThreadPool(EventLoop* base, const std::string name)
  : base_(base),
    name_(name),
    started_(false),
    numThreads_(0),
    nxt_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start(const ThreadInitCB& cb)
{
  assert(!started_);
  base_->assertInLoopThread();
  started_ = true;
  for(int i = 0; i < numThreads_; i++)
  {
    char buf[name_.size() + 32];
    snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
    EventLoopThread* t = new EventLoopThread(cb, buf);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
  }
  if(numThreads_ == 0 && cb) cb(base_);
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
  base_->assertInLoopThread();
  assert(started_);
  EventLoop* loop = base_;
  if(!loops_.empty())
  {
    loop = loops_[nxt_];
    ++nxt_;
    if(static_cast<size_t>(nxt_) >= loops_.size())
      nxt_ = 0;
  }
  return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode)
{
  base_->assertInLoopThread();
  EventLoop* loop = base_;
  if(!loops_.empty())
  {
    loop = loops_[hashCode % numThreads_];
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
  base_->assertInLoopThread();
  assert(started_);
  if(loops_.empty())
    return {base_};
  else return loops_;
}

} // namespace net
} // namespace chtho
