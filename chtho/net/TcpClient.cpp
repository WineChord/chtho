// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TcpClient.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Socket.h"

namespace chtho
{
namespace net
{
TcpClient::TcpClient(EventLoop* loop, const InetAddr& serverAddr, 
    const std::string& name)
  : loop_(loop),
    connector_(new Connector(loop, serverAddr)),
    name_(name),
    connCB_(defaultConnCB),
    msgCB_(defaultMsgCB),
    retry_(false),
    connect_(true),
    nxtConnID_(1)
{
  connector_->setNewConnCB([this](int fd){this->newConn(fd);});
  LOG_INFO << "TcpClient::Client " << name_ << " - connector "
    << connector_.get();
}
TcpClient::~TcpClient()
{
  LOG_INFO << "TcpClient::~Client " << name_ << " - connector "
    << connector_.get();
  TcpConnPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(mutex_);
    unique = conn_.unique();
    conn = conn_;
  }
  if(conn)
  {
    assert(loop_ == conn->loop());
    auto cb = [this](const TcpConnPtr& p){this->rmConn(p);};
    loop_->runInLoop([conn,cb](){conn->setCloseCB(cb);});
    if(unique)
    {
      conn->forceClose();
    }
  }
  else  
  {
    connector_->stop();
  }
}
void TcpClient::connect()
{
  LOG_INFO << "TcpClient::connect " << name_ << " - connecting to "
    << connector_->serverAddr().ipPort();
  connect_ = true;
  connector_->start();
}
void TcpClient::disconnect()
{
  connect_ = false;
  {
    MutexLockGuard lock(mutex_);
    if(conn_)
      conn_->shutdown();
  }
}
void TcpClient::stop()
{
  connect_ = false;
  connector_->stop();
}
void TcpClient::newConn(int sockfd)
{
  loop_->assertInLoopThread();
  InetAddr peer(Socket::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peer.ipPort().c_str(), nxtConnID_);
  ++nxtConnID_;
  std::string connName = name_ + buf;
  InetAddr local(Socket::getLocalAddr(sockfd));
  TcpConnPtr conn(new TcpConnection(loop_, connName, sockfd, local, peer));
  conn->setConnCB(connCB_);
  conn->setMsgCB(msgCB_);
  conn->setWriteCompleteCB(writeCompleteCB_);
  conn->setCloseCB([this](const TcpConnPtr& p){this->rmConn(p);});
  {
    MutexLockGuard lock(mutex_);
    conn_ = conn;
  }
  conn->connEstablished();
}
void TcpClient::rmConn(const TcpConnPtr& conn)
{
  loop_->assertInLoopThread();
  assert(loop_ == conn->loop());

  {
    MutexLockGuard lock(mutex_);
    assert(conn_ == conn);
    conn_.reset();
  }
  loop_->queueInLoop([conn](){conn->connDestroyed();});
  if(retry_ && connect_)
  {
    LOG_INFO << "TcpClient::connect " << name_ << "reconnecting to "
      << connector_->serverAddr().ipPort();
    connector_->restart();
  }
}
} // namespace net
} // namespace chtho
