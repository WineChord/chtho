// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "CountDownLatch.h"
#include "threads/MutexLockGuard.h"

namespace chtho
{
// the primary utility of CountDownLatch is the provide
// synchronization between threads 
// count_ is the number of resources
// wait() will block until count_ reaches zero
// inside other thread, they may decrement count_ 
// using CountDownLatch::countDown() to indicate
// they have finished something
CountDownLatch::CountDownLatch(int count)
  : mutex_(),
    cond_(mutex_), // the init of cond var needs a mutex 
    count_(count)
{}

void CountDownLatch::wait()
{
  MutexLockGuard lock(mutex_);
  while(count_ > 0)
    cond_.wait();
}

void CountDownLatch::countDown()
{
  MutexLockGuard lock(mutex_);
  --count_;
  if(count_ == 0)
    cond_.notifyAll();
}

int CountDownLatch::getCnt() const 
{
  MutexLockGuard lock(mutex_);
  return count_;
}

} // namespace chtho
