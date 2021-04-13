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
  bool waitForSecs(double secs)
  {
    struct timespec abstime; 
    /* Get current value of clock CLOCK_ID and store it in TP.  */
    clock_gettime(CLOCK_REALTIME, &abstime);
    const int64_t nsPerSec = 1000000000;
    int64_t ns = static_cast<int64_t>(secs*nsPerSec);
    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec+ns)/nsPerSec);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec+ns)%nsPerSec);
    RevokeGuard rg(mutex_);
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.pthreadMutex(), &abstime);
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
