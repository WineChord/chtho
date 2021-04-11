// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/net/EventLoop.h"
#include "chtho/net/TimerQueue.h"

#include <unistd.h> 
#include <sys/timerfd.h>

#include <map>

using namespace chtho;
using namespace chtho::net;

class PeriodicTimer
{
private:
  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  const double interval_;
  TimerCB cb_;

  void handleRead()
  {
    loop_->assertInLoopThread();
    chtho::net::readTimerfd(timerfd_, Timestamp::now());
    if(cb_) cb_();
  }
public:
  // interval is in seconds 
  PeriodicTimer(EventLoop* loop, double inter, const TimerCB& cb)
    : loop_(loop),
      timerfd_(chtho::net::createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      interval_(inter),
      cb_(cb)
  {
    timerfdChannel_.setReadCB([this](Timestamp){this->handleRead(); });
    timerfdChannel_.enableRead();
  }
  ~PeriodicTimer()
  {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
  }
  void start()
  {
    struct itimerspec spec;
    memset(&spec, 0, sizeof(spec));
    spec.it_interval = Timestamp::toSpec(
      static_cast<int64_t>(interval_*Timestamp::usPerSec));
    spec.it_value = spec.it_interval;
    int ret = ::timerfd_settime(timerfd_, 0, &spec, NULL);
    if(ret) LOG_SYSERR << "timerfd_settime()";
  }

};

void print(const char* msg)
{
  static std::map<const char*, Timestamp> lasts;
  Timestamp& last = lasts[msg];
  Timestamp now = Timestamp::now();
  LOG_INFO << "delay " << Timestamp::diffInSec(now, last) 
    << " - " << msg;
  last = now;
}

int main()
{
  // chtho::Logger::setLogLevel(chtho::Logger::Level::TRACE);
  LOG_INFO << "pid = " << getpid();
  chtho::net::EventLoop loop;
  PeriodicTimer timer(&loop, 1, [](){print("PeriodicTimer");});
  timer.start();
  loop.runEvery(1, [](){print("EventLoop::runEvery");});
  loop.loop();
}