// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Poller.h"
#include "Poll.h"
#include "EPoll.h"

#include "net/EventLoop.h"

#include <stdlib.h> // getenv

namespace chtho
{
namespace net
{
// used as assertion in EventLoop::hasChannel()
// see also Channel::~Channel()
bool Poller::hasChannel(Channel* channel) const
{
  assertInLoopThread();
  auto it = channels_.find(channel->fd());
  return it != channels_.end() && it->second == channel;
}

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
  if(::getenv("CHTHO_USE_POLL")) return new Poll(loop);
  else return new EPoll(loop);
}

} // namespace net
} // namespace chtho
