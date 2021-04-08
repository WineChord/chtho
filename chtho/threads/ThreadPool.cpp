// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "ThreadPool.h"
#include "threads/MutexLockGuard.h"

namespace chtho
{
// the ctor of ThreadPool only initializes its 
// members 
ThreadPool::ThreadPool(const std::string& name)
  : mutex_(),
    notEmpty_(mutex_), // two condition variables 
    notFull_(mutex_), 
    name_(name),
    maxQueueSz_(0),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if(running_) stop();
}

void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for(int i = 0; i < numThreads; i++)
  {
    char id[32]; // store thread index 
    snprintf(id, sizeof(id), "%d", i+1);
    // threads_ is of type vector<unique_ptr<Thread>>
    auto f = [this](){this->runInThread();};
    threads_.emplace_back(new Thread(f, name_+id));
    // threads_.emplace_back(new Thread(
      // std::bind(&ThreadPool::runInThread, this), name_+id));
    threads_[i]->start(); //! here starts the thread
  }
  if(numThreads == 0 && threadInitCB)
    threadInitCB(); 
}

void ThreadPool::stop()
{
  {
    MutexLockGuard lock(mutex_);
    // first set flag
    running_ = false;
    // if the task queue is empty, 
    // all threads will be block on wait
    // see ThreadPool::take
    // then they will run out of two
    // while loops and reach the end
    // (one inside take(), one inside runInLoop())
    notEmpty_.notifyAll();
    notFull_.notifyAll();
  }
  for(auto& t : threads_)
    t->join();
}

// get the thread running
// during running, it will continuously fetch task
// from task queue and execute it
// this will be executed by each thread 
void ThreadPool::runInThread()
{
  try
  {
    if(threadInitCB) threadInitCB();
    while(running_)
    {
      Task task(take());
      if(task) task();
    }
  }
  catch(const std::exception& e)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", e.what());
    abort();
  }
  catch(...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw;
  }
}

// called by runInThread
// it will check whether the task queue is empty
// if it is empty, then it will sleep on the 
// cond var notEmpty, meaning that it is waiting
// for someone to inform that the task queue is 
// able to draw task again.
// there are two places where this notEmpty cond var
// can be wake up:
// 1. inside ThreadPool::run, this is called directly
// by the client to add new task to the queue
// 2. inside ThreadPool::stop, in order to stop each
// thread, notEmpty should be woken up to check the
// running flag again so as to break the loop and exit.
ThreadPool::Task ThreadPool::take()
{
  MutexLockGuard lock(mutex_);
  // this wait can be woken up in stop()
  while(queue_.empty() && running_)
    notEmpty_.wait();
  Task task;
  if(!queue_.empty())
  {
    // take out a task in the front of the queue
    task = queue_.front();
    queue_.pop_front(); 
    if(maxQueueSz_ > 0)
      notFull_.notify();
  }
  return task;
}

void ThreadPool::run(Task task)
{
  if(threads_.empty()) // no other threads available
    task(); // current thread itself will handle the task
  else 
  {
    MutexLockGuard lock(mutex_);
    // if task queue is full, it will need to sleep
    // on notFull_. it can be woken up at two places:
    // 1. inside ThreadPool::take, obviously if a task
    // is taken out of the task queue, then new task
    // can be push to the task queue. this is the normal
    // case.
    // 2. inside ThreadPool::stop, upon woken up, it will
    // immediately check the running_ flag. if it is false,
    // the notify is sent in function stop. so function run
    // will return.
    while(isFull() && running_)
      notFull_.wait();
    if(!running_) return; // indicating that stop() sends the notify
    assert(!isFull()); 
    queue_.push_back(std::move(task));
    notEmpty_.notify(); // notify that the task queue is available again
    // it will wake up one of the threads blocking inside 
    // ThreadPool::take
  }
}

size_t ThreadPool::queueSz() const
{
  MutexLockGuard lock(mutex_);
  return queue_.size();
}
} // namespace chtho
