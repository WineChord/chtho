// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Channel.h"

#include <assert.h> // assert 
#include <sstream> // ostringstream 

namespace chtho
{
namespace net
{

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    idx_(-1),
    tied_(false),
    addedToLoop_(false),
    handlingEvents_(false),
    logHup_(true)
{}

Channel::~Channel()
{
  assert(!handlingEvents_);
  assert(!addedToLoop_);
  if(loop_->isInLoopThread())
    assert(!loop_->hasChannel(this));
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

// core function of Channel class
// will be call by EventLoop::loop()
void Channel::handleEvent(Timestamp revTime)
{
  std::shared_ptr<void> guard;
  if(tied_)
  {
    guard = tie_.lock();
    if(guard) handleEventWithGuard(revTime);
  } else handleEventWithGuard(revTime);
}


void Channel::handleEventWithGuard(Timestamp revTime)
{
  handlingEvents_ = true;
  LOG_TRACE << reventsToString();
  if((revents_ & POLLHUP) && !(revents_ & POLLIN))
  {
    if(logHup_) LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    if(closeCB_) closeCB_();
  }
  if(revents_ & POLLNVAL) 
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  if(revents_ & (POLLERR | POLLNVAL))
    if(errorCB_) errorCB_();
  if(revents_ & (POLLIN|POLLPRI|POLLRDHUP))
    if(readCB_) readCB_();
  if(revents_ & POLLOUT)
    if(writeCB_) writeCB_();
  handlingEvents_ = false;
}
  
void Channel::remove() // remove this channel from the eventloop 
{
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::update()
{
  addedToLoop_ = true;
  loop_->updateChannel(this);
}

std::string Channel::eventsToString(int fd, int ev) const
{
  std::ostringstream oss;
  oss << fd << ": ";
  if(ev & POLLIN) oss << "IN ";
  if(ev & POLLPRI) oss << "PRI ";
  if(ev & POLLOUT) oss << "OUT ";
  if(ev & POLLHUP) oss << "HUP ";
  if(ev & POLLRDHUP) oss << "RDHUP ";
  if(ev & POLLERR) oss << "ERR ";
  if(ev & POLLNVAL) oss << "NVAL ";
  return oss.str();
}

std::string Channel::eventsToString() const
{
  return eventsToString(fd_, events_);
}
std::string Channel::reventsToString() const
{
  return eventsToString(fd_, revents_);
}
} // namespace net
} // namespace chtho
