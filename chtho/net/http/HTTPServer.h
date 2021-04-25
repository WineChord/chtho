// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_HTTP_HTTPSERVER_H
#define CHTHO_NET_HTTP_HTTPSERVER_H

#include "base/noncopyable.h"
#include "net/TcpServer.h"

namespace chtho
{
namespace net
{
class HTTPRequest;
class HTTPResponse; 

class HTTPServer : noncopyable
{
public:
  using HTTPCB = std::function<void(const HTTPRequest&, HTTPResponse*)>;
private:
  TcpServer server_; 
  HTTPCB httpCB_;   

  void onConn(const TcpConnPtr& conn);
  void onMsg(const TcpConnPtr& conn, Buffer* buf, Timestamp rcvTime);
  void onReq(const TcpConnPtr& conn, const HTTPRequest& req);

public:
  HTTPServer(EventLoop* loop, const InetAddr& listenAddr, const std::string& name,
    TcpServer::PortOpt opt = TcpServer::PortOpt::Noreuse);
  void setThreadNum(int threadNum) { server_.setThreadNum(threadNum); }
  void setHTTPCB(const HTTPCB& cb) { httpCB_ = cb; }
  void start(); 
};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_HTTP_HTTPSERVER_H