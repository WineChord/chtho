// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "HTTPResponse.h"

#include "net/Buffer.h"

namespace chtho
{
namespace net
{
void HTTPResponse::appendToBuf(Buffer* out) const
{
  char buf[32]; 
  snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_);
  out->append(buf);
  out->append(statusMsg_);
  out->append("\r\n");
  if(close_) out->append("Connection: close\r\n");
  else
  {
    snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
    out->append(buf);
    out->append("Connection: Keep-Alive\r\n");
  }
  for(const auto& header : headers_)
  {
    out->append(header.first);
    out->append(": ");
    out->append(header.second);
    out->append("\r\n");
  }
  out->append("\r\n");
  out->append(body_);
}
  
} // namespace net
} // namespace chtho
