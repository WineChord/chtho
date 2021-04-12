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
  
// TcpServer owns an acceptor, which is used to accept connection,
// in the ctor of Acceptor, a listening file descriptor and its 
// corresponding channel will be created and bind the the given
// address. this is equivalent to 
// the usual step: 1. create a listen socket. 2. bind
// TcpServer::start() will then call listen, this is equivalent
// to the usual step: 3. call listen 
// inside Acceptor::listen, the listen channel will enableRead,
// which will add the update the ioloop about the events of 
// the file descriptor which is to be monitored.
// when connection arrives, Acceptor will call handleRead, which
// will in turn call acceptor to return the connection file
// descriptor
// the TcpServer's lifetime is controled by the user 
// the Acceptor will 
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
// this sockfd is the connection socket
// newConn is called through acceptor's handleRead
// when a new connection arrives, the event happens on the
// listen file descriptor, and poll function in that eventloop
// thread returns, then in the loop, it will call acceptor's
// handleRead function, which is registered by the channel.
// handleRead will call 'accept(2)' to accept the connection
// and get a connection socket file descriptor, then handleRead
// will call TcpServer::newConn (this is registered in ctor)
// and gives the connection socket file descriptor to
// this function 
// this function will find the next io loop thread in threadpool,
// and create a new TcpConnection object. then it will queue
// the TcpConnection::connEstablished function inside the picked
// io loop. TcpConnection::connEstablished function
// will set up callback function for the reading 
// channel and call connection callback function which is set
// by this TcpServer::newConn, and typically the connection callback
// function can be customized by the user through TcpServer::setConnCB.
// back to TcpConnection::connEstablished, before calling the
// connection callback, it will register the connection file
// descriptor's callback function through the use of channel.
// by calling channel->enableRead(), the connection socket 
// file descriptor is registered in that io event loop.
// when the connection file descriptor is readable, the 
// corresponding callback function will be invoked.
// specifically, it will call TcpConnection::handleRead,
// which will read the data on the file descriptor into
// its inputBuf_, and then pass that buffer to msgCB_
// by calling the msgCB_. message callback function is also
// set by the following TcpServer::newConn, and can also be
// customized by the user from the outside. 
void TcpServer::newConn(int sockfd, const InetAddr& peerAddr)
{
  loop_->assertInLoopThread();
  // find a thread from the pool to serve the new connection 
  EventLoop* ioloop = threadPool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nxtConnID_);
  ++nxtConnID_;
  std::string connName = name_ + buf;
  LOG_INFO << "TcpServer::newConn [" << name_
    << "] - new conn [" << connName << "] from "
    << peerAddr.ipPort();
  InetAddr localAddr(Socket::getLocalAddr(sockfd));
  // use make_shared is achieve exception-free and eliminate the need to
  // allocte twice 
  // TcpConnection uses shared_ptr because its lifetime is ambiguous,
  // you cannnot delete a TcpConnection when finished processing, because
  // there might be other object referring to it 
  // the connection name is formed by name_ (which is passed by user)
  // and buf ("-ipaddr:port#connid")
  // here the sockfd is get from Acceptor::accept, it is the connection file 
  // descriptor 
  TcpConnPtr conn = std::make_shared<TcpConnection>(ioloop, connName, sockfd, localAddr, peerAddr);
  // save the connection in map, the key is connection name, value is pointer 
  // to this connection 
  // note that TcpServer knows Acceptor and TcpConnection
  // the they don't know TcpServer (keep single direction dependency)
  conns_[connName] = conn;
  conn->setConnCB(connCB_);
  conn->setMsgCB(msgCB_);
  conn->setWriteCompleteCB(writeCompleteCB_);
  conn->setCloseCB([this](const TcpConnPtr& p){this->rmConn(p);});
  // run the TcpConnection::connEstablished function inside the newly 
  // assigned ioloop (different from the main ioloop is thread num > 0)
  ioloop->runInLoop([conn](){conn->connEstablished();});
}

// will be called by TcpConnection::handleClose by closeCB_
void TcpServer::rmConn(const TcpConnPtr& conn)
{
  // the thread executing TcpConnection::handleClose is the
  // io thread for the connection fd, need to transfer control
  // to the main thread to remove the tcp connection stored
  // inside TcpServer 
  loop_->runInLoop([this,conn](){this->rmConnInLoop(conn);});
}

void TcpServer::rmConnInLoop(const TcpConnPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::rmConnInLoop [" << name_
    << "] - connection " << conn->name();
  // erase the connection from the connection map 
  size_t n = conns_.erase(conn->name());
  assert(n == 1);
  // here gets the ioloop of the TcpConnection object
  EventLoop* ioloop = conn->loop();
  // run TcpConnection::connDestroyed from its ioloop 
  ioloop->queueInLoop([conn](){conn->connDestroyed();});
}
} // namespace net
} // namespace chtho
