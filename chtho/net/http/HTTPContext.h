// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_HTTP_HTTPCONTEXT_H
#define CHTHO_NET_HTTP_HTTPCONTEXT_H

#include "HTTPRequest.h"

namespace chtho
{
namespace net
{
class Buffer;

class HTTPContext
{
public:
  enum ParseState { ExpReq, ExpHeader, ExpBody, Done };
private:
  ParseState state_;
  HTTPRequest request_; 

  bool procReq(const char* begin, const char* end);
public:
  HTTPContext() : state_(ExpReq) {} 
  bool parse(Buffer* buf, Timestamp rcvTime);
  bool done() const { return state_ == Done; }
  const HTTPRequest& request() const { return request_; }
  HTTPRequest& request() { return request_; }
};
} // namespace net
} // namespace chtho


#endif //! CHTHO_NET_HTTP_HTTPCONTEXT_H