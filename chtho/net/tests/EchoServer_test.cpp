// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/net/TcpConnection.h"
#include "chtho/net/EventLoop.h"
#include "chtho/net/InetAddr.h" 
#include "chtho/net/TcpServer.h"
#include "chtho/net/Callbacks.h"

#include <unistd.h>

using namespace chtho;
using namespace chtho::net;

int numThreads = 0;

// for a user server, just setup the connection callback and message call
// then call server_.start()
class EchoServer
{
private:
  EventLoop* loop_;
  TcpServer server_;
public:
  EchoServer(EventLoop* loop, const InetAddr& listenAddr)
    : loop_(loop), server_(loop, listenAddr, "EchoServer") 
  {
    auto f1 = [this](const TcpConnPtr& conn){this->onConn(conn);};
    server_.setConnCB(f1);
    auto f2 = [this](const TcpConnPtr& conn, Buffer* buf, Timestamp t){
      this->onMessage(conn, buf, t);
    };
    server_.setMsgCB(f2);
    server_.setThreadNum(numThreads);
  }

  void start() { server_.start(); }

private:
  void onConn(const TcpConnPtr& conn)
  {
    LOG_TRACE << conn->peerAddr().ipPort() << " -> "
      << conn->localAddr().ipPort() << " is "
      << (conn->connected() ? "UP" : "DOWN");
    LOG_INFO << conn->getTcpInfoStr();
    ::sleep(5);
    conn->send("hello\n");
  }
  void onMessage(const TcpConnPtr& conn, Buffer* buf, Timestamp t)
  {
    std::string msg(buf->retrieveAllAsString());
    LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " 
      << t.toString();
    LOG_INFO << msg;
    if(msg == "exit\n")
    {
      conn->send("bye\n");
      conn->shutdown();
    }
    if(msg == "quit\n")
    {
      loop_->quit();
    }
    conn->send(msg);
  }
};


int main(int argc, char const *argv[])
{
  LOG_INFO << "size of TcpConnection = " << sizeof(TcpConnection);
  if(argc > 1) numThreads = atoi(argv[1]);
  bool ipv6 = argc > 2;
  // step1: setup a EventLoop object (a stack object)
  EventLoop loop;
  InetAddr listenAddr(2000, false, ipv6);
  // step2: initialize a TcpServer, setup the corresponding
  // connection and message callback function 
  // the ctor of TcpServer will setup a Acceptor object 
  // which will create a listening file descriptor and 
  // bind the address to the listening socket file descriptor
  // this finish the typical first two steps during a 
  // server setup: create a socket and call bind 
  EchoServer server(&loop, listenAddr);
  // step3: call server.state(), this will call call
  // Acceptor::listen, inside Acceptor::listen,
  // it will call ::listen and enableRead for the
  // listening socket file descriptor, which will update
  // the event of this fd to Poller, so the event loop
  // can monitor the readable event happened on the 
  // listening socket file descriptor 
  server.start();
  // step4: start the main eventloop, which will monitor
  // the readable event (new connection) happened on the
  // listening socket file descriptor 
  loop.loop();
  return 0;
}
