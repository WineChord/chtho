// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Acceptor.h"

#include <fcntl.h> // open
#include <unistd.h> // close 

namespace chtho
{
namespace net
{
  
Acceptor::Acceptor(EventLoop* loop, const InetAddr& listenAddr, bool reuse)
  : loop_(loop),
    acceptSocket_(Socket::createNonBlockOrDie(listenAddr.family())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false),
    idleFd_(::open("/dev/null", O_RDONLY|O_CLOEXEC))
{
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuse);
  acceptSocket_.bindAddr(listenAddr);
  acceptChannel_.setReadCB([this](Timestamp){this->handleRead();});
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);
}

void Acceptor::listen()
{
  loop_->assertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableRead();
}

// handleRead is called when there is a read
// event happened on the connection file descriptor
// so handleRead is called by the ioloop
void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  InetAddr peerAddr;
  int connfd = acceptSocket_.accept(&peerAddr);
  if(connfd >= 0)
  {
    // this new connection callback is actually 
    // TcpServer::newConn, not the one call be
    // set by the user from the outside
    // the user set function onConn is called inside
    // TcpConnection::established
    if(newConnCB_) newConnCB_(connfd, peerAddr);
    else 
    {
      if(::close(connfd) < 0)
        LOG_SYSERR << "Acceptor::handleRead close(connfd)";
    }
  }
  else
  {
    LOG_SYSERR << "in Acceptor::handleRead";
    if(errno == EMFILE)
    {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY|O_CLOEXEC);
    }
  }
}
} // namespace net
} // namespace chtho
