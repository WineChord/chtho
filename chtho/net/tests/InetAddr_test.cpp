// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/net/InetAddr.h"

#include <assert.h> 

#include <string> 

using namespace chtho;
using namespace chtho::net;

void testInetAddr()
{
  // provide port only, used in listening
  InetAddr addr0(1234); 
  assert(addr0.ip() == std::string("0.0.0.0"));
  assert(addr0.ipPort() == std::string("0.0.0.0:1234"));
  assert(addr0.port() == 1234);

  // loop back only
  InetAddr addr1(4321, true);
  assert(addr1.ip() == std::string("127.0.0.1"));
  assert(addr1.ipPort() == std::string("127.0.0.1:4321"));
  assert(addr1.port() == 4321);

  // ip address and port 
  InetAddr addr2("1.2.3.4", 8888);
  assert(addr2.ip() == std::string("1.2.3.4"));
  assert(addr2.ipPort() == std::string("1.2.3.4:8888"));
  assert(addr2.port() == 8888);

  // ip address and port again
  InetAddr addr3("255.254.253.252", 65535);
  assert(addr3.ip() == std::string("255.254.253.252"));
  assert(addr3.ipPort() == std::string("255.254.253.252:65535"));
  assert(addr3.port() == 65535);
}

void testInet6Addr()
{
  InetAddr addr0(1234, false, true);
  assert(addr0.ip() == std::string("::"));
  assert(addr0.ipPort() == std::string("[::]:1234"));
  assert(addr0.port() == 1234);

  InetAddr addr1(1234, true, true);
  assert(addr1.ip() == std::string("::1"));
  assert(addr1.ipPort() == std::string("[::1]:1234"));
  assert(addr1.port() == 1234);

  InetAddr addr2("2001:db8::1", 8888, true);
  assert(addr2.ip() == std::string("2001:db8::1"));
  assert(addr2.ipPort() == std::string("[2001:db8::1]:8888"));
  assert(addr2.port() == 8888);

  InetAddr addr3("fe80::1234:abcd:1", 8888);
  assert(addr3.ip() == std::string("fe80::1234:abcd:1"));
  assert(addr3.ipPort() == std::string("[fe80::1234:abcd:1]:8888"));
  assert(addr3.port() == 8888);
}

void testResolve()
{
  InetAddr addr(80);
  if(InetAddr::resolve("google.com", &addr))
    LOG_INFO << "google.com resolved to " << addr.ipPort();
  else LOG_ERR << "unable to resolve google.com";
}

int main()
{
  testInetAddr();
  testInet6Addr();
  testResolve();
}