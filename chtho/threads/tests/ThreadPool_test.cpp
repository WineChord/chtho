// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/threads/ThreadPool.h"
#include "chtho/threads/CountDownLatch.h"
#include "chtho/threads/CurrentThread.h"
#include "chtho/logging/Logger.h"

#include <cstdio>
#include <unistd.h> // usleep 

void print()
{
  printf("tid=%d\n", chtho::CurrentThread::tid());
}

void printStr(const std::string& str)
{
  LOG_INFO << str;
  usleep(100*1000);
}

void test(int maxSize)
{
  LOG_WARN << "test ThreadPool with max queue size = " << maxSize;
  chtho::ThreadPool pool("MaxThreadPool");
  pool.setMaxQueueSz(maxSize);
  pool.start(5);

  LOG_WARN << "adding";
  pool.run(print);
  pool.run(print);
  for(int i = 0; i < 100; i++)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "task %d", i);
    auto f = [&buf](){ printStr(std::string(buf)); };
    pool.run(f);
  }
  LOG_WARN << "done";
  chtho::CountDownLatch latch(1);
  auto f = [&latch](){ latch.chtho::CountDownLatch::countDown(); };
  pool.run(f);
  latch.wait();
  pool.stop();
}

void longTask(int num)
{
  LOG_INFO << "longTask " << num;
  chtho::CurrentThread::sleepUs(3000000); // 3s
}

void test2()
{
  LOG_WARN << "test ThreadPool by stoping early";
  chtho::ThreadPool pool("ThreadPool");
  pool.setMaxQueueSz(5);
  pool.start(3);

  chtho::Thread thread1([&pool]()
  {
    for(int i = 0; i < 20; i++) 
    {
      auto f = [i](){ longTask(i); };
      pool.run(f);
    }
  }, "thread1");
  thread1.start();
  chtho::CurrentThread::sleepUs(5000000); // 5s
  LOG_WARN << "stop pool";
  pool.stop(); // early stop 
  thread1.join();
  pool.run(print); // run() after stop()
  LOG_WARN << "test2 done";
}

int main()
{
  test(0);
  test(1);
  test(5);
  test(10);
  test(50);
  test2();
}