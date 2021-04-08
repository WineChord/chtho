// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_CONDITION_H
#define CHTHO_THREADS_CONDITION_H

#include "base/noncopyable.h"
#include "threads/MutexLock.h"
#include "threads/RevokeGuard.h"

#include <pthread.h> // pthread_cond_t 

namespace chtho
{
class Condition : noncopyable
{
private:
  MutexLock& mutex_;
  pthread_cond_t cond_;
public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex)
  {
    MCHECK(pthread_cond_init(&cond_, NULL));
  }
  ~Condition() 
  {
    MCHECK(pthread_cond_destroy(&cond_));
  }

  void wait()
  {
    RevokeGuard revoke(mutex_);
    MCHECK(pthread_cond_wait(&cond_, mutex_.pthreadMutex()));
  }
  void notify()
  {
    MCHECK(pthread_cond_signal(&cond_));
  }
  void notifyAll()
  {
    MCHECK(pthread_cond_broadcast(&cond_));
  }
};

} // namespace chtho


#endif // !CHTHO_THREADS_CONDITION_H
