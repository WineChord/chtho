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

using namespace chtho;
using namespace chtho::net;

int numThreads = 0;

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
  EventLoop loop;
  InetAddr listenAddr(2000, false, ipv6);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
  return 0;
}
