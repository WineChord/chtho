// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/net/EventLoop.h"
#include "chtho/net/EventLoopThread.h"

#include <unistd.h> // getpid 

using namespace chtho;
using namespace chtho::net;

void print(EventLoop* p)
{
  LOG_INFO << "loop = " << p;
}

int main()
{
  print(nullptr);
  {
    EventLoopThread t1; // never starts
  }

  {
    // dtor will call quit() 
    EventLoopThread t2;
    EventLoop* loop = t2.startLoop();
    loop->runInLoop(
      [loop](){print(loop);});
    CurrentThread::sleepUs(500*1000); // sleep 500 ms 
  }

  {
    // quit() called before dtor
    EventLoopThread t3;
    EventLoop* loop = t3.startLoop();
    loop->runInLoop([loop](){print(loop);loop->quit();});
    CurrentThread::sleepUs(500*1000); // sleep 500 ms 
  }
  
}