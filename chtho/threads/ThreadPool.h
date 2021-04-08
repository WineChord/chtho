// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_THREADPOOL_H
#define CHTHO_THREADS_THREADPOOL_H

#include "base/noncopyable.h"
#include "threads/MutexLock.h"
#include "threads/Condition.h"
#include "threads/Thread.h"

#include <string>
#include <functional> // std::function
#include <vector>
#include <deque> 
#include <memory> // unique_ptr

namespace chtho
{
// how to use this class:
// 1. first construct a thread pool object
// ThreadPool pool("your fancy thread pool name");
// 2. give the number of threads you want it to have
// pool.start(5);
// 3. give your task(a function) to pool
// pool.run(yourTask);
class ThreadPool : noncopyable
{
public:
  using Task = std::function<void()>; 
private:
  mutable MutexLock mutex_; 
  Condition notEmpty_;
  Condition notFull_;
  std::string name_;
  Task threadInitCB; // callback 
  std::vector<std::unique_ptr<Thread>> threads_;
  std::deque<Task> queue_; 
  size_t maxQueueSz_; // maximum queue size
  bool running_;

  void runInThread();
  // draw a task from task queue
  Task take();
  bool isFull() const 
  {
    mutex_.assertLocked();
    return maxQueueSz_ > 0 && queue_.size() >= maxQueueSz_;
  }
public:
  explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
  ~ThreadPool();
  void start(int numThreads);
  void stop();
  void run(Task task);
  void setMaxQueueSz(int maxSize) { maxQueueSz_ = maxSize; }
  void setThreadInitCB(const Task& cb) { threadInitCB = cb; }
  const std::string& name() const { return name_; }
  size_t queueSz() const;
};
} // namespace chtho


#endif // !CHTHO_THREADS_THREADPOOL_H