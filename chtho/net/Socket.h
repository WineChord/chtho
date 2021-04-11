// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_SOCKET_H
#define CHTHO_NET_SOCKET_H

#include "InetAddr.h"

#include <netinet/tcp.h>

struct tcp_info;

namespace chtho
{
namespace net
{
class Socket
{
private:
  const int sockfd_;
public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {} 
  ~Socket();

  int fd() const { return sockfd_; }

  void setReuseAddr(bool on);
  void setReusePort(bool on);

  void bindAddr(const InetAddr& addr);
  void listen();
  int accept(InetAddr* peeraddr);

  static void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
  static void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
  static void toIp(char* buf, size_t sz, const struct sockaddr* addr);
  static void toIpPort(char* buf, size_t sz, const struct sockaddr* addr);

  static struct sockaddr_in6 getLocalAddr(int sockfd);
  static int getSocketError(int sockfd);

  bool tcpInfo(struct tcp_info* ti) const;
  bool tcpInfoStr(char* buf, int len) const;

  void shutdownWrite();
  void setKeepAlive(bool on);

  static int createNonBlockOrDie(sa_family_t family);

};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_SOCKET_H