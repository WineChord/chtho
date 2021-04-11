// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_LOGGER_H
#define CHTHO_LOGGING_LOGGER_H
#include "SourceFile.h"
#include "LogStream.h"
#include "time/Timestamp.h"
#include "time/TimeZone.h"

namespace chtho
{
class Logger
{
public:
  enum class Level { TRACE, DEBUG, INFO, WARN, ERR, FATAL, NUM_LEVEL };
  // alias for output func, which will be called at ~Logger 
  // always prefer alias declarations to typedefs.
  // see Effective Modern C++ Item 9. 
  using OutputFunc =  void (*)(const char* str, int len);
  using FlushFunc = void (*)();
  static OutputFunc output;
  static FlushFunc flush; 
  static TimeZone timeZone;

private:
  SourceFile basename_;
  int line_; 
  Level level_; 
  LogStream stream_; 
  Timestamp time_;

  void formTimeStr();
  void finish();
  void commonInit();

public:
  Logger(SourceFile file, int line); // for LOG_INFO
  // for LOG_TRACE, LOG_DEBUG 
  Logger(SourceFile file, int line, Level level, const char* func);
  // for LOG_WARN,ERR,FATAL
  Logger(SourceFile file, int line, Level level);
  // for LOG_SYSERR,SYSFATAL
  Logger(SourceFile file, int line, bool abortFlag);


  ~Logger();
  static Level logLevel();
  static void setLogLevel(Level level);
  static void setOutput(OutputFunc out) { output = out; }
  static void setFlush(FlushFunc flu) { flush = flu; }
  static void setTimeZone(const TimeZone& tz) { timeZone = tz; }
  LogStream& stream() { return stream_; }
};

const char* strerror_tl(int savedErrno);

#define LOG_TRACE if(chtho::Logger::logLevel() <= chtho::Logger::Level::TRACE)\
  chtho::Logger(__FILE__, __LINE__, chtho::Logger::Level::TRACE, __func__).stream()
#define LOG_DEBUG if(chtho::Logger::logLevel() <= chtho::Logger::Level::DEBUG)\
  chtho::Logger(__FILE__, __LINE__, chtho::Logger::Level::DEBUG, __func__).stream()
#define LOG_INFO if(chtho::Logger::logLevel() <= chtho::Logger::Level::INFO)\
  chtho::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN chtho::Logger(__FILE__, __LINE__, chtho::Logger::Level::WARN).stream()
#define LOG_ERR chtho::Logger(__FILE__, __LINE__, chtho::Logger::Level::ERR).stream()
#define LOG_FATAL chtho::Logger(__FILE__, __LINE__, chtho::Logger::Level::FATAL).stream()
#define LOG_SYSERR chtho::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL chtho::Logger(__FILE__, __LINE__, true).stream()

} // namespace chtho

#endif // !CHTHO_LOGGING_LOGGER_H