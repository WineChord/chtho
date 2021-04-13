// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef CHTHO_LOGGING_FILEUTIL_H
#define CHTHO_LOGGING_FILEUTIL_H

#include "base/noncopyable.h"

#include <string>

#include <stdio.h> // FILE 

namespace chtho
{
namespace futil
{
class AppendFile : noncopyable
{
private:
  FILE* fp_;
  char buf_[64*1024]; // 64 KiB 
  off_t written_; 

  size_t write(const char* line, size_t len);
public:
  explicit AppendFile(std::string name);
  ~AppendFile();
  void append(const char* line, size_t len);
  void flush();
  off_t written() const { return written_; }
};
} // namespace futil
} // namespace chtho


#endif // !CHTHO_LOGGING_FILEUTIL_H