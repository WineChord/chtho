// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_NET_SOCKET_H
#define CHTHO_NET_SOCKET_H

#include "InetAddr.h"

namespace chtho
{
namespace net
{
class Socket
{
private:
  /* data */
public:
  Socket(/* args */);
  ~Socket();

  static void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
  static void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
  static void toIp(char* buf, size_t sz, const struct sockaddr* addr);
  static void toIpPort(char* buf, size_t sz, const struct sockaddr* addr);

};

} // namespace net
} // namespace chtho


#endif // !CHTHO_NET_SOCKET_H