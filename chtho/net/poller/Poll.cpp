// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Poll.h"

#include "logging/Logger.h"
#include "net/Channel.h"

#include <poll.h> // poll(2) 
#include <algorithm> // iter_swap

namespace chtho
{
namespace net
{
Poll::Poll(EventLoop* loop)
  : Poller(loop)
{}

Timestamp Poll::poll(int timeout, ChannelList* activeChannels) 
{
  // ::poll has the following signature:
  //* poll (struct pollfd *__fds, nfds_t __nfds, int __timeout)
  // you should pass in the file descriptors you are interested in
  // and wait for events to occur
  // and the poll data structure is as follows:
  //* struct pollfd
  //* {
  //*   int fd;			/* File descriptor to poll.  */
  //*   short int events;		/* Types of events poller cares about.  */
  //*   short int revents;		/* Types of events that actually occurred.  */
  //* };
  // each pollfd consists of a file descriptor, interested events and
  // returned events
  // the result (whether some events occurred on some fd) will be
  // reflected on revents
  // however, when you are actually processing the active events
  // you have to loop through all the file descriptors
  // check each pollfd.revents, therefore the complexity is O(n)
  // to summarize, the procedure of using ::poll is as follows:
  // 1. setup your vector of pollfds: vector<struct pollfds> and fill
  // in with your interested file descriptor and the corresponding
  // events. see Poll::updateChannel
  // 2. call ::poll, pass the vector in. 
  // 3. iterate through the vector to check each revents, take your
  // action according to the revents you receive for each of your file
  // descriptor. see Poll::fillActiveChannels
  //* done!
  // you can see that using poll, you only have to use this one 
  // particular system call, nothing more. but you have to pass in
  // a bunch of file descriptors and related events which will incur
  // copying of data from user space to kernel space and back. 
  // this may be negligible when the number of fds are small, but it
  // scales poorly especially with respect to thousands of millions of
  // connections using file descriptors such as socket
  // however, epoll provides a better solution
  // but it uses three system calls:
  // 1. epoll_create: this will create a epoll file descriptor, all
  // the actions performed further will refer to this file descriptor
  // its signature is as follows:
  //* int epoll_create1 (int __flags) __THROW;
  // see EPoll::EPoll
  // 2. epoll_ctl: ctl means control, this will help you form the events
  // you want to minitor. see EPoll::update. signature: 
  //* /* Manipulate an epoll instance "epfd". Returns 0 in case of success,
  //*    -1 in case of error ( the "errno" variable will contain the
  //*    specific error code ) The "op" parameter is one of the EPOLL_CTL_*
  //*    constants defined above. The "fd" parameter is the target of the
  //*    operation. The "event" parameter describes which events the caller
  //*    is interested in and any associated user data.  */
  //* int epoll_ctl (int __epfd, int __op, int __fd,
	//*	      struct epoll_event *__event) __THROW;
  // __epfd is the epoll fd created in step 1
  // __op specifies the operation you want to take, typically:
  //* /* Valid opcodes ( "op" parameter ) to issue to epoll_ctl().  */
  //* #define EPOLL_CTL_ADD 1	/* Add a file descriptor to the interface.  */
  //* #define EPOLL_CTL_DEL 2	/* Remove a file descriptor from the interface.  */
  //* #define EPOLL_CTL_MOD 3	/* Change file descriptor epoll_event structure.  */
  // __fd is the file descriptor you want to monitor
  // __event is of type struct epoll_event, defined as follows:
  //* struct epoll_event
  //* {
  //*   uint32_t events;	/* Epoll events */
  //*   epoll_data_t data;	/* User data variable */
  //* } __EPOLL_PACKED;
  // you specify the interested events for this fd in events field of 
  // epoll_event. as for data field:
  //* typedef union epoll_data
  //* {
  //*   void *ptr;
  //*   int fd;
  //*   uint32_t u32;
  //*   uint64_t u64;
  //* } epoll_data_t;
  // it is a union and has various form. you may use it to store a fd or
  // use it to store a pointer to a bunch thing related to that fd.
  // this whole epoll_event field is used in epoll_wait, here comes the
  // 3rd step:
  // 3. epoll_wait: block until some events occur
  //* /* Wait for events on an epoll instance "epfd". Returns the number of
  //*    triggered events returned in "events" buffer. Or -1 in case of
  //*    error with the "errno" variable set to the specific error code. The
  //*    "events" parameter is a buffer that will contain triggered
  //*    events. The "maxevents" is the maximum number of events to be
  //*    returned ( usually size of "events" ). The "timeout" parameter
  //*    specifies the maximum wait time in milliseconds (-1 == infinite).
  //* 
  //*    This function is a cancellation point and therefore not marked with
  //*    __THROW.  */
  //* int epoll_wait (int __epfd, struct epoll_event *__events,
  //* 		       int __maxevents, int __timeout);
  // __events is the start pointer of a buffer allocated by ourself
  // upon return, epoll_wait will place all the information inside 
  // this buffer. so this buffer stores only the events that happened,
  // unlikely poll/select, where you have to iterate through all the
  // file descriptors you have registered.
  // 4. iterate the buffer __events, this stores only events that does occur
  //* done!
  // however, as you can see, epoll uses three syscalls
  // epoll_create will be called once inside one thread
  // epoll_ctl will be called multiple times equal to the number
  // of fd you have to register
  // epoll_wait will be called at the beginning of every loop (similar to poll)
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if(numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
  }
  else if(numEvents == 0) 
  {
    LOG_TRACE << " nothing happened";
  }
  else
  {
    if(savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "Poll::poll()";
    }
  }
  return now;
}

// loop through file descriptors, save active fd in activeChannels
void Poll::fillActiveChannels(int numEvents, ChannelList* activeChannels) const 
{
  for(auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd)
  {
    if(pfd->revents > 0)
    {
      --numEvents;
      // ! remember: the key is fd (for ChannelMap)
      auto ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      // int -> Channel*
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      activeChannels->push_back(channel);
    }
  }
}

void Poll::updateChannel(Channel* channel) 
{
  // poller here saves a collection of channels (aka fd)
  // new channel is added through this updateChannel func
  // Poller class and Channel class are communicated using
  // index
  // what kind of index? the index into pollfds_
  // which is actually a vector
  // so pollfds_[channel->index] is the file descriptor
  // corresponding to channel
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if(channel->idx() < 0) // a new channel
  {
    assert(channels_.find(channel->fd()) == channels_.end());
    struct pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size()) - 1;
    channel->set_idx(idx);
    channels_[pfd.fd] = channel; // fd -> Channel*
  }
  else // update the existing one 
  {
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->idx();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[idx];
    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if(channel->isNoneEvent()) pfd.fd = -channel->fd()-1; // ignore it 
  }
}

void Poll::removeChannel(Channel* channel) 
{
  Poller::assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  int idx = channel->idx();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd& pfd = pollfds_[idx];
  assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1); 
  // idx is the index into pollfds_ 
  if(static_cast<size_t>(idx) == pollfds_.size()-1)
    pollfds_.pop_back();
  else // is not the back one 
  {
    int channelEnd = pollfds_.back().fd;
    // std::iter_swap swap the contents of the two iters point to
    std::iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
    // so now pollfds_[-1] stores the fd for channel
    if(channelEnd < 0) // means ignore 
      channelEnd = -channelEnd-1;
    // because the back one has been swap with the one at idx
    // so the index infomation of that channel should be updated 
    // therefore this line has nothing to do with 'channel' arg
    // it has been delete by channel_.erase and is going to be
    // remove by pollfds_.pop_back
    channels_[channelEnd]->set_idx(idx);
    pollfds_.pop_back();
  }
}
} // namespace net
} // namespace chtho
