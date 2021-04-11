// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_TCPSERVER_H
#define CHTHO_NET_TCPSERVER_H

#include "base/noncopyable.h"
#include "InetAddr.h"
#include "TcpConnection.h"
#include "Callbacks.h"

#include <memory> 
#include <atomic>
#include <map>

namespace chtho
{
namespace net
{
class EventLoop;
class EventLoopThreadPool;
class Acceptor;

class TcpServer : noncopyable
{
private:
  using ConnMap = std::map<std::string, TcpConnPtr>;

  EventLoop* loop_;
  const std::string ipPort_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  ConnCB connCB_;
  MsgCB msgCB_;
  WriteCompleteCB writeCompleteCB_;
  ThreadInitCB threadInitCB_;
  std::atomic_int32_t started_;
  int nxtConnID_;
  ConnMap conns_;

public:
  enum PortOpt { Noreuse, Reuse };
  TcpServer(EventLoop* loop, const InetAddr& listenAddr,
    const std::string& name, PortOpt opt=Noreuse);
  ~TcpServer();

  void start(); 

  // 0: all IO is in loop's thread
  // 1: all IO is in another thread
  // N: create a thread pool with N threads, which are
  // used to handle new connections 
  void setThreadNum(int numThreads);

  void setConnCB(const ConnCB& cb) { connCB_ = cb; }
  void setMsgCB(const MsgCB& cb) { msgCB_ = cb; }

  void newConn(int sockfd, const InetAddr& peerAddr);
  void rmConn(const TcpConnPtr& conn);
  void rmConnInLoop(const TcpConnPtr& conn);

};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_TCPSERVER_H