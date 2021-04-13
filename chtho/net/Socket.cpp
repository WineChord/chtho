// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Socket.h"
#include "logging/Logger.h"

#include <arpa/inet.h> 
#include <unistd.h> 

namespace chtho
{
namespace net
{
Socket::~Socket()
{
  if(::close(sockfd_) < 0)
    LOG_SYSERR << "Socket::~Socket close";
}

void Socket::setReuseAddr(bool on)
{
  int opt = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, 
    &opt, static_cast<socklen_t>(sizeof(opt)));
}

void Socket::setReusePort(bool on)
{
  int opt = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
    &opt, static_cast<socklen_t>(sizeof(opt)));
  if(ret < 0 && on)
    LOG_SYSERR << "SO_REUSEPORT failed";
}

void Socket::bindAddr(const InetAddr& addr)
{
  addr.sockAddr();
  int ret = ::bind(sockfd_, addr.sockAddr(), static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  if(ret < 0)
    LOG_SYSFATAL << "Socket::bindAddr";
}

void Socket::listen()
{
  int ret = ::listen(sockfd_, SOMAXCONN);
  if(ret < 0)
    LOG_SYSFATAL << "Socket::listen";
}

int Socket::accept(InetAddr* peeraddr)
{
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  int connfd = ::accept4(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), 
    &addrlen, SOCK_NONBLOCK|SOCK_CLOEXEC);
  if(connfd < 0)
  {
    int savederrno = errno;
    switch (savederrno)
    {  
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:
      case EPERM:
      case EMFILE:
        // expected errors
        errno = savederrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << savederrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << savederrno;
        break;
    }
  }
  else  
  {
    peeraddr->setSockAddrInet6(addr);
  }
  return connfd;
}

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

struct sockaddr_in6 Socket::getLocalAddr(int sockfd)
{
  struct sockaddr_in6 res;
  memset(&res, 0, sizeof(res));
  socklen_t len = static_cast<socklen_t>(sizeof(res));
  if(::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&res), &len)<0)
    LOG_SYSERR << "Socket::getLocalAddr";
  return res;
}
struct sockaddr_in6 Socket::getPeerAddr(int sockfd)
{
  struct sockaddr_in6 peeraddr;
  memset(&peeraddr, 0, sizeof(peeraddr));
  socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
  if(::getpeername(sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr),
    &addrlen) < 0)
  {
    LOG_SYSERR << "Socket::getPeerAddr";
  }
  return peeraddr;
}

int Socket::getSocketError(int sockfd)
{
  int opt;
  socklen_t optlen = static_cast<socklen_t>(sizeof(opt));
  /* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, -1 for errors.  */
  if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0)
    return errno;
  else return opt;
}


bool Socket::isSelfConnect(int sockfd)
{
  struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
  struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
  if(localaddr.sin6_family == AF_INET)
  {
    struct sockaddr_in* l = reinterpret_cast<struct sockaddr_in*>(&localaddr);
    struct sockaddr_in* r = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
    return l->sin_port == r->sin_port && l->sin_addr.s_addr == r->sin_addr.s_addr;
  }
  else if(localaddr.sin6_family == AF_INET6)
  {
    return localaddr.sin6_port == peeraddr.sin6_port
      && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr))==0;
  }
  else return false; 
}

bool Socket::tcpInfo(struct tcp_info* ti) const
{
  socklen_t len = sizeof(*ti);
  memset(ti, 0, len);
  return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, ti, &len) == 0;
}
  
bool Socket::tcpInfoStr(char* buf, int len) const
{
  struct tcp_info ti;
  bool ok = tcpInfo(&ti);
  if(ok == false) return ok;
  snprintf(buf, len, "unrecovered=%u "
    "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
    "lost=%u retrans=%u rtt=%u rttvar=%u "
    "sshthresh=%u cwnd=%u total_retrans=%u",
    ti.tcpi_retransmits, // number of unrecovered [RTO] timeouts
    ti.tcpi_rto, // retransmit timeout in us
    ti.tcpi_ato, // predicted tick of soft clock in us
    ti.tcpi_snd_mss,
    ti.tcpi_rcv_mss,
    ti.tcpi_lost, // lost packets
    ti.tcpi_retrans, // retransmitted packets out
    ti.tcpi_rtt, // smoothed round trip time in us
    ti.tcpi_rttvar, // medium deviation
    ti.tcpi_snd_ssthresh,
    ti.tcpi_snd_cwnd,
    ti.tcpi_total_retrans); // total retransmits for entire connection
  return ok;
}

void Socket::shutdownWrite()
{
  if(::shutdown(sockfd_, SHUT_WR) < 0)
    LOG_SYSERR << "Socket::shutdownWrite";
}

void Socket::setKeepAlive(bool on)
{
  int opt = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, 
    static_cast<socklen_t>(sizeof(opt)));
}

int Socket::createNonBlockOrDie(sa_family_t family)
{
  int sockfd = ::socket(family, SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC, IPPROTO_TCP);
  if(sockfd < 0) LOG_SYSFATAL << "Socket::createNonBlockOrDie";
  return sockfd;
}
} // namespace net
} // namespace chtho
