// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/net/EventLoop.h"
#include "chtho/net/EventLoopThreadPool.h"

#include <unistd.h> // sleep 

using namespace chtho;
using namespace chtho::net;

void print(EventLoop* p = nullptr)
{
  LOG_INFO << "loop = " << p;
}

void init(EventLoop* p)
{
  LOG_INFO << "loop = " << p;
}

int main()
{
  print();
  EventLoop loop;
#if __cplusplus >= 201402L
  loop.runAfter(11, [p=&loop](){p->quit();});
#else  
  loop.runAfter(11, [&loop](){loop.quit();});
#endif
  {
    LOG_INFO << "single thread: " << &loop;
    EventLoopThreadPool pool(&loop, "single");
    pool.setNumThread(0);
    pool.start(init);
    assert(pool.getNextLoop() == &loop);
    assert(pool.getNextLoop() == &loop);
    assert(pool.getNextLoop() == &loop);
  }
  {
    LOG_INFO << "another thread: " << &loop;
    EventLoopThreadPool pool(&loop, "another");
    pool.setNumThread(1);
    pool.start(init);
    EventLoop* nxt = pool.getNextLoop();
    nxt->runAfter(2, [nxt](){print(nxt);});
    assert(&loop != nxt);
    assert(pool.getNextLoop() == nxt);
    assert(pool.getNextLoop() == nxt);
    ::sleep(3);
  }
  {
    LOG_INFO << "three thread: " << &loop;
    EventLoopThreadPool pool(&loop, "three");
    pool.setNumThread(3);
    pool.start(init);
    EventLoop* nxt = pool.getNextLoop();
    nxt->runInLoop([nxt](){print(nxt);});
    assert(&loop != nxt);
    assert(pool.getNextLoop() != nxt);
    assert(pool.getNextLoop() != nxt);
    assert(pool.getNextLoop() == nxt);
  }
  loop.loop();
}