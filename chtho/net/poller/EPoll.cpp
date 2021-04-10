// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "EPoll.h"

#include "logging/Logger.h"
#include "net/Channel.h"

#include <unistd.h> // close
#include <sys/epoll.h> // epoll_create

namespace chtho
{
namespace net
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2; 
  
EPoll::EPoll(EventLoop* loop)
  : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSz)
{
  if(epollfd_ < 0)
    LOG_SYSFATAL << "EPoll::EPoll";
}

EPoll::~EPoll()
{
  ::close(epollfd_);
}

// timeout is in milliseconds
Timestamp EPoll::poll(int timeout, ChannelList* activeChannels) 
{
  LOG_TRACE << "fd total cnt: " << channels_.size();
  // epoll_wait function is as follows:
  //* int epoll_wait (int __epfd, struct epoll_event *__events,
  //* 		       int __maxevents, int __timeout);
  // upon return, the events that truly have happened will be
  // stored inside __events, this is different from ::poll
  // recall that ::poll has the signature:
  //* poll (struct pollfd *__fds, nfds_t __nfds, int __timeout)
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeout);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if(numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if(static_cast<size_t>(numEvents) == events_.size())
      events_.reserve(events_.size()*2);
  }
  else if(numEvents == 0)
  {
    LOG_TRACE << "nothing happened";
  }
  else if(savedErrno != EINTR)
  {
    errno = savedErrno;
    LOG_SYSERR << "EPoll::poll()";
  }
  return now;
}

void EPoll::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
  for(int i = 0; i < numEvents; ++i)
  {
    // events_[i] is of type struct epoll_event
    // struct epoll_event has the definition of:
    //* struct epoll_event
    //* {
    //*   uint32_t events;	/* Epoll events */
    //*   epoll_data_t data;	/* User data variable */
    //* } __EPOLL_PACKED;
    // events records the event to monitor
    // epoll_data_t is actually a union:
    //* typedef union epoll_data
    //* {
    //*   void *ptr;
    //*   int fd;
    //*   uint32_t u32;
    //*   uint64_t u64;
    //* } epoll_data_t;
    // you may use epoll_data to store the corresponding fd
    // or you can store a pointer to all sort of thing you think
    // the fd is related to
    // when add a event to epoll,
    // use ::epoll_ctl(epollfd_, op, fd, &epoll_event_);
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void EPoll::updateChannel(Channel* channel) 
{
  Poller::assertInLoopThread();
  const int idx = channel->idx();
  LOG_TRACE << "fd = " << channel->fd() 
    << " events = " << channel->events() << " index = " << idx;
  if(idx == kNew || idx == kDeleted)
  {
    // this is a new channel, use EPOLL_CTL_ADD to add it 
    int fd = channel->fd();
    // fd -> Channel*
    if(idx == kNew) channels_[fd] = channel;
    channel->set_idx(kAdded);
    update(EPOLL_CTL_ADD, channel);
  }
  else  
  {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(idx == kAdded);
    if(channel->isNoneEvent())
    {
      update(EPOLL_CTL_DEL, channel);
      channel->set_idx(kDeleted);
    }
    else update(EPOLL_CTL_MOD, channel);
  }
}

void EPoll::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  int idx = channel->idx();
  size_t n = channels_.erase(fd);
  assert(n == 1);
  if(idx == kAdded) update(EPOLL_CTL_DEL, channel);
  channel->set_idx(kNew);
}

void EPoll::update(int op, Channel* channel)
{
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << opToString(op)
    << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
  if(::epoll_ctl(epollfd_, op, fd, &event) < 0)
  {
    if(op == EPOLL_CTL_DEL)
      LOG_SYSERR << "epoll_ctl op = " << opToString(op) 
        << " fd = " << fd;
    else LOG_SYSFATAL << "epoll_ctl op = " << opToString(op) 
        << " fd = " << fd;
  }
}

const char* EPoll::opToString(int op)
{
  switch (op)
  {
  case EPOLL_CTL_ADD: return "ADD";
  case EPOLL_CTL_DEL: return "DEL";
  case EPOLL_CTL_MOD: return "MOD";
  default: return "UNKNOWN";
  }
}


} // namespace net
} // namespace chtho
