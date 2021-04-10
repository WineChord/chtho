// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_INETADDR_H
#define CHTHO_NET_INETADDR_H

#include <netinet/in.h>

#include <string>
#include <assert.h>

namespace chtho
{
namespace net
{
class InetAddr
{
private:
  union
  {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
  
public:
  // without ip, usually used in listen 
  explicit InetAddr(uint16_t port=0, bool loopbackOnly=false,
    bool ipv6=false);
  // ip is of form "1.2.3.4"
  InetAddr(std::string ip, uint16_t port, bool ipv6=false);
  explicit InetAddr(const struct sockaddr_in& addr)
    : addr_(addr) {}
  explicit InetAddr(const struct sockaddr_in6& addr)
    : addr6_(addr) {} 
  
  sa_family_t family() const { return addr_.sin_family; }
  std::string ip() const;
  std::string ipPort() const;
  uint16_t port() const;

  const struct sockaddr* sockAddr() const 
  { return reinterpret_cast<const struct sockaddr*>(&addr6_);}

  void setSockAddrInet6(const struct sockaddr_in6& addr6) 
  { addr6_ = addr6; }

  uint32_t ipv4NetEndian() const 
  { assert(family() == AF_INET); return addr_.sin_addr.s_addr; }
  uint16_t portNetEndian() const 
  { return addr_.sin_port; }

  static bool resolve(std::string host, InetAddr* res);

  void setScopeID(uint32_t sid)
  {
    if(family() == AF_INET6) 
      addr6_.sin6_scope_id = sid;
  }
};
} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_INETADDR_H