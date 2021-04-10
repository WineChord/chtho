// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Socket.h"
#include "logging/Logger.h"

#include <arpa/inet.h> 

namespace chtho
{
namespace net
{
void Socket::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    LOG_SYSERR << "Socket::fromIpPort";
}
void Socket::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
{
  addr->sin6_family = AF_INET6;
  addr->sin6_port = htons(port);
  if(::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
    LOG_SYSERR << "Socket::fromIpPort";
}

void Socket::toIp(char* buf, size_t sz, const struct sockaddr* addr)
{
  if(addr->sa_family == AF_INET)
  {
    const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(sz));
  }
  else if(addr->sa_family == AF_INET6)
  {
    const struct sockaddr_in6* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(sz));
  }
}
void Socket::toIpPort(char* buf, size_t sz, const struct sockaddr* addr)
{
  if(addr->sa_family == AF_INET6)
  {
    buf[0] = '[';
    toIp(buf+1, sz-1, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in6* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
    uint16_t port = ntohs(addr6->sin6_port);
    assert(sz > end);
    snprintf(buf+end, sz-end, "]:%u", port);
    return;
  }
  toIp(buf, sz, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
  uint16_t port = ntohs(addr4->sin_port);
  assert(sz > end);
  snprintf(buf+end, sz-end, ":%u", port);
}
} // namespace net
} // namespace chtho
