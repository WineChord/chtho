// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_THREADS_THREADDATA_H
#define CHTHO_THREADS_THREADDATA_H

#include "threads/Thread.h"
#include "threads/CurrentThread.h"

#include <sys/prctl.h> // prctl 

namespace chtho
{
struct ThreadData
{
  using Func = Thread::Func;
  Func func_;
  std::string name_;
  pid_t* tid_;
  CountDownLatch* latch_;
  ThreadData(Func func, const std::string& name, pid_t* tid, CountDownLatch* latch)
    : func_(std::move(func)),
      name_(name),
      tid_(tid),
      latch_(latch)
  {}
  void runInThread()
  {
    *tid_ = CurrentThread::tid(); // update tid in calling thread
    tid_ = NULL; // avoid dangling pointer
    latch_->countDown();
    latch_ = NULL; // aboid dangling pointer 

    CurrentThread::thread_name = name_.empty() ? "chtho_thread" : name_.c_str();
    // prctl: control process execution
    // PR_SET_NAME: set process name 
    ::prctl(PR_SET_NAME, CurrentThread::thread_name); 
    try
    {
      // so ThreadData is actually a wrapper class
      func_();
      CurrentThread::thread_name = "done";
    }
    catch(const std::exception& e)
    {
      CurrentThread::thread_name = "crash";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", e.what());
      abort();
    }
    catch(...)
    {
      CurrentThread::thread_name = "crash";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw;
    }
    
  }
};

// this is called by Thread::start
// inside Thread::start, it creates a thread to run 
// this function
// the actual aim is to run func_ 
// so these stuff including ThreadData are all
// means to provide a wrapper of func_
// (so as to provide exception handling)
// also, the true purpose of ThreadData is to
// provide a pointer, so that it can be passed
// as an argument into pthread_create.
void* startThread(void* arg)
{
  ThreadData* data = static_cast<ThreadData*>(arg);
  data->runInThread();
  delete data;
  return NULL;
}
  
} // namespace chtho


#endif // !CHTHO_THREADS_THREADDATA_H