// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TimerQueue.h"
#include "TimerID.h" 
#include "Timer.h"
#include "EventLoop.h" 

#include <sys/timerfd.h> // timerfd_settime 
#include <unistd.h> // close
#include <algorithm> 
#include <iterator> // back_inserter

namespace chtho
{
namespace net
{

int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK|TFD_CLOEXEC);
  if(timerfd < 0) LOG_SYSFATAL << "failed in timerfd_create";
  return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_()
{
  auto f = [this](){ this->handleRead(); };
  timerfdChannel_.setReadCB(f);
  timerfdChannel_.enableRead();
}

TimerQueue::~TimerQueue()
{
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  ::close(timerfd_);
  for(const auto& timer : timers_)
    delete timer.second;
}

void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t res;
  ssize_t n = ::read(timerfd, &res, sizeof(res));
  LOG_TRACE << "handleRead " << res << " at " << now.toString();
  if(n != sizeof(res))
    LOG_ERR << "handleRead reads " << n << " bytes instead of 8";
}

// given current timestamp, getExpired returns a list of timers
// that has expired.
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
  assert(timers_.size() == activeTimers_.size());
  std::vector<Entry> expired;
  // construct an entry that is largest in current timestamp
  // so that lower_bound will return the first timer that
  // is not expired.
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  auto end = timers_.lower_bound(sentry);
  // [timers_.begin(), end) is the list of expired timers 
  std::copy(timers_.begin(), end, std::back_inserter(expired));
  // remove those expired timers from set timers_
  timers_.erase(timers_.begin(), end);
  // also remove them from active timer list
  for(const auto& it : expired)
  {
    ActiveTimer timer(it.second, it.second->seq());
    size_t n = activeTimers_.erase(timer);
    assert(n == 1); 
  }
  assert(timers_.size() == activeTimers_.size());
  return expired;
}

void TimerQueue::handleRead()
{
  // this function is registered as a read callback
  // in timerfdChannel_ 
  // timerfdChannel_ is a wrapper of timerfd_
  // channel can provide the facility of callback function 
  // dispatch upon the events received from file descriptor
  // this function is called when the earliest timer expires
  // 1. read the timerfd event (actual message is useless)
  // 2. get current time, which is the time when the timer 
  // expires, and look into the timer set to find out exactly
  // which timers has expired.
  // 3. iterate through these timers and call the function binded
  // on each timer
  // 4. reset the expiration time of timerfd
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now);
  // ! remember: inside getExpired the expired timers will
  // ! be removed from timers_ and activeTimers_ 
  std::vector<Entry> expired = getExpired(now);

  callingExpiredTimers_ = true;
  // cancelTimers_ is only used when the timerqueue is calling
  // expired timers and the timer to be canceled is inside 
  // the expired timer set (see TimerQueue::cancelInLoop). 
  // and this list is used in TimerQueue::reset to determine 
  // whether to add one timer back if it is repeatable and not 
  // canceled. 
  cancelTimers_.clear();
  for(const auto& it : expired)
    it.second->run(); // run the callback function of the timer
  callingExpiredTimers_ = false;
  reset(expired, now);
}

void resetTimerfd(int timerfd, Timestamp exp)
{
  struct itimerspec newv;
  struct itimerspec oldv;
  memset(&newv, 0, sizeof(newv));
  memset(&oldv, 0, sizeof(oldv));
  newv.it_value = (exp - Timestamp::now()); // return struct timespec
  int ret = ::timerfd_settime(timerfd, 0, &newv, &oldv);
  if(ret) LOG_SYSERR << "timerfd_settime()";
}

// reset will iterate through all the expired timers and restart the one
// that is both repeatable and not canceled
// furthermore the earliest timer in the timer set is selected to 
// reset the timerfd 
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nxt;
  for(const auto& it : expired)
  {
    // sequence number is incremented each time
    // a new timer is created 
    ActiveTimer t(it.second, it.second->seq());
    if(it.second->repeat()
      && cancelTimers_.find(t) == cancelTimers_.end())
    {
      it.second->restart(now);
      insert(it.second);
    }
    else delete it.second;
  }
  // use the earliest timer to reset the timerfd
  if(!timers_.empty())
    nxt = timers_.begin()->second->expir();
  if(nxt.valid()) resetTimerfd(timerfd_, nxt);
}

TimerID TimerQueue::addTimer(TimerCB cb, Timestamp t, double interval)
{
  Timer* timer = new Timer(std::move(cb), t, interval);
  auto f = [this,timer](){ this->addTimerInLoop(timer); };
  loop_->runInLoop(f);
  return TimerID(timer, timer->seq());
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
  loop_->assertInLoopThread();
  bool earliest = insert(timer);
  if(earliest) resetTimerfd(timerfd_, timer->expir());
}



bool TimerQueue::insert(Timer* timer)
{
  loop_->assertInLoopThread();
  bool earliest = false;
  Timestamp exp = timer->expir();
  auto it = timers_.begin();
  if(it == timers_.end() || exp < it->first)
    earliest = true;
  timers_.insert(Entry(exp, timer));
  activeTimers_.insert(ActiveTimer(timer, timer->seq())); 
  return earliest;
}

} // namespace net
} // namespace chtho
