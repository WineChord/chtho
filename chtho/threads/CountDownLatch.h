// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_COUNTDOWNLATCH_H
#define CHTHO_THREADS_COUNTDOWNLATCH_H

#include "base/noncopyable.h"
#include "threads/Condition.h"
#include "threads/MutexLock.h"

namespace chtho
{
class CountDownLatch : noncopyable
{
private:
  mutable MutexLock mutex_;
  Condition cond_;
  int count_;
public:
  explicit CountDownLatch(int count);
  void wait();
  void countDown();
  int getCnt() const;
};

} // namespace chtho


#endif // !CHTHO_THREADS_COUNTDOWNLATCH_H