// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_TIMERQUEUE_H
#define CHTHO_NET_TIMERQUEUE_H

#include "time/Timestamp.h" // Timestamp 
#include "Callbacks.h" // TimerCB 
#include "Channel.h" // Channel 

#include <utility> // pair 
#include <set> // set 

namespace chtho
{
namespace net
{
class EventLoop;
class Timer; 
class TimerID;

class TimerQueue
{
private:
  using Entry = std::pair<Timestamp, Timer*>;
  using Timers = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, int64_t>;
  using ActiveTimers = std::set<ActiveTimer>; 

  EventLoop* loop_;

  const int timerfd_; 
  // channel is a wrapper of fd so as to provide function callback
  // upon certain events happened on the file descriptor
  Channel timerfdChannel_; 

  Timers timers_; 

  ActiveTimers activeTimers_;
  ActiveTimers cancelTimers_;
  
  std::atomic_bool callingExpiredTimers_;
  // this will be called inside timerfdChannel_
  // remember that timerfdChannel_ is a wrapper of timerfd_
  // it will call certain callback function upon the received
  // events happened on timerfd_ 
  void handleRead(); 
  // given current time, getExpired returns a list of
  // expired timers 
  std::vector<Entry> getExpired(Timestamp now);
  void addTimerInLoop(Timer* timer);
  // insert a timer into set
  // return whether timer is the earliest one to expire
  bool insert(Timer* timer);
  // reset will iterate through all the expired timers
  // if the timer is both repeatable and not canceled,
  // put it back to the timer list again 
  void reset(const std::vector<Entry>& expired, Timestamp now); 


public:
  // takes an eventloop as argument
  // timerqueue serves an eventloop
  // it will invoke the callback functions
  // when timer expires
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  // cb: callback function, which will be called when timer expires
  // t: the specifc time when the timer will expire
  // interval: if the timer is repeatable, interval > 0
  TimerID addTimer(TimerCB cb, Timestamp t, double interval);
};

} // namespace net
} // namespace chtho

#endif // !CHTHO_NET_TIMERQUEUE_H