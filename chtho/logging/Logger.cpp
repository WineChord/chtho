// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Logger.h"
#include "threads/CurrentThread.h"
#include "time/TimeZone.h"

#include <stdlib.h> // getenv 
#include <time.h>

namespace chtho
{
// time string e.g. "20210407 15:24:36"
__thread char time_str[64];
// lastly saved second (used to optimize generation of time string)
__thread time_t last_sec; 

Logger::Level initLogLevel()
{
  if(::getenv("CHTHO_LOG_TRACE"))
    return Logger::Level::TRACE;
  if(::getenv("CHTHO_LOG_DEBUG"))
    return Logger::Level::DEBUG;
  return Logger::Level::INFO;
}
// global veriable set for log level
// can be retrieve by Logger::logLevel()
Logger::Level g_logLevel = initLogLevel();
Logger::Level Logger::logLevel() 
{
  return g_logLevel;
}

const char* levelName[static_cast<unsigned long>(Logger::Level::NUM_LEVEL)] = 
{
  // let's have some colorful outputs :)
#ifdef CHTHO_COLORED
  "\033[35mTRACE\033[0m ",
  "\033[36mDEBUG\033[0m ",
  "\033[97mINFO\033[0m  ",
  "\033[33mWARN\033[0m  ",
  "\033[31mERROR\033[0m ",
  "\033[1;31mFATAL\033[0m ",
#else
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
#endif
};

void defaultOutput(const char* str, int len)
{
  fwrite(str, 1, len, stdout);
}

void defaultFlush()
{
  fflush(stdout);
}

Logger::OutputFunc Logger::output = defaultOutput;
Logger::FlushFunc Logger::flush = defaultFlush;
// CST means China Standard Time
// I hate those gays who think everyone know the meaning
// of every abbreviation.
// so the default time zone will be Beijing.
TimeZone Logger::timeZone = TimeZone(8*3600, "CST"); 

void Logger::commonInit()
{
  formTimeStr();
  CurrentThread::pid();
  CurrentThread::tid();
  stream_ << CurrentThread::pidStr();
  stream_ << CurrentThread::tidStr();
  stream_ << levelName[static_cast<int>(level_)];
}

Logger::Logger(SourceFile file, int line)
  : basename_(file),
    line_(line),
    level_(Level::INFO),
    stream_(),
    time_(Timestamp::now())
{
  commonInit();
}

void Logger::formTimeStr()
{
  time_t secs =  time_.secsSinceE();
  int us = time_.us();
  if(secs != last_sec) 
  {
    last_sec = secs;
    struct tm m;
    if(timeZone.valid())
      m = timeZone.toLocal(secs);
    else ::gmtime_r(&secs, &m);
    snprintf(time_str, sizeof(time_str), "%4d%02d%02d %02d:%02d:%02d",
      m.tm_year+1900, m.tm_mon+1, m.tm_mday, m.tm_hour, m.tm_min, m.tm_sec);
  }
  char u[32];
  snprintf(u, 32, ".%06d ", us);
  stream_ << time_str << u;
}

Logger::Logger(SourceFile file, int line, Level level, const char* func)
  : basename_(file),
    line_(line),
    level_(level),
    stream_(),
    time_(Timestamp::now())
{
  commonInit();
  stream_ << func << ' '; 
}

Logger::Logger(SourceFile file, int line, Level level)
  : basename_(file),
    line_(line),
    level_(level),
    stream_(),
    time_(Timestamp::now())
{
  commonInit();
}

Logger::Logger(SourceFile file, int line, bool abortFlag)
  : basename_(file),
    line_(line),
    level_(abortFlag?Level::FATAL:Level::ERR),
    stream_(),
    time_(Timestamp::now())
{
  commonInit();
}

Logger::~Logger()
{
  finish();
  const LogStream::Buffer& buf(stream().buffer());
  output(buf.data(), buf.length());
  if(level_ == Level::FATAL)
  {
    flush();
    abort();
  }
}

void Logger::finish()
{
  stream_ << " " << basename_.data_ << ':' << line_ << '\n';
}

void Logger::setLogLevel(Level level)
{
  g_logLevel = level;
}
} // namespace chtho
