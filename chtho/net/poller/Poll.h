// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_POLLER_POLL_H
#define CHTHO_NET_POLLER_POLL_H

#include "Poller.h"

#include <vector>

struct pollfd;

namespace chtho
{
namespace net
{
class Poll : public Poller
{
private:
  using PollFDList = std::vector<struct pollfd>;
  PollFDList pollfds_;

  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
public:
  Poll(EventLoop* loop);
  ~Poll() = default;

  // timeout is in milli seconds, I somehow don't want to write it
  // into the argument variable name 
  Timestamp poll(int timeout, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;
};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_POLLER_POLL_H