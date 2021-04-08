// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/time/TimeZone.h"

#include <unistd.h> // sleep

int main()
{
  chtho::Logger::setLogLevel(chtho::Logger::Level::TRACE);
  LOG_INFO << "sizeof(Logger)=" << sizeof(chtho::Logger);
  LOG_INFO << "sizeof(LogStream)=" << sizeof(chtho::LogStream);
  LOG_INFO << "sizeof(Buffer)=" << sizeof(chtho::LogStream::Buffer);

  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "hello";
  LOG_WARN << "world";
  LOG_ERR << "error";
  // LOG_FATAL << "fatal";
  // LOG_SYSERR << "syserr";
  // LOG_SYSFATAL << "sysfatal";
  sleep(1); // need to sleep 1 sec

  chtho::Logger::setTimeZone(chtho::TimeZone(0, "UTC"));
  LOG_TRACE << "trace UTC";
  LOG_DEBUG << "debug UTC";
  LOG_INFO << "hello UTC";
  LOG_WARN << "world UTC";
  LOG_ERR << "err UTC";

  return 0;
}