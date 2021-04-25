// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_HTTP_HTTPREQUEST_H
#define CHTHO_NET_HTTP_HTTPREQUEST_H

#include "time/Timestamp.h"

#include <map> 

namespace chtho
{
namespace net
{
class HTTPRequest
{
public:
  enum Method { Invalid, Get, Post, Head, Put, Delete };
  enum Version { Unknown, HTTP10, HTTP11 };
private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  Timestamp rcvTime_;
  std::map<std::string, std::string> headers_; 

public:
  HTTPRequest() : method_(Invalid), version_(Unknown) {} 
  void setRcvTime(Timestamp t) { rcvTime_ = t; }
  bool setMethod(const char* start, const char* end)
  {
    std::string m(start, end);
    if(m == "GET") method_ = Get;
    else if(m == "POST") method_ = Post;
    else if(m == "HEAD") method_ = Head;
    else if(m == "PUT") method_ = Put;
    else if(m == "DELETE") method_ = Delete;
    else method_ = Invalid;
    return method_ != Invalid;
  }
  // something like '/hello'
  void setPath(const char* start, const char* end) { path_.assign(start, end); }
  const std::string& path() const { return path_; }
  // 'hello?yourquery'
  void setQuery(const char* start, const char* end) { query_.assign(start, end); }
  // 'HTTP/1.0' or 'HTTP/1.1' 
  void setVersion(Version v) { version_ = v; }
  Version version() const { return version_; }
  void addHeader(const char* start, const char* colon, const char* end)
  {
    std::string key(start, colon);
    while(++colon < end && isspace(*colon)) /* empty */ ;
    std::string val(colon, end);
    while(!val.empty() && isspace(val[val.size()-1]))
    {
      val.resize(val.size()-1);
    }
    headers_[key] = val; 
  }
  std::string getHeader(const std::string& key) const 
  {
    std::string res;
    auto it = headers_.find(key);
    if(it != headers_.end()) res = it->second;
    return res; 
  }
};
  
} // namespace net
} // namespace chtho


#endif //! CHTHO_NET_HTTP_HTTPREQUEST_H