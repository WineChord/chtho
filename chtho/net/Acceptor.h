// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_ACCEPTOR_H
#define CHTHO_NET_ACCEPTOR_H

#include "base/noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

namespace chtho
{
namespace net
{
class InetAddr;
class EventLoop;

class Acceptor : noncopyable
{
public:
  using NewConnCB = std::function<void(int,const InetAddr&)>;
private:
  EventLoop* loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnCB newConnCB_;
  bool listening_;
  int idleFd_;

  void handleRead();
public:
  Acceptor(EventLoop* loop, const InetAddr& listenAddr, bool reuse);
  ~Acceptor();
  void setNewConnCB(const NewConnCB& cb) { newConnCB_ = cb; }
  void listen();
  bool listening() const { return listening_; }
};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_ACCEPTOR_H