// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Connector.h"
#include "logging/Logger.h"
#include "EventLoop.h"
#include "Socket.h"

#include <unistd.h> 

namespace chtho
{
namespace net
{
const int Connector::maxRetryDelayMs;
const int Connector::initRetryDelayMs;
Connector::Connector(EventLoop* loop, const InetAddr& serverAddr)
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(Disconnected),
    retryDelayMs_(initRetryDelayMs)
{
  LOG_DEBUG << "ctor " << this;
}
Connector::~Connector()
{
  LOG_DEBUG << "dtor " << this;
  assert(!channel_);
}
void Connector::start()
{
  connect_ = true;
  loop_->runInLoop([this](){this->startInLoop();});
}
void Connector::startInLoop()
{
  loop_->assertInLoopThread();
  assert(state_ == Disconnected);
  if(connect_) connect();
  else LOG_DEBUG << "not connected";
}
void Connector::stop()
{
  connect_ = false;
  loop_->queueInLoop([this](){this->stopInLoop();});
}
void Connector::stopInLoop()
{
  loop_->assertInLoopThread();
  if(state_ == Disconnected)
  {
    setState(Disconnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}
void Connector::connect()
{
  int sockfd = Socket::createNonBlockOrDie(serverAddr_.family());
  int ret = ::connect(sockfd, serverAddr_.sockAddr(), 
    static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno)
  {
  case 0:
  case EINPROGRESS:
  case EINTR:
  case EISCONN:
    connecting(sockfd); break; 
  
  case EAGAIN:
  case EADDRINUSE:
  case EADDRNOTAVAIL:
  case ECONNREFUSED:
  case ENETUNREACH:
    retry(sockfd); break;
  
  case EACCES:
  case EPERM:
  case EAFNOSUPPORT:
  case EALREADY:
  case EBADF:
  case EFAULT:
  case ENOTSOCK:
    LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
    ::close(sockfd); break;

  default:
    LOG_SYSERR << "unexpected error in Connector::startInLoop " << savedErrno;
    ::close(sockfd); break;
  }
}
void Connector::restart()
{
  loop_->assertInLoopThread();
  setState(Disconnected);
  retryDelayMs_ = initRetryDelayMs;;
  connect_ = true;
  startInLoop();
}
void Connector::connecting(int sockfd)
{
  setState(Connecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setWriteCB([this](){this->handleWrite();});
  channel_->setErrorCB([this](){this->handleError();});
  channel_->enableWrite();
}
int Connector::removeAndResetChannel()
{
  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  loop_->queueInLoop([this](){this->resetChannel();});
  return sockfd;
}
void Connector::resetChannel()
{
  channel_.reset();
}
void Connector::handleWrite()
{
  LOG_TRACE << "Connector::handleWrite " << static_cast<int>(state_);
  if(state_ == Connecting)
  {
    int sockfd = removeAndResetChannel();
    int err = Socket::getSocketError(sockfd);
    if(err)
    {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err 
        << " " << strerror_tl(err);
      retry(sockfd);
    }
    else if(Socket::isSelfConnect(sockfd))
    {
      LOG_WARN << "Connector::handleWrite - self connect";
      retry(sockfd);
    }
    else  
    {
      setState(Connected);
      if(connect_) newConnCB_(sockfd);
      else ::close(sockfd);
    }
  }
  else  
  {
    assert(state_ == Disconnected);
  }
}
void Connector::handleError()
{
  LOG_ERR << "Connector::handleError state=" << static_cast<int>(state_);
  if(state_ == Connecting)
  {
    int sockfd = removeAndResetChannel();
    int err = Socket::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
  }
}
void Connector::retry(int sockfd)
{
  ::close(sockfd);
  setState(Disconnected);
  if(connect_)
  {
    LOG_INFO << "Connector::retry - retry connecting to "
      << serverAddr_.ipPort() << " in " << retryDelayMs_ << "ms.";
    auto p = shared_from_this();
    loop_->runAfter(retryDelayMs_/1000.0, [p](){p->startInLoop();});
    retryDelayMs_ = std::min(retryDelayMs_*2, maxRetryDelayMs);
  }
  else  
  {
    LOG_DEBUG << "not connect";
  }
}
} // namespace net
} // namespace chtho
