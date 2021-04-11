// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/net/Acceptor.h"

#include "chtho/logging/Logger.h"
#include "chtho/net/EventLoop.h"
#include "chtho/net/InetAddr.h"

#include <unistd.h>
#include <assert.h>

using namespace chtho;
using namespace chtho::net;


void connCB1(int fd, const InetAddr& addr)
{
  LOG_INFO << "from " << addr.ipPort().c_str();
  auto n = ::write(fd, "hello~", 7);
  assert(n > 0);
  ::close(fd);
}
void connCB2(int fd, const InetAddr& addr)
{
  LOG_INFO << "from " << addr.ipPort().c_str();
  auto n = ::write(fd, "world!", 7);
  assert(n > 0);
  ::close(fd);
}

int main()
{
  EventLoop loop;
  InetAddr addr1(9981);
  Acceptor acceptor1(&loop, addr1);
  acceptor1.setNewConnCB(connCB1);
  acceptor1.listen();

  InetAddr addr2(9982);
  Acceptor acceptor2(&loop, addr2);
  acceptor2.setNewConnCB(connCB2);
  acceptor2.listen();
  loop.loop();
}