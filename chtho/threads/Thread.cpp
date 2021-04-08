// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Thread.h"
#include "ThreadData.h"

#include "logging/Logger.h"

namespace chtho
{
std::atomic_int32_t Thread::numCreated_;
Thread::Thread(Func func, const std::string& name)
  : started_(false),
    joined_(false),
    pthreadID_(0),
    tid_(0),
    func_(std::move(func)),
    name_(name),
    latch_(1)
{
  // set default name 
  int num = ++numCreated_; // atomic_int has overloaded operator++()
  if(name_.empty()) 
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "Thread%d", num);
    name_ = buf; 
  }
}

Thread::~Thread()
{
  if(started_ && !joined_)
    pthread_detach(pthreadID_);
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
  if(pthread_create(&pthreadID_, NULL, &startThread, data))
  {
    started_ = false;
    delete data;
    LOG_SYSFATAL << "failed in pthread_create";
  }
  else   
  {
    // wait until tid is updated
    // see ThreadData::runInLoop
    latch_.wait(); 
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadID_, NULL);
}
} // namespace chtho
