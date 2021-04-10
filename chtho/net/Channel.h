// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_CHANNEL_H
#define CHTHO_NET_CHANNEL_H

#include "base/noncopyable.h"

#include "EventLoop.h"
#include "Callbacks.h" // EventCB, ReadEventCB 

#include <sys/poll.h> 

#include <functional> 

namespace chtho
{
namespace net
{
class EventLoop;
class Channel
{
private:
  EventLoop* loop_;
  const int fd_; 
  int events_;
  int revents_; // return events 
  int idx_; // index into Poller 

  // a weak pointer point to the object that
  // wants to be tied to 
  std::weak_ptr<void> tie_; 
  bool tied_; 

  ReadEventCB readCB_;
  EventCB writeCB_;
  EventCB closeCB_;
  EventCB errorCB_;

  static const int kNoneEvent = 0;
  static const int kReadEvent = POLLIN|POLLPRI;
  static const int kWriteEvent = POLLOUT;

  // this is set true by update and set false during init and remove
  bool addedToLoop_; // whether this channel has been added to eventloop 
  bool handlingEvents_; 
  bool logHup_; // whether to log the POLLHUP event

  // update() is called after each enable/disable event
  // it will set addedToLoop to true and call
  // EventLoop::updateChannel(this), so that eventloop can
  // get information of the file descriptor this channel is a
  // wrapper of. the events to be monitored is of great interest
  // to the poller, so EventLoop::updateChannel actually
  // calls nothing more than Poller::updateChannel
  // so that the poller can know the event set it should
  // monitor for this particular file descriptor 
  void update(); 
public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent(Timestamp revTime);
  void handleEventWithGuard(Timestamp revTime);

  // tie() will tie this channel to the owner object
  // managed by shared_ptr, it will prevent the owner object 
  // being destroyed in handleEvent 
  // the implementation is rather simple:
  // the passed argument is a shared pointer
  // inside tie, it will do the following:
  // * tie_ = obj;
  // * tied_ = true;
  // where tie_ is a weak pointer 
  // inside Channel::handleEvent, a shared pointer
  // will be constructed out of tie_, so consider the
  // following scenario:
  // 1. the shared pointer of obj has a ref count = 1
  // 2. the ref count somehow reduces 1 during handlEvent,
  // then *obj will destroyed.
  // using tie_, we can do the following:
  // 1. build a shared pointer out of tie_, e.g.
  // * std::shared_ptr<void> guard;
  // * guard = tie_.lock();
  // then the ref count will be 2
  // 2. the same ref count decrement happens as before
  // but now the ref count still has value 1, therefore
  // the *obj wouldn't be destroyed
  void tie(const std::shared_ptr<void>& obj);

  int fd() const { return fd_; }
  EventLoop* owner() { return loop_; }

  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }

  int idx() const { return idx_; }
  void set_idx(int idx) { idx_ = idx; }

  void setReadCB(ReadEventCB cb) { readCB_ = std::move(cb); }
  void setWriteCB(EventCB cb) { writeCB_ = std::move(cb); }
  void setCloseCB(EventCB cb) { closeCB_ = std::move(cb); }
  void setErrorCB(EventCB cb) { errorCB_ = std::move(cb); }

  void enableRead() { events_ |= kReadEvent; update(); }
  void disableRead() { events_ &= ~kReadEvent; update(); }
  void enableWrite() { events_ |= kWriteEvent; update(); }
  void disableWrite() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }
  bool isReading() const { return events_ & kReadEvent; }
  bool isWriting() const { return events_ & kWriteEvent; }

  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void remove(); // remove this channel from the eventloop 

  std::string eventsToString(int fd, int ev) const;
  std::string eventsToString() const;
  std::string reventsToString() const;
};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_CHANNEL_H