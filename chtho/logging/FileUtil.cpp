// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "FileUtil.h"
#include "logging/Logger.h"

#include <assert.h> 

namespace chtho
{
namespace futil
{
AppendFile::AppendFile(std::string name)
  : fp_(::fopen(name.c_str(), "ae")),  // ae: O_APPEND|O_CLOEXEC
    written_(0)
{
  assert(fp_);
  ::setbuffer(fp_, buf_, sizeof(buf_));
}
AppendFile::~AppendFile()
{
  ::fclose(fp_);
}
void AppendFile::append(const char* line, const size_t len)
{
  size_t n = write(line, len);
  size_t remain = len - n;
  while(remain > 0)
  {
    size_t x = write(line + n, remain);
    if(x == 0)
    {
      int err = ferror(fp_);
      if(err)
      {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
      }
      break;
    }
    n += x; 
    remain -= x; 
  }
  written_ += len;
}
void AppendFile::flush()
{
  ::fflush(fp_);
}
size_t AppendFile::write(const char* line, size_t len)
{
  return ::fwrite_unlocked(line, 1, len, fp_);
}
} // namespace futil
} // namespace chtho
