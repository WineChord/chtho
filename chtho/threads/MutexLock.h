// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_MUTEXLOCK_H
#define CHTHO_THREADS_MUTEXLOCK_H

#include "base/noncopyable.h"
#include "threads/CurrentThread.h"

#include <pthread.h> // pthread_mutex_t 
#include <assert.h> // assert_perror

#define MCHECK(ret) do{\
  __typeof__ (ret) errnum = (ret);\
  if(__glibc_unlikely(errnum != 0))\
    assert_perror(errnum);\
}while(0)

namespace chtho
{
class RevokeGuard;
class MutexLock : noncopyable
{
private:
  friend class RevokeGuard;
  pthread_mutex_t mutex_;
  pid_t holder_;
  void assignHolder() { holder_ = CurrentThread::tid(); }
  void revokeHolder() { holder_ = 0; }
public:
  MutexLock() : holder_(0)
  {
    MCHECK(pthread_mutex_init(&mutex_, NULL));
  }
  ~MutexLock()
  {
    assert(holder_ == 0);
    MCHECK(pthread_mutex_destroy(&mutex_));
  }
  void lock()
  {
    MCHECK(pthread_mutex_lock(&mutex_));
    assignHolder();
  }
  void unlock()
  {
    revokeHolder();
    MCHECK(pthread_mutex_unlock(&mutex_));
  }
  pthread_mutex_t* pthreadMutex() { return &mutex_; }
  void assertLocked() const 
  {
    assert(isLockedByThisThread());
  }
  bool isLockedByThisThread() const 
  {
    return holder_ == CurrentThread::tid();
  }
};

} // namespace chtho


#endif // !CHTHO_THREADS_MUTEXLOCK_H