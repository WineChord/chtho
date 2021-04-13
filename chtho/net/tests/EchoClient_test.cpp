// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/net/EventLoop.h"
#include "chtho/net/InetAddr.h"
#include "chtho/net/TcpClient.h"
#include "chtho/net/TcpConnection.h"

#include <vector> 
#include <string> 
#include <memory> 

using namespace chtho;
using namespace chtho::net;

size_t cur = 0; 
class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;
class EchoClient
{
private:
  EventLoop* loop_;
  TcpClient client_;
public:
  EchoClient(EventLoop* loop, const InetAddr& listenAddr, const std::string& id)
    : loop_(loop), client_(loop, listenAddr, "EchoClient"+id)
  {
    client_.setConnCB([this](const TcpConnPtr& p){this->onConn(p);});
    client_.setMsgCB([this](const TcpConnPtr& conn, Buffer* buf, Timestamp t){this->onMsg(conn,buf,t);});
  }
  void connect() { client_.connect(); }
private:
  void onConn(const TcpConnPtr& conn)
  {
    LOG_INFO << conn->localAddr().ipPort() << " -> " 
      << conn->peerAddr().ipPort().c_str() << " is " 
      << (conn->connected() ? "UP" : "DOWN");
    if(conn->connected())
    {
      ++cur;
      if(cur < clients.size()) clients[cur]->connect();
      LOG_INFO << " connected " << cur;
    }
    conn->send("world\n");
  }
  void onMsg(const TcpConnPtr& conn, Buffer* buf, Timestamp t)
  {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " recv " << msg.size() << " bytes at "
      << t.toString();
    LOG_INFO << msg;
    if(msg == "quit\n")
    {
      conn->send("bye\n");
      conn->shutdown();
    }
    else if(msg == "shutdown\n")
    {
      loop_->quit();
    }
    else  
    {
      conn->send(msg);
    }
  }
};


int main(int argc, char* argv[])
{
  LOG_INFO << "start main";
  // usage:
  // ./echoclient_test serveraddr numclients
  // e.g.
  // ./echoclient_test 127.0.0.1 9
  if(argc < 2)
    LOG_ERR << "Usage: " << argv[0] << " hostip numclient";
  EventLoop loop;
  InetAddr serverAddr(argv[1], 2000);
  int numClients = 1;
  if(argc > 2) numClients = atoi(argv[2]);
  clients.reserve(numClients);
  for(int i = 0; i < numClients; i++)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", i+1);
    clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
  }
  clients[cur]->connect();
  loop.loop();
}