// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_CONNECTOR_H
#define CHTHO_NET_CONNECTOR_H

#include "base/noncopyable.h"
#include "InetAddr.h"

#include <functional> 
#include <memory> // std::enable_shared_from_this 
#include <atomic> 

namespace chtho
{
namespace net
{
class EventLoop;
class Channel; 
class Connector : noncopyable, public std::enable_shared_from_this<Connector>
{
public:
  using NewConnCB = std::function<void(int sockfd)>;
private:
  enum State { Disconnected, Connecting, Connected };
  EventLoop* loop_;
  InetAddr serverAddr_;
  std::atomic_bool connect_;
  State state_;
  std::unique_ptr<Channel> channel_;
  NewConnCB newConnCB_;
  int retryDelayMs_;

  static const int maxRetryDelayMs = 30*1000; // 30 s 
  static const int initRetryDelayMs = 500; // 0.5 s 

  void setState(State s) { state_ = s; }
  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

public:
  Connector(EventLoop* loop, const InetAddr& serverAddr);
  ~Connector();

  void start();
  void restart();
  void stop();

  void setNewConnCB(const NewConnCB& cb) { newConnCB_ = cb; }
  const InetAddr& serverAddr() const { return serverAddr_; }
};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_CONNECTOR_H