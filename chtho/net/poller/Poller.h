// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_POLLER_H
#define CHTHO_NET_POLLER_H

#include "time/Timestamp.h" 
#include "net/Channel.h"

#include <vector> 
#include <map> 

namespace chtho
{
namespace net
{
class Poller
{
private:
  EventLoop* owner_;
public:
  using ChannelList = std::vector<Channel*>;
  Poller(EventLoop* loop) : owner_(loop) {} 
  virtual ~Poller() = default;
  virtual Timestamp poll(int timeout, ChannelList* activeChannels) = 0;
  virtual void updateChannel(Channel* channel) = 0;
  virtual void removeChannel(Channel* channel) = 0;
  virtual bool hasChannel(Channel* channel) const;
  static Poller* newDefaultPoller(EventLoop* loop);
  void assertInLoopThread() const 
  { owner_->assertInLoopThread(); }
protected:
  using ChannelMap = std::map<int, Channel*>;
  // ! the key is fd
  // channels_ are actually a list of monitored file descriptors
  ChannelMap channels_;
};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_POLLER_H