// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "HTTPContext.h"
#include "net/Buffer.h"

namespace chtho
{
namespace net
{
/* request structure
GET /hello.txt HTTP/1.1
User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3
Host: www.example.com
Accept-Language: en, mi
*/
bool HTTPContext::parse(Buffer* buf, Timestamp rcvTime)
{
  bool ok = true, more = true;
  while(more)
  {
    if(state_ == ExpReq)
    {
      const char* crlf = buf->findCRLF();
      if(crlf)
      {
        ok = procReq(buf->peek(), crlf);
        if(ok)
        {
          request_.setRcvTime(rcvTime);
          buf->retrieveUntil(crlf+2);
          state_ = ExpHeader;
        } else more = false;
      } else more = false; 
    }
    else if(state_ == ExpHeader) // retrieve headers 
    {
      const char* crlf = buf->findCRLF();
      if(crlf)
      {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if(colon != crlf) request_.addHeader(buf->peek(), colon, crlf);
        else  // no more headers 
        {
          state_ = Done;
          more = false; 
        }
        buf->retrieveUntil(crlf+2); 
      }
      else more = false; 
    }
  } 
  return ok; 
}
/* request line, something like
GET /hello HTTP/1.1
*/  
bool HTTPContext::procReq(const char* begin, const char* end)
{
  const char* space = std::find(begin, end, ' ');
  if(space == end) return false; 
  if(!request_.setMethod(begin, space)) return false;
  begin = space + 1; // move to something like '/hello' 
  space = std::find(begin, end, ' ');
  if(space == end) return false; 
  const char* question = std::find(begin, space, '?');
  if(question != space)
  {
    request_.setPath(begin, question);
    request_.setQuery(question, space);
  }
  else  
  {
    request_.setPath(begin, space);
  }
  begin = space + 1;
  // should be either 'HTTP/1.0' or 'HTTP/1.1'
  if(end - begin != 8) return false; 
  if(!std::equal(begin, end-1, "HTTP/1.")) return false;
  if(*(end-1) == '1')
    request_.setVersion(HTTPRequest::HTTP11);
  else if(*(end-1) == '0')
    request_.setVersion(HTTPRequest::HTTP10);
  else return false;
  return true; 
}
} // namespace net
} // namespace chtho
