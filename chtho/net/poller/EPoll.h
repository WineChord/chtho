// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_POLLER_EPOLL_H
#define CHTHO_NET_POLLER_EPOLL_H

#include "Poller.h"

#include <vector>

struct epoll_event;

namespace chtho
{
namespace net
{
class EPoll : public Poller
{
private:
  using EventList = std::vector<struct epoll_event>;
  int epollfd_;
  EventList events_; 

  static const int kInitEventListSz = 16;
  static const char* opToString(int op);
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  void update(int op, Channel* channel);
public:
  EPoll(EventLoop* loop);
  ~EPoll() override;

  // timeout is in milli seconds, I somehow don't want to write it
  // into the argument variable name 
  Timestamp poll(int timeout, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;
};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_POLLER_EPOLL_H