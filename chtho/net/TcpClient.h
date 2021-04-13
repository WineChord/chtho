// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_TCPCLIENT_H
#define CHTHO_NET_TCPCLIENT_H

#include "base/noncopyable.h"
#include "threads/MutexLockGuard.h"
#include "InetAddr.h"
#include "Connector.h"
#include "Callbacks.h"

namespace chtho
{
namespace net
{
using ConnectorPtr = std::shared_ptr<Connector>;
// class EventLoop;
class TcpClient : noncopyable
{
private:
  EventLoop* loop_;
  ConnectorPtr connector_; 
  const std::string name_;
  ConnCB connCB_;
  MsgCB msgCB_;
  WriteCompleteCB writeCompleteCB_;
  std::atomic_bool retry_;
  std::atomic_bool connect_;
  int nxtConnID_;
  mutable MutexLock mutex_;
  TcpConnPtr conn_;

  void newConn(int sockfd);
  void rmConn(const TcpConnPtr& conn);
public:
  TcpClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  EventLoop* loop() const { return loop_; }
  const std::string& name() const { return name_; }

  void setConnCB(ConnCB cb) { connCB_ = cb; }
  void setMsgCB(MsgCB cb) { msgCB_ = cb; }
  void setWriteCompleteCB(WriteCompleteCB cb) { writeCompleteCB_ = cb; }

  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }
  TcpConnPtr conn() const 
  {
    MutexLockGuard lock(mutex_);
    return conn_;
  }
};

} // namespace net
} // namespace chtho
#endif // !CHTHO_NET_TCPCLIENT_H