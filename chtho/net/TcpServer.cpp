// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TcpServer.h"
#include "EventLoopThreadPool.h"
#include "Acceptor.h"

namespace chtho
{
namespace net
{
  
TcpServer::TcpServer(EventLoop* loop, const InetAddr& listenAddr,
    const std::string& name, PortOpt opt)
  : loop_(loop),
    ipPort_(listenAddr.ipPort()),
    name_(name),
    acceptor_(new Acceptor(loop, listenAddr, opt==Reuse)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connCB_(defaultConnCB),
    msgCB_(defaultMsgCB),
    nxtConnID_(1)
{
  auto f = [this](int sockfd, const InetAddr& peerAddr){
    this->newConn(sockfd, peerAddr);
  };
  acceptor_->setNewConnCB(f);
}

TcpServer::~TcpServer()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
  for(auto& kv : conns_)
  {
    TcpConnPtr conn(kv.second);
    kv.second.reset();
    conn->loop()->runInLoop([conn](){conn->connDestroyed();});
  }
}

void TcpServer::start()
{
  if(started_.exchange(1) == 0)
  {
    threadPool_->start(threadInitCB_);
    assert(!acceptor_->listening());
    auto ptr = acceptor_.get();
    loop_->runInLoop([ptr](){ptr->listen();});
  }
}
  
void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setNumThread(numThreads);
}
void TcpServer::newConn(int sockfd, const InetAddr& peerAddr)
{
  loop_->assertInLoopThread();
  EventLoop* ioloop = threadPool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nxtConnID_);
  ++nxtConnID_;
  std::string connName = name_ + buf;
  LOG_INFO << "TcpServer::newConn [" << name_
    << "] - new conn [" << connName << "] from "
    << peerAddr.ipPort();
  InetAddr localAddr(Socket::getLocalAddr(sockfd));
  TcpConnPtr conn(new TcpConnection(ioloop, connName, sockfd, localAddr, peerAddr));
  conns_[connName] = conn;
  conn->setConnCB(connCB_);
  conn->setMsgCB(msgCB_);
  conn->setWriteCompleteCB(writeCompleteCB_);
  conn->setCloseCB([this](const TcpConnPtr& p){this->rmConn(p);});
  ioloop->runInLoop([conn](){conn->connEstablished();});
}

void TcpServer::rmConn(const TcpConnPtr& conn)
{
  loop_->runInLoop([this,conn](){this->rmConnInLoop(conn);});
}

void TcpServer::rmConnInLoop(const TcpConnPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::rmConnInLoop [" << name_
    << "] - connection " << conn->name();
  size_t n = conns_.erase(conn->name());
  assert(n == 1);
  EventLoop* ioloop = conn->loop();
  ioloop->queueInLoop([conn](){conn->connDestroyed();});
}
} // namespace net
} // namespace chtho
