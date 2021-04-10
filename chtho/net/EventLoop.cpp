// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EventLoop.h"
#include "TimerQueue.h"
#include "threads/MutexLockGuard.h" // MutexLockGuard
#include "poller/Poller.h"

#include <unistd.h> // write 
#include <sys/eventfd.h> // eventfd 

namespace chtho
{
namespace net
{
  
__thread EventLoop* loopOfThisThread = nullptr;
const int kPollTimeMs = 10000;
EventLoop* EventLoop::eventLoopOfThisThread()
{ return loopOfThisThread; }

int createEventfd()
{
  // innitial value is set to 0 
  int fd = ::eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
  if(fd < 0)
  {
    LOG_SYSERR << "failed to create eventfd";
    abort();
  }
  return fd;
}

EventLoop::EventLoop()
  : threadID_(CurrentThread::tid()),
    looping_(false),
    quit_(false),
    poller_(Poller::newDefaultPoller(this)), // poller should be initialzed early
    timerQueue_(new TimerQueue(this)), // timerfd depends on poller_
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    callingPendingCBs_(false),
    handlingEvents_(false),
    curActiveChannel_(nullptr),
    iter_(0)
{
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadID_;
  if(loopOfThisThread)
    LOG_FATAL << "Another EventLoop " << loopOfThisThread 
              << " exists in this thread " << threadID_;
  else loopOfThisThread = this;
  // setup callback function to be called when waked up 
  auto f = [this](){ this->handleRead(); };
  wakeupChannel_->setReadCB(f);
  assert(poller_ != nullptr);
  wakeupChannel_->enableRead();
}

EventLoop::~EventLoop()
{
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadID_
            << " destructs in thread " << CurrentThread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  loopOfThisThread = nullptr;
}

// general steps for a loop:
// 1. performs poll, return active channels 
// 2. iterate through all the channels, execute the registered callback func
// 3. execute pending function (the source of these funcs are queueInLoop and 
// runInLoop)
void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";
  while(!quit_)
  {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    ++iter_;
    if(Logger::logLevel() <= Logger::Level::TRACE)
      printActiveChannels();
    handlingEvents_ = true;
    for(auto channel : activeChannels_)
    {
      curActiveChannel_ = channel;
      curActiveChannel_->handleEvent(pollReturnTime_);
    }
    curActiveChannel_ = nullptr;
    handlingEvents_ = false;
    doPendingFuncs();
  }
  LOG_TRACE << "EventLoop" << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  // there is a chance that after the quit_ is set
  // (usually called inside doPendingFuncs)
  // and it immediate run out of loop() func and
  // destroy the EventLoop object, then we will be
  // accessing an invalid object
  if(!isInLoopThread()) wakeup();
}

void EventLoop::doPendingFuncs()
{
  std::vector<Func> funcs;
  callingPendingCBs_ = true;
  {
    // reduce the critical section
    MutexLockGuard lock(mutex_);
    funcs.swap(pendingCBs_);
  }
  for(const auto& func : funcs)
    func();
  callingPendingCBs_ = false;
}

void EventLoop::printActiveChannels() const 
{
  for(const auto* channel : activeChannels_)
    LOG_TRACE << "{" << channel->reventsToString() << "} ";
}

void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
  if(n != sizeof(one))
    LOG_ERR << "EventLoop::handleRead() reads " << n 
            << " bytes instead of 8";
}

void EventLoop::abortNotInLoopThread()
{
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this 
            << " was created in threadID_ = " << threadID_
            << ", current thread id = " << CurrentThread::tid();
}

TimerID EventLoop::runAt(Timestamp t, TimerCB cb)
{
  return timerQueue_->addTimer(std::move(cb), t, 0.0);
}

TimerID EventLoop::runAfter(double delay, TimerCB cb)
{
  Timestamp t(Timestamp::now() + delay);
  return runAt(t, std::move(cb));
}

TimerID EventLoop::runEvery(double inter, TimerCB cb)
{
  Timestamp t(Timestamp::now() + inter);
  return timerQueue_->addTimer(std::move(cb), t, inter);
}

void EventLoop::runInLoop(Func cb)
{
  // call it directly if we are in the eventloop thread
  if(isInLoopThread()) cb();
  else queueInLoop(cb); // else put it into the run queue 
}

void EventLoop::queueInLoop(Func cb)
{
  {
    MutexLockGuard lock(mutex_);
    pendingCBs_.push_back(std::move(cb));
  }
  if(!isInLoopThread() || callingPendingCBs_)
    wakeup();
}

void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
  if(n != sizeof(one))
    LOG_ERR << "EventLoop::wakeup() writes " << n 
            << " bytes instead of 8";
}

void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->owner() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->owner() == this); 
  assertInLoopThread();
  if(handlingEvents_)
    assert(curActiveChannel_ == channel ||
      std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  poller_->removeChannel(channel);
}

// called by Channel::~Channel
// perhaps provides only some sort of assertion 
bool EventLoop::hasChannel(Channel* channel)
{
  assert(channel->owner() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

} // namespace net
} // namespace chtho
