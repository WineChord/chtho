// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"

#include <unistd.h> 

namespace chtho
{
namespace net
{
// inside ctor, TcpConnection will setup the corresponding
// channel for the connection socket file descriptor 
// then it will set up the callback functions for each
// event happened on the file descriptors 
// Channel::enableRead will be called in established()
// to add the update the read event of this connection
// file descriptor so that the ioloop's poller will 
// monitor the readable event happened on the 
// connection file descriptor. when read event happens,
// handleRead() will be called
// inside handleRead(), the message callback which is 
// provided by the user will also be called. remember
// that this message callback function is set by 
// TcpServer::newConn short after creating this
// TcpConnection object 
// therefore: there are two function provided to the
// user. connection callback and message callback 
TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddr& localAddr,
                             const InetAddr& peerAddr)
  : loop_(loop),
    name_(name),
    // TcpConnection cannot initiate a connection by itself
    // it can only be constructed by using the sockfd provided
    // from the outside (returned by accept).
    // so the initial state for the TcpConnection object 
    // is State::Connection 
    // this state will be changed to State::Connected
    // in TcpConnection::established 
    state_(State::Connecting),
    reading_(true),
    // TcpConnection owns the connection file descriptor
    // it does this by creating an RAII object of Socket.
    // when TcpConnection is destroyed, the Socket
    // object will also call it dtor, which will call
    // ::close to close the connection file descriptor 
    socket_(new Socket(sockfd)), 
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
{
  channel_->setReadCB([this](Timestamp t){this->handleRead(t);});
  channel_->setWriteCB([this](){this->handleWrite();});
  channel_->setCloseCB([this](){this->handleClose();});
  channel_->setErrorCB([this](){this->handleError();});
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this 
    << " fd=" << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
  LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
    << " fd=" << channel_->fd() << " state=" << stateToStr();
  assert(state_ == State::Disconnected);
}

void TcpConnection::handleRead(Timestamp rcv)
{
  loop_->assertInLoopThread();
  int savedErrno = 0;
  // use Buffer to read data on the connection socket
  // file descriptor 
  ssize_t n = inputBuf_.readFd(channel_->fd(), &savedErrno);
  // this msgCB_ is provided by the user 
  // passed by TcpServer::newConn 
  if(n > 0) msgCB_(shared_from_this(), &inputBuf_, rcv);
  // passive close, happened when the user decide to close
  // the connection, the readblae event happened on the
  // connection file descriptor, but when reading this fd
  // it will return 0, TcpConnection will call handleClose
  else if(n == 0) handleClose();
  else 
  {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}
void TcpConnection::handleWrite()
{
  loop_->assertInLoopThread();
  if(channel_->isWriting())
  {
    ssize_t n = ::write(channel_->fd(), outputBuf_.peek(), outputBuf_.readableBytes());
    if(n > 0)
    {
      outputBuf_.retrieve(n);
      if(outputBuf_.readableBytes() == 0)
      {
        channel_->disableWrite();
        if(writeCompleteCB_)
          loop_->queueInLoop([this](){this->writeCompleteCB_(this->shared_from_this());});
        if(state_ == State::Disconnecting)
          shutdownInLoop();
      }
    }
    else LOG_SYSERR << "TcpConnection::handleWrite";
  }
  else LOG_TRACE << "Connection fd = " << channel_->fd() << "is down, no more writing";
}
// called when the client decides to close the connection
// so the read inside handlRead function will return 0 
void TcpConnection::handleClose()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToStr();
  assert(state_ == State::Connected || state_ == State::Disconnecting);
  // don't close fd, leave it to dtor
  setState(State::Disconnected);
  // will set the event to nonevent and update the Poller
  // for epoll, the updateChannel function will use
  // ::epoll_ctl and pass the EPOLL_CTL_DEL to remove
  // the file descriptor 
  channel_->disableAll();
  TcpConnPtr guardThis(shared_from_this());
  // connCB_(guardThis); // why this will be called?
  // closeCB_ is actually TcpServer::rmConn 
  closeCB_(guardThis);
}
void TcpConnection::handleError()
{
  loop_->assertInLoopThread();
  int err = Socket::getSocketError(channel_->fd());
  LOG_ERR << "TcpConnection::handleError [" << name_ 
    << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::shutdown()
{
  if(state_ == State::Connected)
  {
    setState(State::Disconnecting);
    loop_->runInLoop([this](){this->shutdownInLoop();});
  }
}

void TcpConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if(!channel_->isWriting())
    socket_->shutdownWrite();
}

void TcpConnection::forceClose()
{
  if(state_ == State::Connected || state_ == State::Disconnecting)
  {
    setState(State::Disconnecting);
    auto p = shared_from_this();
    loop_->queueInLoop([p](){p->forceCloseInLoop();});
  }
}

void TcpConnection::forceCloseInLoop()
{
  loop_->assertInLoopThread();
  if(state_ == State::Connected || state_ == State::Disconnecting)
  {
    handleClose();
  }
}

void TcpConnection::connEstablished()
{
  loop_->assertInLoopThread();
  assert(state_ == State::Connecting);
  setState(State::Connected);
  channel_->tie(shared_from_this());
  channel_->enableRead();
  // connCB_ is passed by TcpServer::newConn
  // which is actually set by the user from
  // the outside 
  connCB_(shared_from_this());
}
void TcpConnection::connDestroyed()
{
  loop_->assertInLoopThread();
  // if we have previously executed
  // TcpConnection::handleClose 
  // the following will be omitted 
  if(state_ == State::Connected)
  {
    setState(State::Disconnected);
    channel_->disableAll();
    // connCB_(shared_from_this()); // again: why this should called?
  }
  // 
  channel_->remove();
}

void TcpConnection::sendInLoop(const StringPiece& msg)
{
  sendInLoop(msg.data(), msg.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
  loop_->assertInLoopThread();
  ssize_t nwritten = 0;
  size_t remaining = len;
  bool faulterr = false;
  if(state_ == State::Disconnected)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  if(!channel_->isWriting() && outputBuf_.readableBytes() == 0)
  {
    nwritten = ::write(channel_->fd(), data, len);
    if(nwritten >= 0)
    {
      remaining = len - nwritten;
      if(remaining == 0 && writeCompleteCB_)
      {
#if __cplusplus >= 201402L
        auto f = [this,p=shared_from_this()](){this->writeCompleteCB_(p);};
#else  
        auto f = [this](){this->writeCompleteCB_(this->shared_from_this());};
#endif
        loop_->queueInLoop(f);
      }
    }
    else // nwritten < 0
    {
      nwritten = 0;
      if(errno != EWOULDBLOCK)
      {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if(errno == EPIPE || errno == ECONNRESET)
          faulterr = true;
      }
    }
  }
  assert(remaining <= len);
  if(!faulterr && remaining > 0)
  {
    size_t oldLen = outputBuf_.readableBytes();
    if(oldLen + remaining >= highWaterMark_
      && oldLen < highWaterMark_
      && highWaterMarkCB_)
    {
#if __cplusplus >= 201402L
      auto f = [this,p=shared_from_this(),s=oldLen+remaining](){
        this->highWaterMarkCB_(p,s);
       };
#else  
      size_t s = oldLen + remaining;
      auto p = shared_from_this();
      auto f = [this,p,s](){this->highWaterMarkCB_(p,s);};
#endif
      loop_->queueInLoop(f);
    }
    outputBuf_.append(static_cast<const char*>(data)+nwritten, remaining);
    if(!channel_->isWriting())
      channel_->enableWrite();
  }
}

void TcpConnection::send(Buffer* buf)
{
  if(state_ == State::Connected)
  {
    if(loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else  
    {
      loop_->runInLoop([this,buf](){this->sendInLoop(buf->retrieveAllAsString());});
    }
  }
}

void TcpConnection::send(const StringPiece& msg)
{
  if(state_ == State::Connected)
  {
    if(loop_->isInLoopThread())
      sendInLoop(msg);
    else
    {
#if __cplusplus >= 201402L
      auto f = [this,s=msg.as_string()](){this->sendInLoop(s);};
#else
      auto f = [this,msg](){this->sendInLoop(msg.as_string());};
#endif
      loop_->runInLoop(f);
    }
  }
}

std::string TcpConnection::getTcpInfoStr() const
{
  char buf[1024];
  buf[0] = '\0';
  socket_->tcpInfoStr(buf, sizeof(buf));
  return buf;
}
  
void defaultConnCB(const TcpConnPtr& conn)
{
  LOG_TRACE << conn->localAddr().ipPort() << "->"
    << conn->peerAddr().ipPort() << " is " 
    << (conn->connected() ? "UP" : "DOWN");
}
void defaultMsgCB(const TcpConnPtr&, Buffer* buf, Timestamp)
{
  buf->retrieveAll();
}

const char* TcpConnection::stateToStr() const
{
  switch (state_)
  {
  case State::Disconnected: return "Disconnected";
  case State::Connecting: return "Connecting";
  case State::Connected: return "Connected";
  case State::Disconnecting: return "Disconnecting";
  default: return "Unknown state";
  }
}
} // namespace net
} // namespace chtho
