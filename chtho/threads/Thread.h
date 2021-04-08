// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_THREAD_H
#define CHTHO_THREADS_THREAD_H

#include "base/noncopyable.h"
#include "threads/CountDownLatch.h"

#include <functional> // std::function
#include <pthread.h> // pthread_t 
#include <string>
#include <atomic> // atomic_int 

namespace chtho
{
class Thread : noncopyable
{
public:
  using Func = std::function<void()>;
private:
  bool started_;
  bool joined_;
  pthread_t pthreadID_;
  pid_t tid_;
  Func func_;
  std::string name_;
  CountDownLatch latch_;
  static std::atomic_int32_t numCreated_;

public:
  explicit Thread(Func, const std::string& name = std::string());
  ~Thread();
  void start();
  int join();

  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }

  // the read is atomic 
  static int numCreated() { return numCreated_; }
};

} // namespace chtho


#endif // !CHTHO_THREADS_THREAD_H