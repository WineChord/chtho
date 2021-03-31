// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_LOGGER_H
#define CHTHO_LOGGING_LOGGER_H

namespace chtho
{

class Logger
{
public:
  enum class Level { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LEVEL };

private:

public:
  Logger();
  ~Logger();
};

Logger::Logger()
{
}

Logger::~Logger()
{
}


} // namespace chtho

#endif // !CHTHO_LOGGING_LOGGER_H