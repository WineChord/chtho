// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "HTTPServer.h"

#include "HTTPContext.h"
#include "HTTPResponse.h"

namespace chtho
{
namespace net
{

void defaultHTTPCB(const HTTPRequest&, HTTPResponse* resp)
{
  resp->setStatus(HTTPResponse::NotFound404);
  resp->setStatusMsg("Not Found");
  resp->setClose(true);
}

HTTPServer::HTTPServer(EventLoop* loop, const InetAddr& listenAddr, 
                        const std::string& name, TcpServer::PortOpt opt)
  : server_(loop, listenAddr, name, opt),
    httpCB_(defaultHTTPCB)
{
  server_.setConnCB([this](const TcpConnPtr& conn){this->onConn(conn);});
  server_.setMsgCB([this](const TcpConnPtr& conn, Buffer* buf, Timestamp rcvTime){
    this->onMsg(conn, buf, rcvTime);
  });
}  
void HTTPServer::start()
{
  LOG_WARN << "HTTP server " << server_.name() << " starts listening on "
    << server_.ipPort();
  server_.start();
}
void HTTPServer::onConn(const TcpConnPtr& conn)
{
  LOG_INFO << "HTTPServer::onConn";
}
void HTTPServer::onMsg(const TcpConnPtr& conn, Buffer* buf, Timestamp rcvTime)
{
  HTTPContext context;
  bool ok = context.parse(buf, rcvTime);
  if(!ok)
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }
  if(context.done()) onReq(conn, context.request());
}
// will be called by HTTPServer::onMsg
// and will call user provided httpCB 
void HTTPServer::onReq(const TcpConnPtr& conn, const HTTPRequest& req)
{
  const std::string& c = req.getHeader("Connection");
  bool close = false;
  if(c == "close") close = true;
  if(req.version() == HTTPRequest::HTTP10 && c != "Keep-Alive") close = true;
  HTTPResponse resp(close);
  httpCB_(req, &resp);
  Buffer buf;
  resp.appendToBuf(&buf);
  conn->send(&buf);
  if(resp.close()) conn->shutdown();
}
} // namespace net
} // namespace chtho
