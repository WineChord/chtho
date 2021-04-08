// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_MUTEXLOCKGUARD_H
#define CHTHO_THREADS_MUTEXLOCKGUARD_H

#include "threads/MutexLock.h"

namespace chtho
{
class MutexLockGuard : noncopyable
{
private:
  MutexLock& mutex_;
public:
  MutexLockGuard(MutexLock& mutex) : mutex_(mutex) 
  { mutex_.lock(); }
  ~MutexLockGuard() { mutex_.unlock(); }
};
} // namespace chtho


#endif // !CHTHO_THREADS_MUTEXLOCKGUARD_H