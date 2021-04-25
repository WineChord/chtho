// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_HTTP_HTTPRESPONSE_H
#define CHTHO_NET_HTTP_HTTPRESPONSE_H


#include <map>
#include <string> 

namespace chtho
{
namespace net
{
class Buffer;
class HTTPResponse
{
public:
  enum Status { Unknown, OK200 = 200, 
    MovedPermanently301 = 301,
    BadRequest400 = 400,
    NotFound404 = 404, 
  };
private:
  Status status_; 
  bool close_; 
  std::string statusMsg_;
  std::map<std::string, std::string> headers_; 
  std::string body_;

public:
  explicit HTTPResponse(bool close)
    : status_(Unknown),
      close_(close)
  {}
  void setStatus(Status s) { status_ = s; }
  void setStatusMsg(const std::string& msg) { statusMsg_ = msg; }
  void setClose(bool on) { close_ = on; }
  bool close() const { return close_; }
  void setContentType(const std::string& type) { addHeader("Content-Type", type); }
  void addHeader(const std::string& key, const std::string& val)
  { headers_[key] = val; }
  void setBody(const std::string& body) { body_ = body; }
  void appendToBuf(Buffer* buf) const;
};
} // namespace net
} // namespace chtho

#endif //! CHTHO_NET_HTTP_HTTPRESPONSE_H