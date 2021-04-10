// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "InetAddr.h"
#include "Socket.h"
#include "logging/Logger.h"

#include <string.h>
#include <netdb.h> // struct hostent

namespace chtho
{
namespace net
{
  
InetAddr::InetAddr(uint16_t port, bool loopbackOnly, bool ipv6)
{
  if(ipv6)
  {
    memset(&addr6_, 0, sizeof(addr6_));
    // address family it ipv6 
    addr6_.sin6_family = AF_INET6; 
    //                             ::1              ::
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = htons(port); // host to network short (16)
  }
  else  
  {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
    addr_.sin_addr.s_addr = ::htonl(ip); // host to network long (32)
    addr_.sin_port = htons(port);
  }
}

InetAddr::InetAddr(std::string ip, uint16_t port, bool ipv6)
{
  // strchr return the pointer where ':' first occurs
  if(ipv6 || strchr(ip.c_str(), ':'))
  {
    memset(&addr6_, 0, sizeof(addr6_));
    Socket::fromIpPort(ip.c_str(), port, &addr6_);
  }
  else  
  {
    memset(&addr_, 0, sizeof(addr_));
    Socket::fromIpPort(ip.c_str(), port, &addr_);
  }
}

std::string InetAddr::ip() const
{
  char buf[64] = "";
  Socket::toIp(buf, sizeof(buf), sockAddr());
  return buf;
}

std::string InetAddr::ipPort() const
{ 
  char buf[64] = "";
  Socket::toIpPort(buf, sizeof(buf), sockAddr());
  return buf;
}

uint16_t InetAddr::port() const
{
  return ntohs(addr_.sin_port);
}

static __thread char resBuf[64*1024];

bool InetAddr::resolve(std::string host, InetAddr* out)
{
  assert(out != nullptr);
  struct hostent hent;
  struct hostent* he = NULL;
  int herrno = 0; 
  memset(&hent, 0, sizeof(hent));
  int ret = ::gethostbyname_r(host.c_str(), &hent, resBuf, 
    sizeof(resBuf), &he, &herrno);
  if(ret == 0 && he != NULL)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else  
  {
    if(ret) LOG_SYSERR << "InetAddress:resolve";
    return false;
  }
}
} // namespace net
} // namespace chtho
