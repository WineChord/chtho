// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_ASYNCLOGGING_H
#define CHTHO_LOGGING_ASYNCLOGGING_H
// Pass logs from multiple front ends to the single back end

#include "base/noncopyable.h"
#include "Logger.h"
#include "LogFile.h"
#include "threads/Thread.h"

#include <vector> 
#include <memory> 
#include <atomic> 

namespace chtho
{
class AsyncLogging : noncopyable
{
private:
  using Buf = FixedBuffer<kLarge>;
  using BufVec = std::vector<std::unique_ptr<Buf>>;
  using BufPtr = std::unique_ptr<Buf>;

  const int flushInter_;
  std::atomic_bool running_;
  const std::string name_;
  const off_t rollsz_;
  Thread thread_;
  CountDownLatch latch_;
  MutexLock mutex_;
  Condition cond_;
  BufPtr curBuf_;
  BufPtr nxtBuf_;
  BufVec bufs_; 

  void threadFunc();

public:
  AsyncLogging(const std::string& name, off_t rollsz, int flushInter=3);
  ~AsyncLogging()
  {
    if(running_) stop();
  }
  void append(const char* line, int len);
  void start() 
  {
    running_ = true;
    thread_.start();
    latch_.wait();
  }
  void stop()
  {
    running_ = false;
    cond_.notify();
    thread_.join();
  }
};
} // namespace chtho
#endif // !CHTHO_LOGGING_ASYNCLOGGING_H