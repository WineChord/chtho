// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/net/EventLoop.h"
#include "chtho/threads/Thread.h"
#include "chtho/logging/Logger.h"

#include <cstdio>
#include <unistd.h> // getpid
#include <assert.h> // assert

using namespace chtho;
using namespace chtho::net;

void cb() // callback function 
{
  LOG_DEBUG << "tid = " << CurrentThread::tid();
  // should abort because one thread in only
  // allowed to have one eventloop 
  EventLoop anotherLoop; 
}

void threadFunc()
{
  LOG_DEBUG << "tid = " << CurrentThread::tid();
  assert(EventLoop::eventLoopOfThisThread() == nullptr);
  EventLoop loop;
  assert(EventLoop::eventLoopOfThisThread() == &loop);
  loop.runAfter(1.0, cb);
  loop.loop();
}

int main()
{
  chtho::Logger::setLogLevel(chtho::Logger::Level::TRACE);
  LOG_DEBUG << "tid = " << CurrentThread::tid();
  assert(EventLoop::eventLoopOfThisThread() == nullptr);
  EventLoop loop;
  LOG_DEBUG << "loop initialized";
  assert(EventLoop::eventLoopOfThisThread() == &loop);

  Thread thread(threadFunc);
  thread.start();
  loop.loop();
}