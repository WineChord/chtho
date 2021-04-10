// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_EVENTLOOP_H
#define CHTHO_NET_EVENTLOOP_H


#include "threads/CurrentThread.h"
#include "base/noncopyable.h"
#include "logging/Logger.h"
#include "threads/MutexLock.h"
#include "Channel.h"
#include "Callbacks.h" // TimerCB 
#include "TimerID.h" // TimerID 

#include <atomic>
#include <functional>
#include <vector> 

namespace chtho
{
namespace net
{

class TimerQueue; 
class Poller; 
class Channel;

class EventLoop : noncopyable
{
public:
  using Func = std::function<void()>;
  static EventLoop* eventLoopOfThisThread();
private:
  using ChannelList = std::vector<Channel*>; 
  pid_t threadID_; // each thread can only own one loop 
  std::atomic_bool looping_;
  std::atomic_bool quit_;
  std::unique_ptr<Poller> poller_; // poller should be initialized early
  std::unique_ptr<TimerQueue> timerQueue_;

  // an eventfd which is used to wake up the eventloop thread
  // if some new pending callback functions are added
  // into the queue 
  int wakeupFd_; 
  std::unique_ptr<Channel> wakeupChannel_;


  // indicating that the current eventloop is processing
  // each pending callback functions
  // this flag is needed when there is another callback
  // placed into the pendingCBs_
  // it will need to notify this further new function to
  // the eventloop thread. so in the next loop, they can
  // read the eventfd and return from poll instead of
  // blocking if no other events occur.
  // see EventLoop::queueInLoop() 
  std::atomic_bool callingPendingCBs_; 
  std::atomic_bool handlingEvents_; 

  Channel* curActiveChannel_; 
  ChannelList activeChannels_;

  Timestamp pollReturnTime_; 

  int64_t iter_; // iterations of loop in loop() 

  // this lock is used to protect the concurrent
  // accesses to pendingCBs_ 
  mutable MutexLock mutex_;
  std::vector<Func> pendingCBs_; 

  // handleRead will be called upon wakeup
  // it will read the wakeupFd_ to consume the event 
  void handleRead(); 
  void printActiveChannels() const;
  void doPendingFuncs();
public:
  EventLoop();
  ~EventLoop();
  // loop() is the entry function of EventLoop
  // it loops forever
  // at the beginning of each loop, it will call
  // Poller::poll to wait for events
  // activeChannels_ is passed as the argument
  // to be filled in for active events 
  // after poll returns, it will iterate through
  // all the active channels and let channel
  // handle the event using the callback function
  // they registered before. see? this clearly shows
  // that how channel acts as the wrapper class for
  // a normal file descriptor -- it manages the callback
  // function corresponding to each events happened on
  // the file descriptor 
  // at the end of each loop, it will call 
  // doPendingFuncs(), which will iterate through
  // each pending function and make a call to it.
  // those pending functions are added by EventLoop::queueInLoop,
  // which will be called by other threads when other threads
  // want the loop thread to perform some function
  // queueInLoop() is often called inside EventLoop::runInLoop
  // the difference is that runInLoop will directly execute the
  // function if it is already the loop thread (otherwise it 
  // will try to call queueInLoop to put the function inside
  // run queue), whereas queueInLoop will definitely push the
  // function to the run queue. and these function inside run queue
  // (aka. pendingFuncs_) will be called inside doPendingFuncs(),
  // which is the final step of each loop inside EventLoop::loop()
  // examples of using runInLoop:
  // 1. TimerQueue::addTimer, will let the loop thread calls 
  // TimerQueue::addTimerInLoop, which will insert the timer into
  // the timer set, and reset the timerfd if it is the earliest 
  // timer to be expired 
  // 2. TimerQueue::cancel, will let the loop thread calls
  // TimerQueue::cancel, which will cancel the specific timer
  // ! also remember that TimerQueue::addTimer is called by
  // ! EventLoop::runAt, which tell the thread to run a function
  // ! at a certain time 
  void loop(); 
  // runAt is the primary function, it will trigger
  // callback function at the specific timestamp t 
  // runAfter, runEvery will refer to this function 
  TimerID runAt(Timestamp t, TimerCB cb);
  // run cb after delay seconds 
  // implemented via runAt 
  TimerID runAfter(double delay, TimerCB cb);

  // maybe called by other threads
  // if current thread runs the eventloop
  // execute it directly
  // otherwise push it into the run queue 
  void runInLoop(Func cb); 
  // put the callback funtion to run queue
  // manipulation of the queue will require a lock 
  // then it will call wakeup() to write to the
  // eventfd, the indication of available for reading
  // will be captured during the blocking of poll
  void queueInLoop(Func cb);
  // called when there is new pending callback put in
  // the queue. the wakeup function is provide by 
  // an eventfd 
  void wakeup();

  // updateChannel() is called from Channel::update
  // each channel will store a pointer to the eventloop
  // it belongs to.
  // the actual behavior is to call Poller::updateChannel
  // the whole purpose is to pass events of interest
  // of the file descriptor to Poller. after all, it is 
  // the Poller who is going to "poll" the events on these
  // file descriptors. also remember: channel is a wrapper
  // of a file descriptor, mainly providing the callback
  // function for each return event 
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

  void assertInLoopThread()
  {
    if(!isInLoopThread())
      abortNotInLoopThread();
  }
  bool isInLoopThread() const { return threadID_ == CurrentThread::tid(); }
  void abortNotInLoopThread(); 
};

} // namespace net
} // namespace chtho

#endif // !CHTHO_NET_EVENTLOOP_H